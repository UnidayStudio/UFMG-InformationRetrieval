#include "InvertedIndexMap.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

#include "IMap/IMapChunk.h"
#include "File.h"

InvertedIndexMap::InvertedIndexMap() {
	m_siteCounter = 0;

#ifdef OLD_APPROACH
	//pass
#else
	m_numWords = 0;

	m_chunkCount = 0;
	m_currentChunkId = -1; // Note that this will overflow. It's on purpose.
	m_currentChunk = nullptr;
#endif
}

InvertedIndexMap::~InvertedIndexMap() {
#ifdef OLD_APPROACH
	//pass
#else
	if (m_currentChunk) {
		delete m_currentChunk;
	}

	m_chunkMap.clear();
#endif 

	m_siteUrls.clear();
}

void InvertedIndexMap::Save(File * file){	
#ifdef OLD_APPROACH
	file->WriteN(m_siteCounter, m_siteUrls.size(), m_wordMap.size());

	for (auto it : m_siteUrls) {
		file->Write(it.first);
		file->WriteStr(it.second);
	}
	for (auto it : m_wordMap) {
		file->WriteStr(it.first);
		it.second->Save(file);
	}
#else
	PosID urls = m_siteUrls.size();
	file->WriteN(m_siteCounter, urls);

	for (auto it : m_siteUrls) {
		file->Write(it.first);
		file->WriteStr(it.second);
	}

	// Chunk related
	PosID cMapSize = m_chunkMap.size();
	file->WriteN(m_chunkCount, cMapSize);

	for (auto it : m_chunkMap) {
		file->WriteStr(it.first);
		file->Write(it.second);
	}

	// Finally, saving the current chunk (if exists):
	if (m_currentChunk) {
		// It will gonna save it into its own file, no worries!
		m_currentChunk->Save(m_currentChunkId);
	}
#endif
}

void InvertedIndexMap::Load(File * file){
#ifdef OLD_APPROACH
	size_t urls, words;

	file->ReadN(m_siteCounter, urls, words);

	for (size_t i = 0; i < urls; i++) {
		size_t idx; std::string content;

		file->Read(idx);
		file->ReadStr(content);

		m_siteUrls[idx] = content;
	}
	for (size_t i = 0; i < words; i++) {
		size_t idx; std::string key;

		file->ReadStr(key);

		m_wordMap[key] = new WordInfo();
		m_wordMap[key]->Load(file);
	}
#else
	PosID urls;
	file->ReadN(m_siteCounter, urls);

	for (PosID i = 0; i < urls; i++) {
		PosID idx;
		std::string content;

		file->Read(idx);
		file->ReadStr(content);

		m_siteUrls[idx] = content;
	}

	// Chunk related
	PosID cMapSize;
	file->ReadN(m_chunkCount, cMapSize);

	for (PosID i = 0; i < cMapSize; i++) {
		std::string key;
		PosID id;

		file->ReadStr(key);
		file->Read(id);

		m_chunkMap[key] = id;
	}
#endif
}

void InvertedIndexMap::IndexFromFile(const std::string& filePath){
	File file(filePath, File::READ);
	PosID elements;

	file.Read(elements);
	for (PosID i = 0; i < elements; i++) {
		SiteResult res;
		res.Load(&file);

		IndexSite(res);
	}
}

std::string FormatData(const std::string& html) {
	std::string out;

	int scopeCount = 0;

	for (char c : html) {
		if (c == '<') {
			scopeCount++;
		}
		else if (c == '>') {
			scopeCount--;
		}
		else {
			if (scopeCount == 0) {
				out += isalnum(c) ? c : ' ';
			}
			else if (scopeCount < 0) {
				// Corrupted html...
				out = "";
				scopeCount = 0;
			}
		}
	}

	return out;
}

