#include <print>

#include "jktools/result.hpp"

struct DivideZero
{
    int denominator;
    int numerator;
};

struct HasNegative
{
    int denominator;
    int numerator;
};

struct CalErr : jktools::Error<DivideZero, HasNegative>
{
    using ErrType = jktools::Error<DivideZero, HasNegative>;

    using ErrType::ErrType;
};

jktools::Result<int, CalErr> Calculate(int denominator, int numerator)
{
    if (numerator == 0)
    {
        return CalErr(DivideZero{denominator, numerator});
    }
    else if (denominator < 0 || numerator < 0)
    {
        return CalErr(HasNegative{denominator, numerator});
    }

    return denominator / numerator;
}

int main()
{
    int result;
    auto process_func = jktools::ErrorMatcher{
        [](const DivideZero& d) { std::println("Divide zero: {} / {}", d.denominator, d.numerator); },
        [](const HasNegative& n) { std::println("Has negative: {} / {}", n.denominator, n.numerator); }
    };

    auto print_result = [&](int index) {
        if (result != -1)
            std::println("Result {}: {}", index, result);
    };

    result = Calculate(3, 1).if_failed(process_func).unwrap_or(-1);
    print_result(1);
    result = Calculate(3, 0).if_failed(process_func).unwrap_or(-1);
    print_result(2);
    result = Calculate(-1, 3).if_failed(process_func).unwrap_or(-1);
    print_result(3);
}