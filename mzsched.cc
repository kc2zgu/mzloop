#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <chrono>

#include "Schedule.hh"

using namespace mzloop;
using namespace std;
using namespace std::chrono;

void show_point(const schedule_point *sp)
{
    if (sp != nullptr)
    {
        cout << "Found point: " << sp->wt.day << ":" << sp->wt.seconds << " " << sp->sv << endl;
    }
    else
    {
        cout << "no point found" << endl;
    }
}

int main(int argc, char **argv)
{
    Schedule sched;

    if (argc > 1)
    {
        sched.LoadSchedule(argv[1]);

        auto sp = sched.GetPointExact(week_time{1,64800});
        show_point(sp);
        sp = sched.GetPointPrev(week_time{1, 18000});
        show_point(sp);
        sp = sched.GetPointNext(week_time{1, 18000});
        show_point(sp);
        sp = sched.GetPointPrev(week_time{0, 1800});
        show_point(sp);
        sp = sched.GetPointNext(week_time{6, 85500});
        show_point(sp);
        double sv = sched.GetSv(week_time{1, 18000});
        cout << "SV=" << sv << endl;
        sv = sched.GetSv(week_time{1, 19800});
        cout << "SV=" << sv << endl;
        sv = sched.GetSv(week_time{1, 21900});
        cout << "SV=" << sv << endl;
        sv = sched.GetSv(week_time{3, 75600});
        cout << "SV=" << sv << endl;
        sv = sched.GetSv(week_time{3, 79200});
        cout << "SV=" << sv << endl;
        sv = sched.GetSv(week_time{3, 81000});
        cout << "SV=" << sv << endl;
    }
}
