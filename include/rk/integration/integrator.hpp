#pragma once

#include "rk/numerical/butcher_tableau.hpp"
#include "rk/integration/state.hpp"
#include "rk/numerical/timestep.hpp"

#include "kit/debug/log.hpp"
#include "kit/utility/type_constraints.hpp"
#include <cstdint>

namespace rk
{
template <std::floating_point Float> class integrator final
{
  public:
    static inline constexpr Float TOL_PART = 256.f;

    integrator(const timestep<Float> &ts = {1.e-3f}, const butcher_tableau<Float> &bt = butcher_tableau<Float>::rk4,
               const std::vector<Float> &vars = {}, Float tolerance = 1e-4f);

    rk::state<Float> state;
    timestep<Float> ts;

    Float tolerance;
    Float elapsed = 0.f;

    template <kit::RetCallable<std::vector<float>, float, float, const std::vector<float> &> ODE>
    bool raw_forward(ODE &ode)
    {
        KIT_PERF_FUNCTION()
        m_valid = true;

        if (ts.limited)
            ts.clamp();

        std::vector<Float> &vars = state.m_vars;
        update_kvec(elapsed, ts.value, vars, ode);

        if (m_tableau.embedded)
        {
            const std::vector<Float> aux_state = generate_solution(ts.value, vars, m_tableau.coefs2);
            vars = generate_solution(ts.value, vars, m_tableau.coefs1);
            m_error = embedded_error(vars, aux_state);
        }
        else
            vars = generate_solution(ts.value, vars, m_tableau.coefs1);
        elapsed += ts.value;
        KIT_ASSERT_WARN(m_valid, "NaN encountered when computing runge-kutta solution.")
        return m_valid;
    }

    template <kit::RetCallable<std::vector<float>, float, float, const std::vector<float> &> ODE>
    bool reiterative_forward(ODE &ode, std::uint32_t reiterations = 2)
    {
        KIT_PERF_FUNCTION()
        KIT_ASSERT_CRITICAL(reiterations >= 2,
                            "The amount of reiterations has to be greater than 1, otherwise the algorithm will break.")
        KIT_ASSERT_WARN(
            !m_tableau.embedded(),
            "Butcher tableau has an embedded solution. Use an embedded adaptive method for better efficiency.")

        m_valid = true;

        if (m_error > 0.f)
            ts.value *= timestep_factor();
        if (ts.limited)
            ts.clamp();

        for (;;)
        {
            std::vector<Float> &vars = state.m_vars;
            std::vector<Float> sol1 = vars;
            update_kvec(elapsed, ts.value, vars, ode);

            const std::vector<Float> sol2 = generate_solution(ts.value, vars, m_tableau.coefs1);
            for (std::uint32_t i = 0; i < reiterations; i++)
            {
                update_kvec(elapsed, ts.value / reiterations, sol1, ode);
                sol1 = generate_solution(ts.value / reiterations, sol1, m_tableau.coefs1);
            }
            m_error = reiterative_error(sol1, sol2);

            const bool too_small = ts.too_small();
            if (m_error <= tolerance || too_small)
            {
                vars = sol1;
                if (too_small)
                    ts.value = ts.min;
                break;
            }
            ts.value *= timestep_factor();
        }
        m_error = std::max(m_error, tolerance / TOL_PART);
        elapsed += ts.value;

        KIT_ASSERT_WARN(m_valid, "NaN encountered when computing runge-kutta solution.")
        return m_valid;
    }

    template <kit::RetCallable<std::vector<float>, float, float, const std::vector<float> &> ODE>
    bool embedded_forward(ODE &ode)
    {
        KIT_PERF_FUNCTION()
        KIT_ASSERT_CRITICAL(m_tableau.embedded(),
                            "Cannot perform embedded adaptive stepsize without an embedded solution.")
        m_valid = true;

        if (m_error > 0.f)
            ts.value *= timestep_factor();
        if (ts.limited)
            ts.clamp();

        for (;;)
        {
            std::vector<Float> &vars = state.m_vars;
            update_kvec(elapsed, ts.value, vars, ode);
            const std::vector<Float> sol2 = generate_solution(ts.value, vars, m_tableau.coefs2);
            const std::vector<Float> sol1 = generate_solution(ts.value, vars, m_tableau.coefs1);
            m_error = embedded_error(sol1, sol2);

            const bool too_small = ts.too_small();
            if (m_error <= tolerance || too_small)
            {
                vars = sol1;
                if (too_small)
                    ts.value = ts.min;
                break;
            }
            ts.value *= timestep_factor();
        }
        m_error = std::max(m_error, tolerance / TOL_PART);
        elapsed += ts.value;

        KIT_ASSERT_WARN(m_valid, "NaN encountered when computing runge-kutta solution.")
        return m_valid;
    }

    const butcher_tableau<Float> &tableau() const;
    void tableau(const butcher_tableau<Float> &tableau);

    Float error() const;
    bool valid() const;

  private:
    butcher_tableau<Float> m_tableau;
    Float m_error = 0.f;
    bool m_valid = true;

    template <kit::RetCallable<std::vector<float>, float, float, const std::vector<float> &> ODE>
    void update_kvec(Float time, Float timestep, const std::vector<Float> &vars, ODE &ode)
    {
        KIT_ASSERT_ERROR(timestep >= 0.f, "Timestep must be non-negative")
        KIT_PERF_FUNCTION()
        auto &kvec = state.m_kvec;
        KIT_ASSERT_CRITICAL(vars.size() == kvec[0].size(),
                            "State and k-vectors size mismatch! - vars size: {0}, k-vectors size: {1}", vars.size(),
                            kvec[0].size())
        std::vector<Float> aux_vars(vars.size());

        kvec[0] = ode(time, timestep, vars);
        for (std::uint32_t i = 1; i < m_tableau.stages; i++)
        {
            for (std::size_t j = 0; j < vars.size(); j++)
            {
                Float k_sum = 0.f;
                for (std::uint32_t k = 0; k < i; k++)
                    k_sum += m_tableau.beta[i - 1][k] * kvec[k][j];
                aux_vars[j] = vars[j] + k_sum * timestep;
            }
            kvec[i] = ode(time + m_tableau.alpha[i - 1] * timestep, timestep, aux_vars);
        }
    }

    std::vector<Float> generate_solution(Float timestep, const std::vector<Float> &vars,
                                         const std::vector<Float> &coefs);

    static Float embedded_error(const std::vector<Float> &sol1, const std::vector<Float> &sol2);
    Float reiterative_error(const std::vector<Float> &sol1, const std::vector<Float> &sol2) const;
    Float timestep_factor() const;
};

} // namespace rk
