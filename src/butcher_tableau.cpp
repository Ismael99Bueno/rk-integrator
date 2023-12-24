#include "rk/pch.hpp"
#include "rk/butcher_tableau.hpp"

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

template <typename T> const butcher_tableau<T> butcher_tableau<T>::rk1 = {{}, {}, {1.0}, 1, 1};
template <typename T> const butcher_tableau<T> butcher_tableau<T>::rk2 = {{1.0}, {{1.0}}, {0.5, 0.5}, 2, 2};
template <typename T>
const butcher_tableau<T> butcher_tableau<T>::rk4 = {
    {0.5, 0.5, 1.0}, {{0.5}, {0.0, 0.5}, {0.0, 0.0, 1.0}}, {1.0 / 6.0, 1.0 / 3.0, 1.0 / 3.0, 1.0 / 6.0}, 4, 4};

template <typename T>
const butcher_tableau<T> butcher_tableau<T>::rk38 = {{1.0 / 3.0, 2.0 / 3.0, 1.0},
                                                     {
                                                         {1.0 / 3.0},
                                                         {-1.0 / 3.0, 1.0},
                                                         {1.0, -1.0, 1.0},
                                                     },
                                                     {1.0 / 8.0, 3.0 / 8.0, 3.0 / 8.0, 1.0 / 8.0},
                                                     4,
                                                     4};

template <typename T> const butcher_tableau<T> butcher_tableau<T>::rkf12 = {{1}, {{1}}, {0.5, 0.5}, {1.0, 0.0}, 2, 2};

template <typename T>
const butcher_tableau<T> butcher_tableau<T>::rkf45 = {
    {0.25, 3.0 / 8.0, 12.0 / 13.0, 1.0, 0.5},
    {{0.25},
     {3.0 / 32.0, 9.0 / 32.0},
     {1932.0 / 2197.0, -7200.0 / 2197.0, 7296.0 / 2197.0},
     {439.0 / 216.0, -8.0, 3680.0 / 513.0, -845.0 / 4104.0},
     {-8.0 / 27.0, 2.0, -3544.0 / 2565.0, 1859.0 / 4104.0, -11.0 / 40.0}},
    {16.0 / 135.0, 0.0, 6656 / 12825.0, 28561.0 / 56430.0, -9.0 / 50.0, 2.0 / 55.0},
    {25.0 / 216.0, 0.0, 1408.0 / 2565.0, 2197.0 / 4104.0, -0.2, 0.0},
    6,
    5};

template <typename T>
const butcher_tableau<T> butcher_tableau<T>::rkfck45 = {
    {0.2, 0.3, 0.6, 1.0, 7.0 / 8.0},
    {{0.2},
     {3.0 / 40.0, 9.0 / 40.0},
     {0.3, -0.9, 6.0 / 5.0},
     {-11.0 / 54.0, 2.5, -70.0 / 27.0, 35.0 / 27.0},
     {1631.0 / 55296.0, 175.0 / 512.0, 575.0 / 13824.0, 44275.0 / 110592.0, 253.0 / 4096.0}},
    {37.0 / 378.0, 0.0, 250.0 / 621.0, 125.0 / 594.0, 0.0, 512.0 / 1771.0},
    {2825.0 / 27648.0, 0.0, 18575.0 / 48384.0, 13525.0 / 55296.0, 277.0 / 14336.0, 0.25},
    6,
    5};

