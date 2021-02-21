#ifndef IMAP_CHUNK_H
#define IMAP_CHUNK_H

#include <unordered_map>
#include <string>

class WordInfo;
class File;

class IMapChunk {
public:
	IMapChunk();
	virtual ~IMapChunk();

	void Save(int chunkId);
	void Load(int chunkId);

	virtual void Save(File* file);
	virtual void Load(File* file);

	std::unordered_map<std::string, WordInfo*> wordMap;
};

#endif // !IMAP_CHUNK_H