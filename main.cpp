#include <iostream>
class A
{
private:
    char *c;
public:
    A()
    {
        std::cout << "constructor" << std::endl;
    }
    ~A()
    {
        std::cout << "Desconstructor" << std::endl;
    }
};

int main()
{
    A *a;
    a = new A();
}