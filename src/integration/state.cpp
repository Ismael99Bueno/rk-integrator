#include "rk/internal/pch.hpp"
#include "rk/integration/state.hpp"

namespace rk
{
template <std::floating_point Float>
state<Float>::state(const std::vector<Float> &vars, const std::uint32_t stages)
    : m_vars(vars), m_kvec(stages), m_stages(stages)
{
    resize_kvecs();
}

template <std::floating_point Float> void state<Float>::push_back(const Float elm)
{
    m_vars.push_back(elm);
    resize_kvecs();
}

template <std::floating_point Float> void state<Float>::append(std::initializer_list<Float> lst)
{
    m_vars.insert(m_vars.end(), lst);
    resize_kvecs();
}

template <std::floating_point Float> void state<Float>::resize(const std::size_t size)
{
    m_vars.resize(size);
    resize_kvecs();
}

template <std::floating_point Float> Float state<Float>::operator[](std::size_t index) const
{
    KIT_ASSERT_ERROR(index < m_vars.size(), "Index exceeds container size: {0}", index)
    return m_vars[index];
}
template <std::floating_point Float> Float &state<Float>::operator[](std::size_t index)
{
    KIT_ASSERT_ERROR(index < m_vars.size(), "Index exceeds container size: {0}", index)
    return m_vars[index];
}

template <std::floating_point Float>
Float state<Float>::operator()(const std::uint32_t stage, const std::size_t index) const
{
    KIT_ASSERT_ERROR(stage < m_stages, "Stage exceeds container size: {0}", stage)
    KIT_ASSERT_ERROR(index < m_vars.size(), "Index exceeds container size: {0}", index)
    return m_kvec[stage * m_vars.size() + index];
}

template <std::floating_point Float> Float &state<Float>::operator()(const std::uint32_t stage, const std::size_t index)
{
    KIT_ASSERT_ERROR(stage < m_stages, "Stage exceeds container size: {0}", stage)
    KIT_ASSERT_ERROR(index < m_vars.size(), "Index exceeds container size: {0}", index)
    return m_kvec[stage * m_vars.size() + index];
}

template <std::floating_point Float> void state<Float>::reserve(const std::size_t capacity)
{
    m_vars.reserve(capacity);
    m_kvec.reserve(capacity * m_stages);
}

template <std::floating_point Float> void state<Float>::clear()
{
    m_vars.clear();
    m_kvec.clear();
}

template <std::floating_point Float> void state<Float>::resize_kvecs()
{
    m_kvec.resize(m_stages * m_vars.size());
}

template <std::floating_point Float> std::uint32_t state<Float>::stages() const
{
    return m_stages;
}
template <std::floating_point Float> void state<Float>::stages(const std::uint32_t stages)
{
    m_stages = stages;
    resize_kvecs();
}

template <std::floating_point Float> const std::vector<Float> &state<Float>::vars() const
{
    return m_vars;
}

template <std::floating_point Float> void state<Float>::vars(const std::vector<Float> &vars)
{
    m_vars = vars;
    resize_kvecs();
}

template <std::floating_point Float> std::size_t state<Float>::size() const
{
    return m_vars.size();
}

template class state<float>;
template class state<double>;
template class state<long double>;
} // namespace rk
