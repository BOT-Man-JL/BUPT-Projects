
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
		if (argc < 2)
			throw std::runtime_error (
				"Usage: reader-writer <Input File>\n"
				"Input File: <index> <Role (R/W)> <Start Time> <Duration>");

		// Open file
		std::ifstream ifs { argv[1] };
		if (!ifs.is_open ())
			throw std::runtime_error (
				std::string ("Could not Open ") + argv[1]);

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

		//
		// Reader Preference
		// First readers-writers problem
		//

		auto readerPreference = [&] ()
		{
			// Resource to R/W
			auto resource = CreateSemaphore (NULL, 1, 1, NULL);

			// Reader Count
			auto rmutex = CreateSemaphore (NULL, 1, 1, NULL);
			size_t read_count = 0;

			auto reader = [&] (const ThreadInfo &threadInfo)
			{
				task1 (threadInfo);

				WaitForSingleObject (rmutex, INFINITE);
				read_count++;
				if (read_count == 1)
					WaitForSingleObject (resource, INFINITE);
				ReleaseSemaphore (rmutex, 1, NULL);

				task2 (threadInfo);

				WaitForSingleObject (rmutex, INFINITE);
				read_count--;
				if (read_count == 0)
					ReleaseSemaphore (resource, 1, NULL);
				ReleaseSemaphore (rmutex, 1, NULL);
			};

			auto writer = [&] (const ThreadInfo &threadInfo)
			{
				task1 (threadInfo);

				WaitForSingleObject (resource, INFINITE);

				task2 (threadInfo);

				ReleaseSemaphore (resource, 1, NULL);
			};

			std::vector<std::thread> threads;
			for (const auto &threadInfo : threadInfos)
			{
				if (threadInfo.isReader)
					threads.emplace_back (
						std::thread { reader, threadInfo });
				else
					threads.emplace_back (
						std::thread { writer, threadInfo });
			}

			for (auto &thread : threads)
				if (thread.joinable ()) thread.join ();

			CloseHandle (resource);
			CloseHandle (rmutex);
		};

		//
		// Writer Preference
		// Second readers-writers problem
		//

		auto writerPreference = [&] ()
		{
			// Resource to R/W
			auto resource = CreateSemaphore (NULL, 1, 1, NULL);

			// Reader trying to enter
			auto readTry = CreateSemaphore (NULL, 1, 1, NULL);

			// Reader Count
			auto rmutex = CreateSemaphore (NULL, 1, 1, NULL);
			size_t read_count = 0;

			// Writer Count
			auto wmutex = CreateSemaphore (NULL, 1, 1, NULL);
			size_t write_count = 0;

			auto reader_WP = [&] (const ThreadInfo &threadInfo)
			{
				task1 (threadInfo);

				WaitForSingleObject (readTry, INFINITE);

				WaitForSingleObject (rmutex, INFINITE);
				read_count++;
				if (read_count == 1)
					WaitForSingleObject (resource, INFINITE);
				ReleaseSemaphore (rmutex, 1, NULL);

				ReleaseSemaphore (readTry, 1, NULL);

				task2 (threadInfo);

				WaitForSingleObject (rmutex, INFINITE);
				read_count--;
				if (read_count == 0)
					ReleaseSemaphore (resource, 1, NULL);
				ReleaseSemaphore (rmutex, 1, NULL);
			};

			auto writer_WP = [&] (const ThreadInfo &threadInfo)
			{
				task1 (threadInfo);

				WaitForSingleObject (wmutex, INFINITE);
				write_count++;
				if (write_count == 1)
					WaitForSingleObject (readTry, INFINITE);
				ReleaseSemaphore (wmutex, 1, NULL);

				WaitForSingleObject (resource, INFINITE);

				task2 (threadInfo);

				ReleaseSemaphore (resource, 1, NULL);

				WaitForSingleObject (wmutex, INFINITE);
				write_count--;
				if (write_count == 0)
					ReleaseSemaphore (readTry, 1, NULL);
				ReleaseSemaphore (wmutex, 1, NULL);
			};

			std::vector<std::thread> threads;
			for (const auto &threadInfo : threadInfos)
			{
				if (threadInfo.isReader)
					threads.emplace_back (
						std::thread { reader_WP, threadInfo });
				else
					threads.emplace_back (
						std::thread { writer_WP, threadInfo });
			}

			for (auto &thread : threads)
				if (thread.joinable ()) thread.join ();

			CloseHandle (resource);
			CloseHandle (readTry);
			CloseHandle (rmutex);
			CloseHandle (wmutex);
		};

		std::cout << "Reader Preference" << std::endl;
		readerPreference ();
		std::cout << "Writer Preference" << std::endl;
		writerPreference ();
		std::cout << "End" << std::endl;
	}
	catch (const std::exception &ex)
	{
		std::cout << ex.what () << std::endl;
	}

	getchar ();
	return 0;
}
