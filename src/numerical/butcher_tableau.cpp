#include "rk/internal/pch.hpp"
#include "rk/numerical/butcher_tableau.hpp"

namespace rk
{
template <typename Float>
butcher_tableau<Float>::butcher_tableau(const std::vector<Float> &alpha, const std::vector<std::vector<Float>> &beta,
                                        const std::vector<Float> &coefs, const std::uint32_t stages,
                                        const std::uint32_t order)
    : alpha(alpha), coefs1(coefs), beta(beta), embedded(false), stages(stages), order(order)
{
}

template <typename Float>
butcher_tableau<Float>::butcher_tableau(const std::vector<Float> &alpha, const std::vector<std::vector<Float>> &beta,
                                        const std::vector<Float> &coefs1, const std::vector<Float> &coefs2,
                                        const std::uint32_t stages, const std::uint32_t order)
    : alpha(alpha), coefs1(coefs1), coefs2(coefs2), beta(beta), embedded(true), stages(stages), order(order)
{
}

template struct butcher_tableau<float>;
template struct butcher_tableau<double>;
template struct butcher_tableau<long double>;
} // namespace rk
