#ifndef _SENSOR_HH
#define _SENSOR_HH

#include <string>

namespace mzloop
{
    class Sensor
    {
    public:
        Sensor() :is_valid{false} {};
        virtual ~Sensor() {};

        virtual bool Update() = 0;
        bool IsValid() const { return is_valid; }
        double GetValue() const { return value; }
        virtual std::string GetError() const { return ""; }

    protected:
        bool is_valid;
        double value;
    };
};

#endif
