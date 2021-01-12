#pragma once

#define CRAWL_CHUNK_SIZE 1000
#define CRAWL_LIMIT 100

#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <mutex>

class Crawler {
public:
	Crawler();
	Crawler(const std::vector<std::string>& seeds);
	virtual ~Crawler();

	void AddToQueue(const std::vector<std::string>& urls);
	void AddToQueue(const std::string& url);

	void Run();
	
	size_t crawlLimit;
	size_t chunkSize;
	size_t maxThreads;
protected:
	void LoadThreads();

	void RunNextQueuedURL();

private:
	std::mutex m_lock;

	std::atomic<size_t> m_crawled;

	// The current threads being executed
	std::vector<std::thread> m_threadPool;
	std::mutex m_poolLock;

	// A list of URLs (outbounds) not yet crawled
	std::vector<std::string> m_urlQueue;

	std::vector<std::string> m_urlCrawled;
};