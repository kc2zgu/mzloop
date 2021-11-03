#ifndef _SENSORMQTT_HH
#define _SENSORMQTT_HH

#include "Sensor.hh"
#include "MqttAgent.hh"
#include <string>

namespace mzloop
{
    class SensorMqtt :public Sensor
    {
    public:
        SensorMqtt(MqttAgent *agent, const std::string topic);
        bool Update();

    protected:
        MqttAgent *agent;
        std::string topic;
    };
};

#endif
