#include <xor_list/xor_list.h>
#include <list>
#include <cstdint>
#include <iostream>
#include <chrono>
#include <random>


std::list<int> generateInitializer(std::uint64_t size)
{
    std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<> distr;

    std::list<int> result;

    while (size > 0)
    {
        result.emplace_back(distr(generator));
        --size;
    }

    return result;
}

template<typename List>
std::chrono::duration<long double> measureSorting(const std::list<int> &initializer)
{
    constexpr auto iterations = 10;

    std::chrono::duration<long double> result = std::chrono::duration<long double>::zero();

    for (int i = 0; i < iterations; ++i)
    {
        List list;
        list.assign(initializer.cbegin(), initializer.cend());

        std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

        list.sort();

        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

        result += std::chrono::duration_cast<std::chrono::duration<long double>>(t2 - t1);
    }

    return result / iterations;
}


int main()
{
    for (std::uint64_t size = 10U; size <= 10000000U; size *= 10)
    {
        const auto list = generateInitializer(size);

        std::cout << "Analyzed size : " << size << std::endl;
        std::cout << "std::list : " << measureSorting<std::list<int>>(list).count() << " sec." << std::endl;
        std::cout << "::xor_list : " << measureSorting<xor_list<int>>(list).count() << " sec." << std::endl;

        std::cout << std::endl;
    }
}
