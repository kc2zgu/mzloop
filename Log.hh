#ifndef _LOG_HH
#define _LOG_HH

#include <string>

namespace mzloop
{
    class Log
    {
    public:
        static void Message(std::string text);
    };
};

#endif
