#pragma once

#include "rk/integration/integrator.hpp"
#include "kit/serialization/yaml/codec.hpp"

template <typename T> struct kit::yaml::codec<rk::timestep<T>>
{
    static YAML::Node encode(const rk::timestep<T> &ts)
    {
        YAML::Node node;
        node["Value"] = integ.ts.value;
        node["Min"] = integ.ts.min;
        node["Max"] = integ.ts.max;
        node["Limited"] = integ.ts.limited;
        return node;
    }
    static bool decode(const YAML::Node &node, rk::timestep<T> &ts)
    {
        if (!node.IsMap() || !node.size() != 4)
            return false;
        ts.value = node["Value"].as<T>();
        ts.min = node["Min"].as<T>();
        ts.max = node["Max"].as<T>();
        ts.limited = node["Limited"].as<bool>();
        return true;
    }
};

template <typename T> struct kit::yaml::codec<rk::butcher_tableau<T>>
{
    static YAML::Node encode(const rk::butcher_tableau<T> &tb)
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
    static bool decode(const YAML::Node &node, rk::butcher_tableau<T> &tb)
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
};

template <typename T> struct kit::yaml::codec<rk::state<T>>
{
    static YAML::Node encode(const rk::state<T> &st)
    {
        YAML::Node node;
        node["State variables"] = st.vars();
        node["State variables"].SetStyle(YAML::EmitterStyle::Flow);
        for (std::size_t i = 0; i < st.m_kvec.size(); i++)
        {
            YAML::Node child;
            child = st.m_kvec[i];
            node["K-Vectors"].push_back(child);
            node["K-Vectors"][i].SetStyle(YAML::EmitterStyle::Flow);
        }
        return node;
    }
    static bool decode(const YAML::Node &node, rk::state<T> &st)
    {
        if (!node.IsMap() || node.size() != 2)
            return false;

        std::vector<T> vars;
        std::vector<std::vector<T>> k_vec;
        for (const auto &n1 : node["K-Vectors"])
        {
            auto &v1 = k_vec.emplace_back();
            for (const auto &n2 : n1)
                v1.push_back(n2.as<T>());
        }
        for (const auto &n : node["State variables"])
            vars.push_back(n.as<T>());
        st.m_vars = vars;
        st.m_kvec = k_vec;

        return true;
    }
};
template <typename T> struct kit::yaml::codec<rk::integrator<T>>
{
    static YAML::Node encode(const rk::integrator<T> &integ)
    {
        YAML::Node node;
        node["Tableau"] = integ.tableau();
        node["State"] = integ.state;
        node["Tolerance"] = integ.tolerance;
        node["Elapsed"] = integ.elapsed;
        node["Timestep"] = integ.ts;
        return node;
    }
    static bool decode(const YAML::Node &node, rk::integrator<T> &integ)
    {
        if (!node.IsMap() || node.size() != 5)
            return false;

        integ.tableau(node["Tableau"].as<rk::butcher_tableau<T>>());
        integ.state = node["State"].as<rk::state<T>>();
        integ.tolerance = node["Tolerance"].as<T>();
        integ.elapsed = node["Elapsed"].as<T>();
        integ.ts = node["Timestep"].as<rk::timestep<T>>();
        return true;
    }
};