
//
// Air Conditioner - Time Helper
// BOT Man, 2017
//

#ifndef AC_TIME_HELPER_H
#define AC_TIME_HELPER_H

#include <exception>
#include <chrono>
#include <string>
#include <ctime>
#include <sstream>

#include "server-model.h"

namespace Air_Conditioner
{
    class TimeHelper
    {
    public:
        static std::string TimeToString (const TimePoint &timePoint)
        {
            auto timeT = std::chrono::system_clock::to_time_t (timePoint);
            auto timeTm = std::localtime (&timeT);
            return std::to_string (timeTm->tm_year + 1900) + '-' +
                std::to_string (timeTm->tm_mon + 1) + '-' +
                std::to_string (timeTm->tm_mday);
        }

        static TimePoint TimeFromString (std::string str)
        {
            for (auto &ch : str) if (ch == '-') ch = ' ';
            std::istringstream iss (str);

            std::tm timeTm { 0 };
            auto count = 0;
            while (iss >> str)
            {
                auto num = std::stoi (str);
                if (!num) throw std::runtime_error ("Invalid Time Format");

                if (!timeTm.tm_year) timeTm.tm_year = num - 1900;
                else if (!timeTm.tm_mon) timeTm.tm_mon = num - 1;
                else if (!timeTm.tm_mday) timeTm.tm_mday = num;

                ++count;
            }

            // Validation
            if (count != 3 || timeTm.tm_year < 0 ||
                timeTm.tm_mon < 0 || timeTm.tm_mon > 11 ||
                timeTm.tm_mday < 1 || timeTm.tm_mday > 31)
                throw std::runtime_error ("Invalid Time Format");

            // Double Validation
            auto timeT = mktime (&timeTm);
            if (localtime (&timeT) == nullptr)
                throw std::runtime_error ("Invalid Time Format");

            return std::chrono::system_clock::from_time_t (timeT);
        }
    };
}

#endif // !AC_TIME_HELPER_H