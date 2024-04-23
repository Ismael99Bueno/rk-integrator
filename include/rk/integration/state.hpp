#pragma once

#include "kit/utility/type_constraints.hpp"
#include <vector>

namespace rk
{
template <std::floating_point Float> class state
{
  public:
    state() = default;
    state(const std::vector<Float> &vars, std::uint32_t stages);

    void push_back(Float elm);
    void append(std::initializer_list<Float> lst);

    void resize(std::size_t size);
    void reserve(std::size_t capacity);

    void clear();

    Float operator[](std::size_t index) const;
    Float &operator[](std::size_t index);

    Float operator()(std::uint32_t stage, std::size_t index) const;
    Float &operator()(std::uint32_t stage, std::size_t index);

    const std::vector<Float> &vars() const;
    void vars(const std::vector<Float> &vars);

    std::size_t size() const;
    std::uint32_t stages() const;
    void stages(std::uint32_t stages);

  private:
    void resize_kvecs();

    std::vector<Float> m_vars;
    std::vector<Float> m_kvec;
    std::uint32_t m_stages;

    template <std::floating_point U> friend class integrator;
};
} // namespace rk
