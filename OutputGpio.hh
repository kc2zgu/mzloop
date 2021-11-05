#ifndef _OUTPUTGPIO_HH
#define _OUTPUTGPIO_HH

#include "Output.hh"
#include <gpiod.hpp>

namespace mzloop
{

    class OutputGpio :public Output
    {
    public:
        OutputGpio(std::string name, Zone *zone, std::string chip, int line);
        ~OutputGpio();

        bool SetActive() override;
        bool SetInactive() override;

    protected:
        std::string chip;
        int line;

        gpiod::chip gpio_chip;
        gpiod::line gpio_line;
    };
};

#endif
