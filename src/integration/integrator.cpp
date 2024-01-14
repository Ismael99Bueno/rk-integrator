#include "rk/internal/pch.hpp"
#include "rk/integration/integrator.hpp"
#include <cmath>
#define SAFETY_FACTOR 0.85f

namespace rk
{
template <kit::FloatingPoint Float>
integrator<Float>::integrator(const timestep<Float> &ts, const butcher_tableau<Float> &bt,
                              const std::vector<Float> &vars, const Float tolerance)
    : state(vars, bt.stages), ts(ts), tolerance(tolerance), m_tableau(bt)
{
}

template <kit::FloatingPoint Float>
std::vector<Float> integrator<Float>::generate_solution(const Float timestep, const std::vector<Float> &vars,
                                                        const std::vector<Float> &coefs)
{
    KIT_PERF_FUNCTION()
    std::vector<Float> sol;
    sol.reserve(vars.size());
    for (std::size_t j = 0; j < vars.size(); j++)
    {
        Float sum = 0.0;
        for (std::uint8_t i = 0; i < m_tableau.stages; i++)
            sum += coefs[i] * state.m_kvec[i][j];
        m_valid &= !std::isnan(sum);

        sol.push_back(vars[j] + sum * timestep);
    }
    return sol;
}

static std::uint32_t ipow(std::uint32_t base, std::uint32_t exponent)
{
    int result = 1;
    for (;;)
    {
        if (exponent & 1)
            result *= base;
        exponent >>= 1;
        if (!exponent)
            break;
        base *= base;
    }

    return (std::uint32_t)result;
}

template <kit::FloatingPoint Float>
Float integrator<Float>::embedded_error(const std::vector<Float> &sol1, const std::vector<Float> &sol2)
{
    Float result = 0.0;
    for (std::size_t i = 0; i < sol1.size(); i++)
        result += (sol1[i] - sol2[i]) * (sol1[i] - sol2[i]);
    return result;
}

template <kit::FloatingPoint Float>
Float integrator<Float>::reiterative_error(const std::vector<Float> &sol1, const std::vector<Float> &sol2) const
{
    const std::uint32_t coeff = ipow(2, m_tableau.order) - 1;
    return embedded_error(sol1, sol2) / coeff;
}

template <kit::FloatingPoint Float> Float integrator<Float>::timestep_factor() const
{
    return SAFETY_FACTOR * std::pow(tolerance / m_error, 1.f / m_tableau.order);
}

template <kit::FloatingPoint Float> Float integrator<Float>::error() const
{
    return m_error;
}
template <kit::FloatingPoint Float> bool integrator<Float>::valid() const
{
    return m_valid;
}

template <kit::FloatingPoint Float> const butcher_tableau<Float> &integrator<Float>::tableau() const
{
    return m_tableau;
}

template <kit::FloatingPoint Float> void integrator<Float>::tableau(const butcher_tableau<Float> &tableau)
{
    m_tableau = tableau;
    state.set_stages(tableau.stages);
}

template class integrator<float>;
template class integrator<double>;
template class integrator<long double>;

} // namespace rk
