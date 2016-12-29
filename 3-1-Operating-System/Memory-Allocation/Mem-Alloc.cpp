
#include <array>
#include <iostream>

#include <thread>
#include <Windows.h>

struct Trace
{
	LPVOID start;
	long size;
};

int main (int argc, char *argv[])
{
	try
	{
		size_t outputCount = 0;
		auto outputStatus = [&] (std::ostream &os)
		{
			os << "\nOutput Status " << outputCount++ << ":\n";

			SYSTEM_INFO info;
			GetSystemInfo (&info);
			//os << "dwActiveProcessorMask" << '\t' << info.dwActiveProcessorMask << std::endl;
			//os << "dwAllocationGranularity" << '\t' << info.dwAllocationGranularity << std::endl;
			//os << "dwNumberOfProcessors" << '\t' << info.dwNumberOfProcessors << std::endl;
			//os << "dwOemId" << '\t' << info.dwOemId << std::endl;
			//os << "dwPageSize" << '\t' << info.dwPageSize << std::endl;
			//os << "dwProcessorType" << '\t' << info.dwProcessorType << std::endl;
			//os << "lpMaximumApplicationAddress" << '\t' << info.lpMaximumApplicationAddress << std::endl;
			//os << "lpMinimumApplicationAddress" << '\t' << info.lpMinimumApplicationAddress << std::endl;
			//os << "wProcessorArchitecture" << '\t' << info.wProcessorArchitecture << std::endl;
			//os << "wProcessorLevel" << '\t' << info.wProcessorLevel << std::endl;
			//os << "wProcessorRevision" << '\t' << info.wProcessorRevision << std::endl;
			//os << "wReserved" << '\t' << info.wReserved << std::endl;
			//os << std::endl;

			MEMORYSTATUS status;
			GlobalMemoryStatus (&status);
			os << "dwAvailPageFile" << '\t' << status.dwAvailPageFile << std::endl;
			os << "dwAvailPhys" << '\t' << status.dwAvailPhys << std::endl;
			os << "dwAvailVirtual" << '\t' << status.dwAvailVirtual << std::endl;
			os << "dwLength" << '\t' << status.dwLength << std::endl;
			os << "dwMemoryLoad" << '\t' << status.dwMemoryLoad << std::endl;
			os << "dwTotalPageFile" << '\t' << status.dwTotalPageFile << std::endl;
			os << "dwTotalPhys" << '\t' << status.dwTotalPhys << std::endl;
			os << "dwTotalVirtual" << '\t' << status.dwTotalVirtual << std::endl;
			os << std::endl;

			//MEMORY_BASIC_INFORMATION mem;
			//VirtualQuery (info.lpMinimumApplicationAddress, &mem, sizeof (MEMORY_BASIC_INFORMATION));
			//os << "AllocationBase" << '\t' << mem.AllocationBase << std::endl;
			//os << "AllocationProtect" << '\t' << mem.AllocationProtect << std::endl;
			//os << "BaseAddress" << '\t' << mem.BaseAddress << std::endl;
			//os << "Protect" << '\t' << mem.Protect << std::endl;
			//os << "RegionSize" << '\t' << mem.RegionSize << std::endl;
			//os << "State" << '\t' << mem.State << std::endl;
			//os << "Type" << '\t' << mem.Type << std::endl;
			//os << std::endl;
		};

		auto outputError = [] (std::ostream &os)
		{
			auto dwError = GetLastError ();
			HLOCAL hlocal = NULL; // Buffer that gets the error message string
								  // Use the default system locale since we look for Windows messages
								  // Note: this MAKELANGID combination has a value of 0
			DWORD systemLocale = MAKELANGID (LANG_NEUTRAL, SUBLANG_NEUTRAL);

			// Get the error code's textual description
			BOOL fOk = FormatMessageA (
				FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
				FORMAT_MESSAGE_ALLOCATE_BUFFER,
				NULL, dwError, systemLocale,
				(char *) &hlocal, 0, NULL);
			if (!fOk)
			{
				// Is it a network-related error?
				HMODULE hDll = LoadLibraryEx (TEXT ("netmsg.dll"), NULL,
											  DONT_RESOLVE_DLL_REFERENCES);
				if (hDll != NULL)
				{
					fOk = FormatMessageA (
						FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_ALLOCATE_BUFFER,
						hDll, dwError, systemLocale,
						(char *) &hlocal, 0, NULL);
					FreeLibrary (hDll);
				}
			}

			if (fOk && (hlocal != NULL))
			{
				os << (const char *) LocalLock (hlocal) << std::endl;
				LocalFree (hlocal);
			}
			else
				os << "Could not Get Error Msg for " << dwError << std::endl;
		};

		auto semaphoreAlloc = CreateSemaphore (NULL, 0, 1, NULL);
		auto semaphoreTrack = CreateSemaphore (NULL, 1, 1, NULL);
		auto isEndTrack = false;

		auto threadAlloc = std::thread ([&] ()
		{
			SYSTEM_INFO info;
			GetSystemInfo (&info);

			std::array<int, 5> protectionMap
			{
				PAGE_READONLY,
				PAGE_READWRITE,
				PAGE_EXECUTE,
				PAGE_EXECUTE_READ,
				PAGE_EXECUTE_READWRITE
			};

			auto foreachProtection = [&] (auto callback)
			{
				size_t index = 0;
				for (const auto protection : protectionMap)
				{
					WaitForSingleObject (semaphoreAlloc, INFINITE);
					callback (index++, protection);
					ReleaseSemaphore (semaphoreTrack, 1, NULL);
				}
			};

			std::array<Trace, 5> traceArray;

			// Reverse
			foreachProtection ([&] (size_t index, int)
			{
				traceArray[index].size = (index + 1) * info.dwPageSize;
				traceArray[index].start = VirtualAlloc (NULL, traceArray[index].size,
														MEM_RESERVE, PAGE_NOACCESS);
			});

			// Commit
			foreachProtection ([&] (size_t index, int protection)
			{
				traceArray[index].start = VirtualAlloc (
					traceArray[index].start, traceArray[index].size, MEM_COMMIT, protection);
			});

			// Lock
			foreachProtection ([&] (size_t index, int)
			{
				if (!VirtualLock (traceArray[index].start,
								  traceArray[index].size))
					outputError (std::cout);
			});

			// Unlock
			foreachProtection ([&] (size_t index, int)
			{
				if (!VirtualUnlock (traceArray[index].start,
									traceArray[index].size))
					outputError (std::cout);
			});

			// Decommit
			foreachProtection ([&] (size_t index, int)
			{
				if (!VirtualFree (traceArray[index].start,
								  traceArray[index].size, MEM_DECOMMIT))
					outputError (std::cout);
			});

			// Release
			foreachProtection ([&] (size_t index, int)
			{
				if (!VirtualFree (traceArray[index].start, 0,
								  MEM_RELEASE))
					outputError (std::cout);
			});

			isEndTrack = true;
		});

		auto threadTrack = std::thread ([&] ()
		{
			while (!isEndTrack)
			{
				WaitForSingleObject (semaphoreTrack, INFINITE);
				outputStatus (std::cout);
				ReleaseSemaphore (semaphoreAlloc, 1, NULL);
			}
		});

		if (threadAlloc.joinable ()) threadAlloc.join ();
		if (threadTrack.joinable ()) threadTrack.join ();

		CloseHandle (semaphoreAlloc);
		CloseHandle (semaphoreTrack);
	}
	catch (const std::exception &ex)
	{
		std::cout << ex.what () << std::endl;
	}

	getchar ();
	return 0;
}
