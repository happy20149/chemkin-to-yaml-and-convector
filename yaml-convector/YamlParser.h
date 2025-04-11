#pragma once
#include <string>
#include <map>
#include <vector>
#include <any>
#include <yaml-cpp/yaml.h>

// �򻯵�YAMLֵ��
class YamlValue {
public:
    // ֵ����ö��
    enum class Type {
        Null, String, Number, Boolean, Map, Sequence
    };

    // ���캯��
    YamlValue() : m_type(Type::Null) {}
    YamlValue(const std::string& value) : m_type(Type::String), m_string(value) {}
    YamlValue(double value) : m_type(Type::Number), m_number(value) {}
    YamlValue(bool value) : m_type(Type::Boolean), m_bool(value) {}

    // ��YAML::Node����
    YamlValue(const YAML::Node& node);

    // ���ͼ��
    bool isNull() const { return m_type == Type::Null; }
    bool isString() const { return m_type == Type::String; }
    bool isNumber() const { return m_type == Type::Number; }
    bool isBoolean() const { return m_type == Type::Boolean; }
    bool isMap() const { return m_type == Type::Map; }
    bool isSequence() const { return m_type == Type::Sequence; }

    // ֵ����
    std::string asString() const;
    double asNumber() const;
    bool asBoolean() const;
    const std::map<std::string, YamlValue>& asMap() const;
    const std::vector<YamlValue>& asSequence() const;

    // �������
    void print(int indent = 0) const;

private:
    Type m_type;
    std::string m_string;
    double m_number = 0.0;
    bool m_bool = false;
    std::map<std::string, YamlValue> m_map;
    std::vector<YamlValue> m_sequence;
};

// YAML��������
class YamlParser {
public:
    // ���ļ�����YAML
    static YamlValue loadFile(const std::string& filename);

    // ���ַ�������YAML
    static YamlValue loadString(const std::string& yaml);
};