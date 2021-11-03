#include "Zone.hh"
#include "Log.hh"

using namespace mzloop;
using namespace std;

LeafZone::LeafZone(const string name)
    :Zone{name}
{

}

Command LeafZone::GetOutput()
{
    ReadPresentValue();

    if (pv_valid)
    {
        if (has_sv)
            last_command = GetCommandForSetpoint(last_command, present_value, set_value);
        return last_command;
    }
    else
    {
        Log::Message("LeafZone: no input value");
        return last_command;
    }
}
