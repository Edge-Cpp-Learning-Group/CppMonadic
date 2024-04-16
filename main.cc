#include <iostream>

#include "maybe.h"
#include "chain.h"
#include "behavior.h"

Maybe<int> maybeAdd1(int n)
{
    return Maybe<int>(n + 1);
}

void runMaybe()
{
    Maybe<int> m1;
    std::cout << m1.isJust() << std::endl;
    std::cout << m1.isNothing() << std::endl;

    Maybe<int> m2(5);
    std::cout << m2.isJust() << std::endl;
    std::cout << m2.isNothing() << std::endl;

    Maybe<int> m3 = m2 >>= std::function(maybeAdd1);
    std::cout << m3.isJust() << std::endl;

    Maybe<int> m4 = m2 >>= std::function([](int n) -> Maybe<int>
                                         { return Maybe<int>(); });

    Maybe<int> m5 = fmap(std::function<int(int)>([](int n) -> int
                                                 { return n + 1; }),
                         m2);
}

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

    auto [res1, unob1] = beh([]() { std::cout << "Notified 1" << std::endl; });

    std::cout << res1 << std::endl;

    auto [res2, unob2] = beh([]() { std::cout << "Notified 2" << std::endl; });

    std::cout << res2 << std::endl;

    beh.Notify();

    unob1();

    beh.Notify();
}

int main(int argc, const char *argv[])
{
    runBehavior();
    return 0;
}
