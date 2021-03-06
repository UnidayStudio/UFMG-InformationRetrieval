#include "InvertedIndexMap.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

#include <stdio.h>

#include "CkHtmlToText.h"

#include "File.h"

InvertedIndexMap::InvertedIndexMap() {
	m_lookup.wordCounter = 0;
	m_lookup.siteCounter = 0;

	m_chunkCount = 0;
}

InvertedIndexMap::~InvertedIndexMap() {
	m_lookup.words.clear();
	m_lookup.wordsInverted.clear();
	m_lookup.sites.clear();
	m_wordReferences.clear();
	m_wordRefMap.clear();
}

void InvertedIndexMap::Save() {
	File file("InvertedIndexMap.iMap", File::WRITE);
	Save(&file);
}

void InvertedIndexMap::Save(File * file){
	file->WriteN(
		m_lookup.wordCounter, m_lookup.siteCounter, 
		m_lookup.words.size(), m_lookup.sites.size()
	);

	for (auto it : m_lookup.words) {
		file->Write(it.first);
		file->WriteStr(it.second);
	}
	for (auto it : m_lookup.sites) {
		file->Write(it.first);
		file->WriteStr(it.second);
	}

	file->Write(m_wordRefMap.size());
	for (auto it : m_wordRefMap) {
		file->WriteN(it.first, it.second);
	}

	SaveChunk(1);
}

void InvertedIndexMap::Load() {
	File file("InvertedIndexMap.iMap", File::READ);
	Load(&file);
}

void InvertedIndexMap::Load(File * file){
	size_t words, sites;

	file->ReadN(
		m_lookup.wordCounter, m_lookup.siteCounter,
		words, sites
	);
	
	for (size_t i = 0; i < words; i++) {
		size_t idx; std::string content;

		file->Read(idx);
		file->ReadStr(content);

		m_lookup.words[idx] = content;
		m_lookup.wordsInverted[content] = idx;
	}
	for (size_t i = 0; i < sites; i++) {
		size_t idx; std::string content;

		file->Read(idx);
		file->ReadStr(content);

		m_lookup.sites[idx] = content;
	}

	/*size_t refMapCount;
	file->Read(refMapCount);
	for (size_t i = 0; i < refMapCount; i++) {
		size_t idx;
		std::pair<size_t, size_t> refInfo;
		file->ReadN(idx, refInfo);

		m_wordRefMap[idx] = refInfo;
	}*/
}


bool SortRefs(WordRef a, WordRef b) {
	if (a.wordId == b.wordId) {
		if (a.fileId == b.fileId) {
			return a.position < b.position;
		}
		return a.fileId < b.fileId;
	}
	return a.wordId < b.wordId;
}

void InvertedIndexMap::SaveChunk(size_t maxRefs){
	if (m_wordReferences.size() == 0) {	return; }

	if (m_wordReferences.size() >= maxRefs) {
		File file(
			"IMapChunks\\c" + std::to_string(m_chunkCount++) + ".iMapC", 
			File::WRITE
		);

		std::sort(m_wordReferences.begin(), m_wordReferences.end(), SortRefs);

		file.Write(m_wordReferences.size());
		for (auto& ref : m_wordReferences) {
			file.Write(ref);
		}

		m_wordReferences.clear();
	}
}

struct MergeChunkPart {
	MergeChunkPart(const std::string& path) {
		lastIsValid = false;
		file = new File(path, File::READ);
		file->Read(toRead);
	}
	virtual ~MergeChunkPart() {
		delete file;
	}

	// Returns false if ended (failure)
	bool ReadNext() {
		lastIsValid = false;
		if (toRead == 0) {
			return false;
		}
		auto out = file->Read(last);
		if (!out) {
			toRead = 0;
			return false;
		}
		lastIsValid = true;
		toRead--;
		return true;
	}

	File* file;
	WordRef last;
	bool lastIsValid;
	size_t toRead;
};

bool SortChunkPart(MergeChunkPart* a, MergeChunkPart* b) {
	return SortRefs(a->last, b->last);
}

