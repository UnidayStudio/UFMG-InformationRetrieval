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

	m_numWords = 0;

	m_chunkCount = 0;
}

InvertedIndexMap::~InvertedIndexMap() {
	for (auto it : m_loadedChunks) {
		delete it.second;
	}
	m_loadedChunks.clear();

	m_chunkMap.clear();

	m_siteUrls.clear();
}

void InvertedIndexMap::Save(File * file){	
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

	// Finally, saving the loaded chunks (if exists):
	for (auto it : m_loadedChunks) {
		it.second->Save(it.first);
	}
}

void InvertedIndexMap::Load(File * file){
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

	WordInfo* out = nullptr;

	static PosID iter = 0;
	iter++;

	auto it = m_chunkMap.find(key);
	if (it != m_chunkMap.end()) {
		// Found it!
		// Meaning that the word was already created and added into an
		// existing chunk. 
		// Now it's necessary to check if that existing chunk is loaded.

		bool isNew = false;
		IMapChunk* chunk = GetChunk(it->second, &isNew);
		if (isNew) {
			chunk->Load(it->second);
		}

		// Found it, meaning that it's currently loaded
		out = chunk->wordMap[key];

		// Let's also set this to help us to free
		chunk->lastUsed = iter;
	}
	else {
		// Word not found. 

		// First, let's attempt to find an existing chunk with a free slot
		PosID chunkID = -1;
		IMapChunk* chunk = nullptr;
		for (auto it : m_loadedChunks) {
			if (it.second->wordMap.size() < MAX_CHUNK_WORDS) {
				chunkID = it.first;
				chunk = it.second;
				break;
			}
		}
		if (!chunk) {
			// They're all full! Let's make a new one
			bool isNew;
			chunkID = m_chunkCount++;
			chunk = GetChunk(chunkID, &isNew);
		}

		// Now let's add the word:
		out = new WordInfo();
		chunk->wordMap[key] = out;
		m_chunkMap[key] = chunkID;
		m_numWords++;
	}

	return out;
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

IMapChunk* InvertedIndexMap::GetChunk(PosID id, bool* isNew){
	auto it = m_loadedChunks.find(id);
	if (it != m_loadedChunks.end()) {
		*isNew = false;
		return it->second;
	}

	// Not found. Let's double check to see if the loaded chunk list
	// is full and remove any acceeding ones.
	while (m_loadedChunks.size() > MAX_CHUNKS_LOADED) {
		PosID eKey = m_loadedChunks.begin()->first;
		PosID eUsage = m_loadedChunks.begin()->second->lastUsed;

		for (auto it : m_loadedChunks) {
			if (it.second->lastUsed < eUsage) {
				eKey = it.first;
				eUsage = it.second->lastUsed;
			}
		}
		delete m_loadedChunks[eKey];
		m_loadedChunks.erase(eKey);
	}

	// Now, let's create and return a new one with this id
	*isNew = true;
	m_loadedChunks[id] = new IMapChunk();
	return m_loadedChunks[id];
}

