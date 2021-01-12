#pragma once

#include <string>
#include <string>
#include <vector>

#include "CkSpider.h"

#include "SiteResult.h"

// The SiteCrawler serves to crawl a single specific website
class SiteCrawler {
public:
	SiteCrawler(const std::string& url);
	virtual ~SiteCrawler();

	SiteResult GetNext();

	std::vector<std::string> GetOutboundLinks();

	std::vector<SiteResult> Get(size_t n);

	// Will return the total urls crawled to disk
	size_t GetAllToDisk(size_t chunkSize, size_t limit=100000);

	bool IsFinished();

	double GetAverageCrawlTimeMs();
	double GetAverageCrawlTimeSeconds();

	int sleepTimeMs;

	bool limitReached;
protected:
	//Only keeps the letters in the URL
	std::string GetUrlAsFileName();

	bool m_reachedTheEnd;
	CkSpider m_spider;

	std::string m_url;
	size_t m_crawledUrls;

	double m_elapsedCrawlTimeMs;
};