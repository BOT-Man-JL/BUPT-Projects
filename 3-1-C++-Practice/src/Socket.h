#ifndef BOT_SOCKET_H
#define BOT_SOCKET_H

// Protocol Defined Buf Size
#define BUF_SIZE 1024

#include <string>
#include <exception>
#include <functional>
#include <thread>

// Logging
#include <mutex>
#include <iostream>

#ifdef WIN32

#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define CloseSocket closesocket

namespace BOT_Socket
{
	class SocketInit
	{
		static size_t refCount;
	public:
		SocketInit ()
		{
			if (!refCount)
			{
				WSADATA wsaData;
				if (WSAStartup (MAKEWORD (2, 2), &wsaData) != 0)
					throw std::runtime_error ("Failed at WSAStartup");
			}
			refCount++;
		}

		~SocketInit ()
		{
			refCount--;
			if (!refCount)
				WSACleanup ();
		}
	};
	size_t SocketInit::refCount = 0;
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

			std::mutex ioLock;
			auto clientCount = 0;
			while (true)
			{
				// Accept
				auto connectSock = accept (sock, NULL, NULL);
				if (connectSock == -1)
					continue;

				std::thread ([&ioLock, &clientCount, &callback, connectSock] ()
				{
					auto curClientCount = 0;
					// Log
					{
						std::lock_guard<std::mutex> lck (ioLock);
						curClientCount = ++clientCount;
						std::cout << "\n<" << curClientCount
							<< "> Connected...\n";
					}

					while (true)
					{
						// Recv
						char recvBuf[BUF_SIZE];
						auto bytesRead = 0;
						while (bytesRead < BUF_SIZE)
						{
							auto curRecv = recv (connectSock, recvBuf + bytesRead,
												 BUF_SIZE - bytesRead, 0);
							if (curRecv == 0 || curRecv == -1)
								break;

							bytesRead += curRecv;

							// Read '\0'
							if (bytesRead > 1 && !recvBuf[bytesRead - 1])
								break;
						}

						// Client Close the Connection
						if (!bytesRead)
							break;

						// Callback
						std::string response;
						if (callback)
							callback (recvBuf, response);

						// Client Close the Connection
						if (-1 == send (connectSock, response.c_str (),
										response.size () + 1, 0))
							break;

						// Log
						{
							std::lock_guard<std::mutex> lck (ioLock);
							std::cout << "\n<" << curClientCount
								<< ">\nRequest:\n" << recvBuf <<
								"\nResponse:\n" << response << "\n";
						}
					}

					// Shutdown Conncetion
					// SHUT_RDWR (Linux)/ SD_BOTH (Windows) = 2
					shutdown (connectSock, 2);

					// Close
					CloseSocket (connectSock);

					// Log
					{
						std::lock_guard<std::mutex> lck (ioLock);
						std::cout << "\n<" << curClientCount
							<< "> Disconnected...\n";
					}
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
			char recvBuf[BUF_SIZE];
			auto bytesRead = 0;
			while (bytesRead < BUF_SIZE)
			{
				auto curRecv = recv (_sock, recvBuf + bytesRead,
									 BUF_SIZE - bytesRead, 0);
				if (curRecv == 0 || curRecv == -1)
					break;

				bytesRead += curRecv;

				// Read '\0'
				if (bytesRead > 1 && !recvBuf[bytesRead - 1])
					break;
			}

			// Server Close the Connection
			if (!bytesRead)
				throw std::runtime_error ("Server Close the Connection");

			return recvBuf;
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