#include "rk/timestep.hpp"
#include <cmath>

namespace rk
{
template <typename T> timestep<T>::timestep(const T value) : value(value), limited(false)
{
}
template <typename T>
timestep<T>::timestep(const T value, const T min, const T max) : value(value), min(min), max(max), limited(true)
{
}
template <typename T> bool timestep<T>::off_bounds() const
{
    return limited && (too_small() || too_big());
}
template <typename T> bool timestep<T>::too_small() const
{
    return limited && value < min;
}
template <typename T> bool timestep<T>::too_big() const
{
    return limited && value > max;
}

template <typename T> void timestep<T>::clamp()
{
    value = std::clamp(value, min, max);
}

template struct timestep<float>;
template struct timestep<double>;
template struct timestep<long double>;
} // namespace rk