#include <iostream>

#include "chain.h"
#include "observable.h"

template <typename T>
void printChain(const Chain<T> &chain) {
    chain.ForEach([](const T &value) {
        std::cout << value << ", ";
    });
    std::cout << std::endl;
}

void runChain() {
    Chain<int> chain;
    auto rm1 = chain.Add(1);
    auto rm2 = chain.Add(2);

    printChain(chain);
    rm1.Run();
    printChain(chain);
}

void runObservable() {
    Mutable<int> mut1(1);
    Mutable<int> mut2(1);

    auto ob1 = mut1
        // mut1  + 1
        >> [](int n) { return n + 1; } 
        // (mut1 + 1) * 2
        >> [](int n) { return n * 2; }
        // (mut1 + 1) * 2 * mut2
        >> [mut2](int n) { return mut2 >> [n](int m) { return m * n; }; };


    // (1 + 1) * 2 * 1 = 4
    std::cout << ob1.Value() << std::endl;

    auto unob1 = ob1.Observe([](int val, int valOld) {
        std::cout << "ob1: " << valOld << " -> " << val << std::endl;
    });

    // (2 + 1) * 2 * 1 = 6
    mut1.Update(2);

    // (2 + 1) * 2 * 3 = 18
    mut2.Update(3);

    std::cout << ob1.Value() << std::endl;

    mut1.Update(3);

    Mutable<int> mut3(1);

    auto ob3 = mut1 >> [=](int x) {
        return mut2 >> [=](int y) {
            return mut3 >> [=](int z) {
                return x + y + z;
            };
        };
    };

    auto unob3 = ob3.Observe([](int val, int valOld) {
        std::cout << "ob3: " << valOld << " -> " << val << std::endl;
    });

    mut1.Update(4);
    mut2.Update(5);
    mut3.Update(6);
}

int main(int argc, const char *argv[])
{
    runObservable();
    return 0;
}
