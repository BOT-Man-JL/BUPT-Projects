#ifndef POKEMON_SOCKET_H
#define POKEMON_SOCKET_H

// Protocol Defined Buf Size
#define BUF_SIZE 1024

#include <string>
#include <exception>
#include <functional>
#include <thread>

#ifdef WIN32

#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") 

namespace PokemonGame_Impl
{
	class SocketInit
	{
	public:
		SocketInit ()
		{
			WSADATA wsaData;
			if (WSAStartup (MAKEWORD (2, 2), &wsaData) != 0)
				throw std::runtime_error ("Failed at WSAStartup");
		}

		~SocketInit ()
		{
			WSACleanup ();
		}
	};
}

#define CloseSocket closesocket

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define CloseSocket close

#endif // WIN32

namespace PokemonGame_Impl
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

			while (true)
			{
				// Accept
				auto connectSock = accept (sock, NULL, NULL);
				if (connectSock == -1)
				{
					CloseSocket (sock);
					throw std::runtime_error ("Failed at accept");
				}

				std::thread ([& callback, connectSock] ()
				{
					// Recv
					char recvBuf[BUF_SIZE];
					auto bytesRead = 0;
					recvBuf[bytesRead] = 1;
					while (bytesRead < BUF_SIZE)
					{
						// Read '\0'
						if (!recvBuf[bytesRead - 1])
							break;

						auto ret = recv (connectSock, recvBuf + bytesRead,
										 BUF_SIZE - bytesRead, 0);
						if (ret > 0) bytesRead += ret;
						else break;
					}

					// Callback
					std::string response;
					if (callback)
						callback (recvBuf, response);

					// Send
					if (-1 == send (connectSock, response.c_str (), response.size () + 1, 0))
					{
						CloseSocket (connectSock);
						//CloseSocket (sock);
						throw std::runtime_error ("Failed at send");
					}

					// Shutdown Conncetion
					// SHUT_RDWR (Linux)/ SD_BOTH (Windows) = 2
					if (shutdown (connectSock, 2) == -1)
					{
						CloseSocket (connectSock);
						//CloseSocket (sock);
						throw std::runtime_error ("Failed at shutdown");
					}

					// Close Connection
					CloseSocket (connectSock);
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
			_sa.sin_family = AF_INET;
			_sa.sin_port = htons (port);
			inet_pton (_sa.sin_family,
					   ipAddr.c_str (),
					   &_sa.sin_addr);
		}

		std::string Request (const std::string &request)
		{
			const size_t MAX_TRIAL = 16;

			// Socket
			auto sock = socket (AF_INET, SOCK_STREAM,
								IPPROTO_TCP);
			if (sock == -1)
				throw std::runtime_error ("Can not create socket");

			// Connect
			auto iTry = 0;
			for (; iTry < MAX_TRIAL; iTry++)
			{
				if (!connect (sock, (sockaddr *) &_sa, sizeof (_sa)))
					break;

				std::this_thread::sleep_for (std::chrono::seconds(1));
			}
			if (iTry == MAX_TRIAL)
			{
				CloseSocket (sock);
				throw std::runtime_error ("Failed at conncet");
			}

			// Send
			if (-1 == send (sock, request.c_str (), request.size () + 1, 0))
			{
				CloseSocket (sock);
				throw std::runtime_error ("Failed at send");
			}

			// Recv
			char recvBuf[BUF_SIZE];
			auto bytesRead = 0;
			recvBuf[bytesRead] = 1;
			while (bytesRead < BUF_SIZE)
			{
				// Read '\0'
				if (!recvBuf[bytesRead - 1])
					break;

				auto ret = recv (sock, recvBuf + bytesRead,
								 BUF_SIZE - bytesRead, 0);
				if (ret > 0) bytesRead += ret;
				else break;
			}

			// Shutdown
			// SHUT_RDWR (Linux)/ SD_BOTH (Windows) = 2
			if (shutdown (sock, 2) == -1)
			{
				CloseSocket (sock);
				throw std::runtime_error ("Failed at shutdown");
			}

			// Close
			CloseSocket (sock);

			return recvBuf;
		}

	private:
		sockaddr_in _sa { 0 };
#ifdef WIN32
		SocketInit _pokemonSocket;
#endif // WIN32
	};
}

#endif // !POKEMON_SOCKET_H