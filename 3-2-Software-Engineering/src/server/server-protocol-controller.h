
//
// Air Conditioner - Server MVC Protocol Controller
// BOT Man, 2017
//

#ifndef AC_SERVER_PROTOCOL_CONTROLLER_H
#define AC_SERVER_PROTOCOL_CONTROLLER_H

#include <utility>

#include "server-service.h"

namespace Air_Conditioner
{
    class ProtocolController
    {
    public:
        using ReturnFormat = std::pair<ClientInfo, ServerInfo>;

        ReturnFormat Auth (const GuestInfo &guest)
        {
            GuestManager::AuthGuest (guest);
            auto clientInfo = ScheduleManager::AddClient (guest);
            auto serverInfo = ConfigManager::GetConfig ();
            return std::make_pair (clientInfo, serverInfo);
        }

        ReturnFormat Pulse (const RoomRequest &req)
        {
            auto clientInfo = ScheduleManager::Pulse (req);
            auto serverInfo = ConfigManager::GetConfig ();
            return std::make_pair (clientInfo, serverInfo);
        }
    };
}

#endif // !AC_SERVER_PROTOCOL_CONTROLLER_H