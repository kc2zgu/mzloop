#include "OutputGpio.hh"
#include "Log.hh"

using namespace mzloop;
using namespace std;

OutputGpio::OutputGpio(string name, Zone *zone, string chip, int line)
    :Output{name, zone},
     chip{chip},
     gpio_chip{chip},
     line{line},
     gpio_line{gpio_chip.get_line(line)}
{
    Log::Message("GPIO: Opened " + chip  + ":" + to_string(line));
    gpiod::line_request req;
    req.consumer = "mzloop:" + name;
    req.request_type = gpiod::line_request::DIRECTION_OUTPUT;
    req.flags = 0;
    gpio_line.request(req);
    SetInactive();
}

OutputGpio::~OutputGpio()
{
    SetInactive();
}

bool OutputGpio::SetActive()
{
    gpio_line.set_value(1);
    return true;
}

bool OutputGpio::SetInactive()
{
    gpio_line.set_value(0);
    return true;
}
