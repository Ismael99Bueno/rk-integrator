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

#ifdef KIT_USE_YAML_CPP
template <typename T> YAML::Node butcher_tableau<T>::serializer::encode(const butcher_tableau<T> &tb) const
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
template <typename T> bool butcher_tableau<T>::serializer::decode(const YAML::Node &node, butcher_tableau<T> &tb) const
{
    if (!node.IsMap() || (node.size() < 4 && node.size() > 6))
        return false;

    std::vector<T> alpha, coefs1, coefs2;
    std::vector<std::vector<T>> beta;
    if (node["Beta"])
        for (const auto &n1 : node["Beta"])
        {
            auto &v1 = beta.emplace_back();
            for (const auto &n2 : n1)
                v1.push_back(n2.as<T>());
        }
    for (const auto &n : node["Alpha"])
        alpha.push_back(n.as<T>());

    if (node["Coefs2"])
    {
        for (const auto &n : node["Coefs1"])
            coefs1.push_back(n.as<T>());
        for (const auto &n : node["Coefs2"])
            coefs2.push_back(n.as<T>());
        tb = {alpha, beta, coefs1, coefs2, node["Stage"].as<std::uint32_t>(), node["Order"].as<std::uint32_t>()};
        return true;
    }
    else
        for (const auto &n : node["Coefs"])
            coefs1.push_back(n.as<T>());
    tb = {alpha, beta, coefs1, node["Stage"].as<std::uint32_t>(), node["Order"].as<std::uint32_t>()};
    return true;
}
#endif

template struct butcher_tableau<float>;
template struct butcher_tableau<double>;
template struct butcher_tableau<long double>;
} // namespace rk
