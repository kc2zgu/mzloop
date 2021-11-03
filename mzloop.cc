#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <json/json.h>
#include <sys/epoll.h>
#include <boost/program_options.hpp>

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

int epoll_fd;
struct epoll_event ev, events[8];
int update_time = 10;

std::string version{"0.1"};

void add_poll_fd(int fd)
{
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

int do_poll(int timeout_ms)
{
    if (int ready = epoll_wait(epoll_fd, events, 8, timeout_ms) > 0)
    {
        return ready;
    }
    return 0;
}

int main(int argc, char **argv)
{
    Loop loop;

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
        if (mqagent->Connect(vm["mqtt-broker"].as<string>()))
        {
            Log::Message("MQTT connected");
            Log::Message("MQTT socket: " + std::to_string(mqagent->socket()));
        }
        else
        {
            Log::Message("MQTT connection failed");
            return 1;
        }
    }

    if (vm.count("config"))
    {
        if (!loop.LoadConfig(vm["config"].as<string>()))
        {
            Log::Message("Load config failed");
            return 1;
        }
    }

    epoll_fd = epoll_create(1);
    add_poll_fd(mqagent->socket());

    Log::Message("ready to run loop");
    auto time_start = steady_clock::now();
    auto next_run = time_start + (update_time * 1s);

    while (1)
    {
        auto time_now = steady_clock::now();
        if (time_now > next_run)
        {
            Log::Message("running");
            mqagent->Poll();
            loop.RunIteration();
            next_run += (update_time * 1s);
        } else
        {
            auto time_left = duration_cast<milliseconds>(next_run - time_now).count();
            Log::Message("waiting " + to_string(time_left) + "ms");
            if (do_poll(time_left))
            {
                mqagent->Poll();
            }
        }
    }

    sleep(2);

    return 0;
}

