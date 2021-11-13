#include "Zone.hh"
#include "Log.hh"
#include "Schedule.hh"

using namespace mzloop;
using namespace std;

LeafZone::LeafZone(const string name)
    :Zone{name}
{

}

Command LeafZone::GetOutput(const std::chrono::system_clock::time_point &tp)
{
    ReadPresentValue();

    if (pv_valid)
    {
        double sv;
        if (has_sv)
            sv = set_value;
        if (sched != nullptr)
            sv = sched->GetSv(tp);
        last_command = GetCommandForSetpoint(last_command, present_value, sv);
        return last_command;
    }
    else
    {
        Log::Message("LeafZone: no input value");
        last_command = Off;
        return last_command;
    }
}
