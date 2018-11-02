#include <iostream>

class ZZ
{
public:
    int operator()(const int &value)
    {
        std::cout << value << std::endl;
        aaa = value - 10;
        return value*2;
    }
    int operator[](int value)
    {
        return value+aaa;
    }

    ZZ& operator=(const ZZ& m);

    int aaa;
};

ZZ& ZZ::operator=(const ZZ& m)
{
    aaa = (m.aaa - 6)*2;

    return *this;   //连续（链式）赋值操作  ====>>  a=b=c=d //
}

int main()
{
    ZZ a;
    std::cout << a(56) << std::endl;
    std::cout << a.aaa << std::endl;

    ZZ c;
    ZZ b;
    c = b = a;
    std::cout << b.aaa << std::endl;
    std::cout << c.aaa << std::endl;

    ZZ k;
    k(20);
    std::cout << k.aaa << std::endl;
    std::cout << k[6] << std::endl;
}