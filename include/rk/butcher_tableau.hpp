#ifndef BUTCHER_TABLEAU_HPP
#define BUTCHER_TABLEAU_HPP

#include "rk/core.hpp"
#include <vector>
#include <cstdint>
#ifdef HAS_YAML_CPP
#include <yaml-cpp/yaml.h>
#endif

namespace rk
{
    class butcher_tableau
    {
    private:
    public:
        butcher_tableau() = default;

        butcher_tableau(const std::vector<float> &alpha,
                        const std::vector<std::vector<float>> &beta,
                        const std::vector<float> &coefs,
                        std::uint16_t stage,
                        std::uint16_t order);

        butcher_tableau(const std::vector<float> &alpha,
                        const std::vector<std::vector<float>> &beta,
                        const std::vector<float> &coefs1,
                        const std::vector<float> &coefs2,
                        std::uint16_t stage,
                        std::uint16_t order);

        const std::vector<float> &alpha() const;
        const std::vector<std::vector<float>> &beta() const;
        const std::vector<float> &coefs() const;
        const std::vector<float> &coefs1() const;
        const std::vector<float> &coefs2() const;
        bool embedded() const;
        std::uint16_t stage() const;
        std::uint16_t order() const;

    private:
        std::vector<float> m_alpha, m_coefs1, m_coefs2;
        std::vector<std::vector<float>> m_beta;
        bool m_embedded;
        std::uint16_t m_stage, m_order;
    };

#ifdef HAS_YAML_CPP
    YAML::Emitter &operator<<(YAML::Emitter &out, const butcher_tableau &bt);
#endif
}

#ifdef HAS_YAML_CPP
namespace YAML
{
    template <>
    struct convert<rk::butcher_tableau>
    {
        static Node encode(const rk::butcher_tableau &bt);
        static bool decode(const Node &node, rk::butcher_tableau &bt);
    };
}
#endif
#endif