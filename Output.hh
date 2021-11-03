#ifndef _OUTPUT_HH
#define _OUTPUT_HH

#include <string>
#include "Zone.hh"

namespace mzloop
{

    class Output
    {
    public:
        Output(std::string name, Zone *zone): name{name}, zone{zone}, commanded{false}, active{false} {}

        virtual ~Output() {}

        const std::string &GetName() const { return name; }
        Zone *GetZone() { return zone; }
        const Zone *GetZone() const { return zone; }

        bool SetOn()
            {
                if (!commanded)
                {
                    commanded = true;
                    active = SetActive();
                }
                return (active == commanded);
            }
        bool SetOff()
            {
                if (commanded)
                {
                    commanded = false;
                    active = SetInactive();
                }
                return (active == commanded);
            }

        virtual bool SetActive() = 0;
        virtual bool SetInactive() = 0;

        bool CommandedState() const;
        bool ActiveState() const;

    protected:
        std::string name;
        bool commanded, active;
        Zone *zone;
    };

};

#endif
