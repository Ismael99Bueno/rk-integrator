#ifndef STATE_HPP
#define STATE_HPP

#include <vector>
#ifdef YAML_CPP_COMPAT
#include <yaml-cpp/yaml.h>
#endif

namespace rk
{
class state
{
  public:
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
#ifdef YAML_CPP_COMPAT
    friend YAML::Emitter &operator<<(YAML::Emitter &, const state &);
    friend struct YAML::convert<state>;
#endif
};
#ifdef YAML_CPP_COMPAT
YAML::Emitter &operator<<(YAML::Emitter &out, const state &st);
#endif
} // namespace rk

#ifdef YAML_CPP_COMPAT
namespace YAML
{
template <> struct convert<rk::state>
{
    static Node encode(const rk::state &st);
    static bool decode(const Node &node, rk::state &st);
};
} // namespace YAML
#endif
#endif