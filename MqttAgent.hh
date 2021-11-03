#ifndef _MQTTAGENT_HH
#define _MQTTAGENT_HH

#include <mosquittopp.h>
#include <string>
#include <optional>
#include <map>

namespace mzloop
{
    class MqttAgent :public mosqpp::mosquittopp
    {
    public:
        MqttAgent();
        ~MqttAgent();

        bool Connect(std::string host, int port = 1883);
        void Poll();
        int GetSocket();

        void SubscribeTopic(std::string topic);
        void PublishTopic(std::string topic, std::string payload, bool retain = false);
        std::optional<std::string> GetTopicValue(std::string topic) const;

        void on_message(const struct mosquitto_message *message);

    protected:
        std::map<std::string, std::string> topic_values;
    };
};


#endif
