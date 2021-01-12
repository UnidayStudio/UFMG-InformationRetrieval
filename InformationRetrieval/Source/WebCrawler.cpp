#include "WebCrawler.h"

#include <iostream>
#include <chrono>

#include "CkSpider.h"

#include "Timer.h"
#include "File.h"

#define CRAWL_LIMIT 100000
#define RESULTS_FILE_SIZE 2000
#define MAX_THREADS 1000

#define SLEEP_TIME_MS 100


static std::atomic<bool> __forceInterrupt = false;


WebCrawler::WebCrawler() {
	m_forceInterrupt = false;
	m_activeThreadCount = 0;
}

WebCrawler::~WebCrawler() {
	__forceInterrupt = true;

	m_threadPoolLock.lock();

	for (auto& t : m_threadPool) {
		if (t.joinable()) {
			t.join();
		}
	}
	m_threadPool.clear();

	m_threadPoolLock.unlock();
}

void WebCrawler::AddToQueue(const std::vector<std::string>& urls){
	m_crawlQueueLock.lock();
	m_crawlQueue.insert(m_crawlQueue.end(), urls.begin(), urls.end());
	m_crawlQueueLock.unlock();

	std::cout << " +" << urls.size() << " URLs added to queue!\n";
}

void __UserInputs() {
	while (!__forceInterrupt) {
		std::string value;
		std::cin >> value;

		if (value[0] == 'q') {
			__forceInterrupt = true;
			std::cout << "\n\nFORCE INTERRUPT ACTIVATED!\n\n";
		}
	}
}

void WebCrawler::Run(){
	Timer timer;

	std::thread t(__UserInputs);

	size_t counter = 0;
	while (m_crawledUrlCount < CRAWL_LIMIT) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		counter++;
		// Print a Report every ~two seconds.
		if (counter > 20) {
			counter = 0;
			std::cout << "\n\n============================================\n";
			std::cout << "STATUS: " << m_crawledSeedCount << " seeds, ";
			std::cout << m_crawledUrlCount << " of " << CRAWL_LIMIT << " URLs crawled.";
			std::cout << "\n============================================\n\n";
		}

		m_crawlQueueLock.lock();
		while (m_activeThreadCount < MAX_THREADS && m_crawlQueue.size() > 0) {
			SpawnThread(m_crawlQueue.back());
			m_crawlQueue.pop_back();
		}
		if (m_activeThreadCount == 0 && m_crawlQueue.size() == 0) {
			break;
		}
		m_crawlQueueLock.unlock();

		if (__forceInterrupt) {
			break;
		}
	}
	m_forceInterrupt = true;
	__forceInterrupt = true;

	std::cout << "\n\n\nTASK FINISHED!\n";
	if (t.joinable()) {
		std::cout << "\nWaiting for USER INPUT... (Press any key)\n";
		t.join();
	}

	std::cout << "\n\n\n----------------------\nRESULTS:\n";
	std::cout << " - Domains Crawled: " << m_crawledSeedCount << "\n";
	std::cout << " - URLs Crawled: " << m_crawledUrlCount << "\n";
	std::cout << " - Time Spent: " << timer.Get() << "ms\n";
}

std::string GetUrlAsFileName(const std::string& url) {
	std::string name;
	for (auto c : url) {
		if (isalpha(c)) { name += c; }
	}
	return name;
}

void WebCrawler::CrawlUrl(const std::string & url){
	std::cout << "STARTED: " << url << "\n";

	m_crawledUrlsLock.lock();
	m_crawledUrls.push_back(url);
	m_crawledUrlsLock.unlock();

	m_crawledSeedCount++;
	m_activeThreadCount++;
	{
		std::vector<SiteResult> results;
		size_t fileOutputs = 1;

		CkSpider spider;
		spider.Initialize(url.c_str());
		spider.AddUnspidered(url.c_str());

		size_t crawledHere = 0;
		size_t pageSizes = 0;
		double timeSpent = 0.0;

		while (1) {
			bool finished = false;
			Timer timer;
			spider.SleepMs(SLEEP_TIME_MS);
			timer.Reset();

			bool success = spider.CrawlNext();
			if (success) {
				SiteResult site;

				site.title = spider.lastHtmlTitle();
				site.url = spider.lastUrl();

				site.keywords = spider.lastHtmlKeywords();
				site.description = spider.lastHtmlDescription();

				std::string html = spider.lastHtml();
				site.pageSize = html.size();

				site.crawlTimeMs = timer.Get();

				results.push_back(site);

				m_crawledUrlCount++;

				// For the stats to this website:
				{
					crawledHere++;
					pageSizes += site.pageSize;
					timeSpent += site.crawlTimeMs;
				}
			}
			else {
				finished = true;
			}

			if (m_crawledUrlCount > CRAWL_LIMIT || m_forceInterrupt) {
				finished = true;
			}

			// Saving to Disk...
			if (results.size() > RESULTS_FILE_SIZE || (finished && results.size() > 0)) {
				WriteResults(results, url, fileOutputs);
				fileOutputs++;
				results.clear();
			}

			// Spawning Outbound URLs..
			int outbounds = spider.get_NumOutboundLinks();
			if (outbounds > 0 && !m_forceInterrupt){
				std::vector<std::string> out;
				out.resize(outbounds);
				for (int i = 0; i < outbounds; i++) {
					out[i] = spider.getOutboundLink(i);
				}
				spider.ClearOutboundLinks();

				AddToQueue(out);
			}

			if (finished) { 
				if (crawledHere > 0) {
					// Writting the Final report...
					std::string name = "Result\\_REPORT_";
					name += GetUrlAsFileName(url) + ".report";

					pageSizes /= crawledHere;
					timeSpent /= crawledHere;

					File f(name, File::WRITE);
					f.WriteStr(url);
					f.WriteN(
						crawledHere,
						pageSizes,
						timeSpent
					);
					std::cout << "\nREPORTING: " << url << "\n";
					std::cout << " - Crawled Here: " << crawledHere << " urls\n";
					std::cout << " - Average Time: " << timeSpent << "ms\n";
					std::cout << " - Average Page Size: " << pageSizes << "\n";
					std::cout << " - STATUS: " << m_crawledUrlCount << "/" << CRAWL_LIMIT <<  "\n";
				}
				break;
			}
		}
	}
	m_activeThreadCount--;

	std::cout << "FINISHED: " << url << "\n";
}

void WebCrawler::SpawnThread(const std::string & url){
	m_threadPoolLock.lock();
	try {
		std::thread t(&WebCrawler::CrawlUrl, this, url);
		m_threadPool.push_back(std::move(t));
	}
	catch (std::bad_alloc& ba) {
		std::cerr << "bad_alloc caught: " << ba.what() << '\n';
	}
	m_threadPoolLock.unlock();
}

void WebCrawler::WriteResults(std::vector<SiteResult>& results, const std::string & url, const size_t & iteration){
	std::string name = "Result\\" + GetUrlAsFileName(url);
	name += "_" + std::to_string(iteration) + ".crawlData";

	File f(name, File::WRITE);

	f.Write(results.size());
	for (auto& res : results) {
		res.Save(&f);
	}
	std::cout << " +" << results.size() << " urls to File: " << name << "\n";
}
