#ifndef _SCHEDULE_HH
#define _SCHEDULE_HH

#include <map>
#include <chrono>
#include <string>
#include <optional>

namespace mzloop
{

    struct week_time
    {
        week_time(int day, int seconds)
            :day{day}, seconds{seconds} {}
        week_time(const std::chrono::system_clock::time_point &tp)
            {
                auto tt = std::chrono::system_clock::to_time_t(tp);
                auto lt = localtime(&tt);
                int s = lt->tm_hour * 3600 + lt->tm_min * 60 + lt->tm_sec;

                day = lt->tm_wday;
                seconds = s;
            }

        int day;
        int seconds;

        bool operator<(const week_time &other) const
        {
            return day < other.day || (day == other.day && seconds < other.seconds);
        }

        int raw_seconds() const
        {
            return day*86400 + seconds;
        }
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
        schedule_point(week_time wt, double sv, SchedInterpolate interp = SchedDefault)
            :wt{wt}, sv{sv}, interp{interp} {}
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
        void AddPoint(const week_time &wt, double sv, SchedInterpolate interpolate = SchedDefault);
        void RemovePoint(const week_time &wt);
        void Clear();
        void LoadSchedule(std::string file);
        void SetRamRate(double newrate) {ramp_rate = newrate;}

        void SetOverride(double override_sv);
        void SetOverrideHold(bool hold);
        void ClearOverride() {current_override.reset(); override_hold = false;}
        const schedule_point *GetOverride() const {return current_override ? &*current_override : nullptr;}
        const bool IsOverrideHold() const {return override_hold;}

        const schedule_point *GetPointExact(const week_time &wt) const;
        const schedule_point *GetPointPrev(const week_time &wt) const;
        const schedule_point *GetPointNext(const week_time &wt) const;
        double GetSv(const week_time &wt);
        double GetSv(const std::chrono::system_clock::time_point &tp);

    protected:
        std::map<week_time, schedule_point> schedule_points;
        std::optional<schedule_point> current_override;
        bool override_hold;
        SchedInterpolate default_interpolate;
        double ramp_rate;
    };

};

#endif
