#include "rk/pch.hpp"
#include "rk/butcher_tableau.hpp"

namespace rk
{
butcher_tableau::butcher_tableau(const std::vector<float> &alpha, const std::vector<std::vector<float>> &beta,
                                 const std::vector<float> &coefs, const std::uint16_t stage, const std::uint16_t order)
    : m_alpha(alpha), m_coefs1(coefs), m_beta(beta), m_embedded(false), m_stage(stage), m_order(order)
{
}

butcher_tableau::butcher_tableau(const std::vector<float> &alpha, const std::vector<std::vector<float>> &beta,
                                 const std::vector<float> &coefs1, const std::vector<float> &coefs2,
                                 const std::uint16_t stage, const std::uint16_t order)
    : m_alpha(alpha), m_coefs1(coefs1), m_coefs2(coefs2), m_beta(beta), m_embedded(true), m_stage(stage), m_order(order)
{
}

const std::vector<float> &butcher_tableau::alpha() const
{
    return m_alpha;
}
const std::vector<std::vector<float>> &butcher_tableau::beta() const
{
    return m_beta;
}
const std::vector<float> &butcher_tableau::coefs() const
{
    return m_coefs1;
}
const std::vector<float> &butcher_tableau::coefs1() const
{
    return m_coefs1;
}
const std::vector<float> &butcher_tableau::coefs2() const
{
    KIT_ASSERT_ERROR(m_embedded, "Cannot access to the second set of coefficients if the tableau is not embedded")
    return m_coefs2;
}
bool butcher_tableau::embedded() const
{
    return m_embedded;
}
std::uint16_t butcher_tableau::stage() const
{
    return m_stage;
}
std::uint16_t butcher_tableau::order() const
{
    return m_order;
}
#ifdef KIT_USE_YAML_CPP
YAML::Node butcher_tableau::serializer::encode(const butcher_tableau &tb) const
{
    YAML::Node node;
    node["Alpha"] = tb.alpha();
    node[tb.embedded() ? "Coefs1" : "Coefs"] = tb.coefs1();
    if (tb.embedded())
        node["Coefs2"] = tb.coefs2();

    for (auto it = node.begin(); it != node.end(); ++it)
        it->second.SetStyle(YAML::EmitterStyle::Flow);

    for (const auto &v : tb.beta())
    {
        YAML::Node child;
        child = v;
        child.SetStyle(YAML::EmitterStyle::Flow);
        node["Beta"].push_back(v);
    }

    return node;
}
bool butcher_tableau::serializer::decode(const YAML::Node &node, butcher_tableau &tb) const
{
    if (!node.IsMap() || (node.size() != 5 && node.size() != 6))
        return false;

    std::vector<float> alpha, coefs1, coefs2;
    std::vector<std::vector<float>> beta;
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
        tb = {alpha, beta, coefs1, coefs2, node["Stage"].as<std::uint16_t>(), node["Order"].as<std::uint16_t>()};
        return true;
    }
    for (const auto &n : node["Coefs"])
        coefs1.push_back(n.as<float>());
    tb = {alpha, beta, coefs1, node["Stage"].as<std::uint16_t>(), node["Order"].as<std::uint16_t>()};
    return true;
}
#endif
} // namespace rk
