#include "SensorMqtt.hh"
#include "Log.hh"

using namespace mzloop;
using namespace std;

SensorMqtt::SensorMqtt(MqttAgent *agent, const string topic)
    :agent(agent), topic(topic)
{
    if (agent == nullptr)
        Log::Message("Null MQTT agent!");
    else
    {
        agent->SubscribeTopic(topic);
        Log::Message("SensorMqtt: subscribed to " + topic);
    }
}

bool SensorMqtt::Update()
{
    if (agent == nullptr)
    {
        Log::Message("Null MQTT agent!");
        return false;
    }
    auto payload = agent->GetTopicValue(topic);
    if (payload)
    {
        Log::Message("SensorMqtt: " + topic + " is " + *payload);
        is_valid = true;
        value = stod(*payload);
        return true;
    }
    else
    {
        Log::Message("SensorMqtt: no value for " + topic);
        is_valid = false;
        return false;
    }
}
