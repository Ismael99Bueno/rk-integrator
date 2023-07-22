#ifndef RK_STATE_HPP
#define RK_STATE_HPP

#include <vector>
#include "kit/interface/serialization.hpp"

namespace rk
{
class state
{
  public:
#ifdef KIT_USE_YAML_CPP
    class serializer : public kit::serializer<state>
    {
      public:
        YAML::Node encode(const state &tb) const override;
        bool decode(const YAML::Node &node, state &tb) const override;
    };
#endif

    state() = default;
    state(const std::vector<float> &vars, std::uint16_t stage);

    void push_back(float elm);
    void append(std::initializer_list<float> lst);
    void resize(std::size_t size);

    void reserve(std::size_t capacity);
    void clear();

    const float &operator[](std::size_t index) const;
    float &operator[](std::size_t index);

    const std::vector<float> &vars() const;
    void vars(const std::vector<float> &vars);

    std::size_t size() const;

  private:
    void resize();
    void resize_kvec(std::uint16_t stage);

    std::vector<float> m_vars;
    std::vector<std::vector<float>> m_kvec;

    friend class integrator;
};
} // namespace rk

#endif