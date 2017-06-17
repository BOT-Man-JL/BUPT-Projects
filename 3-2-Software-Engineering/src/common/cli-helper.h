
//
// Air Conditioner - CLI View Helper
// BOT Man, 2017
//

#ifndef AC_CLI_VIEW_HELPER_H
#define AC_CLI_VIEW_HELPER_H

#include <iostream>
#include <sstream>

namespace Air_Conditioner
{
    class InputHelper
    {
        // Use non-template to reuse buffer :-)
        static const char *_GetLine ()
        {
            static char buf[1024];
            std::cin.getline (buf, 1024);
            return buf;
        }

    public:
        template<typename T>
        static T Get (const std::string &field)
        {
            T ret;
            while (true)
            {
                std::cout << field << ": ";
                if (GetByRef (ret))
                    return ret;
                std::cout << "Invalid " << field << std::endl;
            }
        }

        // Use stringstream for marshalling input :-)
        template<typename T>
        static bool GetByRef (T &val)
        {
            auto line = _GetLine ();
            std::istringstream iss (line);
            iss >> val;
            return !iss.fail () && !iss.bad ();
        }

        static inline void GetLine () { _GetLine (); }
    };
}

#endif // !AC_CLI_VIEW_HELPER_H