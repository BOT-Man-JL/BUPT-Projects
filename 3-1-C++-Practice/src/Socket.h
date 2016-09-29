#ifndef POKEMON_SOCKET_H
#define POKEMON_SOCKET_H

#include <string>
#include <exception>
#include <functional>
#include <thread>

#ifdef WIN32

#include <winsock2.h>
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

			sockaddr_in sa;
			memset (&sa, 0, sizeof (sa));
			sa.sin_family = AF_INET;
			sa.sin_port = htons (port);
			sa.sin_addr.s_addr = htonl (INADDR_ANY);

			// Bind
			if (bind (sock, (sockaddr *) &sa, sizeof (sa)) == -1)
			{
				closesocket (sock);
				throw std::runtime_error ("Failed at bind");
			}

			// Listen
			const auto BACK_LOG = 10;
			if (listen (sock, BACK_LOG) == -1)
			{
				closesocket (sock);
				throw std::runtime_error ("Failed at listen");
			}

			while (true)
			{
				// Accept
				auto connectSock = accept (sock, NULL, NULL);
				if (connectSock == -1)
				{
					closesocket (sock);
					throw std::runtime_error ("Failed at accept");
				}

				std::thread ([=] ()
				{
					const size_t BUF_SIZE = 1024;

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
						closesocket (connectSock);
						closesocket (sock);
						throw std::runtime_error ("Failed at send");
					}

					// Shutdown Conncetion
					// SHUT_RDWR (Linux)/ SD_BOTH (Windows) = 2
					if (shutdown (connectSock, 2) == -1)
					{
						closesocket (connectSock);
						closesocket (sock);
						throw std::runtime_error ("Failed at shutdown");
					}

					// Close Connection
					closesocket (connectSock);
				}).detach ();
			}

			// Close
			closesocket (sock);
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
			memset (&_sa, 0, sizeof (_sa));
			_sa.sin_family = AF_INET;
			_sa.sin_port = htons (port);
			_sa.sin_addr.s_addr = inet_addr (ipAddr.c_str ());
		}

		std::string Request (const std::string &request)
		{
			const size_t BUF_SIZE = 1024;
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

				using namespace std::chrono_literals;
				std::this_thread::sleep_for (1s);
			}
			if (iTry == MAX_TRIAL)
			{
				closesocket (sock);
				throw std::runtime_error ("Failed at conncet");
			}

			// Send
			if (-1 == send (sock, request.c_str (), request.size () + 1, 0))
			{
				closesocket (sock);
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
				closesocket (sock);
				throw std::runtime_error ("Failed at shutdown");
			}

			// Close
			closesocket (sock);

			return recvBuf;
		}

	private:
		sockaddr_in _sa;
#ifdef WIN32
		SocketInit _pokemonSocket;
#endif // WIN32
	};
}

#endif // !POKEMON_SOCKET_H