void InvertedIndexMap::IndexSite(const SiteResult & site){
	PosID sId = AddSite(site.url);
	std::string html = site.html;

	html = FormatData(html);
	
	std::istringstream iss(html);
	std::string input;
	PosID position = 0;
	while (iss >> input) {
		std::vector<std::string> words;

		std::string tmpW = "";
		tmpW += input[0];

		// The code will split into different words
		// if a single input is CamelCased or have
		// numbers and letters at the same time.
		for (int i = 1; i < input.size(); i++) {
			if (
				(isupper(input[i]) && islower(input[i - 1])) ||
				(isdigit(tmpW[0]) != isdigit(input[i]))
			) {
				words.push_back(tmpW);
				tmpW = "";
			}
			tmpW += input[i];
		}

		if (tmpW != "") {
			words.push_back(tmpW);
		}

		for (auto word : words) {
			WordInfo* wInfo = GetWordInfo(word);

			if (wInfo) {
				wInfo->references.push_back({ sId, position });
				position++;
			}
		}
	}
}

WordInfo* InvertedIndexMap::GetWordInfo(const std::string & word){
	std::string key = "";
	for (char c : word) { key += tolower(c); }

#ifdef OLD_APPROACH
	auto it = m_wordMap.find(key);
	if (it == m_wordMap.end()) {
		m_wordMap[key] = new WordInfo();
	}
	return m_wordMap[key];
#else
	WordInfo* out = nullptr;

	static PosID iter = 0;
	iter++;

	auto it = m_chunkMap.find(key);
	if (it == m_chunkMap.end()) {
		// Does not exist... add in the current chunk;

		// First check if the current chunk is full
		if (m_currentChunk) {
			if (m_currentChunk->wordMap.size() >= MAX_CHUNK_WORDS) {
				// Save and close it if so.
				m_currentChunk->Save(m_currentChunkId);

				std::cout << iter << " Chunk is full:\n";
				std::cout << "\t - ID:" << m_currentChunkId << "\n";
				std::cout << "\t - Words:" << m_currentChunk->wordMap.size() << "\n";
				std::cout << "\t - Max:" << MAX_CHUNK_WORDS << "\n";

				delete m_currentChunk;
				m_currentChunk = nullptr;
			}
		}

		// Then check if it's necessary to create a new one
		if (m_currentChunk == nullptr) {
			m_currentChunk = new IMapChunk();
			m_currentChunkId = m_chunkCount++;
			std::cout << iter << " New Chunk created..." << m_currentChunkId << ". word count: " << m_numWords << "\n";
			std::cout << "\t - Words:" << m_currentChunk->wordMap.size() << "\n";
		}

		// Finally, add the new wordInfo
		out = new WordInfo();

		m_currentChunk->wordMap[key] = out;
		m_chunkMap[key] = m_currentChunkId;
		m_numWords++;
	}
	else {
		// The word was already indexed...
		PosID fId = it->second;

		if (fId == m_currentChunkId && m_currentChunk) {
			// Good luck, it's in the current opened chunk!
			out = m_currentChunk->wordMap[key];
		}
		else {
			// It's not in the current chunk so it needs to be loaded from disk.
			if (m_currentChunk) {
				// Saving and freeing the current one...
				m_currentChunk->Save(m_currentChunkId);
				delete m_currentChunk;
			}

			std::cout << iter << " FYI:" << m_currentChunkId << " will become " << fId << "\n";

			// Loading the chunk from disk...
			m_currentChunk = new IMapChunk();
			m_currentChunkId = fId;
			std::cout << iter << " Loading...\n\t - Before:" << m_currentChunk->wordMap.size() << "\n";
			m_currentChunk->Load(fId);
			std::cout << "\t - After:" << m_currentChunk->wordMap.size() << "\n";

			// Finally get the WordInfo from it:
			out = m_currentChunk->wordMap[key];
		}
	}
	return out;
#endif
}

PosID InvertedIndexMap::AddSite(const std::string & url){
	PosID sId = m_siteCounter++;
	m_siteUrls[sId] = url;
	return sId;
}

std::string InvertedIndexMap::GetSiteUrl(PosID id){
	auto it = m_siteUrls.find(id);
	if (it != m_siteUrls.end()) {
		return it->second;
	}
	return "";
}

bool SortRefs(WordRef a, WordRef b) {
	if (a.fileId == b.fileId) {
		return a.position < b.position;
	}
	return a.fileId < b.fileId;
}

void InvertedIndexMap::PrintResults(){
	// TODO
}

void InvertedIndexMap::WriteCsvReport(const std::string & filePath){
	// TODO
}

