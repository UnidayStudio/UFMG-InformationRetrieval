#include "IMap/Word.h"

#include "File.h"

void WordInfo::Save(File * file) {
	PosID size = (PosID)references.size();

	file->Write(size);
	file->Write(references[0], references.size());
}

void WordInfo::Load(File * file) {
	PosID size;
	file->Read(size);

	references.resize(size);
	file->Read(references[0], size);
}