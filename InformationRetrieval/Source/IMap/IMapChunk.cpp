#include "IMap/IMapChunk.h"

#include "IMap/Word.h"
#include "File.h"

IMapChunk::IMapChunk() {
	lastUsed = 0;
}

IMapChunk::~IMapChunk() {
	for (auto it : wordMap) {
		delete it.second;
	}
	wordMap.clear();
}

void IMapChunk::Save(int chunkId) {
	auto fName = "IMapChunks\\" + std::to_string(chunkId) + ".iMapC";

	File f(fName, File::WRITE);
	Save(&f);
}

void IMapChunk::Load(int chunkId) {
	auto fName = "IMapChunks\\" + std::to_string(chunkId) + ".iMapC";

	File f(fName, File::READ);
	Load(&f);
}

void IMapChunk::Save(File * file) {
	file->WriteN(wordMap.size());

	for (auto it : wordMap) {
		if (it.second) {
			file->WriteStr(it.first);
			it.second->Save(file);
		}
	}
}

void IMapChunk::Load(File * file) {
	PosID words;

	file->ReadN(words);

	for (PosID i = 0; i < words; i++) {
		PosID idx; 
		std::string key;

		file->ReadStr(key);

		wordMap[key] = new WordInfo();
		wordMap[key]->Load(file);
	}
}
