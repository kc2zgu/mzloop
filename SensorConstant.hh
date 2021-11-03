#ifndef _SENSORCONSTANT_HH
#define _SENSORCONSTANT_HH

#include "Sensor.hh"

namespace mzloop
{
    class SensorConstant :public Sensor
    {
    public:
        SensorConstant(double value) { is_valid = true; this->value = value; }
        bool Update() { return true; }
    };
};

#endif
