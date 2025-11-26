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

struct CalErr : jktools::Error<DivideZero, HasNegative> {};

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
    auto process_func = [](auto&& val) {
        using ValType = std::remove_cvref_t<decltype(val)>;
        if constexpr (std::is_same_v<ValType, DivideZero>)
        {
            std::println("Devided by zero: {} / {}", val.denominator, val.numerator);
        }
        else if constexpr (std::is_same_v<ValType, HasNegative>)
        {
            std::println("Has negative: {} / {}", val.denominator, val.numerator);
        }
    };

    auto print_result = [&](int index) {
        if (result != -1)
            std::println("Result {}: {}", index, result);
    };

    result = Calculate(3, 1).unwrap_or(-1, process_func);
    print_result(1);
    result = Calculate(3, 0).unwrap_or(-1, process_func);
    print_result(2);
    result = Calculate(-1, 3).unwrap_or(-1, process_func);
    print_result(3);
}