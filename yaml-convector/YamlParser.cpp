#include "YamlParser.h"
#include <iostream>
#include <fstream>

// 将YAML::Node转换为YamlValue
YamlValue::YamlValue(const YAML::Node& node) {
    if (node.IsNull()) {
        m_type = Type::Null;
    }
    else if (node.IsScalar()) {
        // 解析标量值
        std::string value = node.Scalar();

        // 尝试解析为布尔值
        if (value == "true" || value == "yes" || value == "True") {
            m_type = Type::Boolean;
            m_bool = true;
        }
        else if (value == "false" || value == "no" || value == "False") {
            m_type = Type::Boolean;
            m_bool = false;
        }
        else {
            // 尝试解析为数字
            try {
                m_number = std::stod(value);
                m_type = Type::Number;
            }
            catch (...) {
                // 默认为字符串
                m_type = Type::String;
                m_string = value;
            }
        }
    }
    else if (node.IsMap()) {
        // 解析Map
        m_type = Type::Map;
        for (const auto& kv : node) {
            std::string key = kv.first.Scalar();
            m_map[key] = YamlValue(kv.second);
        }
    }
    else if (node.IsSequence()) {
        // 解析序列
        m_type = Type::Sequence;
        for (const auto& item : node) {
            m_sequence.push_back(YamlValue(item));
        }
    }
}

std::string YamlValue::asString() const {
    if (!isString()) {
        throw std::runtime_error("Value is not a string");
    }
    return m_string;
}

double YamlValue::asNumber() const {
    if (!isNumber()) {
        throw std::runtime_error("Value is not a number");
    }
    return m_number;
}

bool YamlValue::asBoolean() const {
    if (!isBoolean()) {
        throw std::runtime_error("Value is not a boolean");
    }
    return m_bool;
}

const std::map<std::string, YamlValue>& YamlValue::asMap() const {
    if (!isMap()) {
        throw std::runtime_error("Value is not a map");
    }
    return m_map;
}

const std::vector<YamlValue>& YamlValue::asSequence() const {
    if (!isSequence()) {
        throw std::runtime_error("Value is not a sequence");
    }
    return m_sequence;
}

void YamlValue::print(int indent) const {
    std::string spaces(indent * 2, ' ');

    switch (m_type) {
    case Type::Null:
        std::cout << spaces << "null" << std::endl;
        break;
    case Type::String:
        std::cout << spaces << "\"" << m_string << "\"" << std::endl;
        break;
    case Type::Number:
        std::cout << spaces << m_number << std::endl;
        break;
    case Type::Boolean:
        std::cout << spaces << (m_bool ? "true" : "false") << std::endl;
        break;
    case Type::Map:
        std::cout << spaces << "{" << std::endl;
        for (const auto& [key, value] : m_map) {
            std::cout << spaces << "  " << key << ": ";
            value.print(indent + 1);
        }
        std::cout << spaces << "}" << std::endl;
        break;
    case Type::Sequence:
        std::cout << spaces << "[" << std::endl;
        for (const auto& value : m_sequence) {
            std::cout << spaces << "  - ";
            value.print(indent + 1);
        }
        std::cout << spaces << "]" << std::endl;
        break;
    }
}

YamlValue YamlParser::loadFile(const std::string& filename) {
    try {
        YAML::Node rootNode = YAML::LoadFile(filename);
        return YamlValue(rootNode);
    }
    catch (const YAML::Exception& e) {
        throw std::runtime_error("YAML parsing error: " + std::string(e.what()));
    }
}

YamlValue YamlParser::loadString(const std::string& yaml) {
    try {
        YAML::Node rootNode = YAML::Load(yaml);
        return YamlValue(rootNode);
    }
    catch (const YAML::Exception& e) {
        throw std::runtime_error("YAML parsing error: " + std::string(e.what()));
    }
}