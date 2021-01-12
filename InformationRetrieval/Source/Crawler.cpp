#include "Crawler.h"

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>

#include "SiteCrawler.h"
#include "File.h"

#define CRAWL_LIMIT 100
#define CHUNK_SIZE 30
#define MAX_THREADS 1000

Crawler::Crawler(){
	crawlLimit = CRAWL_LIMIT;
	chunkSize = CHUNK_SIZE;
	maxThreads = MAX_THREADS;

	m_crawled = 0;
}

Crawler::Crawler(const std::vector<std::string>& seeds){
	crawlLimit = CRAWL_LIMIT;
	chunkSize = CHUNK_SIZE;
	maxThreads = MAX_THREADS;

	m_crawled = 0;
	m_activeThreads = 0;

	AddToQueue(seeds);
}

Crawler::~Crawler(){
	for (auto& t : m_threadPool) {
		if (t.joinable()) {
			t.join();
		}
	}
}

void Crawler::AddToQueue(const std::vector<std::string>& urls){
	m_lock.lock();
	m_urlQueue.insert(m_urlQueue.end(), urls.begin(), urls.end());
	m_lock.unlock();

	LoadThreads();
}

void Crawler::AddToQueue(const std::string & url){
	m_lock.lock();
	m_urlQueue.push_back(url);
	m_lock.unlock();

	LoadThreads();
}

void Crawler::Run(){
	while (m_crawled < crawlLimit) {
		LoadThreads();

		/*m_poolLock.lock();
		if (m_threadPool.size() > 0) {
			m_threadPool.back().join();
			m_threadPool.pop_back();
		}

		m_lock.lock();
		if (m_urlQueue.size() == 0 && m_threadPool.size() == 0) {
			m_lock.unlock();
			break;
		}
		m_lock.unlock();
		m_poolLock.unlock();*/
		//m_activeThreads
		m_lock.lock();
		size_t qSize = m_urlQueue.size();
		m_lock.unlock();

		if (qSize == 0 && m_activeThreads == 0) {
			break;
		}
	}
}

void Crawler::LoadThreads(){
	// Starting as many threads as possible. 
	// The important variables here are:
	// - Current number of running threads
	// - Number of URLs in the queue
	// - Max number of threads defined...

	m_lock.lock();
	size_t toStart = m_urlQueue.size();
	size_t currentT = m_activeThreads;
	m_lock.unlock();

	if (toStart + currentT > maxThreads) {
		toStart = maxThreads - currentT;
	}

	for (size_t i = 0; i < toStart; i++) {
		m_poolLock.lock();
		m_threadPool.push_back(std::thread(&Crawler::RunNextQueuedURL, this));
		m_poolLock.unlock();
	}
}

void Crawler::RunNextQueuedURL(){
	m_activeThreads++;

	std::string url;
	{
		m_lock.lock();
		if (m_urlQueue.size() == 0) {
			m_lock.unlock();
			return;
		}
		url = m_urlQueue.back();
		m_urlQueue.pop_back();

		m_urlCrawled.push_back(url);

		m_lock.unlock();
	}

	std::cout << "- Crawling: " << url << "\n";
	SiteCrawler crawl(url);

	size_t pageCount = 0;
	size_t outCount = 0;
	while (!crawl.IsFinished()) {
		pageCount += crawl.GetAllToDisk(chunkSize, 2);
		m_crawled += pageCount;

		std::vector<std::string> out = crawl.GetOutboundLinks();
		outCount += out.size();
		AddToQueue(out);
	}

	std::cout << "- Finished: " << url << ".\n\t- Pages: " << pageCount << "\n";
	std::cout << "\t- Outbounds: " << outCount << "\n";
	std::cout << "\t- Average Time: " << crawl.GetAverageCrawlTimeMs() << "ms\n";
	m_activeThreads--;
}
