#include "rk/internal/pch.hpp"
#include "rk/numerical/timestep.hpp"
#include <cmath>

namespace rk
{
template <std::floating_point Float> timestep<Float>::timestep(const Float value) : value(value), limited(false)
{
}
template <std::floating_point Float>
timestep<Float>::timestep(const Float value, const Float min, const Float max)
    : value(value), min(min), max(max), limited(true)
{
}
template <std::floating_point Float> bool timestep<Float>::off_bounds() const
{
    return limited && (too_small() || too_big());
}
template <std::floating_point Float> bool timestep<Float>::too_small() const
{
    return limited && value < min;
}
template <std::floating_point Float> bool timestep<Float>::too_big() const
{
    return limited && value > max;
}

template <std::floating_point Float> void timestep<Float>::clamp()
{
    value = std::clamp(value, min, max);
}

template struct timestep<float>;
template struct timestep<double>;
template struct timestep<long double>;
} // namespace rk