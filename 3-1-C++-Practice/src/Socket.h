#ifndef POKEMON_SOCKET_H
#define POKEMON_SOCKET_H

#include <iostream>
#include <string>
#include <exception>
#include <functional>

#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib") 
#endif // WIN32

namespace PokemonGame
{
	class PokemonSocket
	{
	public:
		PokemonSocket ()
		{
#ifdef WIN32
			WSADATA wsaData;
			if (WSAStartup (MAKEWORD (2, 2), &wsaData) != 0)
			{
				std::cerr << "Failed at WSAStartup" << std::endl;
				throw std::runtime_error ("Failed at WSAStartup");
			}
#endif // WIN32
		}

		~PokemonSocket ()
		{
#ifdef WIN32
			WSACleanup ();
#endif // WIN32
		}

		// Server Side
		bool Response (unsigned short port,
					   std::function<void (std::string)> callback)
		{
			const size_t BUF_SIZE = 1024;

			SOCKET sock = socket (AF_INET, SOCK_STREAM,
								  IPPROTO_TCP);
			if (sock == -1)
			{
				std::cerr << "Can not create socket" << std::endl;
				return false;
			}

			sockaddr_in sa;
			memset (&sa, 0, sizeof (sa));

			sa.sin_family = AF_INET;
			sa.sin_port = htons (port);
			sa.sin_addr.s_addr = htonl (INADDR_ANY);

			if (bind (sock, (sockaddr *) &sa, sizeof (sa)) == -1)
			{
				closesocket (sock);
				std::cerr << "Failed at bind" << std::endl;
				return false;
			}

			const auto BACK_LOG = 10;
			if (listen (sock, BACK_LOG) == -1)
			{
				closesocket (sock);
				std::cerr << "Failed at listen" << std::endl;
				return false;
			}

			while (true)
			{
				auto connectSock = accept (sock, NULL, NULL);

				if (connectSock == -1)
				{
					closesocket (sock);
					std::cerr << "Failed at accept" << std::endl;
					return false;
				}

				char recvBuf[BUF_SIZE];
				auto bytesRead = 0;
				while (true)
				{
					if (bytesRead >= BUF_SIZE)
						break;
					auto ret = recv (connectSock, recvBuf + bytesRead,
									 BUF_SIZE - bytesRead, 0);
					if (ret > 0) bytesRead += ret;
					else break;
				}
				recvBuf[bytesRead] = 0;

				if (callback)
					callback (recvBuf);

				// SHUT_RDWR (Linux)/ SD_BOTH (Windows) = 2
				if (shutdown (connectSock, 2) == -1)
				{
					closesocket (connectSock);
					closesocket (sock);
					std::cerr << "Failed at accept" << std::endl;
					return false;
				}
				closesocket (connectSock);
			}
			closesocket (sock);
		}

		// Client Side
		bool Request (const std::string &ipAddr,
					  unsigned short port,
					  const std::string &request,
					  std::string &response)
		{
			const size_t BUF_SIZE = 1024;

			SOCKET sock = socket (AF_INET, SOCK_STREAM,
								  IPPROTO_TCP);
			if (sock == -1 || sock == -2)
			{
				std::cerr << "Can not create socket" << std::endl;
				return false;
			}

			sockaddr_in sa;
			sa.sin_family = AF_INET;
			sa.sin_port = htons (port);
			sa.sin_addr.s_addr = inet_addr (ipAddr.c_str ());

			if (!connect (sock, (sockaddr *) &sa, sizeof (sa)))
			{
				std::cerr << "Can not connect: " << ipAddr << std::endl;
				closesocket (sock);
				return false;
			};

			if (-1 == send (sock, request.c_str (),
							request.size (), 0))
			{
				std::cerr << "Send error" << std::endl;
				closesocket (sock);
				return false;
			}

			char recvBuf[BUF_SIZE];
			auto bytesRead = 0;
			while (true)
			{
				if (bytesRead >= BUF_SIZE)
					break;
				auto ret = recv (sock, recvBuf + bytesRead,
								 BUF_SIZE - bytesRead, 0);
				if (ret > 0) bytesRead += ret;
				else break;
			}
			recvBuf[bytesRead] = 0;
			response = recvBuf;

			closesocket (sock);
			return true;
		}
	};
}

#endif // !POKEMON_SOCKET_H