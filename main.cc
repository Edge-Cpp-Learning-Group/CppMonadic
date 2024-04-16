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

void runChain() {
    Chain<int> chain;
    auto rm1 = chain.Add(1);
    auto rm2 = chain.Add(2);

    printChain(chain);
    rm1();
    printChain(chain);
}

void runBehavior() {
    Subject<int> sub(1);

    auto [res1, unob1] = sub([](std::optional<int> legacy) { std::cout << "Notified 1" << std::endl; });

    std::cout << res1 << std::endl;

    std::optional<Behavior<int>::Canceller> unob3(std::nullopt);

    auto [res2, unob2] = sub([&sub, &unob3](std::optional<int> legacy) mutable {
        std::cout << "Notified 2" << std::endl;
        auto [res3, unob3_] = sub([](std::optional<int> legacy2) {
            std::cout << "Notified 3" << std::endl;
        });
        std::cout << res3 << std::endl;
        unob3 = std::move(unob3_);
    });

    std::cout << res2 << std::endl;

    std::cout << "Will update 2" << std::endl;
    sub.Update(2);

    unob1();

    std::cout << "Will update 3" << std::endl;

    sub.Update(3);
}

int main(int argc, const char *argv[])
{
    runBehavior();
    return 0;
}
