
//
// Air Conditioner - Client Server Base Module
// BOT Man, 2017
//

#ifndef BOT_CLIENT_SERVER_H
#define BOT_CLIENT_SERVER_H

#include <string>
#include <unordered_map>
#include <mutex>

#include "json/json.hpp"
#include "bot-socket.h"

#define REQUEST "request"
#define PARAM "param"

#define SUCCESS "success"
#define RESPONSE "response"

#define BAD_REQUEST "Bad Request"
#define BAD_RESPONSE "Bad Response"

namespace BOT_CS
{
    class Server
    {
        using Handler = std::function<
            void (nlohmann::json &response,
                  const nlohmann::json &request)>;
        std::unordered_map<std::string, Handler> _handlers;
        std::mutex _mtx;
        BOT_Socket::Server _server;

        void _callback (const std::string &request,
                        std::string &response)
        {
            try
            {
                const json req = json::parse (request.c_str ());
                json res { { SUCCESS, true } };
                {
                    // Make sure only one thread is Handling
                    std::lock_guard<std::mutex> lg (_mtx);

                    // Callback
                    _handlers.at (req.at (REQUEST)) (
                        res[RESPONSE], req.at (PARAM));
                }

                // Write back response
                response = res.dump ();
            }
            catch (const std::logic_error &)
            {
                // Exception of json
                response = json {
                    { SUCCESS, false },
                    { RESPONSE, BAD_REQUEST }
                }.dump ();
            }
            catch (const std::runtime_error &ex)
            {
                response = json {
                    { SUCCESS, false },
                    { RESPONSE, ex.what () }
                }.dump ();
            }
        }

    protected:
        using json = nlohmann::json;

        Server (unsigned short port)
            : _server (port, std::bind (&Server::_callback, this,
                       std::placeholders::_1, std::placeholders::_2))
        {}

        inline void SetHandler (std::string event, Handler handler)
        {
            _handlers.emplace (std::move (event), std::move (handler));
        }

    public:
        void Run ()
        {
            _server.Run ();
        }

        void Stop ()
        {
            _server.Stop ();
        }
    };

    class Client
    {
        BOT_Socket::Client _sockClient;

    protected:
        using json = nlohmann::json;

        Client (const std::string &ipAddr,
                unsigned short port)
            : _sockClient (ipAddr, port)
        {}

        json Request (const std::string &request,
                      const json &param)
        {
            try
            {
                // Send & Recv
                auto response = _sockClient.Request (json {
                    { REQUEST, request }, { PARAM, param }
                }.dump ());

                // Check is succeeded
                const json res = json::parse (response.c_str ());
                if (!res.at (SUCCESS).get<bool> ())
                    throw std::runtime_error (
                        res.at (RESPONSE).get<std::string> ());

                return std::move (res.at (RESPONSE));
            }
            catch (const std::logic_error &)
            {
                // Exception of json
                throw std::runtime_error (BAD_RESPONSE);
            }
            catch (const std::runtime_error &)
            {
                throw;
            }
        }
    };
}

#undef REQUEST
#undef PARAM
#undef SUCCESS
#undef RESPONSE
#undef BAD_REQUEST
#undef BAD_RESPONSE

#endif // !BOT_CLIENT_SERVER_H