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

	std::vector<SiteResult> Get(size_t n);


	double GetAverageCrawlTimeMs();
	double GetAverageCrawlTimeSeconds();

	int sleepTimeMs;
protected:
	CkSpider m_spider;

	std::string m_url;
	size_t m_crawledUrls;

	double m_elapsedCrawlTimeMs;
};