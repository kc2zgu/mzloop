#include "OutputMqtt.hh"

using namespace mzloop;
using namespace std;

OutputMqtt::OutputMqtt(string name, Zone *zone, MqttAgent *agent, string topic, string onval, string offval)
    :Output{name, zone}, agent{agent}, topic{topic}, onval{onval}, offval{offval}
{
    
}

bool OutputMqtt::SetActive()
{
    agent->PublishTopic(topic, onval);
    return true;
}

bool OutputMqtt::SetInactive()
{
    agent->PublishTopic(topic, offval);
    return false;
}

