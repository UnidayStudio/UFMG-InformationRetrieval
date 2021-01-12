#pragma warning(disable:4996)


#include "File.h"

#include <stdio.h>
#include <string>


File::File(const std::string & fileName, FileMode mode){
	m_mode = mode;
	m_isValid = true;
	
	m_file = fopen(fileName.c_str(),
		mode == FileMode::READ ? "rb" : "w+b"
	);
	m_isValid = (m_file != NULL);
}

File::~File() {
	if (IsValid()) {
		fclose(m_file);
	}
}

bool File::IsValid() {
	return m_isValid;
}

bool File::Read(void * buffer, size_t elementSize, size_t elements) {
	if (IsValid()) {
		return fread(buffer, elementSize, elements, m_file) == elements;
	}
	return false;
}

bool File::Write(void * buffer, size_t elementSize, size_t elements) {
	if (IsValid()) {
		return fwrite(buffer, elementSize, elements, m_file) == elements;
	}
	return false;
}

bool File::ReadStr(std::string & buffer) {
	bool out1, out2;
	size_t aux;

	out1 = Read(aux);
	buffer.resize(aux);
	out2 = Read(&buffer[0], aux);

	return out1 && out2;
}

bool File::WriteStr(const std::string & buffer) {
	bool out1, out2;

	out1 = Write(buffer.size());
	out2 = Write(&buffer[0], buffer.size());

	return out1 && out2;
}
