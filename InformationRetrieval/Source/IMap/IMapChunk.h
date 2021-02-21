#ifndef IMAP_CHUNK_H
#define IMAP_CHUNK_H

#include <unordered_map>
#include <string>

#include "IMap/Word.h"

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

	// This variable is set to the last used time this chunk
	// got an access. It's used internally to determine if
	// the chunk is inactive and can be freed.
	PosID lastUsed;
};

#endif // !IMAP_CHUNK_H