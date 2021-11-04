#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <chrono>

#include "Schedule.hh"

using namespace mzloop;
using namespace std;
using namespace std::chrono;

int main(int argc, char **argv)
{
    Schedule sched;

    if (argc > 1)
    {
        sched.LoadSchedule(argv[1]);
    }
}
