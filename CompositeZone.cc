#include "Zone.hh"
#include "Log.hh"
#include "Schedule.hh"

using namespace mzloop;
using namespace std;

CompositeZone::CompositeZone(const string name)
    :Zone{name}
{

}

void CompositeZone::ReadPresentValue()
{
    double pv_total, pv_weight_sum = 0;
    int valid_zones = 0;
    for (auto zone: member_zones)
    {
        zone->ReadPresentValue();
        auto pv = zone->GetPresentValue();
        if (pv)
        {
            pv_total += *pv;
            pv_weight_sum += 1;
            valid_zones++;
        }
    }
    if (valid_zones > 0)
    {
        pv_valid = true;
        present_value = pv_total / pv_weight_sum;
        Log::Message("CompositeZone " + name + " " + to_string(valid_zones) + " zones with PV");
    }
    else
    {
        pv_valid = false;
        Log::Message("CompositeZone " + name + " no PV");
    }
}

void CompositeZone::AddMemberZone(Zone *zone)
{
    member_zones.push_back(zone);
}

Command CompositeZone::GetOutput(const std::chrono::system_clock::time_point &tp)
{
    if (has_sv || (sched != nullptr))
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
            Log::Message("CompositeZone: no input value");
            return last_command;
        }
    }
    else
    {
        int zones_on = 0, zones_urgent = 0;
        double accept_weight = 0;
        for (auto zone: member_zones)
        {
            Log::Message("Checking member zone " + zone->GetName());
            Command member_cmd = zone->GetOutput();
            Log::Message("Output command: " + CommandString(member_cmd));
            if (member_cmd == RequireUrgent)
            {
                zones_urgent++;
            }
            else if (member_cmd == RequireNormal)
            {
                zones_on++;
            } else if (member_cmd == Accept)
            {
                accept_weight += 0.1;
            }
        }
        Log::Message("CompositeZone " + name + ": " + std::to_string(zones_on) + " zones on, " + std::to_string(zones_urgent) + " urgent, total accept_weight " + std::to_string(accept_weight));
        if(zones_urgent > 0)
        {
            last_command = RequireUrgent;
        }
        else if (zones_on > 0)
        {
            last_command = RequireNormal;
        } else if (accept_weight > 1.0)
        {
            last_command = RequireNormal;
        } else if (accept_weight > 0.0)
        {
            last_command = Accept;
        } else
        {
            last_command = Off;
        }
        return last_command;
    }
}
