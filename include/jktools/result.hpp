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
        explicit Error(SE&& specific_error) : std::variant<Ts...>(std::forward<SE>(specific_error)) {}

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
        Result() : std::expected<T, E>() {}
        Result(const T& result) : std::expected<T, E>(result) {}
        Result(T&& result) : std::expected<T, E>(std::move(result)) {}
        Result(const E& error) : std::expected<T, E>(std::unexpected(error)) {}
        Result(E&& error) : std::expected<T, E>(std::unexpected(std::move(error))) {}

    public:
        using std::expected<T, E>::error;
        using std::expected<T, E>::value;

        bool successful() const
        {
            return this->has_value();
        }

        bool failed() const
        {
            return !successful();
        }

        operator bool() const
        {
            return successful();
        }

        /** If failed, using the the function passed in to process the error.
         *
         * @param function The function used to process the error.
         * 
         * @return The result itself.
         */
        template<typename F>
        decltype(auto) if_failed(F&& function)
        {
            if (!successful())
                error().process(std::forward<F>(function));

            return *this;
        }

        /** Unwrap the result, if succeeded, return the actual result;
         * if not, return the default value passed in.
         * 
         * @param def_val The default value if unsuccessful.
         */
        T unwrap_or(const T& def_val)
        {
            return this->value_or(def_val);
        }

        template<typename F>
        T unwrap(F&& function)
        {
            if (successful())
            {
                return value();
            }
            else
            {
                return error().process(function);
            }
        }
    };

    template<typename T, ErrorType E>
    requires std::is_void_v<T>
    struct Result<T, E> : private std::expected<void, E>
    {
    public:
        Result() : std::expected<void, E>() {}
        Result(const E& error) : std::expected<void, E>(std::unexpected(error)) {}
        Result(E &&error) : std::expected<void, E>(std::unexpected(std::move(error))) {}

    public:
        using std::expected<void, E>::error;

        bool successful() const
        {
            return this->has_value();
        }

        bool failed() const
        {
            return !successful();
        }

        operator bool() const
        {
            return successful();
        }

        /** If failed, using the the function passed in to process the error.
         *
         * @param function The function used to process the error.
         *
         * @return The result itself.
         */
        template <typename F>
        decltype(auto) if_failed(F &&function)
        {
            if (!successful())
                error().process(std::forward<F>(function));

            return *this;
        }
    };

    template<typename... Ts>
    struct ErrorMatcher : Ts... { using Ts::operator()...; };
}

#endif // JKTOOLS_RESULT_H_