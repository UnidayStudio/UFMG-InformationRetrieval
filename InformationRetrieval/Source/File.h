#ifndef FILE_H
#define FILE_H

#include <string>
#include <vector>


class File {
public:
	enum FileMode {
		READ, WRITE
	};

	File(const std::string & fileName, FileMode mode);
	virtual ~File();

	bool IsValid();

	virtual bool Read(void* buffer, size_t elementSize, size_t elements);
	virtual bool Write(void* buffer, size_t elementSize, size_t elements);



	template <typename T>
	bool Read(T* buffer, size_t elements = 1) {
		return Read((void*)buffer, sizeof(T), elements);
	}
	template <typename T>
	bool Write(T* buffer, size_t elements = 1) {
		return Write((void*)buffer, sizeof(T), elements);
	}

	template <typename T>
	bool Read(T& buffer, size_t elements = 1) {
		return Read((void*)&buffer, sizeof(T), elements);
	}
	template <typename T>
	bool Write(const T& buffer, size_t elements = 1) {
		return Write((void*)&buffer, sizeof(T), elements);
	}

	//-------------------

	bool ReadStr(std::string& buffer);
	bool WriteStr(const std::string& buffer);

	//-------------------

	template <typename T>
	void ReadN(T& obj) {
		Read(&obj, 1);
	}

	template <typename T, typename... ARGS>
	void ReadN(T& obj, ARGS&... others) {
		ReadN(obj);
		ReadN(others...);
	}

	template <typename T>
	void WriteN(const T& obj) {
		Write(&obj, 1);
	}

	template <typename T, typename... ARGS>
	void WriteN(const T& obj, const ARGS&... others) {
		WriteN(obj);
		WriteN(others...);
	}

protected:
	FileMode m_mode;
	FILE* m_file;

	bool m_isValid;
};

#endif //!FILES_H