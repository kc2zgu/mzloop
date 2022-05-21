#ifndef _LOOP_HH
#define _LOOP_HH

#include <vector>
#include <memory>
#include <optional>

#include "Schedule.hh"

namespace Json { class Value; };

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

        const std::string* GetConfigMisc(std::string key) const
            {
                auto found = misc_config.find(key);
                return (found != misc_config.end()) ? &(found->second) : nullptr;
            }
        std::optional<double> GetConfigMiscNum(std::string key) const
            {
                auto found = misc_config.find(key);
                if (found != misc_config.end())
                {
                    try
                    {
                        return std::stod(found->second);
                    } catch (const std::exception&)
                    {
                        return std::nullopt;
                    }
                }
                return std::nullopt;
            }

    protected:
        std::vector<std::unique_ptr<Zone>> zones;
        std::vector<std::unique_ptr<Output>> outputs;
        std::map<std::string, std::string> misc_config;

        MqttAgent *mqtt_agent;
        Schedule sched;

        bool LoadConfigZones(const Json::Value &zones);
        bool LoadConfigOutputs(const Json::Value &outputs);
        bool LoadConfigMisc(const Json::Value &misc);
    };
};

#endif
