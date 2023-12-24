#include "rk/pch.hpp"
#include "rk/integrator.hpp"
#include <cmath>
#define SAFETY_FACTOR 0.85f

namespace rk
{
template <typename T>
integrator<T>::integrator(const timestep<T> &ts, const butcher_tableau<T> &bt, const std::vector<T> &vars,
                          const T tolerance)
    : state(vars, bt.stages), ts(ts), tolerance(tolerance), m_tableau(bt)
{
}

template <typename T>
std::vector<T> integrator<T>::generate_solution(const T timestep, const std::vector<T> &vars,
                                                const std::vector<T> &coefs)
{
    KIT_PERF_FUNCTION()
    std::vector<T> sol;
    sol.reserve(vars.size());
    for (std::size_t j = 0; j < vars.size(); j++)
    {
        T sum = 0.0;
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

template <typename T> T integrator<T>::embedded_error(const std::vector<T> &sol1, const std::vector<T> &sol2)
{
    T result = 0.0;
    for (std::size_t i = 0; i < sol1.size(); i++)
        result += (sol1[i] - sol2[i]) * (sol1[i] - sol2[i]);
    return result;
}

template <typename T> T integrator<T>::reiterative_error(const std::vector<T> &sol1, const std::vector<T> &sol2) const
{
    const std::uint32_t coeff = ipow(2, m_tableau.order) - 1;
    return embedded_error(sol1, sol2) / coeff;
}

template <typename T> T integrator<T>::timestep_factor() const
{
    return SAFETY_FACTOR * std::pow(tolerance / m_error, 1.f / m_tableau.order);
}

template <typename T> T integrator<T>::error() const
{
    return m_error;
}
template <typename T> bool integrator<T>::valid() const
{
    return m_valid;
}

template <typename T> const butcher_tableau<T> &integrator<T>::tableau() const
{
    return m_tableau;
}

template <typename T> void integrator<T>::tableau(const butcher_tableau<T> &tableau)
{
    m_tableau = tableau;
    state.set_stages(tableau.stages);
}

#ifdef KIT_USE_YAML_CPP
template <typename T> YAML::Node integrator<T>::serializer::encode(const integrator &integ) const
{
    YAML::Node node;
    node["Tableau"] = integ.tableau();
    node["State"] = integ.state;
    node["Tolerance"] = integ.tolerance;
    node["Elapsed"] = integ.elapsed;
    node["Timestep"] = integ.ts.value;
    node["Min timestep"] = integ.ts.min;
    node["Max timestep"] = integ.ts.max;
    return node;
}
template <typename T> bool integrator<T>::serializer::decode(const YAML::Node &node, integrator &integ) const
{
    if (!node.IsMap() || node.size() != 8)
        return false;

    integ.tableau(node["Tableau"].as<rk::butcher_tableau<T>>());
    integ.state = node["State"].as<rk::state<T>>();
    integ.tolerance = node["Tolerance"].as<T>();
    integ.elapsed = node["Elapsed"].as<T>();
    integ.ts.value = node["Timestep"].as<T>();
    integ.ts.min = node["Min timestep"].as<T>();
    integ.ts.max = node["Max timestep"].as<T>();
    return true;
}
#endif

template class integrator<float>;
template class integrator<double>;
template class integrator<long double>;

} // namespace rk
