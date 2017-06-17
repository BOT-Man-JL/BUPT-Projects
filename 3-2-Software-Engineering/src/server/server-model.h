
//
// Air Conditioner - Server MVC Model
// BOT Man, 2017
//

#ifndef AC_SERVER_MODEL_H
#define AC_SERVER_MODEL_H

#include <chrono>
#include <unordered_map>
#include <list>

#include "../common/common-model.h"

namespace Air_Conditioner
{
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

    struct LogOnOff
    {
        TimePoint timeBeg, timeEnd;
    };

    using LogOnOffList = std::unordered_map<RoomId, std::list<LogOnOff>>;

    struct LogRequest
    {
        TimePoint timeBeg, timeEnd;
        Temperature tempBeg, tempEnd;
        Cost costBeg, costEnd;
        Wind wind;
    };

    using LogRequestList = std::unordered_map<RoomId, std::list<LogRequest>>;

    struct ClientState
    {
        GuestId guest;

        Temperature current;
        Temperature target;
        Wind wind;

        bool hasWind;
        Energy energy;
        Cost cost;

        TimePoint pulse;

        LogOnOff lastOnOff;
        LogRequest lastRequest;
    };

    using ClientList = std::unordered_map<RoomId, ClientState>;
}

#endif // !AC_SERVER_MODEL_H