#ifdef _MSC_VER
// Get rid of Visual Studio's complaints about sscanf
#pragma warning(disable:4996)
#endif

#include <iostream>

#include "SiteCrawler.h"


int main(int argc, char** argv){
	if (argc != 3) {
		std::cerr <<
			"This program takes exactly two arguments:\n"
			"- a URL of a page to to crawl and"
			"- a number n of additional links within this page to crawl next.";
		return -1;
	}

	SiteCrawler crawl(argv[1]);
	int crawlCount;
	sscanf(argv[2], "%d", &crawlCount);

	std::cout << "Crawling " << argv[1] << " (with " << crawlCount << " addicional links)...\n\n";

	for (int i = 0; i <= crawlCount; i++) {
		try {
			SiteResult site = crawl.GetNext();

			site.Print();
		}
		catch (std::exception e) {
			std::cerr << e.what() << "\n";
			break;
		}
	}

	std::cout << "\nAverage Crawl Time = ";
	std::cout << crawl.GetAverageCrawlTimeMs() << "ms, ";
	std::cout << crawl.GetAverageCrawlTimeSeconds() << "s\n";

#ifdef __GNUC__
#else
	system("pause");
#endif
	return 0;
}