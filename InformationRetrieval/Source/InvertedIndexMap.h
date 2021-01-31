#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <thread>

#include "SiteResult.h"

struct WordRef {
	size_t fileId;
	size_t position;
};

struct WordInfo {
	std::vector<WordRef> references;

	void Save(File* file);
	void Load(File* file);
};

class File;

class InvertedIndexMap {
public:
	InvertedIndexMap();
	virtual ~InvertedIndexMap();

	virtual void Save(File* file);
	virtual void Load(File* file);

	void IndexFromFile(const std::string& filePath);

	// This method will index the site, adding its
	// Url into the siteUrls (in case of future needs)
	// and then parsing its html. It will add to the
	// wordmap every word and ref found;
	void IndexSite(const SiteResult& site);

	// Gonna return the WordInfo struct of that specific 
	// word. If it doesn't exists, this method will 
	// automatically create and return it for you.
	WordInfo* GetWordInfo(const std::string& word);

	size_t AddSite(const std::string& url);

	void PrintResults();
private:
	size_t m_siteCounter;
	std::unordered_map<size_t, std::string> m_siteUrls;

	std::unordered_map<std::string, WordInfo*> m_wordMap;
};

#endif // !INVERTED_INDEX_H