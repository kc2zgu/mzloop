#include "Loop.hh"
#include "Zone.hh"
#include "Output.hh"
#include "Log.hh"
#include "SensorMqtt.hh"
#include "OutputMqtt.hh"
#include "OutputGpio.hh"
#include <fstream>
#include <chrono>
#include <json/json.h>

using namespace mzloop;
using namespace std;
using namespace std::chrono;

Loop::Loop()
    :mqtt_agent{nullptr}
{

}

Loop::~Loop()
{
    Log::Message("Loop: destroying");
    for (auto &output: outputs)
    {
        Log::Message("Forcing output " + output->GetName() + " to OFF");
        output->SetInactive();
    }
}

bool Loop::LoadConfig(const std::string config_file)
{
    Log::Message("Opening " + config_file);
    ifstream config_stream{config_file};

    if (config_stream.good())
    {
        Json::Value conf_root;
        config_stream >> conf_root;

        auto& conf_main = conf_root["main"];
        if (conf_main.type() == Json::objectValue)
        {
            Log::Message("Processing main options");
            if (conf_main["schedule_file"].type() == Json::stringValue)
            {
                sched.LoadSchedule(conf_main["schedule_file"].asString());
            }
        }

        auto& conf_zones = conf_root["zones"];
        if (conf_zones.type() == Json::arrayValue) // read all zones
        {
            for (auto& zone: conf_zones)
            {
                if (zone.type() == Json::objectValue)
                {
                    auto& name = zone["name"];
                    Log::Message("Found zone " + name.asString());
                    Zone *newzone;
                    if (!zone.isMember("members"))
                    {
                        Log::Message("Leaf zone");
                        newzone = CreateLeafZone(name.asString());
                        if (zone.isMember("input"))
                        {
                            Log::Message("Processing input sensor config");
                            auto& input = zone["input"];
                            if (input["type"].asString() == "mqtt")
                            {
                                Log::Message("MQTT input");
                                auto *sensor = new SensorMqtt{mqtt_agent, input["mqtt_topic"].asString()};
                                newzone->AssignInput(sensor);
                            }
                            else
                            {
                                Log::Message("Invalid input type");
                                return false;
                            }
                        }
                        else
                        {
                            Log::Message("Leaf zone with no input");
                        }
                    }
                    else
                    {
                        Log::Message("Composite zone");
                        newzone = CreateCompositeZone(name.asString());
                    }
                    if (zone.isMember("setpoint"))
                    {
                        newzone->SetValue(zone["setpoint"].asDouble());
                        if (zone.isMember("hysteresis") && zone["hysteresis"].type() == Json::arrayValue)
                        {
                            auto& hysteresis = zone["hysteresis"];
                            newzone->SetHysteresis(hysteresis[0].asDouble(),
                                                   hysteresis[1].asDouble(),
                                                   hysteresis[2].asDouble(),
                                                   hysteresis[3].asDouble());
                        }
                    }
                    if (zone.isMember("use_schedule"))
                    {
                        newzone->UseSchedule(&sched);
                    }
                }
                else
                {
                    Log::Message("Config error: zones member is not an object");
                    return false;
                }
            }
        }
        else
        {
            Log::Message("Config error: zones is not an array");
            return false;
        }
        auto& outputs = conf_root["outputs"];
        if (outputs.type() == Json::arrayValue) // read all outputs
        {
            for (auto& output: outputs)
            {
                if (output.type() == Json::objectValue)
                {
                    auto& name = output["name"];
                    auto& zone = output["zone"];
                    Log::Message("Found output " + name.asString() + " for zone " + zone.asString());
                    if (output["output"]["type"].asString() == "mqtt")
                    {
                        Log::Message("MQTT output");
                        Zone *zoneobj = GetZone(zone.asString());
                        if (zoneobj == nullptr)
                        {
                            Log::Message("Zone not defined");
                            return false;
                        }
                        auto *outputobj = new OutputMqtt{name.asString(), zoneobj,
                            mqtt_agent, output["output"]["mqtt_topic"].asString(), "1", "0"};
                        AddOutput(outputobj);
                    } else if (output["output"]["type"].asString() == "gpio")
                    {
                        Log::Message("GPIO output");
                        Zone *zoneobj = GetZone(zone.asString());
                        if (zoneobj == nullptr)
                        {
                            Log::Message("Zone not defined");
                            return false;
                        }
                        auto *outputobj = new OutputGpio{name.asString(), zoneobj,
                                                         output["output"]["gpio_chip"].asString(),
                                                         output["output"]["gpio_line"].asInt()};
                        AddOutput(outputobj);
                    }
                }
            }
        }
        else
        {
            Log::Message("Config error: outputs is not an array");
            return false;
        }
        for (auto& zone: conf_zones) // populate composite zone members
        {
            auto& name = zone["name"];
            CompositeZone *zoneobj = dynamic_cast<CompositeZone*>(GetZone(name.asString()));
            if (zoneobj != nullptr)
            {
                if (zone["members"].type() == Json::arrayValue)
                {
                    for (auto& member: zone["members"])
                    {
                        Log::Message("Composite zone " + zoneobj->GetName() + ": member " + member.asString());
                        Zone *member_zone = GetZone(member.asString());
                        if (member_zone != nullptr)
                            zoneobj->AddMemberZone(member_zone);
                    }
                }
            }
            else
            {
                Log::Message("Not a composite zone: " + name.asString());
            }
        }
        // subscribe to mqtt command topics
        for (auto& zone: zones)
        {
            string name = zone->GetName();
            string base = "mzloop/zones/" + name;
            mqtt_agent->SubscribeTopic(base + "/sv");
        }
        return true;
    }
    return false;
}

