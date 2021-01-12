#include "Timer.h"

Timer::Timer() {
	Reset();
}
Timer::~Timer() {}

// In ms
double Timer::Get() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now() - m_time
		).count();
}
void Timer::Reset() {
	m_time = std::chrono::system_clock::now();
}