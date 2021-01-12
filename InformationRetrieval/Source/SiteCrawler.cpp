#include "SiteCrawler.h"

#include <iostream>

#include "Timer.h"

SiteCrawler::SiteCrawler(const std::string& url) {
	m_url = url;
	m_spider.Initialize(url.c_str());
	m_spider.AddUnspidered(url.c_str());

	m_crawledUrls = 0;
	m_elapsedCrawlTimeMs = 0.0;

	sleepTimeMs = 100;
}

SiteCrawler::~SiteCrawler() {

}

SiteResult SiteCrawler::GetNext() {
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

std::vector<SiteResult> SiteCrawler::Get(size_t n) {
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

double SiteCrawler::GetAverageCrawlTimeMs() {
	if (m_crawledUrls == 0) {
		return 0;
	}
	return m_elapsedCrawlTimeMs / m_crawledUrls;
}
double SiteCrawler::GetAverageCrawlTimeSeconds() {
	return GetAverageCrawlTimeMs() / 1000.0;
}