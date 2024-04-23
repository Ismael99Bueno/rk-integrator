#pragma once
#ifdef KIT_USE_YAML_CPP

#include "rk/integration/integrator.hpp"
#include "kit/serialization/yaml/codec.hpp"

template <std::floating_point Float> struct kit::yaml::codec<rk::timestep<Float>>
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

template <std::floating_point Float> struct kit::yaml::codec<rk::butcher_tableau<Float>>
{
    static YAML::Node encode(const rk::butcher_tableau<Float> &tb)
    {
        YAML::Node node;
        for (const float elm : tb.alpha)
            node["Alpha"].push_back(elm);
        for (const float elm : tb.coefs1)
            node[tb.embedded ? "Coefs1" : "Coefs"].push_back(elm);
        if (tb.embedded)
            for (const float elm : tb.coefs2)
                node["Coefs2"].push_back(elm);

        for (auto it = node.begin(); it != node.end(); ++it)
            it->second.SetStyle(YAML::EmitterStyle::Flow);

        for (std::size_t i = 0; i < tb.beta.size(); i++)
        {
            YAML::Node child;
            for (const float elm : tb.beta[i])
                child.push_back(elm);
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
        using array1 = typename rk::butcher_tableau<Float>::array1;
        using array2 = typename rk::butcher_tableau<Float>::array2;

        array1 alpha, coefs1, coefs2;
        array2 beta;
        if (node["Beta"])
            for (const auto &n1 : node["Beta"])
            {
                array1 v1;
                for (const auto &n2 : n1)
                    v1.push_back(n2.as<Float>());
                beta.push_back(v1);
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

template <std::floating_point Float> struct kit::yaml::codec<rk::state<Float>>
{
    static YAML::Node encode(const rk::state<Float> &st)
    {
        YAML::Node node;
        node["State variables"] = st.vars();
        node["State variables"].SetStyle(YAML::EmitterStyle::Flow);
        node["Stages"] = st.stages();
        return node;
    }
    static bool decode(const YAML::Node &node, rk::state<Float> &st)
    {
        if (!node.IsMap() || node.size() != 2)
            return false;

        st.stages(node["Stages"].as<std::uint32_t>());
        std::vector<Float> vars;
        for (const auto &n : node["State variables"])
            vars.push_back(n.as<Float>());
        st.vars(vars);

        return true;
    }
};
template <std::floating_point Float> struct kit::yaml::codec<rk::integrator<Float>>
{
    static YAML::Node encode(const rk::integrator<Float> &integ)
    {
        YAML::Node node;
        node["Tableau"] = integ.tableau();
        node["State"] = integ.state;
        node["Tolerance"] = integ.tolerance;
        node["Elapsed"] = integ.elapsed;
        node["Timestep"] = integ.ts;
        return node;
    }
    static bool decode(const YAML::Node &node, rk::integrator<Float> &integ)
    {
        if (!node.IsMap() || node.size() != 5)
            return false;

        integ.state = node["State"].as<rk::state<Float>>();
        integ.tableau(node["Tableau"].as<rk::butcher_tableau<Float>>());
        integ.tolerance = node["Tolerance"].as<Float>();
        integ.elapsed = node["Elapsed"].as<Float>();
        integ.ts = node["Timestep"].as<rk::timestep<Float>>();
        return true;
    }
};
#endif