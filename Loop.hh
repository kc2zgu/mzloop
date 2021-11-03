#ifndef _LOOP_HH
#define _LOOP_HH

#include <vector>
#include <memory>

#include "Schedule.hh"

namespace mzloop
{
    class Zone;
    class Output;
    class MqttAgent;

    class Loop
    {
    public:
        Loop();
        virtual ~Loop();

        bool LoadConfig(const std::string config_file);
        MqttAgent *GetMqttAgent();
        const MqttAgent *GetMqttAgent() const { return mqtt_agent; }

        void RunIteration();

        Zone *GetZone(const std::string name) const;

        Zone *CreateLeafZone(const std::string name);
        Zone *CreateCompositeZone(const std::string name);

        void AddOutput(Output *output);

    protected:
        std::vector<std::unique_ptr<Zone>> zones;
        std::vector<std::unique_ptr<Output>> outputs;

        MqttAgent *mqtt_agent;
        Schedule sched;
    };
};

#endif
