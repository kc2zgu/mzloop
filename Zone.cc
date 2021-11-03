#include "Zone.hh"
#include "Sensor.hh"
#include "Log.hh"

using namespace mzloop;
using namespace std;

Zone::Zone(const string name)
    :name{name},
     last_command{Off},
     input{nullptr},
     has_sv{false},
     pv_valid{false}
{
    Log::Message("Zone: created " + name);
}

Zone::~Zone()
{
    Log::Message("Zone: destroying " + name);
}

void Zone::SetValue(double sv)
{
    set_value = sv;
    has_sv = true;
    Log::Message("Zone " + name + ": setpoint " + std::to_string(sv));
}

void Zone::SetHysteresis(double off, double accept, double norm, double urgent)
{
    if (!((off < accept && accept < norm && norm < urgent) ||
          (off > accept && accept > norm && norm > urgent)))
    {
        Log::Message("Inconsistent hysteresis order");
    }
    hyst_off = off;
    hyst_accept = accept;
    hyst_norm = norm;
    hyst_urgent = urgent;
    Log::Message("Zone " + name + ": set hysteresis off=" + std::to_string(hyst_off)
                 + " acc=" + std::to_string(hyst_accept)
                 + " norm=" + std::to_string(hyst_norm)
                 + " urg=" + std::to_string(hyst_urgent));
}

void Zone::ReadPresentValue()
{
    if (input != nullptr)
    {
        input->Update();
        if (input->IsValid())
        {
            pv_valid = true;
            present_value = input->GetValue();
        }
        else
        {
            pv_valid = false;
        }
    }
    else
    {
        Log::Message("zone: ReadPresentValue: no input sensor");
    }
}

void Zone::AssignInput(Sensor *newinput)
{
    input = newinput;
}

Command Zone::GetCommandForSetpoint(Command last, double pv, double sv)
{
    if (hyst_norm > 0.0)
    {
        Log::Message("negative action, PV=" + std::to_string(pv) + ", SV=" + std::to_string(sv));
        if (last == Off)
        {
            if (pv > (sv + hyst_urgent))
            {
                return RequireUrgent;
            }
            else if (pv > (sv + hyst_norm))
            {
                return RequireNormal;
            }
            else if (pv > (sv + hyst_accept))
            {
                return Accept;
            }
        }
        else
        {
            if (pv < (sv + hyst_off))
            {
                return Off;
            }
            else if (last == Accept)
            {
                if (pv > (sv + hyst_urgent))
                {
                    return RequireUrgent;
                }
                else if (pv > (sv + hyst_norm))
                {
                    return RequireNormal;
                }
            }
            else if (last == RequireNormal)
            {
                if (pv > (sv + hyst_urgent))
                {
                    return RequireUrgent;
                }
            }
        }
    }
    else
    {
        Log::Message("positive action, PV=" + std::to_string(pv) + ", SV=" + std::to_string(sv));
        if (last == Off)
        {
            if (pv < (sv + hyst_urgent))
            {
                Log::Message("off->urgent");
                return RequireUrgent;
            }
            else if (pv < (sv + hyst_norm))
            {
                return RequireNormal;
            }
            else if (pv < (sv + hyst_accept))
            {
                return Accept;
            }
        }
        else
        {
            if (pv > (sv + hyst_off))
            {
                return Off;
            }
            else if (last == Accept)
            {
                if (pv < (sv + hyst_urgent))
                {
                    return RequireUrgent;
                }
                else if (pv < (sv + hyst_norm))
                {
                    return RequireNormal;
                }
            }
            else if (last == RequireNormal)
            {
                if (pv < (sv + hyst_urgent))
                {
                    return RequireUrgent;
                }
            }
        }
    }
    Log::Message("Zone: no command change for PV=" + std::to_string(pv) + ", SV=" + std::to_string(sv));
    return last;
}
