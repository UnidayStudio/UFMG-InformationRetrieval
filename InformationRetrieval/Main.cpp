#ifdef _MSC_VER
// Get rid of Visual Studio's complaints about sscanf
#pragma warning(disable:4996)
#endif

#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include "CkSpider.h"

// Simple Timer (in ms) using std::chrono.
class Timer {
public:
	Timer() {
		Reset();
	}
	virtual ~Timer(){}

	// In ms
	double Get() {
		return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now() - m_time
			).count();
	}
	void Reset() {
		m_time = std::chrono::system_clock::now();
	}
private:
	std::chrono::time_point<std::chrono::system_clock> m_time;
};


struct SiteResult {
	std::string title;
	std::string url;
	double crawlTimeMs;
};


// The SiteCrawler serves to crawl a single specific website
class SiteCrawler {
public:
	SiteCrawler(const std::string& url) {
		m_url = url;
		m_spider.Initialize(url.c_str());
		m_spider.AddUnspidered(url.c_str());

		m_crawledUrls = 0;
		m_elapsedCrawlTimeMs = 0.0;

		sleepTimeMs = 1000;
	}
	virtual ~SiteCrawler() {}

	SiteResult GetNext() {
		Timer timer;
		SiteResult out;
		bool success;

		if (sleepTimeMs > 0) {
			m_spider.SleepMs(sleepTimeMs);
		}
		timer.Reset();

		success = m_spider.CrawlNext();
		if (success) {
			out.title = m_spider.lastHtmlTitle();
			out.url = m_spider.lastUrl();
		}
		else {
			throw std::exception(
				m_spider.get_NumUnspidered() == 0 ? 
				"No more URLs to crawl" : 
				m_spider.lastErrorText()
			);
		}
		out.crawlTimeMs = timer.Get();

		m_crawledUrls++;
		m_elapsedCrawlTimeMs += out.crawlTimeMs;
		return out;
	}

	std::vector<SiteResult> Get(size_t n) {
		std::vector<SiteResult> out;
		for (size_t i = 0; i <= n; i++) {
			try {
				out.push_back(GetNext());
			}
			catch (std::exception e) {
				std::cerr << "Exception: " << e.what() << "\n";
				break;
			}
		}
		return out;
	}

	double GetAverageCrawlTimeMs() {
		if (m_crawledUrls == 0) {
			return 0;
		}
		return m_elapsedCrawlTimeMs / m_crawledUrls;
	}
	double GetAverageCrawlTimeSeconds() {
		return GetAverageCrawlTimeMs() / 1000.0;
	}

	int sleepTimeMs;
protected:
	CkSpider m_spider;

	std::string m_url;
	size_t m_crawledUrls;

	double m_elapsedCrawlTimeMs;
};


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

			std::cout << site.title << "\n" << site.url << "\n";
			std::cout << "Crawl Time = " << site.crawlTimeMs << "ms\n\n";
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