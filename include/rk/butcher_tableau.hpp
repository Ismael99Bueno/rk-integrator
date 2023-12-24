#pragma once

#include <vector>
#include <cstdint>
#include "kit/interface/serialization.hpp"

namespace rk
{
template <typename T> struct butcher_tableau
{
#ifdef KIT_USE_YAML_CPP
    class serializer : public kit::serializer<butcher_tableau>
    {
      public:
        YAML::Node encode(const butcher_tableau &tb) const override;
        bool decode(const YAML::Node &node, butcher_tableau &tb) const override;
    };
#endif

    butcher_tableau() = default;
    butcher_tableau(const std::vector<T> &alpha, const std::vector<std::vector<T>> &beta, const std::vector<T> &coefs,
                    std::uint16_t stages, std::uint16_t order);

    butcher_tableau(const std::vector<T> &alpha, const std::vector<std::vector<T>> &beta, const std::vector<T> &coefs1,
                    const std::vector<T> &coefs2, std::uint16_t stages, std::uint16_t order);

    std::vector<T> alpha;
    std::vector<T> coefs1;
    std::vector<T> coefs2;
    std::vector<std::vector<T>> beta;

    bool embedded;
    std::uint16_t stages;
    std::uint16_t order;

    static const butcher_tableau rk1;
    static const butcher_tableau rk2;
    static const butcher_tableau rk4;
    static const butcher_tableau rk38;
    static const butcher_tableau rkf12;
    static const butcher_tableau rkf45;
    static const butcher_tableau rkfck45;
    static const butcher_tableau rkf78;

  private:
};
} // namespace rk
