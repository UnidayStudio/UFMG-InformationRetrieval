#ifndef WEB_CRAWLER_H
#define WEB_CRAWLER_H

#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>

#include "SiteResult.h"

#include "IMap/Word.h"

class WebCrawler {
public:
	WebCrawler();
	virtual ~WebCrawler();

	WebCrawler(const WebCrawler& other) = delete;
	WebCrawler& operator=(const WebCrawler& other) = delete;

	void AddToQueue(const std::vector<std::string>& urls);

	void Run(PosID limit);

protected:
	// Will crawl the specific URL and save to disk.
	// Will also add outbounds URLs to queue
	void CrawlUrl(const std::string& url);
private:
	std::atomic<PosID> m_crawlLimit;
	std::atomic<bool> m_forceInterrupt;

	void SpawnThread(const std::string& url);
	static void WriteResults(std::vector<SiteResult>& results, const std::string& url, const PosID& iteration);

	std::atomic<PosID> m_crawledSeedCount; // Different domains
	std::atomic<PosID> m_crawledUrlCount;

	std::mutex m_crawledUrlsLock;
	std::vector<std::string> m_crawledUrls;

	std::atomic<PosID> m_activeThreadCount;

	std::mutex m_threadPoolLock;
	std::vector<std::thread> m_threadPool;

	std::mutex m_crawlQueueLock;
	std::vector<std::string> m_crawlQueue;
};

#endif // !WEB_CRAWLER_H