#include "InvertedIndexMap.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

#include "File.h"

InvertedIndexMap::InvertedIndexMap() {
	m_siteCounter = 0;
}

InvertedIndexMap::~InvertedIndexMap() {
	for (auto it : m_wordMap) {
		delete it.second;
	}
	m_wordMap.clear();

	m_siteUrls.clear();
}

void InvertedIndexMap::Save(File * file){
	file->WriteN(m_siteCounter, m_siteUrls.size(), m_wordMap.size());

	for (auto it : m_siteUrls) {
		file->Write(it.first);
		file->WriteStr(it.second);
	}
	for (auto it : m_wordMap) {
		file->WriteStr(it.first);
		it.second->Save(file);
	}
}

void InvertedIndexMap::Load(File * file){
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
	size_t sId = AddSite(site.url);
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

	auto it = m_wordMap.find(key);
	if (it == m_wordMap.end()) {
		m_wordMap[key] = new WordInfo();
		m_wordMap[key]->frequency = -1.f;
	}
	return m_wordMap[key];
}

size_t InvertedIndexMap::AddSite(const std::string & url){
	size_t sId = m_siteCounter++;
	m_siteUrls[sId] = url;
	return sId;
}

std::string InvertedIndexMap::GetSiteUrl(size_t id){
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

void InvertedIndexMap::CalculateWordsFrequency(){
	for (auto it : m_wordMap) {
		auto& refs = it.second->references;
		if (refs.size() == 0) {
			continue;
		}
		std::sort(refs.begin(), refs.end(), SortRefs);

		int siteCount = 1;
		for (int i = 1; i < refs.size(); i++) {
			if (refs[i - 1].fileId != refs[i].fileId) {
				siteCount++;
			}
		}
		it.second->frequency = float(siteCount) / float(m_siteCounter);
	}
}

void InvertedIndexMap::PrintIMapInfo(){
	std::cout << "Status:\n";
	std::cout << "URLs: " << m_siteUrls.size() << "\n";
	uint64_t sizeUrls = 0;
	for (auto it : m_siteUrls) {
		sizeUrls += sizeof(size_t);
		sizeUrls += sizeof(std::string) + sizeof(char) * it.second.size();
	}
	std::cout << "\t - Size: " << sizeUrls << "bytes\n";

	std::cout << "Words: " << m_wordMap.size() << "\n";
	int refs = 0;
	uint64_t sizeWords = 0;
	for (auto word : m_wordMap) {
		refs += word.second->references.size();

		sizeWords += sizeof(std::string);
		sizeWords += sizeof(WordInfo*) + sizeof(WordInfo);
		sizeWords += sizeof(WordRef) * word.second->references.size();
	}
	std::cout << "\t - References: " << refs << "\n";
	std::cout << "\t - Size: " << sizeWords << "bytes\n";
	std::cout << "\n";
}

void InvertedIndexMap::PrintResults(){
	size_t minRef = SIZE_MAX;
	std::string minWord = "";

	size_t maxRef = 0;	
	std::string maxWord = "";

	size_t avgRefs = 0;
	std::vector<std::string> samples;
	for (auto it : m_wordMap) {
		size_t refs = it.second->references.size();
		avgRefs += refs;

		if (refs < minRef) {
			minRef = refs;
			minWord = it.first;
		}
		if (refs > maxRef) {
			maxRef = refs;
			maxWord = it.first;
		}
	}
	avgRefs /= m_wordMap.size();
	
	std::cout << "----------------------------\n";
	std::cout << "Sites Indexed: " << m_siteUrls.size() << "\n";
	std::cout << "Words Indexed: " << m_wordMap.size() << "\n";

	std::cout << " - Less used word: " << minWord;
	std::cout << " (" << minRef << ")\n";

	std::cout << " - Most used word: " << maxWord;
	std::cout << " (" << maxRef << ")\n";

	std::cout << " - Average Ref count: " << avgRefs << "\n";
	std::cout << "----------------------------\n";
}

void InvertedIndexMap::WriteCsvReport(const std::string & filePath){
	std::ofstream report(filePath, std::ofstream::out);
	// Header:
	report << "Word, Frequence, Occurrences\n";

	for (auto it : m_wordMap) {
		report << it.first << ", ";
		report << std::fixed << it.second->frequency << ", ";
		report << it.second->references.size() << "\n";
	}

	report.close();
}

void WordInfo::Save(File * file){
	file->Write(frequency);
	file->Write(references.size());
	file->Write(references[0], references.size());
}

void WordInfo::Load(File * file){
	file->Read(frequency);

	size_t size;
	file->Read(size);

	references.resize(size);
	file->Read(references[0], size);
}
