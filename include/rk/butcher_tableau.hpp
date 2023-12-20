#pragma once

#include <vector>
#include <cstdint>
#include "kit/interface/serialization.hpp"

namespace rk
{
class butcher_tableau
{
  public:
#ifdef KIT_USE_YAML_CPP
    class serializer : public kit::serializer<butcher_tableau>
    {
      public:
        YAML::Node encode(const butcher_tableau &tb) const override;
        bool decode(const YAML::Node &node, butcher_tableau &tb) const override;
    };
#endif

    butcher_tableau() = default;
    butcher_tableau(const std::vector<float> &alpha, const std::vector<std::vector<float>> &beta,
                    const std::vector<float> &coefs, std::uint16_t stage, std::uint16_t order);

    butcher_tableau(const std::vector<float> &alpha, const std::vector<std::vector<float>> &beta,
                    const std::vector<float> &coefs1, const std::vector<float> &coefs2, std::uint16_t stage,
                    std::uint16_t order);

    const std::vector<float> &alpha() const;
    const std::vector<std::vector<float>> &beta() const;
    const std::vector<float> &coefs() const;
    const std::vector<float> &coefs1() const;
    const std::vector<float> &coefs2() const;
    bool embedded() const;
    std::uint16_t stage() const;
    std::uint16_t order() const;

    static const butcher_tableau rk1;
    static const butcher_tableau rk2;
    static const butcher_tableau rk4;
    static const butcher_tableau rk38;
    static const butcher_tableau rkf12;
    static const butcher_tableau rkf45;
    static const butcher_tableau rkfck45;
    static const butcher_tableau rkf78;

  private:
    std::vector<float> m_alpha;
    std::vector<float> m_coefs1;
    std::vector<float> m_coefs2;
    std::vector<std::vector<float>> m_beta;

    bool m_embedded;
    std::uint16_t m_stage;
    std::uint16_t m_order;
};
} // namespace rk
