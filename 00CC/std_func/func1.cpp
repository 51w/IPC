#include <iostream>
#include <functional>
#include <memory>
 
class classA
{
typedef std::function<void(int i)> callback_t;
 
public:
    classA() {}
    ~classA() {}
    
    void handle(int i)
    {
        std::cout << "classA::handle" << std::endl;
        
        cbHandle(i);
    }
 
    void registCb(callback_t func)
    {cbHandle = std::move(func);}
private:
    callback_t cbHandle;
};
 
class classB
{
public:
    classB(classA& cA) 
    {       
        cA.registCb([this](int i){this->classB::handle(i);});
    }
    ~classB() {}
    
    void handle(int i)
    {
        std::cout << "classB, handle message" << i << std::endl;
    }
};
 
int main()
{
    classA testa;
    classB testb(testa);
 
    testa.handle(10);
}




// #include <iostream>
// #include <functional>
// using namespace std;

// auto g_Minus = [](int i, int j){ return i - j; };

// int main()
// {
//     function<int(int, int)> f = g_Minus;
//     cout << f(1, 5) << endl;                                            // -1
//     return 1;
// }








// #include <iostream>
// #include <functional>
// using namespace std;

// int g_Minus(int i, int j)
// {
//     return i - j;
// }

// int main()
// {
//     function<int(int, int)> f = g_Minus;
//     cout << f(1, 2) << endl;                                            // -1
//     return 1;
// }





/* #include <iostream>
#include <functional>
using namespace std;

template <class T>
T g_Minus(T i, T j)
{
    return i - j;
}

int main()
{
    function<int(int, int)> f = g_Minus<int>;
    cout << f(1, 3) << endl;                                            // -1
    return 1;
} */




/* #include <iostream>
#include <functional>
using namespace std;

struct Minus
{
    int operator() (int i, int j)
    {
        return i - j;
    }
};

int main()
{
    function<int(int, int)> f = Minus();
    cout << f(1, 6) << endl;                                            // -1
    return 1;
} */


/* #include <iostream>
#include <functional>
using namespace std;

template <class T>
struct Minus
{
    T operator() (T i, T j)
    {
        return i - j;
    }
};

int main()
{
    function<int(int, int)> f = Minus<int>();
    cout << f(1, 7) << endl;                                            // -1
    return 1;
} */

/* #include <iostream>
#include <functional>
using namespace std;

class Math
{
public:
    static int Minus(int i, int j)
    {
        return i - j;
    }
};

int main()
{
    function<int(int, int)> f = &Math::Minus;
    cout << f(1, 8) << endl;                                            // -1
    return 1;
} */

/* #include <iostream>
#include <functional>
using namespace std;

class Math
{
public:
    template <class T>
    static T Minus(T i, T j)
    {
        return i - j;
    }
};

int main()
{
    function<int(int, int)> f = &Math::Minus<int>;
    cout << f(1, 9) << endl;                                            // -1
    return 1;
} */




/* #include <iostream>
#include <functional>
using namespace std;

class Math
{
public:
    int Minus(int i, int j)
    {
        return i - j;
    }
};

int main()
{
    Math m;
    function<int(int, int)> f = bind(&Math::Minus, &m, placeholders::_1, placeholders::_2);
    cout << f(1, 3) << endl;                                            // -1
    return 1;
} */





// #include <iostream>
// #include <functional>
// using namespace std;

// class Math
// {
// public:
//     template <class T>
//     T Minus(T i, T j)
//     {
//         return i - j;
//     }
// };

// int main()
// {
//     Math m;
//     function<int(int, int)> f = bind(&Math::Minus<int>, &m, placeholders::_1, placeholders::_2);
//     cout << f(1, 4) << endl;                                            // -1
//     return 1;
// }