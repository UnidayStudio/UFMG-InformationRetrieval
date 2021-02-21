#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <thread>

#include "SiteResult.h"

#define MAX_REFS_PER_CHUNK 10000000 // 10 M refs per chunk

struct WordRef {
	size_t wordId; // To lookup in a table later on!
	size_t fileId;
	size_t position;
};

class File;

class InvertedIndexMap {
public:
	InvertedIndexMap();
	virtual ~InvertedIndexMap();

	void Save();
	virtual void Save(File* file);

	void Load();
	virtual void Load(File* file);

	// If m_wordReferences.size() >= maxRefs,
	// then save it into a file and clear the vector.
	void SaveChunk(size_t maxRefs = MAX_REFS_PER_CHUNK);

	// Will merge all the chunks inside the IMapChunks folder
	// Into a single external sorted chunk file.
	void MergeChunks(bool deleteParts=false);

	void IndexFromFile(const std::string& filePath);
	void IndexSite(const SiteResult& site);
	
	size_t AddSite(const std::string& url);
	std::string GetSite(size_t id);

	size_t AddWord(const std::string& word);
	std::string GetWord(size_t id);
	size_t GetWordId(const std::string& word);

	void AddReference(size_t word, size_t site, size_t position);
private:
	struct {
		size_t wordCounter;
		std::unordered_map<size_t, std::string> words;
		std::unordered_map<std::string, size_t> wordsInverted;

		size_t siteCounter;
		std::unordered_map<size_t, std::string> sites;

	} m_lookup; // Lookup Table

	size_t m_chunkCount;
	std::vector<WordRef> m_wordReferences;

	// To store where in the chunk each word is located.
	std::unordered_map<size_t, std::pair<size_t, size_t>> m_wordRefMap;
};

#endif // !INVERTED_INDEX_H