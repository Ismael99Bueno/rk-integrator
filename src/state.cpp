#include "rk/pch.hpp"
#include "rk/state.hpp"

namespace rk
{
    state::state(const std::vector<float> &vars, const std::uint16_t stage) : m_vars(vars),
                                                                              m_kvec(stage)
    {
        resize();
    }

    void state::push_back(const float elm)
    {
        m_vars.push_back(elm);
        resize();
    }

    void state::append(std::initializer_list<float> lst)
    {
        m_vars.insert(m_vars.end(), lst);
        resize();
    }

    void state::resize(const std::size_t size)
    {
        m_vars.resize(size);
        resize();
    }

    const float &state::operator[](std::size_t index) const { return m_vars[index]; }
    float &state::operator[](std::size_t index) { return m_vars[index]; }

    void state::reserve(const std::size_t capacity)
    {
        m_vars.reserve(capacity);
        m_vars.reserve(capacity);
        for (std::vector<float> &v : m_kvec)
            v.reserve(capacity);
    }

    void state::clear()
    {
        m_vars.clear();
        m_step.clear();
        for (std::vector<float> &v : m_kvec)
            v.clear();
    }

    void state::resize()
    {
        for (std::vector<float> &v : m_kvec)
            v.resize(m_vars.size());
        m_step.resize(m_vars.size());
    }

    void state::resize_kvec(const std::uint16_t stage)
    {
        m_kvec.resize(stage);
        resize();
    }

    const std::vector<float> &state::vars() const { return m_vars; }
    const std::vector<float> &state::step() const { return m_step; }

    void state::vars(const std::vector<float> &vars)
    {
        m_vars = vars;
        resize();
    }

    std::size_t state::size() const { return m_vars.size(); }
#ifdef HAS_YAML_CPP
    YAML::Emitter &operator<<(YAML::Emitter &out, const state &st)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "vars" << YAML::Value << YAML::Flow << st.vars();
        out << YAML::Key << "step" << YAML::Value << YAML::Flow << st.step();
        out << YAML::Key << "k_vec" << YAML::Value << YAML::BeginSeq;
        for (const auto &v : st.m_kvec)
            out << YAML::Flow << v;
        out << YAML::EndSeq << YAML::EndMap;
        return out;
    }
#endif
}

#ifdef HAS_YAML_CPP
namespace YAML
{
    Node convert<rk::state>::encode(const rk::state &st)
    {
        Node node;
        node["vars"] = st.vars();
        node["step"] = st.step();
        node["vars"].SetStyle(YAML::EmitterStyle::Flow);
        node["step"].SetStyle(YAML::EmitterStyle::Flow);
        for (const auto &v : st.m_kvec)
        {
            Node child;
            child = v;
            child.SetStyle(YAML::EmitterStyle::Flow);
            node["k_vec"].push_back(v);
        }
        return node;
    }
    bool convert<rk::state>::decode(const Node &node, rk::state &st)
    {
        if (!node.IsMap() || node.size() != 3)
            return false;

        std::vector<float> vars, step;
        std::vector<std::vector<float>> k_vec;
        for (const auto &n1 : node["k_vec"])
        {
            auto &v1 = k_vec.emplace_back();
            for (const auto &n2 : n1)
                v1.push_back(n2.as<float>());
        }
        for (const auto &n : node["vars"])
            vars.push_back(n.as<float>());
        for (const auto &n : node["step"])
            step.push_back(n.as<float>());
        st.m_vars = vars;
        st.m_step = step;
        st.m_kvec = k_vec;

        return true;
    };
}
#endif