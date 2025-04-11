#pragma once
#include <string>
#include <vector>
#include <map>
#include <any>
#include <stdexcept>
#include <iostream>

// 前向声明
class AnyMap;

// 用于错误处理的异常类
class CanteraError : public std::runtime_error
{
public:
    CanteraError(const std::string& procedure, const std::string& msg)
        : std::runtime_error(procedure + ": " + msg), m_procedure(procedure), m_msg(msg) {}

    const std::string& procedure() const { return m_procedure; }
    const std::string& getMessage() const { return m_msg; }

private:
    std::string m_procedure;
    std::string m_msg;
};

// 能够存储任意类型值的容器类
class AnyValue
{
public:
    // 构造函数
    AnyValue() = default;
    AnyValue(const std::string& value) : m_value(value) {}
    AnyValue(double value) : m_value(value) {}
    AnyValue(int value) : m_value(static_cast<long int>(value)) {}
    AnyValue(long int value) : m_value(value) {}
    AnyValue(bool value) : m_value(value) {}
    AnyValue(const std::vector<double>& value) : m_value(value) {}
    AnyValue(const std::vector<std::string>& value) : m_value(value) {}
    AnyValue(const std::vector<AnyValue>& value) : m_value(value) {}
    AnyValue(const AnyMap& value);

    // 类型检查方法
    bool isString() const { return m_value.type() == typeid(std::string); }
    bool isDouble() const { return m_value.type() == typeid(double); }
    bool isInt() const { return m_value.type() == typeid(long int); }
    bool isBool() const { return m_value.type() == typeid(bool); }
    bool isVector() const;
    bool isMap() const;
    bool isScalar() const { return isString() || isDouble() || isInt() || isBool(); }

    // 值访问方法
    const std::string& asString() const;
    double asDouble() const;
    long int asInt() const;
    bool asBool() const;
    const std::vector<double>& asVector() const;
    const std::vector<std::string>& asStringVector() const;
    const std::vector<AnyValue>& asValueVector() const;
    const AnyMap& asMap() const;

    // 获取类型信息
    std::string type() const;

    // 位置信息 (用于错误报告)
    void setLocation(int line, int column) {
        m_line = line;
        m_column = column;
    }

    int getLine() const { return m_line; }
    int getColumn() const { return m_column; }

    // 设置键名
    void setKey(const std::string& key) { m_key = key; }
    const std::string& getKey() const { return m_key; }

private:
    std::string m_key; // 当此值存储在AnyMap中时的键名
    std::any m_value; // 实际存储的值
    int m_line = -1;  // 在YAML文件中的行号
    int m_column = -1; // 在YAML文件中的列号
};