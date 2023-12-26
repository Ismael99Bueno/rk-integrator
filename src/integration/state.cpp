#include "rk/internal/pch.hpp"
#include "rk/integration/state.hpp"

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

template class state<float>;
template class state<double>;
template class state<long double>;
} // namespace rk
