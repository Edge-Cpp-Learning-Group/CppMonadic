#include <iostream>

#include "chain.h"
#include "behavior.h"

auto getSingleExec() {
    return SingleExecution([] {
        std::cout << "Executed" << std::endl;
    });
}

void runSingleExec() {
    auto exec = getSingleExec();
    std::cout << "Mark" << std::endl;
}

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
    rm1();
    printChain(chain);
}

void runBehavior() {
    Behavior<int> beh(1);

    auto [res1, unob1] = beh([](std::optional<int> legacy) { std::cout << "Notified 1" << std::endl; });

    std::cout << res1 << std::endl;

    auto [res2, unob2] = beh([](std::optional<int> legacy) { std::cout << "Notified 2" << std::endl; });

    std::cout << res2 << std::endl;

    // beh.Notify();

    unob1();

    // beh.Notify();
}

int main(int argc, const char *argv[])
{
    runBehavior();
    return 0;
}
