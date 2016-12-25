
//
// Pokemon Game - Socket Module
// BOT Man, 2016
//

#ifndef BOT_SOCKET_H
#define BOT_SOCKET_H

// is Logging
#define LOGGING

// Logging
#include <mutex>
#include <chrono>
#include <iostream>

#include <string>
#include <exception>
#include <functional>
#include <thread>
#include <sstream>

#define BUF_SIZE 256

#ifdef WIN32

#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define CloseSocket closesocket

namespace BOT_Socket
{
	class SocketInit
	{
		static size_t &refCount ()
		{
			static size_t refCount = 0;
			return refCount;
		}

	public:
		SocketInit ()
		{
			if (!refCount ())
			{
				WSADATA wsaData;
				if (WSAStartup (MAKEWORD (2, 2), &wsaData) != 0)
					throw std::runtime_error ("Failed at WSAStartup");
			}
			refCount ()++;
		}

		~SocketInit ()
		{
			refCount ()--;
			if (!refCount ())
				WSACleanup ();
		}
	};
}

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define CloseSocket close

#endif // WIN32

namespace BOT_Socket
{
	class Server
	{
	public:
		Server (unsigned short port,
				std::function<void (const std::string &request,
									std::string &response)> callback)
		{
			// Socket
			auto sock = socket (AF_INET, SOCK_STREAM,
								IPPROTO_TCP);
			if (sock == -1)
				throw std::runtime_error ("Can not create socket");

			sockaddr_in sa { 0 };
			sa.sin_family = AF_INET;
			sa.sin_port = htons (port);
			sa.sin_addr.s_addr = htonl (INADDR_ANY);

			// Bind
			if (bind (sock, (sockaddr *) &sa, sizeof (sa)) == -1)
			{
				CloseSocket (sock);
				throw std::runtime_error ("Failed at bind");
			}

			// Listen
			const auto BACK_LOG = 10;
			if (listen (sock, BACK_LOG) == -1)
			{
				CloseSocket (sock);
				throw std::runtime_error ("Failed at listen");
			}

			auto timeStamp = [] (std::ostream &os)
				-> std::ostream &
			{
				auto timePoint = std::chrono::system_clock::now ();
				auto now_c = std::chrono::system_clock::to_time_t (timePoint);
				return os << std::put_time (std::gmtime (&now_c), "%Y-%m-%d-%H:%M:%S");
			};

			std::mutex ioLock;
			auto clientCount = 0;
			while (true)
			{
				// Accept
				auto connectSock = accept (sock, NULL, NULL);
				if (connectSock == -1)
					continue;

				std::thread (
					[&ioLock, &timeStamp, &clientCount, &callback, connectSock] ()
				{
					auto curClientCount = 0;
#ifdef LOGGING
					// Log
					{
						std::lock_guard<std::mutex> lck (ioLock);
						curClientCount = ++clientCount;
						std::cout << "\n<" << curClientCount
							<< "> Connected...\n";
						timeStamp (std::cout) << std::endl;
					}
#endif
					while (true)
					{
						// Recv
						std::ostringstream oss;
						char recvBuf[BUF_SIZE];
						auto isRead = false;
						while (true)
						{
							auto curRecv = recv (connectSock, recvBuf, BUF_SIZE - 1, 0);
							if (curRecv == 0 || curRecv == -1)
								break;

							recvBuf[curRecv] = 0;
							oss << recvBuf;
							isRead = true;

							// Read '\0'
							if (recvBuf[curRecv - 1] == 0)
								break;
						}

						// Client Close the Connection
						if (!isRead)
							break;

						// Callback
						std::string response;
						if (callback) callback (oss.str (), response);

						// Client Close the Connection
						if (-1 == send (connectSock, response.c_str (),
										response.size () + 1, 0))
							break;
#ifdef LOGGING
						// Log
						{
							std::lock_guard<std::mutex> lck (ioLock);
							std::cout << "\n<" << curClientCount
								<< ">\n";
							timeStamp (std::cout) << std::endl;
							std::cout << "<Request>\n" << recvBuf <<
								"\n<Response>\n" << response << "\n";
						}
#endif
					}

					// Shutdown Conncetion
					// SHUT_RDWR (Linux)/ SD_BOTH (Windows) = 2
					shutdown (connectSock, 2);

					// Close
					CloseSocket (connectSock);
#ifdef LOGGING
					// Log
					{
						std::lock_guard<std::mutex> lck (ioLock);
						std::cout << "\n<" << curClientCount
							<< "> Disconnected...\n";
						timeStamp (std::cout) << std::endl;
					}
#endif
				}).detach ();
			}

			// Close
			CloseSocket (sock);
		}

	private:
#ifdef WIN32
		SocketInit _pokemonSocket;
#endif // WIN32
	};

	class Client
	{
	public:
		Client (const std::string &ipAddr,
				unsigned short port)
		{
			sockaddr_in sa { 0 };
			sa.sin_family = AF_INET;
			sa.sin_port = htons (port);
			inet_pton (sa.sin_family,
					   ipAddr.c_str (),
					   &sa.sin_addr);

			const size_t MAX_TRIAL = 3;

			// Socket
			_sock = socket (AF_INET, SOCK_STREAM,
							IPPROTO_TCP);
			if (_sock == -1)
				throw std::runtime_error ("Can not create socket");

			// Connect
			auto iTry = 0;
			for (; iTry < MAX_TRIAL; iTry++)
			{
				if (!connect (_sock, (sockaddr *) &sa, sizeof (sa)))
					break;

				std::this_thread::sleep_for (std::chrono::seconds (1));
			}
			if (iTry == MAX_TRIAL)
			{
				CloseSocket (_sock);
				_sock = -1;
				throw std::runtime_error ("Failed at conncet");
			}
		}

		std::string Request (const std::string &request)
		{
			// Send
			if (-1 == send (_sock, request.c_str (), request.size () + 1, 0))
			{
				CloseSocket (_sock);
				_sock = -1;
				throw std::runtime_error ("Server Close the Connection");
			}

			// Recv
			std::ostringstream oss;
			char recvBuf[BUF_SIZE];
			auto isRead = false;
			while (true)
			{
				auto curRecv = recv (_sock, recvBuf, BUF_SIZE - 1, 0);
				if (curRecv == 0 || curRecv == -1)
					break;

				recvBuf[curRecv] = 0;
				oss << recvBuf;
				isRead = true;

				// Read '\0'
				if (recvBuf[curRecv - 1] == 0)
					break;
			}

			// Server Close the Connection
			if (!isRead)
				throw std::runtime_error ("Server Close the Connection");

			return oss.str ();
		}

		~Client ()
		{
			// Shutdown
			// SHUT_RDWR (Linux)/ SD_BOTH (Windows) = 2
			shutdown (_sock, 2);

			// Close
			if (_sock != -1)
				CloseSocket (_sock);
		}

	private:
		SOCKET _sock;
#ifdef WIN32
		SocketInit _pokemonSocket;
#endif // WIN32
	};
}

#endif // !BOT_SOCKET_H