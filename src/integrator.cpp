#include "rk/pch.hpp"
#include "rk/integrator.hpp"
#include <cmath>
#define SAFETY_FACTOR 0.85f

namespace rk
{
integrator::integrator(const butcher_tableau &tb, const std::vector<float> &vars, const float tolerance,
                       const float min_dt, const float max_dt)
    : m_tableau(tb), m_state(vars, tb.stage()), m_tolerance(tolerance), m_min_dt(min_dt), m_max_dt(max_dt),
      m_error(0.f), m_valid(true)
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
            sum += coefs[i] * m_state.m_kvec[i][j];
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
    return m_limited_timestep && dt < m_min_dt;
}
bool integrator::dt_too_big(const float dt) const
{
    return m_limited_timestep && dt > m_max_dt;
}
bool integrator::dt_off_bounds(const float dt) const
{
    return m_limited_timestep && (dt_too_small(dt) || dt_too_big(dt));
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
    return SAFETY_FACTOR * std::pow(m_tolerance / m_error, 1.f / m_tableau.order());
}

const butcher_tableau &integrator::tableau() const
{
    return m_tableau;
}

const rk::state &integrator::state() const
{
    return m_state;
}
rk::state &integrator::state()
{
    return m_state;
}

void integrator::tableau(const butcher_tableau &tableau)
{
    m_tableau = tableau;
    m_state.resize_kvec(tableau.stage());
}

float integrator::tolerance() const
{
    return m_tolerance;
}
float integrator::min_dt() const
{
    return m_min_dt;
}
float integrator::max_dt() const
{
    return m_max_dt;
}
float integrator::error() const
{
    return m_error;
}

void integrator::tolerance(const float val)
{
    m_tolerance = val;
}
void integrator::min_dt(const float val)
{
    m_min_dt = val;
}
void integrator::max_dt(const float val)
{
    m_max_dt = val;
}

bool integrator::reversed() const
{
    return m_reversed;
}
void integrator::reversed(bool reversed)
{
    m_reversed = reversed;
}

bool integrator::limited_timestep() const
{
    return m_limited_timestep;
}
void integrator::limited_timestep(bool limited_timestep)
{
    m_limited_timestep = limited_timestep;
}

#ifdef KIT_USE_YAML_CPP
YAML::Emitter &operator<<(YAML::Emitter &out, const integrator &integ)
{
    out << YAML::BeginMap;
    out << YAML::Key << "Tableau" << YAML::Value << integ.tableau();
    out << YAML::Key << "State" << YAML::Value << integ.state();
    out << YAML::Key << "Tolerance" << YAML::Value << integ.tolerance();
    out << YAML::Key << "Min timestep" << YAML::Value << integ.min_dt();
    out << YAML::Key << "Max timestep" << YAML::Value << integ.max_dt();
    out << YAML::Key << "Reversed" << YAML::Value << integ.reversed();
    out << YAML::Key << "Limited timestep" << YAML::Value << integ.limited_timestep();
    out << YAML::EndMap;
    return out;
}
#endif
} // namespace rk

#ifdef KIT_USE_YAML_CPP
namespace YAML
{
Node convert<rk::integrator>::encode(const rk::integrator &integ)
{
    Node node;
    node["Tableau"] = integ.tableau();
    node["State"] = integ.state();
    node["Tolerance"] = integ.tolerance();
    node["Min timestep"] = integ.min_dt();
    node["Max timestep"] = integ.max_dt();
    node["Reversed"] = integ.reversed();
    node["Limited timestep"] = integ.limited_timestep();
    return node;
}
bool convert<rk::integrator>::decode(const Node &node, rk::integrator &integ)
{
    if (!node.IsMap() || node.size() != 7)
        return false;

    integ.tableau(node["Tableau"].as<rk::butcher_tableau>());
    integ.m_state = node["State"].as<rk::state>();
    integ.tolerance(node["Tolerance"].as<float>());
    integ.min_dt(node["Min timestep"].as<float>());
    integ.max_dt(node["Max timestep"].as<float>());
    integ.reversed(node["Reversed"].as<bool>());
    integ.limited_timestep(node["Limited timestep"].as<bool>());
    return true;
};
} // namespace YAML
#endif