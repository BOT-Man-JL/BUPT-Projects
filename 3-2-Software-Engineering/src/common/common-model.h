
//
// Air Conditioner - Common MVC Model
// BOT Man, 2017
//

#ifndef AC_COMMON_MODEL_H
#define AC_COMMON_MODEL_H

#include <string>

namespace Air_Conditioner
{
    using RoomId = std::string;
    using GuestId = std::string;

    using Temperature = double;
    using Wind = int;  // Stop, Weak, Mid, Strong = 0, 1, 2, 3
    using Energy = double;
    using Cost = double;

    using WorkingMode = int; // Summer, Winter = 0, 1
}

namespace Air_Conditioner
{
    struct GuestInfo
    {
        RoomId room;
        GuestId guest;
    };

    struct RoomRequest
    {
        RoomId room;
        Temperature current;
        Temperature target;
        Wind wind;
    };

    struct ClientInfo
    {
        bool hasWind;
        Energy energy;
        Cost cost;
    };

    struct ServerInfo
    {
        bool isOn = false;
        WorkingMode mode = 0;
    };
}

#endif // !AC_COMMON_MODEL_H