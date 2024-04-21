#pragma once
#include <functional>
#include <map>

template<class T>
class Config
{
public:
    Config(const T& val);
    typedef std::function<void(const T& oldVal, const T& newVal)> on_change_func;
    void AddListener(on_change_func cb);
    void SetVal(const T& newVal);
    const T& GetVal() {return m_val;}
private:
    std::map<int, on_change_func> m_cbs;
    T m_val;
};

template<class T>
Config<T>::Config(const T& val): m_val(val)
{
}

template<class T>
void Config<T>::AddListener(on_change_func cb)
{
    static int no = 0;
    m_cbs[no++] = cb;
}

template<class T>
void Config<T>::SetVal(const T& newVal)
{
    if (newVal == m_val) {
        return ;
    }
    for (auto& i : m_cbs) {
        i.second(m_val, newVal);
    }
    m_val = newVal;
}

/* template<class T>
void Config<T>::GetVal()
{
    
} */

