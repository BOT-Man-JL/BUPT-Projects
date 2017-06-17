
//
// Air Conditioner - Time Helper
// BOT Man, 2017
//

#ifndef AC_LOG_HELPER_H
#define AC_LOG_HELPER_H

#include <string>
#include <sstream>

#include "server-model.h"
#include "time-helper.h"

namespace Air_Conditioner
{
    class LogHelper
    {
    public:
        static std::string LogOnOffListToCsv (const LogOnOffList &list)
        {
            std::ostringstream oss;
            oss << "room,begin,end";
            for (const auto &item : list)
                for (const auto &entry : item.second)
                {
                    oss << "\n" << item.first
                        << "," << TimeHelper::TimeToString (entry.timeBeg)
                        << "," << TimeHelper::TimeToString (entry.timeEnd);
                }
            return oss.str ();
        }

        static std::string LogRequestListToCsv (const LogRequestList &list)
        {
            static std::unordered_map<Wind, std::string> windStr
            {
                { 1, "Weak" },
                { 2, "Mid" },
                { 3, "Strong" }
            };

            std::ostringstream oss;
            oss << "room,begin,end,temp-begin,temp-end,wind,cost";
            for (const auto &item : list)
                for (const auto &entry : item.second)
                {
                    oss << "\n" << item.first
                        << "," << TimeHelper::TimeToString (entry.timeBeg)
                        << "," << TimeHelper::TimeToString (entry.timeEnd)
                        << "," << entry.tempBeg
                        << "," << entry.tempEnd
                        << "," << windStr[entry.wind]
                        << "," << entry.costEnd - entry.costBeg;
                }
            return oss.str ();
        }
    };
}

#endif // !AC_LOG_HELPER_H