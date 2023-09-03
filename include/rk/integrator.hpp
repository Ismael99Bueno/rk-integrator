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
               float tolerance = 1e-4f, float min_timestep = 1e-6f, float max_timestep = 1.f);

    rk::state state;
    float tolerance, min_timestep, max_timestep;
    bool reversed = false;

    template <typename ODE> bool raw_forward(float &time, float timestep, ODE &ode)
    {
        KIT_PERF_FUNCTION()
        KIT_ASSERT_ERROR(!timestep_off_bounds(timestep),
                         "Timestep is not between established limits. Change the timestep or adjust the limits to "
                         "include the current value - current: {0}, min: {1}, max: {2}",
                         timestep, min_timestep, max_timestep)
        m_valid = true;

        timestep = std::clamp(timestep, min_timestep, max_timestep);
        const float signed_timestep = reversed ? -timestep : timestep;

        std::vector<float> &vars = state.m_vars;
        update_kvec(time, signed_timestep, vars, ode);
        if (m_tableau.embedded())
        {
            const std::vector<float> aux_state = generate_solution(signed_timestep, vars, m_tableau.coefs2());
            vars = generate_solution(signed_timestep, vars, m_tableau.coefs1());
            m_error = embedded_error(vars, aux_state);
        }
        else
            vars = generate_solution(signed_timestep, vars, m_tableau.coefs());
        time += signed_timestep;
        KIT_ASSERT_WARN(m_valid, "NaN encountered when computing runge-kutta solution.")
        return m_valid;
    }

    template <typename ODE>
    bool reiterative_forward(float &time, float &timestep, ODE &ode, std::uint8_t reiterations = 2)
    {
        KIT_PERF_FUNCTION()
        KIT_ASSERT_CRITICAL(reiterations >= 2,
                            "The amount of reiterations has to be greater than 1, otherwise the algorithm will break.")
        KIT_ASSERT_WARN(
            !m_tableau.embedded(),
            "Butcher tableau has an embedded solution. Use an embedded adaptive method for better efficiency.")

        m_valid = true;
        timestep = std::clamp(m_error > 0.f ? (timestep * timestep_factor()) : timestep, min_timestep, max_timestep);
        for (;;)
        {
            const float signed_timestep = reversed ? -timestep : timestep;
            std::vector<float> &vars = state.m_vars;
            std::vector<float> sol1 = vars;
            update_kvec(time, signed_timestep, vars, ode);

            const std::vector<float> sol2 = generate_solution(signed_timestep, vars, m_tableau.coefs());
            for (std::uint8_t i = 0; i < reiterations; i++)
            {
                update_kvec(time, signed_timestep / reiterations, sol1, ode);
                sol1 = generate_solution(signed_timestep / reiterations, sol1, m_tableau.coefs());
            }
            m_error = reiterative_error(sol1, sol2);

            const bool too_small = timestep_too_small(timestep);
            if (m_error <= tolerance || too_small)
            {
                vars = sol1;
                if (too_small)
                    timestep = min_timestep;
                break;
            }
            timestep *= timestep_factor();
        }
        m_error = std::max(m_error, tolerance / TOL_PART);
        time += reversed ? -timestep : timestep;
        KIT_ASSERT_WARN(m_valid, "NaN encountered when computing runge-kutta solution.")
        return m_valid;
    }

    template <typename ODE> bool embedded_forward(float &time, float &timestep, ODE &ode)
    {
        KIT_PERF_FUNCTION()
        KIT_ASSERT_CRITICAL(m_tableau.embedded(),
                            "Cannot perform embedded adaptive stepsize without an embedded solution.")
        m_valid = true;

        timestep = std::clamp(m_error > 0.f ? (timestep * timestep_factor()) : timestep, min_timestep, max_timestep);
        for (;;)
        {
            const float signed_timestep = reversed ? -timestep : timestep;
            std::vector<float> &vars = state.m_vars;
            update_kvec(time, signed_timestep, vars, ode);
            const std::vector<float> sol2 = generate_solution(signed_timestep, vars, m_tableau.coefs2());
            const std::vector<float> sol1 = generate_solution(signed_timestep, vars, m_tableau.coefs1());
            m_error = embedded_error(sol1, sol2);

            const bool too_small = timestep_too_small(timestep);
            if (m_error <= tolerance || too_small)
            {
                vars = sol1;
                if (too_small)
                    timestep = min_timestep;
                break;
            }
            timestep *= timestep_factor();
        }
        m_error = std::max(m_error, tolerance / TOL_PART);
        time += reversed ? -timestep : timestep;
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

    std::vector<float> generate_solution(float timestep, const std::vector<float> &vars,
                                         const std::vector<float> &coefs);

    bool timestep_too_small(float timestep) const;
    bool timestep_too_big(float timestep) const;
    bool timestep_off_bounds(float timestep) const;
    static float embedded_error(const std::vector<float> &sol1, const std::vector<float> &sol2);
    float reiterative_error(const std::vector<float> &sol1, const std::vector<float> &sol2) const;
    float timestep_factor() const;

    template <typename ODE> void update_kvec(float time, float timestep, const std::vector<float> &vars, ODE &ode)
    {
        KIT_PERF_FUNCTION()
        auto &kvec = state.m_kvec;
        KIT_ASSERT_CRITICAL(vars.size() == kvec[0].size(),
                            "State and k-vectors size mismatch! - vars size: {0}, k-vectors size: {1}", vars.size(),
                            kvec[0].size())
        std::vector<float> aux_vars(vars.size());

        kvec[0] = ode(time, timestep, vars);
        for (std::uint8_t i = 1; i < m_tableau.stage(); i++)
        {
            for (std::size_t j = 0; j < vars.size(); j++)
            {
                float k_sum = 0.f;
                for (std::uint8_t k = 0; k < i; k++)
                    k_sum += m_tableau.beta()[i - 1][k] * kvec[k][j];
                aux_vars[j] = vars[j] + k_sum * timestep;
            }
            kvec[i] = ode(time + m_tableau.alpha()[i - 1] * timestep, timestep, aux_vars);
        }
    }
};

} // namespace rk

#endif