
//
// Air Conditioner - Socket Module
// BOT Man, 2017
//

#ifndef BOT_SOCKET_H
#define BOT_SOCKET_H

#define LOGGING
#define LOGGINGFILE "log.txt"

#ifdef LOGGING
#include <mutex>
#include <chrono>
#ifdef LOGGINGFILE
#include <fstream>
#else
#include <iostream>
#endif
#endif

#include <string>
#include <list>
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
#include <stdlib.h>

#define CloseSocket close
#define SOCKET int

#endif // WIN32

namespace BOT_Socket
{
    class Server
    {
    public:
        using Callback = std::function<void (const std::string &request,
                                             std::string &response)>;
    private:
        class Worker
        {
            SOCKET _sock;
            std::thread _thread;

#ifdef LOGGING
            static inline std::mutex &_mtxLog ()
            {
                static std::mutex mtxLog;
                return mtxLog;
            }

            static inline std::ostream &_osLog ()
            {
#ifdef LOGGINGFILE
                static std::ofstream ofsLog (LOGGINGFILE, std::ios::app);
                return ofsLog;
#else
                return std::cout;
#endif
            }
#endif

        public:
            Worker (SOCKET sock, size_t clientCount, Callback &callback)
                : _sock (sock)
            {
                static auto timeStamp = [] (std::ostream &os)
                    -> std::ostream &
                {
                    auto timePoint = std::chrono::system_clock::now ();
                    auto now_c = std::chrono::system_clock::to_time_t (timePoint);
                    return os << std::put_time (std::gmtime (&now_c), "%Y-%m-%d-%H:%M:%S");
                };

                _thread = std::thread ([&callback, sock, clientCount]
                {
#ifdef LOGGING
                    auto &mtxLog = _mtxLog ();
                    auto &osLog = _osLog ();
                    {
                        std::lock_guard<std::mutex> lck (mtxLog);
                        osLog << "\n<" << clientCount
                            << "> Connected...\n";
                        timeStamp (osLog) << std::endl;
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
                            auto curRecv = recv (sock, recvBuf, BUF_SIZE - 1, 0);
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
                        if (-1 == send (sock, response.c_str (),
                            (int) response.size () + 1, 0))
                            break;
#ifdef LOGGING
                        {
                            std::lock_guard<std::mutex> lck (mtxLog);
                            osLog << "\n<" << clientCount << ">\n";
                            timeStamp (osLog)
                                << "\n<Request>\n" << recvBuf
                                << "\n<Response>\n" << response
                                << std::endl;
                        }
#endif
                    }
#ifdef LOGGING
                    {
                        std::lock_guard<std::mutex> lck (mtxLog);
                        osLog << "\n<" << clientCount
                            << "> Disconnected...\n";
                        timeStamp (osLog) << std::endl;
                    }
#endif
                });
            }

            ~Worker () { Stop (); }

            void Stop ()
            {
                // Shutdown Conncetion
                // SHUT_RDWR (Linux)/ SD_BOTH (Windows) = 2
                shutdown (_sock, 2);

                // Close
                CloseSocket (_sock);

                if (_thread.joinable ()) _thread.join ();
            }

            bool Joinable () const { return _thread.joinable (); }
        };

    public:
        Server (unsigned short port, Callback &&callback)
            : _callback (callback)
        {
            // Socket
            _sock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (_sock == -1)
                throw std::runtime_error ("Can not create socket");

            sockaddr_in sa;
            memset (&sa, 0, sizeof sa);

            sa.sin_family = AF_INET;
            sa.sin_port = htons (port);
            sa.sin_addr.s_addr = htonl (INADDR_ANY);

            // Bind
            if (bind (_sock, (sockaddr *) &sa, sizeof (sa)) == -1)
            {
                CloseSocket (_sock);
                throw std::runtime_error ("Failed at bind");
            }

            // Listen
            const auto BACK_LOG = 10;
            if (listen (_sock, BACK_LOG) == -1)
            {
                CloseSocket (_sock);
                throw std::runtime_error ("Failed at listen");
            }
        }

        ~Server () { Stop (); }

        void Run ()
        {
            size_t clientCount = 0;
            while (true)
            {
                // Accept
                auto connectSock = accept (_sock, NULL, NULL);
                if (connectSock == -1) break;  // Close by Stop ()

                // Delegate to worker
                _ClearWorkers ();
                _workers.emplace_back (
                    connectSock, ++clientCount, _callback);
            }
        }

        void Stop ()
        {
            if (_sock != -1)
            {
                CloseSocket (_sock);
                _sock = -1;
            }
            _ClearWorkers ();
            _StopWorkers ();
        }

    private:
        SOCKET _sock;
        Callback _callback;
        std::list<Worker> _workers;

        void _ClearWorkers ()
        {
            for (auto p = _workers.begin (); p != _workers.end (); )
                if (!p->Joinable ()) p = _workers.erase (p);
                else ++p;
        }

        void _StopWorkers ()
        {
            for (auto &worker : _workers)
                worker.Stop ();
        }

#ifdef WIN32
        SocketInit _sockInit;
#endif // WIN32
    };

    class Client
    {
        constexpr static size_t MAX_TRIAL = 3;

    public:
        Client (const std::string &ipAddr,
                unsigned short port)
        {
            sockaddr_in sa;
            memset (&sa, 0, sizeof sa);

            sa.sin_family = AF_INET;
            sa.sin_port = htons (port);
            inet_pton (sa.sin_family,
                       ipAddr.c_str (),
                       &sa.sin_addr);

            // Socket
            _sock = socket (AF_INET, SOCK_STREAM,
                            IPPROTO_TCP);
            if (_sock == -1)
                throw std::runtime_error ("Can not create socket");

            // Connect
            size_t iTry = 0;
            for (; iTry < MAX_TRIAL; iTry++)
            {
                if (!connect (_sock, (sockaddr *) &sa, sizeof (sa)))
                    break;

                std::this_thread::sleep_for (std::chrono::seconds (1));
            }
            if (iTry == MAX_TRIAL)
            {
                CloseSocket (_sock);
                throw std::runtime_error ("Failed to Connect to the Server");
            }
        }

        std::string Request (const std::string &request)
        {
            // Send
            if (-1 == send (_sock, request.c_str (), (int) request.size () + 1, 0))
            {
                CloseSocket (_sock);
                _sock = -1;
                throw 0;
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
            if (!isRead) throw 0;

            return oss.str ();
        }

        ~Client ()
        {
            // Shutdown
            // SHUT_RDWR (Linux)/ SD_BOTH (Windows) = 2
            shutdown (_sock, 2);

            // Close
            CloseSocket (_sock);
        }

    private:
        SOCKET _sock;
#ifdef WIN32
        SocketInit _sockInit;
#endif // WIN32
    };
}

#endif // !BOT_SOCKET_H