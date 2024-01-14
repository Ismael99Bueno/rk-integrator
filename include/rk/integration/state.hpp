#pragma once

#include "kit/utility/type_constraints.hpp"
#include <vector>

namespace rk
{
template <kit::FloatingPoint Float> class state
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

    const std::vector<Float> &vars() const;
    void vars(const std::vector<Float> &vars);

    std::size_t size() const;

  private:
    void resize_kvec_length();
    void set_stages(std::uint32_t stages);

    std::vector<Float> m_vars;
    std::vector<std::vector<Float>> m_kvec;

    template <kit::FloatingPoint U> friend class integrator;
};
} // namespace rk
