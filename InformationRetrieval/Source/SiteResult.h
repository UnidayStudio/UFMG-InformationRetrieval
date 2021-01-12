#pragma once

#include <string>

class File;

class SiteResult {
public:
	SiteResult();
	SiteResult(const SiteResult& other);
	virtual ~SiteResult();

	void Save(File* file);
	void Load(File* file);

	void Print();

	std::string title;
	std::string url;
	std::string keywords;
	std::string description;

	double crawlTimeMs;
	size_t pageSize;
private:

};