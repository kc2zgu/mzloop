#ifndef _OUTPUTMQTT_HH
#define _OUTPUTMQTT_HH

#include "Output.hh"
#include "MqttAgent.hh"

namespace mzloop
{

    class OutputMqtt :public Output
    {
    public:
        OutputMqtt(std::string name, Zone *zone, MqttAgent *agent, std::string topic, std::string onval, std::string offval);

        bool SetActive() override;
        bool SetInactive() override;

    protected:
        MqttAgent *agent;
        std::string topic, onval, offval;
    };
};

#endif
