#ifndef JKTOOLS_RESULT_H_
#define JKTOOLS_RESULT_H_

#include <variant>
#include <expected>

namespace jktools
{
    template<typename... Ts>
    struct Error : public std::variant<Ts...>
    {
        using __JKTOOLS_ERRORTYPE_FLAG = int;
        using VariantType = std::variant<Ts...>;

        template<typename SE>
        Error(SE&& specific_error) : std::variant<Ts...>(std::forward<SE>(specific_error)) {}

        decltype(auto) as_variant()
        {
            using RetType = decltype(*this);
            if constexpr (std::is_lvalue_reference_v<RetType>)
            {
                if constexpr (std::is_const_v<RetType>)
                {
                    return static_cast<const VariantType&>(*this);
                }
                else
                {
                    return static_cast<VariantType&>(*this);
                }
            }
            else
            {
                return static_cast<VariantType&&>(*this);
            }
        }

        template<typename F>
        decltype(auto) process(F&& function)
        {
            return std::visit(std::forward<F>(function), this->as_variant());
        }
    };

    template<typename T>
    concept ErrorType = requires {
        typename T::__JKTOOLS_ERRORTYPE_FLAG;
    };

    template<typename T, ErrorType E>
    struct Result : private std::expected<T, E>
    {
    public:
        Result(T&& result) : std::expected<T, E>(std::forward<T>(result)) {}
        Result(E&& error) : std::expected<T, E>(std::unexpected(std::forward<E>(error))) {}

    public:
        bool successful() const
        {
            return this->has_value();
        }

        operator bool() const
        {
            return successful();
        }

        T unwrap()
        {
            return this->value();
        }

        /** Unwrap the result, if succeeded, return the actual result;
         * if not, process the error with the function passed in, and return the default value.
         * 
         * @param def_val The default value if unsuccessful.
         * @param function The function used to process the error.
         */
        template<typename F>
        T unwrap_or(T&& def_val, F&& function)
        {
            if (!successful())
                this->error().process(function);

            return this->value_or(std::forward<T>(def_val));
        }

        /** Unwrap the result, if succeeded, return the actual result;
         * if not, process the error with the function passed in, and return the default value.
         * The default value is return by the function.
         *
         * @param def_val The default value if unsuccessful.
         * @param function The function used to process the error.
         */
        template <typename F>
        T unwrap_or(F&& function)
        {
            if (!successful())
                return this->error().process(function);

            return this->value();
        }

    private:
        Result() = default;
    };

    template<typename... Ts>
    struct ErrorMatcher : Ts... { using Ts::operator()...; };
}

#endif // JKTOOLS_RESULT_H_