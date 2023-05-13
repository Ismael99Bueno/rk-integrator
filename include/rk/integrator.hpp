#ifndef INTEGRATOR_HPP
#define INTEGRATOR_HPP

#include "rk/butcher_tableau.hpp"
#include "dbg/log.hpp"
#include "perf/perf.hpp"
#include "rk/state.hpp"
#include <cstdint>
#include <cmath>

#define TOL_PART 256.f

namespace rk
{
    class integrator
    {
    private:
    public:
        integrator() = default;
        integrator(const butcher_tableau &tb,
                   const std::vector<float> &vars = {},
                   float tolerance = 1e-4f,
                   float min_dt = 1e-6f,
                   float max_dt = 1.f);

        template <typename T>
        bool raw_forward(float &t,
                         float dt,
                         T &params,
                         std::vector<float> (*ode)(float, const std::vector<float> &, T &))
        {
            PERF_SCOPE("-Physics-")
            DBG_ASSERT_ERROR(!dt_off_bounds(dt), "Timestep is not between established limits. Change the timestep or adjust the limits to include the current value - current: {0}, min: {1}, max: {2}", dt, m_min_dt, m_max_dt)
            m_valid = true;
            const float sdt = m_reversed ? -dt : dt;

            std::vector<float> &vars = m_state.m_vars;
            update_kvec(t, sdt, vars, params, ode);
            if (m_tableau.embedded())
            {
                const std::vector<float> aux_state = generate_solution(sdt, vars, m_tableau.coefs2());
                vars = generate_solution(sdt, vars, m_tableau.coefs1());
                m_error = embedded_error(vars, aux_state);
            }
            else
                vars = generate_solution(sdt, vars, m_tableau.coefs());
            t += sdt;
            DBG_ASSERT_WARN(m_valid, "NaN encountered when computing runge-kutta solution.")
            return m_valid;
        }

        template <typename T>
        bool reiterative_forward(float &t,
                                 float &dt,
                                 T &params,
                                 std::vector<float> (*ode)(float, const std::vector<float> &, T &),
                                 std::uint8_t reiterations = 2)
        {
            PERF_SCOPE("-Physics-")
            DBG_ASSERT_CRITICAL(reiterations >= 2, "The amount of reiterations has to be greater than 1, otherwise the algorithm will break.")
            DBG_ASSERT_ERROR(!dt_off_bounds(dt), "Timestep is not between established limits. Change the timestep or adjust the limits to include the current value - current: {0}, min: {1}, max: {2}", dt, m_min_dt, m_max_dt)
            DBG_ASSERT_WARN(!m_tableau.embedded(), "Butcher tableau has an embedded solution. Use an embedded adaptive method for better efficiency.")

            m_valid = true;
            if (m_error > 0.f)
                dt = std::clamp(dt * timestep_factor(), m_min_dt, m_max_dt);
            for (;;)
            {
                const float sdt = m_reversed ? -dt : dt;
                std::vector<float> &vars = m_state.m_vars;
                std::vector<float> sol1 = vars;
                update_kvec(t, sdt, vars, params, ode);

                const std::vector<float> sol2 = generate_solution(sdt, vars, m_tableau.coefs());
                for (std::uint8_t i = 0; i < reiterations; i++)
                {
                    update_kvec(t, sdt / reiterations, sol1, params, ode);
                    sol1 = generate_solution(sdt / reiterations, sol1, m_tableau.coefs());
                }
                m_error = reiterative_error(sol1, sol2);

                const bool too_small = dt_too_small(dt);
                if (m_error <= m_tolerance || too_small)
                {
                    vars = sol1;
                    if (too_small)
                        dt = m_min_dt;
                    break;
                }
                dt *= timestep_factor();
            }
            m_error = std::max(m_error, m_tolerance / TOL_PART);
            t += m_reversed ? -dt : dt;
            DBG_ASSERT_WARN(m_valid, "NaN encountered when computing runge-kutta solution.")
            return m_valid;
        }

