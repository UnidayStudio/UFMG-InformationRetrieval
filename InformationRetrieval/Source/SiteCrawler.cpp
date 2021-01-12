#include "SiteCrawler.h"

#include <iostream>
#include <atomic>

#include "Timer.h"
#include "File.h"

SiteCrawler::SiteCrawler(const std::string& url) {
	m_reachedTheEnd = false;

	m_url = url;
	m_spider.Initialize(url.c_str());
	m_spider.AddUnspidered(url.c_str());

	m_crawledUrls = 0;
	m_elapsedCrawlTimeMs = 0.0;

	sleepTimeMs = 128;
	limitReached = false;
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

		out.keywords = m_spider.lastHtmlKeywords();
		out.description = m_spider.lastHtmlDescription();

		std::string html = m_spider.lastHtml();
		out.pageSize = html.size();
	}
	else {
		throw std::exception(
			m_spider.get_NumUnspidered() == 0 ?
			"No more URLs to crawl" :
			m_spider.lastErrorText()
		);
		m_reachedTheEnd = true;
	}
	out.crawlTimeMs = timer.Get();

	m_crawledUrls++;
	m_elapsedCrawlTimeMs += out.crawlTimeMs;
	return out;
}

std::vector<std::string> SiteCrawler::GetOutboundLinks(){
	std::vector<std::string> out;

	int outbounds = m_spider.get_NumOutboundLinks();

	out.resize(outbounds);

	for (int i = 0; i < outbounds; i++) {
		out[i] = m_spider.getOutboundLink(i);
	}
	m_spider.ClearOutboundLinks();

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

size_t SiteCrawler::GetAllToDisk(size_t chunkSize, size_t limit){
	static std::atomic<size_t> _totalCrawled = 0;

	size_t crawled = 0;
	size_t files = 0;

	std::string prefix = "Result\\" + GetUrlAsFileName();
	std::string sufix = ".crawlData";

	while (1) {
		std::vector<SiteResult> result = Get(chunkSize);
		size_t resultSize = result.size();
		crawled += resultSize;
		_totalCrawled += resultSize;

		files++;
		// Saving to disk...
		{
			File f(prefix + std::to_string(files) + sufix, File::WRITE);

			f.Write(result.size());
			for (auto& res : result) {
				res.Save(&f);
			}
		}

		if (m_reachedTheEnd) {
			break;
		}

		if (_totalCrawled > limit) {
			limitReached = true;
			break;
		}
	}
	//std::cout << "\nCrawler Result:\n\t- Files Created: " << files << "\n";
	//std::cout << "\t- Crawled URLS: " << crawled << "\n";
	return crawled;
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

std::string SiteCrawler::GetUrlAsFileName(){
	std::string out = "";

	for (auto c : m_url) {
		if (isalpha(c)) {
			out += c;
		}
	}
	return out;
}
