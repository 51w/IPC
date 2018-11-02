#include <iostream>
#include <functional>

//函数指针写法
typedef int(*FuncPoint)(const int&);

std::function<int(const int &)> Function;

//普通函数
int FuncDemo(const int &value)
{
    std::cout << "普通函数\t\t:" << value << "\n";
    return value;
}

//lamda函数
auto lmdFuc = [](const int &value) -> int
{
    std::cout << "lamda函数\t\t:" << value << "\n";
    return value;
};

//仿函数
class Functor
{
    public:
    int operator()(const int &value)
    {
        std::cout << "仿函数\t\t\t:" << value << "\n";
        return value;
    }
};

//类内静态函数和类成员函数
class Testclass
{
    public:
    int TestDemo(const int &value)
    {
        std::cout << "类内成员函数\t\t:" << value << "\n";
        return value;
    }

    static int TestStatic(const int &value)
    {
        std::cout << "类内静态成员函数\t:" << value << "\n";
        return value;
    }
};


int main()
{
    //以往函数指针调用
    FuncPoint Func(FuncDemo);
    Func(10);
    //普通函数
    Function = FuncDemo;
    Function(10);

    //lamda函数
    Function = lmdFuc;
    Function(20);

    //仿函数（函数对象）
    Functor Fobj;
    Function=Fobj;
    Function(30);

    //类内成员函数(需要bind())
    Testclass testclass;
    Function=std::bind(&Testclass::TestDemo,testclass,std::placeholders::_1);
    Function(40);

    //类内静态成员函数
    Function=Testclass::TestStatic;
    Function(50);


    return 0;
}