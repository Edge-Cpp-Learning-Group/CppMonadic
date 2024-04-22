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

    // auto f = Bound([](int x, int y) -> int { return x + y; }, 1);
    // int r = f(2);
    // std::cout << r << std::endl;

    // auto ob2 = Lift([](int x) { return x + 1; })(mut1);

    // auto unob2 = ob2.Observe([](int val, int valOld) {
    //     std::cout << "ob2: " << valOld << " -> " << val << std::endl;
    // });

    mut1.Update(3);

    Mutable<int> mut3(1);

    // auto lf = Lift(std::function([](int x, int y){ return x * y; }));

    // auto ob3 = Lift([](int x, int y, int z){ return x + y + z; })(mut1, mut2, mut3);
    auto ob3 = mut1 >> [=](int x) {
        return mut2 >> [=](int y) {
            return mut3 >> [=](int z) {
                return x + y + z;
            };
        };
    };

    // std::cout<< "Get here" << std::endl;

    auto unob3 = ob3.Observe([](int val, int valOld) {
        std::cout << "ob3: " << valOld << " -> " << val << std::endl;
    });

    mut1.Update(4);
    mut2.Update(5);
    mut3.Update(6);
    // std::unique_ptr<int> u(new int(1));
    // auto ob4 = mut1.map([u = std::move(u)](int x) { return x + *u; });
}

int main(int argc, const char *argv[])
{
    runObservable();
    return 0;
}
