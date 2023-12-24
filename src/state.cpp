#include "rk/pch.hpp"
#include "rk/state.hpp"

namespace rk
{
template <typename T>
state<T>::state(const std::vector<T> &vars, const std::uint32_t stages) : m_vars(vars), m_kvec(stages)
{
    resize_kvec_length();
}

template <typename T> void state<T>::push_back(const T elm)
{
    m_vars.push_back(elm);
    resize_kvec_length();
}

template <typename T> void state<T>::append(std::initializer_list<T> lst)
{
    m_vars.insert(m_vars.end(), lst);
    resize_kvec_length();
}

template <typename T> void state<T>::resize(const std::size_t size)
{
    m_vars.resize(size);
    resize_kvec_length();
}

template <typename T> T state<T>::operator[](std::size_t index) const
{
    return m_vars[index];
}
template <typename T> T &state<T>::operator[](std::size_t index)
{
    return m_vars[index];
}

template <typename T> void state<T>::reserve(const std::size_t capacity)
{
    m_vars.reserve(capacity);
    m_vars.reserve(capacity);
    for (std::vector<T> &v : m_kvec)
        v.reserve(capacity);
}

template <typename T> void state<T>::clear()
{
    m_vars.clear();
    for (std::vector<T> &v : m_kvec)
        v.clear();
}

template <typename T> void state<T>::resize_kvec_length()
{
    for (std::vector<T> &v : m_kvec)
        v.resize(m_vars.size());
}

template <typename T> void state<T>::set_stages(const std::uint32_t stages)
{
    m_kvec.resize(stages);
    resize_kvec_length();
}

template <typename T> const std::vector<T> &state<T>::vars() const
{
    return m_vars;
}

template <typename T> void state<T>::vars(const std::vector<T> &vars)
{
    m_vars = vars;
    resize_kvec_length();
}

template <typename T> std::size_t state<T>::size() const
{
    return m_vars.size();
}

#ifdef KIT_USE_YAML_CPP
template <typename T> YAML::Node state<T>::serializer::encode(const state &st) const
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
template <typename T> bool state<T>::serializer::decode(const YAML::Node &node, state &st) const
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
#endif

template class state<float>;
template class state<double>;
template class state<long double>;
} // namespace rk
