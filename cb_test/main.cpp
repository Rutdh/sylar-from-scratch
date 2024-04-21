#include "config.h"
#include "data.h"
#include <iostream>

using namespace std;


int main()
{
    Data data;
    Config<string> config {"hello"};
    config.AddListener([&](const string& oldVal, const string& newVal)->void {
        data.setData(oldVal, newVal);
        return ;
    });

    cout << config.GetVal() << endl;
    config.SetVal("world");
    cout << config.GetVal() << endl;

    return 0;
}