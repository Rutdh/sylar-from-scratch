#pragma once
#include <string>

class Data{
private:
    std::string m_data;
public:
    void setData(const std::string& oldVal, const std::string& newVal);
    std::string& getData() {return m_data;}
};