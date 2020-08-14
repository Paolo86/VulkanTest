#include "Timer.h"

map<std::string, std::chrono::time_point<high_resolution_clock>> Timer::timers;
std::unique_ptr<Timer> Timer::m_instance;

void Timer::StartTimer(std::string name)
{
	timers[name] = std::chrono::high_resolution_clock::now();
}

double Timer::StopTimer(std::string name)
{
	auto duration = duration_cast<microseconds>(std::chrono::high_resolution_clock::now() - timers[name]);
	timers.erase(name);
	auto count = duration.count() / 1000000.0;
	return count;
}