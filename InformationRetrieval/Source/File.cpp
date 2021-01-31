#pragma warning(disable:4996)

#include "File.h"

#include <stdio.h>
#include <string>
#include <iostream>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;


std::vector<std::string> GetFilesAt(const std::string & path) {
	std::vector<std::string> names;

	for (const auto & entry : fs::directory_iterator(path)) {
		names.push_back(entry.path().string());
	}
	return names;
}

std::string GetFileName(const std::string & filePath) {
	return fs::path(filePath).filename().string();
}

std::string replaceAll(const std::string& entry, const std::string& from, const std::string& to) {
	std::string str = entry;

	if (from.empty())
		return str;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}

	return str;
}


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
	if (!out1) {
		return false;
	}
	try {
		buffer.resize(aux);
	}
	catch (std::bad_alloc e) {
		return false;
	}
	catch (std::length_error e) {
		return false;
	}
	out2 = Read(&buffer[0], aux);

	return out1 && out2;
}

bool File::WriteStr(const std::string & buffer) {
	bool out1, out2;

	out1 = Write(buffer.size());
	out2 = Write(&buffer[0], buffer.size());

	return out1 && out2;
}

