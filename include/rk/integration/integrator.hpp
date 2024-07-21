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
    using array1 = typename butcher_tableau<Float>::array1;
    using array2 = typename butcher_tableau<Float>::array2;

    static inline constexpr Float TOL_PART = 256.f;

    integrator(const butcher_tableau<Float> &bt, const timestep<Float> &ts = {1.e-3f},
               const std::vector<Float> &vars = {}, Float tolerance = 1e-4f);

    rk::state<Float> state;
    timestep<Float> ts;

    Float tolerance;
    Float elapsed = 0.f;

    template <kit::RetCallable<std::vector<Float>, Float, Float, const std::vector<Float> &> ODE>
    bool raw_forward(ODE &&ode)
    {
        m_valid = true;

        if (ts.limited)
            ts.clamp();

        std::vector<Float> &vars = state.m_vars;
        update_kvec(elapsed, ts.value, vars, std::forward<ODE>(ode));

        if (m_tableau.embedded)
        {
            static std::vector<Float> aux_state;
            aux_state = generate_solution(ts.value, vars, m_tableau.coefs2);
            vars = generate_solution(ts.value, vars, m_tableau.coefs1);
            m_error = embedded_error(vars, aux_state);
        }
        else
            vars = generate_solution(ts.value, vars, m_tableau.coefs1);
        elapsed += ts.value;
        KIT_ASSERT_WARN(m_valid, "NaN encountered when computing runge-kutta solution.")
        return m_valid;
    }

    template <kit::RetCallable<std::vector<Float>, Float, Float, const std::vector<Float> &> ODE>
    bool reiterative_forward(ODE &&ode, std::uint32_t reiterations = 2)
    {
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
            static std::vector<Float> sol1;
            static std::vector<Float> sol2;

            std::vector<Float> &vars = state.m_vars;
            sol1 = vars;
            update_kvec(elapsed, ts.value, vars, std::forward<ODE>(ode));

            sol2 = generate_solution(ts.value, vars, m_tableau.coefs1);
            for (std::uint32_t i = 0; i < reiterations; i++)
            {
                update_kvec(elapsed, ts.value / reiterations, sol1, std::forward<ODE>(ode));
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

    template <kit::RetCallable<std::vector<Float>, Float, Float, const std::vector<Float> &> ODE>
    bool embedded_forward(ODE &&ode)
    {
        KIT_ASSERT_CRITICAL(m_tableau.embedded(),
                            "Cannot perform embedded adaptive stepsize without an embedded solution.")
        m_valid = true;

        if (m_error > 0.f)
            ts.value *= timestep_factor();
        if (ts.limited)
            ts.clamp();

        for (;;)
        {
            static std::vector<Float> sol1;
            static std::vector<Float> sol2;

            std::vector<Float> &vars = state.m_vars;
            update_kvec(elapsed, ts.value, vars, std::forward<ODE>(ode));
            sol2 = generate_solution(ts.value, vars, m_tableau.coefs2);
            sol1 = generate_solution(ts.value, vars, m_tableau.coefs1);
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

    template <kit::RetCallable<std::vector<Float>, Float, Float, const std::vector<Float> &> ODE>
    void update_kvec(Float time, Float timestep, const std::vector<Float> &vars, ODE &&ode)
    {
        KIT_ASSERT_ERROR(timestep >= 0.f, "Timestep must be non-negative")
        KIT_ASSERT_ERROR(vars.size() * m_tableau.stages == state.m_kvec.size(),
                         "State and k-vectors size mismatch! - vars size: {0}, k-vectors size: {1}", vars.size(),
                         state.m_kvec.size() / m_tableau.stages)

        static std::vector<Float> aux_vars;
        aux_vars.resize(vars.size());

        auto state_derivative = std::forward<ODE>(ode)(time, timestep, vars);
        KIT_ASSERT_ERROR(state_derivative.size() == vars.size(),
                         "ODE function must return a vector of the same size as the state vector")
        for (std::size_t i = 0; i < vars.size(); i++)
            state(0, i) = state_derivative[i];

        for (std::uint32_t i = 1; i < m_tableau.stages; i++)
        {
            for (std::size_t j = 0; j < vars.size(); j++)
            {
                Float k_sum = 0.f;
                for (std::uint32_t k = 0; k < i; k++)
                    k_sum += m_tableau.beta[i - 1][k] * state(k, j);
                aux_vars[j] = vars[j] + k_sum * timestep;
            }
            state_derivative = std::forward<ODE>(ode)(time + m_tableau.alpha[i - 1] * timestep, timestep, aux_vars);
            KIT_ASSERT_ERROR(state_derivative.size() == vars.size(),
                             "ODE function must return a vector of the same size as the state vector")
            for (std::size_t j = 0; j < vars.size(); j++)
                state(i, j) = state_derivative[j];
        }
    }

    std::vector<Float> generate_solution(Float timestep, const std::vector<Float> &vars, const array1 &coefs);

    static Float embedded_error(const std::vector<Float> &sol1, const std::vector<Float> &sol2);
    Float reiterative_error(const std::vector<Float> &sol1, const std::vector<Float> &sol2) const;
    Float timestep_factor() const;
};

} // namespace rk
