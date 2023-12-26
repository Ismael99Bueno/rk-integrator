#include "rk/internal/pch.hpp"
#include "rk/numerical/timestep.hpp"
#include <cmath>

namespace rk
{
template <typename Float> timestep<Float>::timestep(const Float value) : value(value), limited(false)
{
}
template <typename Float>
timestep<Float>::timestep(const Float value, const Float min, const Float max)
    : value(value), min(min), max(max), limited(true)
{
}
template <typename Float> bool timestep<Float>::off_bounds() const
{
    return limited && (too_small() || too_big());
}
template <typename Float> bool timestep<Float>::too_small() const
{
    return limited && value < min;
}
template <typename Float> bool timestep<Float>::too_big() const
{
    return limited && value > max;
}

template <typename Float> void timestep<Float>::clamp()
{
    value = std::clamp(value, min, max);
}

template struct timestep<float>;
template struct timestep<double>;
template struct timestep<long double>;
} // namespace rk