void Loop::RunIteration()
{
    auto time_now = system_clock::now();
    Log::Message("Loop: run");
    for (auto &output: outputs)
    {
        Log::Message("Loop: Updating output " + output->GetName());
        Zone *zone = output->GetZone();
        Log::Message("Checking zone " + zone->GetName());
        Command cmd = zone->GetOutput();

        Log::Message("Output command: " + CommandString(cmd));
        if (cmd == RequireNormal || cmd == RequireUrgent)
        {
            Log::Message("Setting output " + output->GetName() + " to ON");
            output->SetOn();
            mqtt_agent->PublishTopic("mzloop/outputs/" + output->GetName(), "on");
        }
        else
        {
            Log::Message("Setting output " + output->GetName() + " to OFF");
            output->SetOff();
            mqtt_agent->PublishTopic("mzloop/outputs/" + output->GetName(), "off");
        }
    }

    for (auto &zone: zones)
    {
        string name = zone->GetName();
        string base = "mzloop/zones/" + name;
        auto pv = zone->GetPresentValue();
        mqtt_agent->PublishTopic(base + "/pv", pv ? to_string(*pv) : "none");
        auto sv = zone->GetSetValue(time_now);
        mqtt_agent->PublishTopic(base + "/sv", sv ? to_string(*sv) : "none");
    }
}

Zone *Loop::GetZone(const string name) const
{
    for (auto &zone: zones)
    {
        if (zone->GetName() == name)
            return zone.get();
    }
    return nullptr;
}

Zone *Loop::CreateLeafZone(const string name)
{
    Zone *newzone = new LeafZone{name};
    Log::Message("Loop: created zone " + name);
    zones.push_back(unique_ptr<Zone>{newzone});
    return newzone;
}

Zone *Loop::CreateCompositeZone(const string name)
{
    Zone *newzone = new CompositeZone{name};
    Log::Message("Loop: created zone " + name);
    zones.push_back(unique_ptr<Zone>{newzone});
    return newzone;
}

MqttAgent *Loop::GetMqttAgent()
{
    if (mqtt_agent == nullptr)
        mqtt_agent = new MqttAgent();
    return mqtt_agent;
}

void Loop::AddOutput(Output *output)
{
    outputs.push_back(unique_ptr<Output>{output});
    Log::Message("Loop: added output " + output->GetName());
}
