#include "SiteResult.h"

#include <iostream>

#include "File.h"

SiteResult::SiteResult(){

}

SiteResult::SiteResult(const SiteResult & other){
	this->title			= other.title;
	this->url			= other.url;
	this->keywords		= other.keywords;
	this->description	= other.description;
	this->crawlTimeMs	= other.crawlTimeMs;
	this->pageSize		= other.pageSize;
}

SiteResult::~SiteResult(){

}

void SiteResult::Save(File * file){
	file->WriteStr(title);
	file->WriteStr(url);
	file->WriteStr(keywords);
	file->WriteStr(description);

	file->Write(crawlTimeMs);
	file->Write(pageSize);
}

void SiteResult::Load(File * file){
	file->ReadStr(title);
	file->ReadStr(url);
	file->ReadStr(keywords);
	file->ReadStr(description);

	file->Read(crawlTimeMs);
	file->Read(pageSize);
}

void SiteResult::Print(){
	std::cout << title << "\n" << description << "\n" << url << "\n";
	std::cout << keywords << "\n";
	std::cout << "Html contains " << pageSize << " characters. Crawled in " << crawlTimeMs << "ms\n\n";
}
