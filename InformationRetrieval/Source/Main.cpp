#include <iostream>
#include <fstream>

#include "WebCrawler.h"
#include "File.h"

//#define CRAWLER_RUN
#define CRAWLER_ANALYZE


int main(int argc, char** argv) {
#ifdef CRAWLER_RUN
	{
		std::vector<std::string> out = {
			"http://www.tim.com.br",
			"http://jornaldotempo.uol.com.br",
			"http://casa.abril.com.br"
		};

		WebCrawler crawl;

		crawl.AddToQueue(out);
		crawl.Run();
	}
#endif

#ifdef CRAWLER_ANALYZE
	// Analyze the Collected Data
	{
		auto results = GetFilesAt("Result\\");

		std::ofstream report("report.csv", std::ofstream::out);
		 // Header:
		report << "Seed, Level 1 URLs, Avg Time (ms), Avg Size (bytes)\n";

		std::ofstream urlReport("urls.csv", std::ofstream::out);
		// Header:
		urlReport << "URL, Title, Keywords, Description, Crawl Time (ms), Size (bytes)\n";

		size_t badAllocs = 0;
		for (auto& path : results) {
			std::string name = GetFileName(path);

			if (name[0] == '_') { // REPORT file
				File f(path, File::READ);

				std::string url;
				size_t crawledHere = 0;
				size_t pageSizes = 0;
				double timeSpent = 0.0;
			
				f.ReadStr(url);
				f.ReadN(crawledHere, pageSizes, timeSpent);

				report << replaceAll(url, ",", " ") << ", ";
				report << crawledHere	<< ", ";
				report << timeSpent		<< ", ";
				report << pageSizes		<< "\n";
			}
			else { // Crawl File
				File f(path, File::READ);

				size_t s;
				f.Read(s);

				for (size_t i = 0; i < s; i++) {
					SiteResult site;
					try {
						site.Load(&f);

						if (site.url != "") {
							urlReport << replaceAll(site.url, ",", " ") << ", ";
							urlReport << replaceAll(site.title, ",", " ") << ", ";
							urlReport << replaceAll(site.keywords, ",", " ") << ", ";
							urlReport << replaceAll(site.description, ",", " ") << ", ";
							urlReport << site.crawlTimeMs << ", ";
							urlReport << site.pageSize << "\n";
						}
						else {
							std::cerr << "Empty URL...\n";
						}
					}
					catch (std::bad_alloc& ba) {
						std::cerr << "Bad Alloc: " << ba.what() << "\n";
						std::cerr << "\t - File: " << name << "\n";
						badAllocs++;
					}
				}
			}
		}
		std::cout << "Bad Allocs (total): " << badAllocs << "\n";

		report.close();
		urlReport.close();
	}
#endif

	system("pause");
	return 0;
}