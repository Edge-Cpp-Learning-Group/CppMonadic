#include <iostream>

#include "chain.h"
#include "observable.h"

void runChain() {
    Chain<int> chain;
    auto rm1 = chain.Add(1);
    auto rm2 = chain.Add(2);

    printChain(chain);
    rm1();
    printChain(chain);
}

void runObservable() {
    Subject<int> sub(1);

    int num = 0;
    auto unob = sub.Observe([&num](std::optional<int> val) {
        if (val.has_value()) {
            num = val.value();
        }
    });

    std::cout << num << std::endl;

    sub.Update(2);

    std::cout << num << std::endl;

    unob();
    sub.Update(3);

    std::cout << num << std::endl;

    auto mp1 = sub | [](int val) { return val + 1; };
    auto mp2 = mp1 | [](int val) { return val * 2; };
    // auto mp2 = sub | [](int val) { return val + 1; }
    //             | [](int val) { return val * 2; };

    auto unob2 = mp2.Observe([&num](std::optional<int> val) {
        if (val.has_value()) {
            num = val.value();
        }
    });

    std::cout << num << std::endl;

    sub.Update(4);
    std::cout << num << std::endl;

    unob2();
    sub.Update(5);

    std::cout << num << std::endl;
}

int main(int argc, const char *argv[])
{
    runObservable();
    return 0;
}
