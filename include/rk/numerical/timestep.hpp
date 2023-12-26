#pragma once

namespace rk
{
template <typename T> struct timestep
{
    timestep() = default;
    timestep(T value);
    timestep(T value, T min, T max);

    T value;
    T min;
    T max;

    bool limited;

    bool off_bounds() const;
    bool too_small() const;
    bool too_big() const;

    void clamp();
};

} // namespace rk