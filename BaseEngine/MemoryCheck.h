#pragma once
#include <iostream>
#include <windows.h>
#include <string>

class MemoryCheck
{
public:
	MemoryCheck();
	void printMemoryAvailable();
	void CheckMemoryLeak(const std::string& logFilePath);
	void CalculateMemoryUsage();

private:
	MEMORYSTATUSEX memStatus;
};

