#pragma once
#include <string>
#include <map>
#include <vector>
#include <any>
#include <yaml-cpp/yaml.h>

// 简化的YAML值类
class YamlValue {
public:
    // 值类型枚举
    enum class Type {
        Null, String, Number, Boolean, Map, Sequence
    };

    // 构造函数
    YamlValue() : m_type(Type::Null) {}
    YamlValue(const std::string& value) : m_type(Type::String), m_string(value) {}
    YamlValue(double value) : m_type(Type::Number), m_number(value) {}
    YamlValue(bool value) : m_type(Type::Boolean), m_bool(value) {}

    // 从YAML::Node构造
    YamlValue(const YAML::Node& node);

    // 类型检查
    bool isNull() const { return m_type == Type::Null; }
    bool isString() const { return m_type == Type::String; }
    bool isNumber() const { return m_type == Type::Number; }
    bool isBoolean() const { return m_type == Type::Boolean; }
    bool isMap() const { return m_type == Type::Map; }
    bool isSequence() const { return m_type == Type::Sequence; }

    // 值访问
    std::string asString() const;
    double asNumber() const;
    bool asBoolean() const;
    const std::map<std::string, YamlValue>& asMap() const;
    const std::vector<YamlValue>& asSequence() const;

    // 调试输出
    void print(int indent = 0) const;

private:
    Type m_type;
    std::string m_string;
    double m_number = 0.0;
    bool m_bool = false;
    std::map<std::string, YamlValue> m_map;
    std::vector<YamlValue> m_sequence;
};

// YAML解析器类
class YamlParser {
public:
    // 从文件加载YAML
    static YamlValue loadFile(const std::string& filename);

    // 从字符串加载YAML
    static YamlValue loadString(const std::string& yaml);
};