#pragma once
#include <iostream>
#include <chrono>
#include <ctime>   // localtime
#include <sstream> // stringstream
#include <iomanip> // put_time
#include <string>  // string

#if defined(_WIN32) || defined(_WIN64)
#include<Windows.h>
#endif

#define DEBUG 0

/**
	* @brief Check if it's the first variable in the pack
	*/
static bool first = true;
/**
* @class Logger
* @brief Helper class used to log information to the console
*
*
* @author Paolo Ferri
* @version 01
* @date 16/03/2019
*
* @bug No known bugs.
*/
class Logger
{
public:
	/**
	* @brief Log information to the console (green color)
	*
	* @post The information is printed to the console in green color
	* @param f The first argument of the parameter pack
	* @param pack Parameter pack
	*/
	template<class T, class...Args>
	static void LogInfo(T&& f, Args&&...pack);

	/**
	* @brief Log information to the console (green color)
	*
	* Method called the the parameter pack has one variable left
	* @post The information is printed to the console in green color
	* @param f The first argument of the parameter pack
	*/
	template<class T>
	static void LogInfo(T&& f);


	/**
	* @brief Log information to the console (red color)
	*
	* @post The information is printed to the console in red color
	* @param f The first argument of the parameter pack
	*/
	template<class T, class...Args>
	static void LogError(T&& f, Args&&...pack);


	/**
	* @brief Log information to the console (red color)
	*
	* Method called the the parameter pack has one variable left
	* @post The information is printed to the console in red color
	* @param f The first argument of the parameter pack
	*/
	template<class T>
	static void LogError(T&& f);


	/**
	* @brief Log information to the console (blue color)
	*
	* @post The information is printed to the console in blue color
	* @param f The first argument of the parameter pack
	*/
	template<class T, class...Args>
	static void LogWarning(T&& f, Args&&...pack);


	/**
	* @brief Log information to the console (blue color)
	*
	* Method called the the parameter pack has one variable left
	* @post The information is printed to the console in blue color
	* @param f The first argument of the parameter pack
	*/
	template<class T>
	static void LogWarning(T&& f);


	/**
	* @brief Log information to the console (white color)
	*
	* @post The information is printed to the console in blue color
	* @param f The first argument of the parameter pack
	*/
	template<class T, class...Args>
	static void Log(T&& f, Args&&...pack);


	/**
	* @brief Log information to the console (white color)
	*
	* Method called the the parameter pack has one variable left
	* @post The information is printed to the console in blue color
	* @param f The first argument of the parameter pack
	*/
	template<class T>
	static void Log(T&& f);

private:

	/**
	* @brief Get current time in string format
	*/
	static std::string GetCurrentTime()
	{
		std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::string s(20, '\0');
		std::strftime(&s[0], s.size(), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
		return s;
	}

};

template<class T, class...Args>
void Logger::LogInfo(T&& f, Args&&...pack)
{
#if DEBUG
#if defined(_WIN32) || defined(_WIN64)
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_INTENSITY);

#endif	

	if (first)
	{
		first = 0;
		std::cout << GetCurrentTime() << f << " ";
	}
	else
		std::cout << f << " ";

	LogInfo(std::forward<Args>(pack)...);
#endif

}

template<class T>
void Logger::LogInfo(T&& f)
{
#if DEBUG
#if defined(_WIN32) || defined(_WIN64)
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_INTENSITY);

#endif
	if (first)
	{
		first = 0;
		std::cout << GetCurrentTime() << f << "\n";
	}
	else
		std::cout << f << "\n";

	first = 1;
#endif
}

template<class T, class...Args>
void Logger::LogError(T&& f, Args&&...pack)
{
#if DEBUG
#if defined(_WIN32) || defined(_WIN64)
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif	

	if (first)
	{
		first = 0;
		std::cout << GetCurrentTime() << f << " ";
	}
	else
		std::cout << f << " ";

	LogError(std::forward<Args>(pack)...);
#endif

}

template<class T>
void Logger::LogError(T&& f)
{
#if DEBUG
#if defined(_WIN32) || defined(_WIN64)
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);

#endif
	if (first)
	{

		first = 0;
		std::cout << GetCurrentTime() << f << "\n";
	}
	else
		std::cout << f << "\n";
	first = 1;
#endif
}


template<class T, class...Args>
void Logger::LogWarning(T&& f, Args&&...pack)
{
#if DEBUG
#if defined(_WIN32) || defined(_WIN64)
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#endif	

	if (first)
	{
		first = 0;
		std::cout << GetCurrentTime() << f << " ";
	}
	else
		std::cout << f << " ";

	LogWarning(std::forward<Args>(pack)...);
#endif

}

template<class T>
void Logger::LogWarning(T&& f)
{
#if DEBUG
#if defined(_WIN32) || defined(_WIN64)
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE | FOREGROUND_INTENSITY);

#endif
	if (first)
	{

		first = 0;
		std::cout << GetCurrentTime() << f << "\n";
	}
	else
		std::cout << f << "\n";
	first = 1;
#endif
}

template<class T, class...Args>
void Logger::Log(T&& f, Args&&...pack)
{
#if DEBUG
#if defined(_WIN32) || defined(_WIN64)
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, 0x000f);
#endif	

	if (first)
	{
		first = 0;
		std::cout << GetCurrentTime() << f << " ";
	}
	else
		std::cout << f << " ";

	LogWarning(std::forward<Args>(pack)...);
#endif

}

template<class T>
void Logger::Log(T&& f)
{
#if DEBUG
#if defined(_WIN32) || defined(_WIN64)
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, 0x000f);

#endif
	if (first)
	{

		first = 0;
		std::cout << GetCurrentTime() << f << "\n";
	}
	else
		std::cout << f << "\n";
	first = 1;
#endif
}