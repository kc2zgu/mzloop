#include "MqttAgent.hh"
#include "Log.hh"

using namespace mzloop;
using namespace std;

MqttAgent::MqttAgent()
{

}

MqttAgent::~MqttAgent()
{
    disconnect();
    Poll();
}

bool MqttAgent::Connect(std::string host, int port)
{
    Log::Message("MQTT: connecting to " + host);
    int result = connect(host.c_str(), port, 10);
    return (result == MOSQ_ERR_SUCCESS);
}

void MqttAgent::Poll()
{
    Log::Message("MQTT: polling");
    loop_read();
    if (want_write())
        loop_write();
    loop_misc();
}

int MqttAgent::GetSocket()
{
    return socket();
}

void MqttAgent::SubscribeTopic(std::string topic)
{
    subscribe(nullptr, topic.c_str(), 1);
    Log::Message("MQTT: subscribed to " + topic);
}

void MqttAgent::PublishTopic(std::string topic, std::string payload, bool retain)
{
    publish(nullptr, topic.c_str(), payload.size(), payload.c_str(), 1, retain);
    Log::Message("MQTT: published " + topic + "=" + payload + (retain ? "[r]" : ""));
}

std::optional<std::string> MqttAgent::GetTopicValue(std::string topic) const
{
    auto value = topic_values.find(topic);
    if (value != topic_values.end())
    {
        return value->second;
    }
    else
    {
        return nullopt;
    }
}

void MqttAgent::on_message(const struct mosquitto_message *message)
{
    if (message->topic != nullptr && message->payload != nullptr)
    {
        std::string topic{message->topic};
        std::string payload{(const char*)message->payload, (size_t)message->payloadlen};
        Log::Message("MQTT: message: " + topic + "=" + payload);
        topic_values[topic] = payload;
    }
}