void InvertedIndexMap::MergeChunks(){
	printf("[MERGE CHUNK]");
	// Preparing the structures for the merge...
	std::vector<MergeChunkPart*> parts;

	size_t totalRefs = 0;
	for (auto& f : GetFilesAt("IMapChunks\\")) {
		auto part = new MergeChunkPart(f);

		totalRefs += part->toRead;
		part->ReadNext();

		parts.push_back(part);
	}

	File out("IMapReferences.iMapRef", File::WRITE);
	out.Write(totalRefs);

	//
	bool lwInit = false;
	size_t lastWord = -1; // Overflow, that's ok
	size_t lwStart = 0;

	size_t parsedRefs = 0;
	int _lastPercent = 0;
	while (parsedRefs < totalRefs && parts.size() > 0) {
		{
			int percentage = (float(parsedRefs) / float(totalRefs)) * 100.f;
			if (percentage != _lastPercent) {
				_lastPercent = percentage;
				printf("%3.2f%%... %10zu of %10zu\n",
					(float(parsedRefs) / float(totalRefs)) * 100.f,
					parsedRefs, totalRefs
				);
			}
		}
		
		std::sort(parts.begin(), parts.end(), SortChunkPart);

		if (parts[0]->lastIsValid) {
			WordRef ref = parts[0]->last;
			out.Write(ref);

			if (lastWord != ref.wordId || !lwInit) {
				if (lwInit) {
					m_wordRefMap[lastWord] = { lwStart, parsedRefs };
				}
				else {
					lwInit = true;
				}
				lastWord = ref.wordId;
				lwStart = parsedRefs;
			}
			parsedRefs++;
		}

		if (parts[0]->toRead == 0 || !parts[0]->lastIsValid) {
			// Nothing left to read!
			parts.erase(parts.begin());
		}
		else {
			// Read the next one!
			// I made a small optimization here: If the next one is
			// the same word as the last, there is no need to run
			// everything again, just add it to the output!

			auto lastP = parts[0]->last;

			while (parts.size() > 0) {
				bool success = parts[0]->ReadNext();
				if (!success) {
					parts.erase(parts.begin());
				}
				else {
					if (lastP.wordId == parts[0]->last.wordId) {
						WordRef ref = parts[0]->last;
						out.Write(ref);
						parsedRefs++;
					}
					else {
						break;
					}
				}
			}
		}
	}
	for (auto part : parts) {
		delete part;
	}
	parts.clear();

	printf("\nFinished!\n");
}

void InvertedIndexMap::IndexFromFile(const std::string& filePath){
	File file(filePath, File::READ);
	size_t elements;

	file.Read(elements);
	for (size_t i = 0; i < elements; i++) {
		SiteResult res;
		res.Load(&file);

		IndexSite(res);
	}
}

std::string FormatData(const std::string& html) {
	/*
	The Chilkat HTML parser is good, but too slow. 
	So I'm pre-parsing the content, trying to remove
	as much as possible from the html tags to make it
	easier for the Chilkat parser to run.

	Why not only use my parser?
	Because it's a naive parser and not very efficient.
	It let's a lot of trash to pass by.
	*/
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

	//CkHtmlToText htmlParser;
	//out = htmlParser.toText(out.c_str());

	return out;
}

void InvertedIndexMap::IndexSite(const SiteResult & site){
	size_t siteId = AddSite(site.url);
	std::string html = site.html;

	html = FormatData(html);

	std::istringstream iss(html);
	std::string input;
	size_t position = 0;
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
			size_t wordId = GetWordId(word);
			AddReference(wordId, siteId, position);
		}
	}
}

size_t InvertedIndexMap::AddSite(const std::string & url){
	size_t sId = m_lookup.siteCounter++;
	m_lookup.sites[sId] = url;
	return sId;
}

std::string InvertedIndexMap::GetSite(size_t id){
	auto it = m_lookup.sites.find(id);
	if (it != m_lookup.sites.end()) {
		return it->second;
	}
	return "";
}

size_t InvertedIndexMap::AddWord(const std::string & word){
	size_t sId = m_lookup.wordCounter++;
	m_lookup.words[sId] = word;
	m_lookup.wordsInverted[word] = sId;
	return sId;
}

std::string InvertedIndexMap::GetWord(size_t id){
	auto it = m_lookup.words.find(id);
	if (it != m_lookup.words.end()) {
		return it->second;
	}
	return "";
}

size_t InvertedIndexMap::GetWordId(const std::string & word){
	auto it = m_lookup.wordsInverted.find(word);
	if (it != m_lookup.wordsInverted.end()) {
		return it->second;
	}
	return AddWord(word);
}

void InvertedIndexMap::AddReference(size_t word, size_t site, size_t position){
	m_wordReferences.push_back({ word, site, position });
	SaveChunk();
}
