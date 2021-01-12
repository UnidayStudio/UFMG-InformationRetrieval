#ifdef _MSC_VER
// Get rid of Visual Studio's complaints about sscanf
#pragma warning(disable:4996)
#endif

#include <iostream>

#include "SiteCrawler.h"
#include "File.h"

#include "Crawler.h"

int main(int argc, char** argv){
#if 0
	// To test the save/load
	File f(argv[1], File::READ);
	size_t s;

	f.Read(s);
	for (size_t i = 0; i < s; i++) {
		SiteResult result;

		result.Load(&f);
		result.Print();
	}
#else
	std::vector<std::string> out = {
		"http://www.tim.com.br",
		"http://jornaldotempo.uol.com.br",
		"http://casa.abril.com.br"
	};

	Crawler myCrawl(out);

	myCrawl.Run();
	
#endif

	system("pause");
	return 0;
}