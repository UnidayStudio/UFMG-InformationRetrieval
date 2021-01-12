#include "SiteResult.h"

#include <iostream>

#include "File.h"

SiteResult::SiteResult(){

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

}

void SiteResult::Print(){
	std::cout << title << "\n" << description << "\n" << url << "\n";
	std::cout << keywords << "\n";
	std::cout << "Html contains " << pageSize << " characters. Crawled in " << crawlTimeMs << "ms\n\n";
}
