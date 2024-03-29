#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <json/json.h>
#include <sys/epoll.h>
#include <boost/program_options.hpp>
#include <signal.h>
#include <uvw.hpp>
#include <sys/random.h>
#include <stdlib.h>

#include "Loop.hh"
#include "Zone.hh"
#include "Log.hh"
#include "SensorConstant.hh"
#include "SensorMqtt.hh"
#include "MqttAgent.hh"

using namespace mzloop;
using namespace std;
using namespace std::chrono;
namespace po = boost::program_options;

int update_time = 2;
string mqtt_broker;
bool mqtt_connected = false;
Loop loop;

std::string version{"0.1"};

shared_ptr<uvw::PollHandle> mqtt_start_poll(shared_ptr<uvw::Loop> uvloop, MqttAgent *agent)
{
    int s = agent->GetSocket();
    auto loop_mqtt_poll = uvloop->resource<uvw::PollHandle>(s);
    loop_mqtt_poll->on<uvw::PollEvent>([agent](uvw::PollEvent &event, uvw::PollHandle&)
        {
            if (event.flags & uvw::PollHandle::Event::READABLE)
            {
                if (!agent->Poll()) {
                    Log::Error("mqtt connection lost in poll");
                    mqtt_connected = false;
                }
                Log::Message("mqtt poll done");
            }
            if (event.flags & uvw::PollHandle::Event::DISCONNECT)
            {
                Log::Error("mqtt: disconnect event");
                mqtt_connected = false;
            }
        });
    loop_mqtt_poll->start(uvw::Flags(uvw::PollHandle::Event::READABLE) | uvw::Flags(uvw::PollHandle::Event::DISCONNECT));
    Log::Message("Started polling mqtt socket " + to_string(s));

    return loop_mqtt_poll;
}

int main(int argc, char **argv)
{
    // mosquitto 1.5 uses rand() for client ID, seed it first
    char random_seed[sizeof(int)];
    getrandom(random_seed, sizeof(int), 0);
    int seed = *((int*)random_seed);
    srand(seed);
    MqttAgent *mqagent = loop.GetMqttAgent();

    po::options_description desc("Options");
    desc.add_options()
        ("help", "Show help message")
        ("version", "Show version information")
        ("config", po::value<string>(), "Main configuration file")
        ("mqtt-broker", po::value<string>(), "MQTT broker")
        ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        cout << desc << endl;
        return 0;
    }

    if (vm.count("version"))
    {
        cout << "mzloop version " << version << endl;
        return 0;
    }

    Log::Message("mzloop starting");

    if (vm.count("mqtt-broker"))
    {
        mqtt_broker = vm["mqtt-broker"].as<string>();
    }

    if (vm.count("config"))
    {
        if (!loop.LoadConfig(vm["config"].as<string>()))
        {
            Log::Message("main: Load config failed");
            return 1;
        }
    }

    if (mqtt_broker.empty())
    {
        if (auto mqtt_conf = loop.GetConfigMisc("mqtt_broker"))
        {
            mqtt_broker = *mqtt_conf;
        }
    }

    if (auto update_time_conf = loop.GetConfigMiscNum("update_time"))
    {
        update_time = *update_time_conf;
    }

    if (!mqtt_broker.empty())
    {
        if (mqagent->Connect(mqtt_broker))
        {
            Log::Message("main: MQTT connected");
            Log::Message("main: MQTT socket: " + std::to_string(mqagent->socket()));
            mqtt_connected = true;
        }
        else
        {
            Log::Message("main: MQTT connection failed");
            return 1;
        }
    }

    auto uvloop = uvw::Loop::getDefault();

    shared_ptr<uvw::PollHandle> loop_mqtt_poll;
    auto loop_timer = uvloop->resource<uvw::TimerHandle>();
    loop_timer->start(1s, update_time * 1s);
    loop_timer->on<uvw::TimerEvent>([&loop, &loop_mqtt_poll, mqagent, &vm, &uvloop](uvw::TimerEvent&, uvw::TimerHandle&)
        {
            Log::Message("uv timer");
            loop.RunIteration();

            if (!mqagent->Poll()) {
                Log::Error("mqtt connection lost in poll");
                mqtt_connected = false;
            }

            if (!mqtt_connected && !mqtt_broker.empty())
            {
                Log::Message("main: Trying to reconnect MQTT");
                loop_mqtt_poll->close();
                loop_mqtt_poll.reset();
                if (mqagent->Connect(mqtt_broker))
                {
                    Log::Message("main: MQTT connected");
                    Log::Message("main: MQTT socket: " + std::to_string(mqagent->socket()));
                    mqtt_connected = true;
                    loop_mqtt_poll = mqtt_start_poll(uvloop, mqagent);
                }
            }
        });

    auto loop_sigint = uvloop->resource<uvw::SignalHandle>();
    loop_sigint->start(SIGINT);
    loop_sigint->on<uvw::SignalEvent>([&loop_timer, &loop_sigint, &loop_mqtt_poll](uvw::SignalEvent&, uvw::SignalHandle&)
        {
            Log::Message("interrupted!");
            loop_timer->close();
            loop_sigint->close();
            if (loop_mqtt_poll)
            {
                loop_mqtt_poll->close();
            }
        });

    if (mqtt_connected)
    {
        loop_mqtt_poll = mqtt_start_poll(uvloop, mqagent);
    }

    Log::Message("main: Ready to run loop");
    auto time_start = steady_clock::now();
    auto next_run = time_start + (update_time * 1s);

    uvloop->run();

    Log::Message("main: Exiting");

    return 0;
}

