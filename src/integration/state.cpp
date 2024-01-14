#include "rk/internal/pch.hpp"
#include "rk/integration/state.hpp"

namespace rk
{
template <kit::FloatingPoint Float>
state<Float>::state(const std::vector<Float> &vars, const std::uint32_t stages) : m_vars(vars), m_kvec(stages)
{
    resize_kvec_length();
}

template <kit::FloatingPoint Float> void state<Float>::push_back(const Float elm)
{
    m_vars.push_back(elm);
    resize_kvec_length();
}

template <kit::FloatingPoint Float> void state<Float>::append(std::initializer_list<Float> lst)
{
    m_vars.insert(m_vars.end(), lst);
    resize_kvec_length();
}

template <kit::FloatingPoint Float> void state<Float>::resize(const std::size_t size)
{
    m_vars.resize(size);
    resize_kvec_length();
}

template <kit::FloatingPoint Float> Float state<Float>::operator[](std::size_t index) const
{
    KIT_ASSERT_ERROR(index < m_vars.size(), "Index exceeds container size: {0}", index)
    return m_vars[index];
}
template <kit::FloatingPoint Float> Float &state<Float>::operator[](std::size_t index)
{
    KIT_ASSERT_ERROR(index < m_vars.size(), "Index exceeds container size: {0}", index)
    return m_vars[index];
}

template <kit::FloatingPoint Float> void state<Float>::reserve(const std::size_t capacity)
{
    m_vars.reserve(capacity);
    m_vars.reserve(capacity);
    for (std::vector<Float> &v : m_kvec)
        v.reserve(capacity);
}

template <kit::FloatingPoint Float> void state<Float>::clear()
{
    m_vars.clear();
    for (std::vector<Float> &v : m_kvec)
        v.clear();
}

template <kit::FloatingPoint Float> void state<Float>::resize_kvec_length()
{
    for (std::vector<Float> &v : m_kvec)
        v.resize(m_vars.size());
}

template <kit::FloatingPoint Float> void state<Float>::set_stages(const std::uint32_t stages)
{
    m_kvec.resize(stages);
    resize_kvec_length();
}

template <kit::FloatingPoint Float> const std::vector<Float> &state<Float>::vars() const
{
    return m_vars;
}

template <kit::FloatingPoint Float> void state<Float>::vars(const std::vector<Float> &vars)
{
    m_vars = vars;
    resize_kvec_length();
}

template <kit::FloatingPoint Float> std::size_t state<Float>::size() const
{
    return m_vars.size();
}

template class state<float>;
template class state<double>;
template class state<long double>;
} // namespace rk
