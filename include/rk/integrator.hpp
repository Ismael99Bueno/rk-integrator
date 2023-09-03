#ifndef RK_INTEGRATOR_HPP
#define RK_INTEGRATOR_HPP

#include "rk/butcher_tableau.hpp"
#include "rk/state.hpp"
#include "kit/profile/perf.hpp"
#include <cstdint>
#include <cmath>

#define TOL_PART 256.f

namespace rk
{
class integrator final
{
  public:
#ifdef KIT_USE_YAML_CPP
    class serializer : public kit::serializer<integrator>
    {
      public:
        YAML::Node encode(const integrator &tb) const override;
        bool decode(const YAML::Node &node, integrator &tb) const override;
    };
#endif

    integrator(const butcher_tableau &tb = butcher_tableau::rk4, const std::vector<float> &vars = {},
               float tolerance = 1e-4f, float min_dt = 1e-6f, float max_dt = 1.f);

    rk::state state;
    float tolerance, min_dt, max_dt;
    bool reversed = false, limited_timestep = true;

    template <typename ODE> bool raw_forward(float &t, float dt, ODE &ode)
    {
        KIT_PERF_SCOPE("-Physics-")
        KIT_ASSERT_ERROR(!dt_off_bounds(dt),
                         "Timestep is not between established limits. Change the timestep or adjust the limits to "
                         "include the current value - current: {0}, min: {1}, max: {2}",
                         dt, min_dt, max_dt)
        m_valid = true;
        const float sdt = reversed ? -dt : dt;

        std::vector<float> &vars = state.m_vars;
        update_kvec(t, sdt, vars, ode);
        if (m_tableau.embedded())
        {
            const std::vector<float> aux_state = generate_solution(sdt, vars, m_tableau.coefs2());
            vars = generate_solution(sdt, vars, m_tableau.coefs1());
            m_error = embedded_error(vars, aux_state);
        }
        else
            vars = generate_solution(sdt, vars, m_tableau.coefs());
        t += sdt;
        KIT_ASSERT_WARN(m_valid, "NaN encountered when computing runge-kutta solution.")
        return m_valid;
    }

    template <typename ODE> bool reiterative_forward(float &t, float &dt, ODE &ode, std::uint8_t reiterations = 2)
    {
        KIT_PERF_SCOPE("-Physics-")
        KIT_ASSERT_CRITICAL(reiterations >= 2,
                            "The amount of reiterations has to be greater than 1, otherwise the algorithm will break.")
        KIT_ASSERT_ERROR(!dt_off_bounds(dt),
                         "Timestep is not between established limits. Change the timestep or adjust the limits to "
                         "include the current value - current: {0}, min: {1}, max: {2}",
                         dt, min_dt, max_dt)
        KIT_ASSERT_WARN(
            !m_tableau.embedded(),
            "Butcher tableau has an embedded solution. Use an embedded adaptive method for better efficiency.")

        m_valid = true;
        if (m_error > 0.f)
            dt = std::clamp(dt * timestep_factor(), min_dt, max_dt);
        for (;;)
        {
            const float sdt = reversed ? -dt : dt;
            std::vector<float> &vars = state.m_vars;
            std::vector<float> sol1 = vars;
            update_kvec(t, sdt, vars, ode);

            const std::vector<float> sol2 = generate_solution(sdt, vars, m_tableau.coefs());
            for (std::uint8_t i = 0; i < reiterations; i++)
            {
                update_kvec(t, sdt / reiterations, sol1, ode);
                sol1 = generate_solution(sdt / reiterations, sol1, m_tableau.coefs());
            }
            m_error = reiterative_error(sol1, sol2);

            const bool too_small = dt_too_small(dt);
            if (m_error <= tolerance || too_small)
            {
                vars = sol1;
                if (too_small)
                    dt = min_dt;
                break;
            }
            dt *= timestep_factor();
        }
        m_error = std::max(m_error, tolerance / TOL_PART);
        t += reversed ? -dt : dt;
        KIT_ASSERT_WARN(m_valid, "NaN encountered when computing runge-kutta solution.")
        return m_valid;
    }

    template <typename ODE> bool embedded_forward(float &t, float &dt, ODE &ode)
    {
        KIT_PERF_SCOPE("-Physics-")
        KIT_ASSERT_CRITICAL(m_tableau.embedded(),
                            "Cannot perform embedded adaptive stepsize without an embedded solution.")
        KIT_ASSERT_ERROR(!dt_off_bounds(dt),
                         "Timestep is not between established limits. Change the timestep or adjust the limits to "
                         "include the vars value - vars: {0}, min: {1}, max: {2}",
                         dt, min_dt, max_dt)
        m_valid = true;

        if (m_error > 0.f)
            dt = std::clamp(dt * timestep_factor(), min_dt, max_dt);
        for (;;)
        {
            const float sdt = reversed ? -dt : dt;
            std::vector<float> &vars = state.m_vars;
            update_kvec(t, sdt, vars, ode);
            const std::vector<float> sol2 = generate_solution(sdt, vars, m_tableau.coefs2());
            const std::vector<float> sol1 = generate_solution(sdt, vars, m_tableau.coefs1());
            m_error = embedded_error(sol1, sol2);

            const bool too_small = dt_too_small(dt);
            if (m_error <= tolerance || too_small)
            {
                vars = sol1;
                if (too_small)
                    dt = min_dt;
                break;
            }
            dt *= timestep_factor();
        }
        m_error = std::max(m_error, tolerance / TOL_PART);
        t += reversed ? -dt : dt;
        KIT_ASSERT_WARN(m_valid, "NaN encountered when computing runge-kutta solution.")
        return m_valid;
    }

    const butcher_tableau &tableau() const;
    void tableau(const butcher_tableau &tableau);

    float error() const;
    bool valid() const;

  private:
    butcher_tableau m_tableau;
    float m_error = 0.f;
    bool m_valid = true;

    std::vector<float> generate_solution(float dt, const std::vector<float> &vars, const std::vector<float> &coefs);

    bool dt_too_small(float dt) const;
    bool dt_too_big(float dt) const;
    bool dt_off_bounds(float dt) const;
    static float embedded_error(const std::vector<float> &sol1, const std::vector<float> &sol2);
    float reiterative_error(const std::vector<float> &sol1, const std::vector<float> &sol2) const;
    float timestep_factor() const;

    template <typename ODE> void update_kvec(float t, float dt, const std::vector<float> &vars, ODE &ode)
    {
        KIT_PERF_FUNCTION()
        auto &kvec = state.m_kvec;
        KIT_ASSERT_CRITICAL(vars.size() == kvec[0].size(),
                            "State and k-vectors size mismatch! - vars size: {0}, k-vectors size: {1}", vars.size(),
                            kvec[0].size())
        std::vector<float> aux_vars(vars.size());

        kvec[0] = ode(t, dt, vars);
        for (std::uint8_t i = 1; i < m_tableau.stage(); i++)
        {
            for (std::size_t j = 0; j < vars.size(); j++)
            {
                float k_sum = 0.f;
                for (std::uint8_t k = 0; k < i; k++)
                    k_sum += m_tableau.beta()[i - 1][k] * kvec[k][j];
                aux_vars[j] = vars[j] + k_sum * dt;
            }
            kvec[i] = ode(t + m_tableau.alpha()[i - 1] * dt, dt, aux_vars);
        }
    }
};

} // namespace rk

#endif