template <typename T>
const butcher_tableau<T> butcher_tableau<T>::rkf78 = {
    {2.0 / 27.0, 1.0 / 9.0, 1.0 / 6.0, 5.0 / 12.0, 0.5, 5.0 / 6.0, 1.0 / 6.0, 2.0 / 3.0, 1.0 / 3.0, 1.0, 0.0, 1.0},
    {{2.0 / 27.0},
     {1.0 / 36.0, 1.0 / 12.0},
     {1.0 / 24.0, 0.0, 1.0 / 8.0},
     {5.0 / 12.0, 0.0, -25.0 / 16.0, 25.0 / 16.0},
     {1.0 / 20.0, 0.0, 0.0, 0.25, 0.2},
     {-25.0 / 108.0, 0.0, 0.0, 125.0 / 108.0, -65.0 / 27.0, 125.0 / 54.0},
     {31.0 / 300.0, 0.0, 0.0, 0.0, 61.0 / 225.0, -2.0 / 9.0, 13.0 / 900.0},
     {2.0, 0.0, 0.0, -53.0 / 6.0, 704.0 / 45.0, -107.0 / 9.0, 67.0 / 90.0, 3.0},
     {-91.0 / 108.0, 0.0, 0.0, 23.0 / 108.0, -976.0 / 135.0, 311.0 / 54.0, -19.0 / 60.0, 17.0 / 6.0, -1.0 / 12.0},
     {2383.0 / 4100.0, 0.0, 0.0, -341.0 / 164.0, 4496.0 / 1025.0, -301.0 / 82.0, 2133.0 / 4100.0, 45.0 / 82.0,
      45.0 / 164.0, 18.0 / 41.0},
     {3.0 / 205.0, 0.0, 0.0, 0.0, 0.0, -6.0 / 41.0, -3.0 / 205.0, -3.0 / 41.0, 3.0 / 41.0, 6.0 / 41.0, 0.0},
     {-1777.0 / 4100.0, 0.0, 0.0, -341.0 / 164.0, 4496.0 / 1025.0, -289.0 / 82.0, 2193.0 / 4100.0, 51.0 / 82.0,
      33.0 / 164.0, 12.0 / 41.0, 0.0, 1.0}},
    {0.0, 0.0, 0.0, 0.0, 0.0, 34.0 / 105.0, 9.0 / 35.0, 9.0 / 35.0, 9.0 / 280.0, 9.0 / 280.0, 0.0, 41.0 / 840.0,
     41.0 / 840.0},
    {41.0 / 840.0, 0.0, 0.0, 0.0, 0.0, 34.0 / 105.0, 9.0 / 35.0, 9.0 / 35.0, 9.0 / 280.0, 9.0 / 280.0, 41.0 / 840.0,
     0.0, 0.0},
    13,
    8};

#ifdef KIT_USE_YAML_CPP
template <typename T> YAML::Node butcher_tableau<T>::serializer::encode(const butcher_tableau &tb) const
{
    YAML::Node node;
    node["Alpha"] = tb.alpha;
    node[tb.embedded ? "Coefs1" : "Coefs"] = tb.coefs1;
    if (tb.embedded)
        node["Coefs2"] = tb.coefs2;

    for (auto it = node.begin(); it != node.end(); ++it)
        it->second.SetStyle(YAML::EmitterStyle::Flow);

    for (std::size_t i = 0; i < tb.beta.size(); i++)
    {
        YAML::Node child;
        child = tb.beta[i];
        node["Beta"].push_back(child);
        node["Beta"][i].SetStyle(YAML::EmitterStyle::Flow);
    }
    node["Stage"] = tb.stages;
    node["Order"] = tb.order;

    return node;
}
template <typename T> bool butcher_tableau<T>::serializer::decode(const YAML::Node &node, butcher_tableau &tb) const
{
    if (!node.IsMap() || (node.size() < 4 && node.size() > 6))
        return false;

    std::vector<float> alpha, coefs1, coefs2;
    std::vector<std::vector<float>> beta;
    if (node["Beta"])
        for (const auto &n1 : node["Beta"])
        {
            auto &v1 = beta.emplace_back();
            for (const auto &n2 : n1)
                v1.push_back(n2.as<float>());
        }
    for (const auto &n : node["Alpha"])
        alpha.push_back(n.as<float>());

    if (node["Coefs2"])
    {
        for (const auto &n : node["Coefs1"])
            coefs1.push_back(n.as<float>());
        for (const auto &n : node["Coefs2"])
            coefs2.push_back(n.as<float>());
        tb = {alpha, beta, coefs1, coefs2, node["Stage"].as<std::uint32_t>(), node["Order"].as<std::uint32_t>()};
        return true;
    }
    else
        for (const auto &n : node["Coefs"])
            coefs1.push_back(n.as<float>());
    tb = {alpha, beta, coefs1, node["Stage"].as<std::uint32_t>(), node["Order"].as<std::uint32_t>()};
    return true;
}
#endif

template class butcher_tableau<float>;
template class butcher_tableau<double>;
template class butcher_tableau<long double>;
} // namespace rk
