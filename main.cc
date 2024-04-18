#include <iostream>

#include "chain.h"
#include "observable.h"

void runChain() {
    Chain<int> chain;
    auto rm1 = chain.Add(1);
    auto rm2 = chain.Add(2);

    printChain(chain);
    rm1->Run();
    printChain(chain);
}

void runObservable() {
    Mutable<int> sub(1);

    std::cout << sub.Value() << std::endl;

    auto unob = sub.Observe([](int val, int valOld) {
        std::cout << valOld << " -> " << val << std::endl;
    });

    sub.Update(2);
    sub.Update(3);

    unob->Run();

    sub.Update(4);

    std::cout << sub.Value() << std::endl;

    auto ob = sub
        .map([](int n) { return n + 1; })
        .map([](int n) { return n * 2; }) 
        .bind([](int n) { return Observable<int>(n + 1); });


    std::cout << ob.Value() << std::endl;

    auto unob2 = ob.Observe([](int val, int valOld) {
        std::cout << valOld << " -> " << val << std::endl;
    });

    sub.Update(5);

    std::cout << ob.Value() << std::endl;
}

int main(int argc, const char *argv[])
{
    runObservable();
    return 0;
}
