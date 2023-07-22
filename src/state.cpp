#include "rk/pch.hpp"
#include "rk/state.hpp"

namespace rk
{
state::state(const std::vector<float> &vars, const std::uint16_t stage) : m_vars(vars), m_kvec(stage)
{
    resize();
}

void state::push_back(const float elm)
{
    m_vars.push_back(elm);
    resize();
}

void state::append(std::initializer_list<float> lst)
{
    m_vars.insert(m_vars.end(), lst);
    resize();
}

void state::resize(const std::size_t size)
{
    m_vars.resize(size);
    resize();
}

const float &state::operator[](std::size_t index) const
{
    return m_vars[index];
}
float &state::operator[](std::size_t index)
{
    return m_vars[index];
}

void state::reserve(const std::size_t capacity)
{
    m_vars.reserve(capacity);
    m_vars.reserve(capacity);
    for (std::vector<float> &v : m_kvec)
        v.reserve(capacity);
}

void state::clear()
{
    m_vars.clear();
    for (std::vector<float> &v : m_kvec)
        v.clear();
}

void state::resize()
{
    for (std::vector<float> &v : m_kvec)
        v.resize(m_vars.size());
}

void state::resize_kvec(const std::uint16_t stage)
{
    m_kvec.resize(stage);
    resize();
}

const std::vector<float> &state::vars() const
{
    return m_vars;
}

void state::vars(const std::vector<float> &vars)
{
    m_vars = vars;
    resize();
}

std::size_t state::size() const
{
    return m_vars.size();
}

#ifdef KIT_USE_YAML_CPP
YAML::Node state::serializer::encode(const state &st) const
{
    YAML::Node node;
    node["State variables"] = st.vars();
    node["State variables"].SetStyle(YAML::EmitterStyle::Flow);
    node["Step"].SetStyle(YAML::EmitterStyle::Flow);
    for (const auto &v : st.m_kvec)
    {
        YAML::Node child;
        child = v;
        child.SetStyle(YAML::EmitterStyle::Flow);
        node["K-Vectors"].push_back(v);
    }
    return node;
}
bool state::serializer::decode(const YAML::Node &node, state &st) const
{
    if (!node.IsMap() || node.size() != 2)
        return false;

    std::vector<float> vars;
    std::vector<std::vector<float>> k_vec;
    for (const auto &n1 : node["K-Vectors"])
    {
        auto &v1 = k_vec.emplace_back();
        for (const auto &n2 : n1)
            v1.push_back(n2.as<float>());
    }
    for (const auto &n : node["State variables"])
        vars.push_back(n.as<float>());
    st.m_vars = vars;
    st.m_kvec = k_vec;

    return true;
}
#endif
} // namespace rk
