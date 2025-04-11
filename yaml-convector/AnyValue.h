#pragma once
#include <string>
#include <vector>
#include <map>
#include <any>
#include <stdexcept>
#include <iostream>

// ǰ������
class AnyMap;

// ���ڴ�������쳣��
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

// �ܹ��洢��������ֵ��������
class AnyValue
{
public:
    // ���캯��
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

    // ���ͼ�鷽��
    bool isString() const { return m_value.type() == typeid(std::string); }
    bool isDouble() const { return m_value.type() == typeid(double); }
    bool isInt() const { return m_value.type() == typeid(long int); }
    bool isBool() const { return m_value.type() == typeid(bool); }
    bool isVector() const;
    bool isMap() const;
    bool isScalar() const { return isString() || isDouble() || isInt() || isBool(); }

    // ֵ���ʷ���
    const std::string& asString() const;
    double asDouble() const;
    long int asInt() const;
    bool asBool() const;
    const std::vector<double>& asVector() const;
    const std::vector<std::string>& asStringVector() const;
    const std::vector<AnyValue>& asValueVector() const;
    const AnyMap& asMap() const;

    // ��ȡ������Ϣ
    std::string type() const;

    // λ����Ϣ (���ڴ��󱨸�)
    void setLocation(int line, int column) {
        m_line = line;
        m_column = column;
    }

    int getLine() const { return m_line; }
    int getColumn() const { return m_column; }

    // ���ü���
    void setKey(const std::string& key) { m_key = key; }
    const std::string& getKey() const { return m_key; }

private:
    std::string m_key; // ����ֵ�洢��AnyMap��ʱ�ļ���
    std::any m_value; // ʵ�ʴ洢��ֵ
    int m_line = -1;  // ��YAML�ļ��е��к�
    int m_column = -1; // ��YAML�ļ��е��к�
};