#pragma once

#include <chrono>

// Simple Timer (in ms) using std::chrono.
class Timer {
public:
	Timer();
	virtual ~Timer();

	// In ms
	double Get();
	void Reset();
private:
	std::chrono::time_point<std::chrono::system_clock> m_time;
};