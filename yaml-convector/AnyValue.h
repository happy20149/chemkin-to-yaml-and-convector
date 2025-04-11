#pragma once
#include <string>
#include <vector>
#include <map>
#include <any>
#include <stdexcept>
#include <iostream>


class AnyMap;

//用于错误处理的自定义异常类  这是一个自定义异常类，继承自 std::runtime_error：
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

//这是核心的数据容器类，能够存储多种不同类型的数据：
class AnyValue
{
public:
    
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

    
    bool isString() const { return m_value.type() == typeid(std::string); }
    bool isDouble() const { return m_value.type() == typeid(double); }
    bool isInt() const { return m_value.type() == typeid(long int); }
    bool isBool() const { return m_value.type() == typeid(bool); }
    bool isVector() const;
    bool isMap() const;
    bool isScalar() const { return isString() || isDouble() || isInt() || isBool(); }

    
    const std::string& asString() const;
    double asDouble() const;
    long int asInt() const;
    bool asBool() const;
    const std::vector<double>& asVector() const;
    const std::vector<std::string>& asStringVector() const;
    const std::vector<AnyValue>& asValueVector() const;
    const AnyMap& asMap() const;

    
    std::string type() const;

   //// 设置YAML节点在原文件中的位置信息
    void setLocation(int line, int column) {
        m_line = line;
        m_column = column;
    }

    int getLine() const { return m_line; }
    int getColumn() const { return m_column; }

   //// 设置YAML节点的键名
    void setKey(const std::string& key) { m_key = key; }
    const std::string& getKey() const { return m_key; }

private:
    std::string m_key; 
    std::any m_value; 
    int m_line = -1;  
    int m_column = -1; 
};