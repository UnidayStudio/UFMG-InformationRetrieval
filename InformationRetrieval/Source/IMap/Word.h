#ifndef IMAP_WORD_H
#define IMAP_WORD_H

#include <vector>
#include <string>

// unsigned int 32 is able to store from 0 to 2,147,483,647.
// If you want, you can change it to unsigned int 64 to be able
// to index the range 0 to 9,223,372,036,854,775,807.

//typedef uint32_t PosID;
typedef size_t PosID;

class File;

struct WordRef {
	PosID fileId;
	PosID position;
};

struct WordInfo {
	std::vector<WordRef> references;

	void Save(File* file);
	void Load(File* file);
};

#endif  // !IMAP_WORD_H