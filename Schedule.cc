#include "Schedule.hh"
#include "Log.hh"
#include <fstream>
#include <regex>

using namespace mzloop;
using namespace std;
using namespace std::chrono;

Schedule::Schedule()
    :default_interpolate{SchedStep},
     ramp_rate{5}
{
    
}

Schedule::~Schedule()
{
    
}

void Schedule::SetDefaultInterpolate(SchedInterpolate new_interpolate)
{
    default_interpolate = new_interpolate;
}

void Schedule::AddPoint(const week_time &wt, double sv, SchedInterpolate interpolate)
{
    auto newpoint = schedule_points.try_emplace(wt, schedule_point{wt, sv,
            interpolate == SchedDefault ? default_interpolate : interpolate});
    Log::Message("Schedule: Added " + to_string(wt.day) + ":" + to_string(wt.seconds) + " " + to_string(sv));
}

void Schedule::RemovePoint(const week_time &wt)
{
    schedule_points.erase(wt);
}

void Schedule::Clear()
{
    schedule_points.clear();
}

void Schedule::LoadSchedule(std::string file)
{
    Clear();
    Log::Message("Schedule: reading " + file);
    ifstream stream {file};
    regex pattern{"([umtwrfs]*)@(\\d{4}) (\\d+(?:\\.\\d*)?)(?: ([srec]))?"};
    while (!stream.eof())
    {
        string line;
        smatch match;
        getline(stream, line);
        //Log::Message("line: " + line);
        if (regex_match(line, match, pattern))
        {
            const auto& days = match.str(1);
            const auto& time = match.str(2);
            const auto& sv = match.str(3);
            const auto& interp = match.str(4);
            //Log::Message("regex matched");
            //Log::Message("days: " + days);
            //Log::Message("time: " + time);
            //Log::Message("setpoint: " + sv);
            //Log::Message("interp: " + interp);
            int time_sec = (time[0] - '0') * 36000 + (time[1] - '0') * 3600
                + (time[2] - '0') * 600 + (time[3] - '0') * 60;
            //Log::Message("time(s): " + to_string(time_sec));
            double sv_num = stod(sv);
            SchedInterpolate interp_value = default_interpolate;
            switch (interp[0])
            {
            case 's': interp_value = SchedStep; break;
            case 'r': interp_value = SchedFullRamp; break;
            case 'e': interp_value = SchedEndRamp; break;
            case 'c': interp_value = SchedCosine; break;
            }
            for (char c: days)
            {
                int day;
                switch (c)
                {
                case 'm': day = 1; break;
                case 't': day = 2; break;
                case 'w': day = 3; break;
                case 'r': day = 4; break;
                case 'f': day = 5; break;
                case 's': day = 6; break;
                case 'u': day = 0; break;
                }
                week_time wt{day,time_sec};
                AddPoint(wt, sv_num, interp_value);
            }
        }
    }
}

const schedule_point *Schedule::GetPointExact(const week_time &wt) const
{
    auto sp = schedule_points.find(wt);
    if (sp != schedule_points.end())
        return &(*sp).second;
    else
        return nullptr;
}

const schedule_point *Schedule::GetPointPrev(const week_time &wt) const
{
    auto sp = schedule_points.lower_bound(wt);
    if (sp == schedule_points.end())
    {
        return nullptr;
    }
    sp--;
    if (sp == schedule_points.end())
    {
        sp--;
    }
    if (sp == schedule_points.end())
    {
        return nullptr;
    }
    return &(*sp).second;
}

const schedule_point *Schedule::GetPointNext(const week_time &wt) const
{
    auto sp = schedule_points.upper_bound(wt);
    if (sp == schedule_points.end())
    {
        sp = schedule_points.begin();
    }
    if (sp == schedule_points.end())
    {
        return nullptr;
    }
    return &(*sp).second;
}

double Schedule::GetSv(const week_time &wt) const
{
    auto exact = GetPointExact(wt);
    if (exact != nullptr)
        return exact->sv;

    auto before = GetPointPrev(wt);
    auto after = GetPointNext(wt);

    if (after == nullptr)
    {
        if (before == nullptr)
        {
            return 0;
        }
        return before->sv;
    }

    if (before == nullptr)
    {
        return after->sv;
    }

    int raw_seconds_1 = before->wt.raw_seconds();
    int raw_seconds_2 = after->wt.raw_seconds();
    int raw_seconds_now = wt.raw_seconds();
    //Log::Message("t1=" + to_string(raw_seconds_1) + " t2=" + to_string(raw_seconds_2) + " t=" + to_string(raw_seconds_now));
    double sv;

    switch (after->interp)
    {
    case SchedStep:
        return after->sv;
    case SchedFullRamp:
    {
        double dsv = after->sv - before->sv;
        double dt = raw_seconds_2 - raw_seconds_1;

        double l = (raw_seconds_now - raw_seconds_1) / dt;
        //Log::Message("l=" + to_string(l));

        return before->sv + l * dsv;
    }
    case SchedEndRamp:
        //Log::Message("dt=" + to_string(raw_seconds_2 - raw_seconds_now));
        //Log::Message("dt_h=" + to_string((raw_seconds_2 - raw_seconds_now) / 3600.0));
        if (after->sv > before->sv)
        {
            sv = after->sv - ramp_rate * ((raw_seconds_2 - raw_seconds_now) / 3600.0);
            if (sv < before->sv)
                sv = before->sv;
        }
        else
        {
            sv = after->sv + ramp_rate * ((raw_seconds_2 - raw_seconds_now) / 3600.0);
            if (sv > before->sv)
                sv = before->sv;            
        }
        return sv;
    }

    return before->sv;
}

double Schedule::GetSv(const std::chrono::system_clock::time_point &tp) const
{
    auto tt = system_clock::to_time_t(tp);
    auto lt = localtime(&tt);
    int s = lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec;
    Log::Message("localtime: " + to_string(lt->tm_wday) + " " + to_string(s));

    return GetSv(week_time{lt->tm_wday, s});
}
