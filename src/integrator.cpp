#include "rk/pch.hpp"
#include "rk/integrator.hpp"
#include <cmath>
#define SAFETY_FACTOR 0.85f

namespace rk
{
integrator::integrator(const butcher_tableau &tb, const std::vector<float> &vars, const float tolerance,
                       const float min_dt, const float max_dt)
    : state(vars, tb.stage()), tolerance(tolerance), min_dt(min_dt), max_dt(max_dt), m_tableau(tb)
{
}

std::vector<float> integrator::generate_solution(const float dt, const std::vector<float> &vars,
                                                 const std::vector<float> &coefs)
{
    KIT_PERF_FUNCTION()
    std::vector<float> sol;
    sol.reserve(vars.size());
    for (std::size_t j = 0; j < vars.size(); j++)
    {
        float sum = 0.f;
        for (std::uint8_t i = 0; i < m_tableau.stage(); i++)
            sum += coefs[i] * state.m_kvec[i][j];
        m_valid &= !std::isnan(sum);

        sol.push_back(vars[j] + sum * dt);
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

bool integrator::dt_too_small(const float dt) const
{
    return limited_timestep && dt < min_dt;
}
bool integrator::dt_too_big(const float dt) const
{
    return limited_timestep && dt > max_dt;
}
bool integrator::dt_off_bounds(const float dt) const
{
    return limited_timestep && (dt_too_small(dt) || dt_too_big(dt));
}

float integrator::embedded_error(const std::vector<float> &sol1, const std::vector<float> &sol2)
{
    float result = 0.f;
    for (std::size_t i = 0; i < sol1.size(); i++)
        result += (sol1[i] - sol2[i]) * (sol1[i] - sol2[i]);
    return result;
}

float integrator::reiterative_error(const std::vector<float> &sol1, const std::vector<float> &sol2) const
{
    const std::uint32_t coeff = ipow(2, m_tableau.order()) - 1;
    return embedded_error(sol1, sol2) / coeff;
}

float integrator::timestep_factor() const
{
    return SAFETY_FACTOR * std::pow(tolerance / m_error, 1.f / m_tableau.order());
}

float integrator::error() const
{
    return m_error;
}
bool integrator::valid() const
{
    return m_valid;
}

const butcher_tableau &integrator::tableau() const
{
    return m_tableau;
}

void integrator::tableau(const butcher_tableau &tableau)
{
    m_tableau = tableau;
    state.resize_kvec(tableau.stage());
}

#ifdef KIT_USE_YAML_CPP
YAML::Node integrator::serializer::encode(const integrator &integ) const
{
    YAML::Node node;
    node["Tableau"] = integ.tableau();
    node["State"] = integ.state;
    node["Tolerance"] = integ.tolerance;
    node["Min timestep"] = integ.min_dt;
    node["Max timestep"] = integ.max_dt;
    node["Reversed"] = integ.reversed;
    node["Limited timestep"] = integ.limited_timestep;
    return node;
}
bool integrator::serializer::decode(const YAML::Node &node, integrator &integ) const
{
    if (!node.IsMap() || node.size() != 7)
        return false;

    integ.tableau(node["Tableau"].as<rk::butcher_tableau>());
    integ.state = node["State"].as<rk::state>();
    integ.tolerance = node["Tolerance"].as<float>();
    integ.min_dt = node["Min timestep"].as<float>();
    integ.max_dt = node["Max timestep"].as<float>();
    integ.reversed = node["Reversed"].as<bool>();
    integ.limited_timestep = node["Limited timestep"].as<bool>();
    return true;
}
#endif

} // namespace rk
