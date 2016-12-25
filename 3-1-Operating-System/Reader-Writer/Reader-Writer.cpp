
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>

#include <mutex>
#include <thread>
#include <Windows.h>

struct ThreadInfo
{
	int index;
	bool isReader;
	double start;
	double duration;
};

std::ostream &operator << (std::ostream &os,
						   const ThreadInfo &threadInfo)
{
	return os << "Thread " << threadInfo.index
		<< (threadInfo.isReader ? " - Reader " : " - Writer ");
}

int main (int argc, char *argv[])
{
	try
	{
		if (argc < 3)
			throw std::runtime_error (
				"Usage: reader-writer <Running Mode> <Input File>\n"
				"Running Mode: 0 - Reader Preference, others - Writer Preference\n"
				"Input File: <index> <Role (R/W)> <Start Time> <Duration>");

		// Running Mode
		auto isReaderPreference = argv[1] == std::string ("0");

		// Open file
		std::ifstream ifs { argv[2] };
		if (!ifs.is_open ())
			throw std::runtime_error (
				std::string ("Could not Open ") + argv[2]);

		// Input
		std::vector<ThreadInfo> threadInfos;
		char buf[64];
		while (ifs.getline (buf, 64))
		{
			ThreadInfo threadInfo;
			std::stringstream tss (buf);
			char roleChar;
			tss >> threadInfo.index >> roleChar
				>> threadInfo.start >> threadInfo.duration;
			threadInfo.isReader = (roleChar == 'R' || roleChar == 'r');

			threadInfos.emplace_back (threadInfo);
		}

		std::mutex mtxCout;

		auto task1 = [&] (const ThreadInfo &threadInfo)
		{
			auto duration = int (threadInfo.start * 1000);
			std::this_thread::sleep_for (
				std::chrono::milliseconds { duration });

			{
				std::lock_guard<std::mutex> lg { mtxCout };
				std::cout << threadInfo << " - Request" << std::endl;
			}
		};

		auto task2 = [&] (const ThreadInfo &threadInfo)
		{
			{
				std::lock_guard<std::mutex> lg { mtxCout };
				std::cout << threadInfo << " - Begin" << std::endl;
			}

			auto duration = int (threadInfo.duration * 1000);
			std::this_thread::sleep_for (
				std::chrono::milliseconds { duration });

			{
				std::lock_guard<std::mutex> lg { mtxCout };
				std::cout << threadInfo << " - End" << std::endl;
			}
		};

		// Algorithm Refer to:
		// https://en.wikipedia.org/wiki/Readers%E2%80%93writers_problem

		// Resource to R/W
		CRITICAL_SECTION resource;
		InitializeCriticalSection (&resource);

		// Reader Count
		CRITICAL_SECTION rmutex;
		InitializeCriticalSection (&rmutex);
		size_t read_count = 0;

		//
		// Reader Preference
		// First readers-writers problem
		//

		auto reader_RP = [&] (const ThreadInfo &threadInfo)
		{
			task1 (threadInfo);

			EnterCriticalSection (&rmutex);
			read_count++;
			if (read_count == 1)
				EnterCriticalSection (&resource);
			LeaveCriticalSection (&rmutex);

			task2 (threadInfo);

			EnterCriticalSection (&rmutex);
			read_count--;
			if (read_count == 0)
				LeaveCriticalSection (&resource);
			LeaveCriticalSection (&rmutex);
		};

		auto writer_RP = [&] (const ThreadInfo &threadInfo)
		{
			task1 (threadInfo);

			EnterCriticalSection (&resource);

			task2 (threadInfo);

			LeaveCriticalSection (&resource);
		};

		//
		// Writer Preference
		// Second readers-writers problem
		//

		// Reader trying to enter
		CRITICAL_SECTION readTry;
		InitializeCriticalSection (&readTry);

		// Writer Count
		CRITICAL_SECTION wmutex;
		InitializeCriticalSection (&wmutex);
		size_t write_count = 0;

		auto reader_WP = [&] (const ThreadInfo &threadInfo)
		{
			task1 (threadInfo);

			EnterCriticalSection (&readTry);

			EnterCriticalSection (&rmutex);
			read_count++;
			if (read_count == 1)
				EnterCriticalSection (&resource);
			LeaveCriticalSection (&rmutex);

			LeaveCriticalSection (&readTry);

			task2 (threadInfo);

			EnterCriticalSection (&rmutex);
			read_count--;
			if (read_count == 0)
				LeaveCriticalSection (&resource);
			LeaveCriticalSection (&rmutex);
		};

		auto writer_WP = [&] (const ThreadInfo &threadInfo)
		{
			task1 (threadInfo);

			EnterCriticalSection (&wmutex);
			write_count++;
			if (write_count == 1)
				EnterCriticalSection (&readTry);
			LeaveCriticalSection (&wmutex);

			EnterCriticalSection (&resource);

			task2 (threadInfo);

			LeaveCriticalSection (&resource);

			EnterCriticalSection (&wmutex);
			write_count--;
			if (write_count == 0)
				LeaveCriticalSection (&readTry);
			LeaveCriticalSection (&wmutex);
		};

		// Start
		std::vector<std::thread> threads;
		for (const auto &threadInfo : threadInfos)
		{
			if (isReaderPreference)
			{
				if (threadInfo.isReader)
					threads.emplace_back (
						std::thread { reader_RP, threadInfo });
				else
					threads.emplace_back (
						std::thread { writer_RP, threadInfo });
			}
			else
			{
				if (threadInfo.isReader)
					threads.emplace_back (
						std::thread { reader_WP, threadInfo });
				else
					threads.emplace_back (
						std::thread { writer_WP, threadInfo });
			}
		}

		// Join
		for (auto &thread : threads)
			if (thread.joinable ()) thread.join ();

		std::cout << "End" << std::endl;
	}
	catch (const std::exception &ex)
	{
		std::cout << ex.what () << std::endl;
	}

	getchar ();
	return 0;
}
