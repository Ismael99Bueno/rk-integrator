#pragma once

namespace rk
{
template <typename Float> struct timestep
{
    timestep() = default;
    timestep(Float value);
    timestep(Float value, Float min, Float max);

    Float value;
    Float min;
    Float max;

    bool limited;

    bool off_bounds() const;
    bool too_small() const;
    bool too_big() const;

    void clamp();
};

} // namespace rk