
//
// Air Conditioner - Client MVC Controller
// BOT Man, 2017
//

#ifndef AC_CLIENT_CONTROLLER_H
#define AC_CLIENT_CONTROLLER_H

#include <chrono>
#include <algorithm>

#include "client-model.h"
#include "client-view.h"
#include "client-protocol.h"

// Fix hell of Windows.h
#ifdef max
#undef max
#endif // max
#ifdef min
#undef min
#endif // min

namespace Air_Conditioner
{
    class ClientFacadeController
    {
        ProtocolClient &_client;
        ClientViewManager &_viewManager;

        using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
        TimePoint _lastSim;
        bool _hasStartSim;

    public:
        ClientFacadeController (ClientViewManager &viewManager)
            : _client (ProtocolClient::Instance ()), _viewManager (viewManager),
            _hasStartSim (false)
        {}

        void Auth (const GuestInfo &guest)
        {
            auto serverInfo = _client.Auth (guest);
            _viewManager.ToControlView (guest, serverInfo);
        }

        std::pair<ClientInfo, ServerInfo> Pulse (const RoomRequest &req)
        {
            return _client.Pulse (req);
        }

        void Simulate (RoomRequest &request,
                       const bool hasWind)
        {
            // Init Sim
            if (!_hasStartSim)
            {
                _lastSim = std::chrono::system_clock::now ();
                request.current = DefaultRoomTemp;
                _hasStartSim = true;
                return;
            }

            // Get Delta Time
            auto now = std::chrono::system_clock::now ();
            std::chrono::duration<double> deltaTime = now - _lastSim;
            _lastSim = now;

            // Get Delta Temp and Target Temp
            auto deltaTemp = Temperature { deltaTime.count () / 2.0 };
            auto targetTemp = hasWind ? request.target : DefaultRoomTemp;

            // Spec for has wind cases
            if (hasWind)
                deltaTemp += deltaTemp * (request.wind - 2) / 4.0;
            else
                deltaTemp /= 4.0;  // changing slower than hasWind

            // Handle changes
            if (request.current < targetTemp)
            {
                request.current += deltaTemp;
                request.current = std::min (request.current, targetTemp);
            }
            else if (request.current > targetTemp)
            {
                request.current -= deltaTemp;
                request.current = std::max (request.current, targetTemp);
            }
        }
    };
}

#endif // !AC_CLIENT_CONTROLLER_H