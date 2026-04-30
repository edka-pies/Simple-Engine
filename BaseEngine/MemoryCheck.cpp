#include "MemoryCheck.h"

MemoryCheck::MemoryCheck()
{
	memStatus.dwLength = sizeof(memStatus);
	GlobalMemoryStatusEx(&memStatus);
}

void MemoryCheck::printMemoryAvailable()
{
	std::cout << "There is "
		<< memStatus.ullAvailPhys / (1024 * 1024)
		<< " MB of physical memory available." << std::endl;
	std::cout << "There is "
		<< memStatus.ullAvailVirtual / (1024 * 1024)
		<< " MB of virtual memory total." << std::endl;
}

void MemoryCheck::CheckMemoryLeak(const std::string& logFilePath)
{
}

void MemoryCheck::CalculateMemoryUsage()
{
	if (memStatus.ullAvailPhys / (1024 * 1024) < 2000)
	{
		std::cout << "Memory usage is high!" << std::endl;
	}
}
