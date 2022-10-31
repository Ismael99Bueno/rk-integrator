#include "integrator.hpp"
#include "debug.h"
#include <cmath>
#define SAFETY_FACTOR 0.85f

namespace rk
{
    integrator::integrator(const tableau &tb,
                           vector &state,
                           const float tolerance,
                           const float min_dt,
                           const float max_dt) : m_tableau(tb),
                                                 m_state(state),
                                                 m_tolerance(tolerance),
                                                 m_min_dt(min_dt),
                                                 m_max_dt(max_dt),
                                                 m_error(0.f),
                                                 m_valid(true) { resize_kvec(); }

    void integrator::resize_kvec()
    {
        m_kvec.resize(m_tableau.stage());
        for (vector &v : m_kvec)
            v.resize(m_state.size());
    }

    integrator::vector integrator::generate_solution(const float dt,
                                                     const vector &state,
                                                     const vector &coefs)
    {
        vector sol;
        sol.reserve(state.size());
        for (std::size_t j = 0; j < state.size(); j++)
        {
            float sum = 0.f;
            for (uint8 i = 0; i < m_tableau.stage(); i++)
                sum += coefs[i] * m_kvec[i][j];
            m_valid &= !isnan(sum);
            DBG_LOG_IF(!m_valid, "NaN encountered when computing runge-kutta solution.\n")
            sol.emplace_back(state[j] + sum * dt);
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

        return result;
    }

    bool integrator::dt_too_small(const float dt) const { return dt < m_min_dt; }
    bool integrator::dt_too_big(const float dt) const { return dt > m_max_dt; }
    bool integrator::dt_off_bounds(const float dt) const { return dt_too_small(dt) || dt_too_big(dt); }

    float integrator::embedded_error(const vector &sol1, const vector &sol2)
    {
        float result = 0.f;
        for (std::size_t i = 0; i < sol1.size(); i++)
            result += (sol1[i] - sol2[i]) * (sol1[i] - sol2[i]);
        return result;
    }

    float integrator::reiterative_error(const vector &sol1, const vector &sol2) const
    {
        const uint32 coeff = ipow(2, m_tableau.order()) - 1;
        return embedded_error(sol1, sol2) / coeff;
    }

    float integrator::timestep_factor() const
    {
        return SAFETY_FACTOR * std::pow(m_tolerance / m_error, 1.f / m_tableau.order());
    }

    void integrator::reserve_state(std::size_t size)
    {
        m_state.reserve(size);
        for (vector &v : m_kvec)
            v.reserve(size);
    }

    void integrator::resize_state(std::size_t size)
    {
        m_state.resize(size);
        resize_kvec();
    }

    float integrator::tolerance() const { return m_tolerance; }
    float integrator::min_dt() const { return m_min_dt; }
    float integrator::max_dt() const { return m_max_dt; }
    float integrator::error() const { return m_error; }

    void integrator::tolerance(const float val) { m_tolerance = val; }
    void integrator::min_dt(const float val) { m_min_dt = val; }
    void integrator::max_dt(const float val) { m_max_dt = val; }
}