        template <typename T>
        bool embedded_forward(float &t,
                              float &dt,
                              T &params,
                              std::vector<float> (*ode)(float, const std::vector<float> &, T &))
        {
            PERF_SCOPE("-Physics-")
            DBG_ASSERT_CRITICAL(m_tableau.embedded(), "Cannot perform embedded adaptive stepsize without an embedded solution.")
            DBG_ASSERT_ERROR(!dt_off_bounds(dt), "Timestep is not between established limits. Change the timestep or adjust the limits to include the vars value - vars: {0}, min: {1}, max: {2}", dt, m_min_dt, m_max_dt)
            m_valid = true;

            if (m_error > 0.f)
                dt = std::clamp(dt * timestep_factor(), m_min_dt, m_max_dt);
            for (;;)
            {
                const float sdt = m_reversed ? -dt : dt;
                std::vector<float> &vars = m_state.m_vars;
                update_kvec(t, sdt, vars, params, ode);
                const std::vector<float> sol2 = generate_solution(sdt, vars, m_tableau.coefs2());
                const std::vector<float> sol1 = generate_solution(sdt, vars, m_tableau.coefs1());
                m_error = embedded_error(sol1, sol2);

                const bool too_small = dt_too_small(dt);
                if (m_error <= m_tolerance || too_small)
                {
                    vars = sol1;
                    if (too_small)
                        dt = m_min_dt;
                    break;
                }
                dt *= timestep_factor();
            }
            m_error = std::max(m_error, m_tolerance / TOL_PART);
            t += m_reversed ? -dt : dt;
            DBG_ASSERT_WARN(m_valid, "NaN encountered when computing runge-kutta solution.")
            return m_valid;
        }

        const butcher_tableau &tableau() const;
        const rk::state &state() const;
        rk::state &state();

        void tableau(const butcher_tableau &tableau);

        float tolerance() const;
        float min_dt() const;
        float max_dt() const;
        float error() const;

        void tolerance(float val);
        void min_dt(float val);
        void max_dt(float val);

        bool reversed() const;
        void reversed(bool reversed);

        bool limited_timestep() const;
        void limited_timestep(bool limited_timestep);

    private:
        butcher_tableau m_tableau;
        rk::state m_state;
        float m_tolerance, m_min_dt, m_max_dt, m_error;
        bool m_valid, m_reversed = false, m_limited_timestep = true;

        std::vector<float> generate_solution(float dt,
                                             const std::vector<float> &vars,
                                             const std::vector<float> &coefs);

        bool dt_too_small(float dt) const;
        bool dt_too_big(float dt) const;
        bool dt_off_bounds(float dt) const;
        static float embedded_error(const std::vector<float> &sol1, const std::vector<float> &sol2);
        float reiterative_error(const std::vector<float> &sol1, const std::vector<float> &sol2) const;
        float timestep_factor() const;

        template <typename T>
        void update_kvec(float t,
                         float dt,
                         const std::vector<float> &vars,
                         T &params,
                         std::vector<float> (*ode)(float, const std::vector<float> &, T &))
        {
            PERF_FUNCTION()
            auto &kvec = m_state.m_kvec;
            DBG_ASSERT_CRITICAL(vars.size() == kvec[0].size(), "State and k-vectors size mismatch! - vars size: {0}, k-vectors size: {1}", vars.size(), kvec[0].size())
            std::vector<float> aux_vars(vars.size());

            kvec[0] = ode(t, vars, params);
            for (std::uint8_t i = 1; i < m_tableau.stage(); i++)
            {
                for (std::size_t j = 0; j < vars.size(); j++)
                {
                    float k_sum = 0.f;
                    for (std::uint8_t k = 0; k < i; k++)
                        k_sum += m_tableau.beta()[i - 1][k] * kvec[k][j];
                    aux_vars[j] = vars[j] + k_sum * dt;
                }
                kvec[i] = ode(t + m_tableau.alpha()[i - 1] * dt, aux_vars, params);
            }
        }
#ifdef HAS_YAML_CPP
        friend YAML::Emitter &operator<<(YAML::Emitter &, const integrator &);
        friend struct YAML::convert<integrator>;
#endif
    };

#ifdef HAS_YAML_CPP
    YAML::Emitter &operator<<(YAML::Emitter &out, const integrator &integ);
#endif
}

#ifdef HAS_YAML_CPP
namespace YAML
{
    template <>
    struct convert<rk::integrator>
    {
        static Node encode(const rk::integrator &integ);
        static bool decode(const Node &node, rk::integrator &integ);
    };
}
#endif
#endif