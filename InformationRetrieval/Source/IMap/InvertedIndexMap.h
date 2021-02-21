#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <thread>

#include "SiteResult.h"

#include "IMap/Word.h"

#define MAX_CHUNK_WORDS 50000
#define MAX_CHUNKS_LOADED 10

class File; 
class IMapChunk;

//#define OLD_APPROACH

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

	PosID AddSite(const std::string& url);
	std::string GetSiteUrl(PosID id);
		
	void PrintResults();
	void WriteCsvReport(const std::string& filePath);
private:
	PosID m_siteCounter;
	std::unordered_map<PosID, std::string> m_siteUrls;
	
#ifdef OLD_APPROACH
	std::unordered_map<std::string, WordInfo*> m_wordMap;
#else
	// Specifies in which chunk is the world located
	std::unordered_map<std::string, PosID> m_chunkMap;

	PosID		m_numWords;

	PosID		m_chunkCount;
	PosID		m_currentChunkId;
	IMapChunk*	m_currentChunk;
#endif
};

#endif // !INVERTED_INDEX_H