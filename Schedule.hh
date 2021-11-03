#ifndef _SCHEDULE_HH
#define _SCHEDULE_HH

#include <map>
#include <chrono>
#include <memory>
#include <string>

namespace mzloop
{

    struct week_time
    {
        int day;
        int seconds;
    };

    enum SchedInterpolate
    {
        SchedDefault,
        SchedStep,
        SchedFullRamp,
        SchedEndRamp,
        SchedCosine
    };

    struct schedule_point
    {
        struct week_time wt;
        double sv;
        SchedInterpolate interp;
    };

    class Schedule
    {
    public:
        Schedule();
        ~Schedule();

        void SetDefaultInterpolate(SchedInterpolate new_interpolate);
        void AddPoint(week_time wt, double sv, SchedInterpolate interpolate = SchedDefault);
        void RemovePoint(week_time wt);
        void Clear();
        void LoadSchedule(std::string file);

        void SetOverride(double override_sv);
        void ClearOverride() {current_override.reset();}
        const std::optional<schedule_point>& GetOverride() const {return current_override;}

        const std::optional<schedule_point>& GetPointExact(week_time wt) const;
        const std::optional<schedule_point>& GetPointPrev(week_time wt) const;
        const std::optional<schedule_point>& GetPointNext(week_time wt) const;
        double GetSv(week_time wt) const;

    protected:
        std::map<week_time, schedule_point> schedule_points;
        std::optional<schedule_point> current_override;
        SchedInterpolate default_interpolate;
        double ramp_rate;
    };

};

#endif
