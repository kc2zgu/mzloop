#include "Schedule.hh"
#include "Log.hh"
#include <fstream>
#include <regex>

using namespace mzloop;
using namespace std;

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

void Schedule::AddPoint(week_time &wt, double sv, SchedInterpolate interpolate)
{
    schedule_points.try_emplace(wt, schedule_point{wt, sv,
            interpolate == SchedDefault ? default_interpolate : interpolate});
}

void Schedule::RemovePoint(week_time &wt)
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
        Log::Message("line: " + line);
        if (regex_match(line, match, pattern))
        {
            Log::Message("regex matched");
            Log::Message("days: " + match.str(1));
            Log::Message("time: " + match.str(2));
            Log::Message("setpoint: " + match.str(3));
            Log::Message("interp: " + match.str(4));
        }
    }
}

const schedule_point *Schedule::GetPointExact(week_time &wt) const
{
    auto sp = schedule_points.find(wt);
    if (sp != schedule_points.end())
        return &(*sp).second;
    else
        return nullptr;
}

const schedule_point *Schedule::GetPointPrev(week_time &wt) const
{
    auto sp = schedule_points.lower_bound(wt);
}

const schedule_point *Schedule::GetPointNext(week_time &wt) const
{
    auto sp = schedule_points.upper_bound(wt);
    
}

double Schedule::GetSv(week_time &wt) const
{

}

