#include "rk/internal/pch.hpp"
#include "rk/numerical/butcher_tableau.hpp"

namespace rk
{
template <typename T>
butcher_tableau<T>::butcher_tableau(const std::vector<T> &alpha, const std::vector<std::vector<T>> &beta,
                                    const std::vector<T> &coefs, const std::uint32_t stages, const std::uint32_t order)
    : alpha(alpha), coefs1(coefs), beta(beta), embedded(false), stages(stages), order(order)
{
}

template <typename T>
butcher_tableau<T>::butcher_tableau(const std::vector<T> &alpha, const std::vector<std::vector<T>> &beta,
                                    const std::vector<T> &coefs1, const std::vector<T> &coefs2,
                                    const std::uint32_t stages, const std::uint32_t order)
    : alpha(alpha), coefs1(coefs1), coefs2(coefs2), beta(beta), embedded(true), stages(stages), order(order)
{
}

template struct butcher_tableau<float>;
template struct butcher_tableau<double>;
template struct butcher_tableau<long double>;
} // namespace rk
