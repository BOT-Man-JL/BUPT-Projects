
//
// Air Conditioner - Client MVC Model
// BOT Man, 2017
//

#ifndef AC_CLIENT_MODEL_H
#define AC_CLIENT_MODEL_H

#include "../common/common-model.h"

namespace Air_Conditioner
{
    // Default temperatures
    constexpr Temperature DefaultRoomTemp = Temperature { 25 };
    constexpr Temperature DefaultSummerTemp = Temperature { 22 };
    constexpr Temperature DefaultWinterTemp = Temperature { 28 };
    constexpr Temperature MinTemp = Temperature { 18 };
    constexpr Temperature MaxTemp = Temperature { 30 };
}

#endif // !AC_CLIENT_MODEL_H