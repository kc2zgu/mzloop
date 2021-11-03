#ifndef _ZONE_HH
#define _ZONE_HH

#include <string>
#include <vector>
#include <optional>

namespace mzloop
{
    enum Command
    {
        None,
        Off,
        Accept,
        RequireNormal,
        RequireUrgent
    };

    inline std::string CommandString(const Command c)
    {
        switch (c)
        {
        case None: return "None";
        case Off: return "Off";
        case Accept: return "Accept";
        case RequireNormal: return "RequireNormal";
        case RequireUrgent: return "RequireUrgent";
        default: return "Unknown";
        }
    }

    class Sensor;
    
    class Zone
    {
    public:
        Zone(const std::string name);
        virtual ~Zone();

        const std::string& GetName() const { return name; };
        std::optional<double> GetPresentValue() const
            {
                if (pv_valid)
                    return present_value;
                else
                    return std::nullopt;
            };
        std::optional<double> GetSetValue() const
            {
                if (has_sv)
                    return set_value;
                else
                    return std::nullopt;
            };

        void SetValue(double sv);
        void SetHysteresis(double off, double accept, double norm, double urgent);
        double GetHysteresisOff() const { return hyst_off; }
        double GetHysteresisAccept() const { return hyst_accept; }
        double GetHysteresisNorm() const { return hyst_norm; }
        double GetHysteresisUrgent() const { return hyst_urgent; }
        void AssignInput(Sensor *newinput);
        Command GetCommandForSetpoint(Command last, double pv, double sv);

        virtual Command GetOutput() = 0;

        virtual void ReadPresentValue();

    protected:
        std::string name;
        double set_value, present_value;
        double hyst_off, hyst_accept, hyst_norm, hyst_urgent;
        bool pv_valid, has_sv;

        Sensor *input;
        Command last_command;
    };

    class LeafZone : public Zone
    {
    public:
        LeafZone(const std::string name);
        Command GetOutput() override;
    };

    class CompositeZone : public Zone
    {
    public:
        CompositeZone(const std::string name);
        Command GetOutput() override;
        void ReadPresentValue() override;

        void AddMemberZone(Zone *zone);

    protected:
        std::vector<Zone*> member_zones;
    };
};

#endif
