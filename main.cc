#include <iostream>

#include "chain.h"
#include "observable.h"

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

    auto ob = mut1
        // mut1  + 1
        >> [](int n) { return n + 1; } 
        // (mut1 + 1) * 2
        >> [](int n) { return n * 2; }
        // (mut1 + 1) * 2 * mut2
        >> [mut2](int n) { return mut2 >> [n](int m) { return m * n; }; };


    // (1 + 1) * 2 * 1 = 4
    std::cout << ob.Value() << std::endl;

    auto unob = ob.Observe([](int val, int valOld) {
        std::cout << valOld << " -> " << val << std::endl;
    });

    // (2 + 1) * 2 * 1 = 6
    mut1.Update(2);

    // (2 + 1) * 2 * 3 = 18
    mut2.Update(3);

    std::cout << ob.Value() << std::endl;
}

int main(int argc, const char *argv[])
{
    runObservable();
    return 0;
}
