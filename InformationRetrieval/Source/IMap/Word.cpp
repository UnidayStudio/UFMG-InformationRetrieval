#include "IMap/Word.h"

#include <iostream>

#include "File.h"

void WordInfo::Save(File * file) {
	PosID size = (PosID)references.size();

	file->Write(size);
	file->Write(references[0], references.size());
}

void WordInfo::Load(File * file) {
	PosID size;
	try {
		file->Read(size);

		references.resize(size);
		file->Read(references[0], size);
	}
	catch (std::bad_alloc e) {
		std::cerr << e.what() << "\n";
	}
}