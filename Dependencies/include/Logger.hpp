#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <unordered_map>
#include <string>

#include <stacktrace>



enum LogTypes {
	LogTypeLog,
	LogTypeWarning,
	LogTypeError,
	LogTypeSuggestion
};

class Logger {
public:
	Logger(std::filesystem::path file, bool isCout = false, bool clear = true, bool isThrow = false)
			: m_file(file, std::ios::out | (!clear * std::ios::app)), m_isCout(isCout), m_isThrow(isThrow) {
		if (clear) {
			m_file.clear();
		}
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		m_file << "Started logging: " << start << std::endl;
	}
	~Logger() {
		std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
		m_file << "Logging ended: " << end << std::endl << std::endl;
		m_file.close();
	}

	void AddLog(LogTypes type, std::string message, std::string file, std::string func, int line) {
		std::chrono::system_clock::time_point time = std::chrono::system_clock::now();
		m_file << "[" << m_typeStrings[type] << "] " << time << " File: " << file << " At line: " << line << " Message: " << message << std::endl;
		if (m_isCout) {
			std::cout << "[" << m_typeStrings[type] << "] " << " File: " << file << " At line: " << line << " Message: " << message << std::endl << std::endl;
		}
		if (m_isThrow && type == LogTypeError) {
			throw std::runtime_error(std::format("\nStack trace:\n{}\nException thrown In file: {}, Function: {}, Line: {}, with message: {}",
				std::pmr::stacktrace::current(), file, func, line, message));
		}
	}

	void SetIsCout(bool value) {
		m_isCout = value;
	}
	void SetIsThrow(bool value) {
		m_isThrow = value;
	}

protected:
	std::fstream m_file;
	bool m_isCout;
	bool m_isThrow;

	std::unordered_map<LogTypes, std::string> m_typeStrings{
		{LogTypeLog, "LOG"},
		{LogTypeWarning, "WARNING"},
		{LogTypeError, "ERROR"},
		{LogTypeSuggestion, "SUGGESTION"},
	};

private:

};