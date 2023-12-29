#pragma once
#ifdef KIT_USE_YAML_CPP

#include "rk/integration/integrator.hpp"
#include "kit/serialization/yaml/codec.hpp"

template <typename Float> struct kit::yaml::codec<rk::timestep<Float>>
{
    static YAML::Node encode(const rk::timestep<Float> &ts)
    {
        YAML::Node node;
        node["Value"] = ts.value;
        node["Min"] = ts.min;
        node["Max"] = ts.max;
        node["Limited"] = ts.limited;
        return node;
    }
    static bool decode(const YAML::Node &node, rk::timestep<Float> &ts)
    {
        if (!node.IsMap() || node.size() != 4)
            return false;
        ts.value = node["Value"].as<Float>();
        ts.min = node["Min"].as<Float>();
        ts.max = node["Max"].as<Float>();
        ts.limited = node["Limited"].as<bool>();
        return true;
    }
};

template <typename Float> struct kit::yaml::codec<rk::butcher_tableau<Float>>
{
    static YAML::Node encode(const rk::butcher_tableau<Float> &tb)
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
    static bool decode(const YAML::Node &node, rk::butcher_tableau<Float> &tb)
    {
        if (!node.IsMap() || (node.size() < 4 && node.size() > 6))
            return false;

        std::vector<Float> alpha, coefs1, coefs2;
        std::vector<std::vector<Float>> beta;
        if (node["Beta"])
            for (const auto &n1 : node["Beta"])
            {
                auto &v1 = beta.emplace_back();
                for (const auto &n2 : n1)
                    v1.push_back(n2.as<Float>());
            }
        for (const auto &n : node["Alpha"])
            alpha.push_back(n.as<Float>());

        if (node["Coefs2"])
        {
            for (const auto &n : node["Coefs1"])
                coefs1.push_back(n.as<Float>());
            for (const auto &n : node["Coefs2"])
                coefs2.push_back(n.as<Float>());
            tb = {alpha, beta, coefs1, coefs2, node["Stage"].as<std::uint32_t>(), node["Order"].as<std::uint32_t>()};
            return true;
        }
        else
            for (const auto &n : node["Coefs"])
                coefs1.push_back(n.as<Float>());
        tb = {alpha, beta, coefs1, node["Stage"].as<std::uint32_t>(), node["Order"].as<std::uint32_t>()};
        return true;
    }
};

template <typename Float> struct kit::yaml::codec<rk::state<Float>>
{
    static YAML::Node encode(const rk::state<Float> &st)
    {
        YAML::Node node;
        node["State variables"] = st.vars();
        node["State variables"].SetStyle(YAML::EmitterStyle::Flow);
        return node;
    }
    static bool decode(const YAML::Node &node, rk::state<Float> &st)
    {
        if (!node.IsMap() || node.size() != 1)
            return false;

        std::vector<Float> vars;
        for (const auto &n : node["State variables"])
            vars.push_back(n.as<Float>());
        st.vars(vars);

        return true;
    }
};
template <typename Float> struct kit::yaml::codec<rk::integrator<Float>>
{
    static YAML::Node encode(const rk::integrator<Float> &integ)
    {
        YAML::Node node;
        node["Tableau"] = integ.tableau();
        node["State"] = integ.state;
        node["Semi-implicit"] = integ.semi_implicit;
        node["Tolerance"] = integ.tolerance;
        node["Elapsed"] = integ.elapsed;
        node["Timestep"] = integ.ts;
        return node;
    }
    static bool decode(const YAML::Node &node, rk::integrator<Float> &integ)
    {
        if (!node.IsMap() || node.size() != 6)
            return false;

        integ.state = node["State"].as<rk::state<Float>>();
        integ.tableau(node["Tableau"].as<rk::butcher_tableau<Float>>());
        integ.semi_implicit = node["Semi-implicit"].as<bool>();
        integ.tolerance = node["Tolerance"].as<Float>();
        integ.elapsed = node["Elapsed"].as<Float>();
        integ.ts = node["Timestep"].as<rk::timestep<Float>>();
        return true;
    }
};
#endif