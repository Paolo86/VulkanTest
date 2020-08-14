#pragma once
#include <chrono>
#include <map>
#include <iostream> 

using namespace std;
using namespace std::chrono;

class Timer
{
public:
	static Timer& Instance()
	{
		if (!m_instance)
			m_instance = std::unique_ptr<Timer>(new Timer());

		return *m_instance;
	}

	void StartTimer(std::string name);
	double StopTimer(std::string name);


private:
	static std::unique_ptr<Timer> m_instance;
	static map<std::string, std::chrono::time_point<high_resolution_clock>> timers;
};