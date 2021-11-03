#include "Schedule.hh"

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

