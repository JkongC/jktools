#ifndef JKTOOLS_RESULT_H_
#define JKTOOLS_RESULT_H_

#include <variant>
#include <expected>
#include <type_traits>

namespace jktools
{
    template<typename... Ts>
    struct Error
    {
        using __JKTOOLS_ERRORTYPE_FLAG = int;

        template <typename SE>
        explicit Error(SE&& specific_error) : m_Variant(std::forward<SE>(specific_error)) {}

        Error(const Error& other) : m_Variant(other.m_Variant) {}
        
        Error(Error&& other)
            noexcept((std::is_nothrow_move_constructible_v<Ts> && ...))
            : m_Variant(std::move(other.m_Variant))
        {}

        Error& operator=(this Error& self, const Error& other)
        {
            m_Variant = other.m_Variant;
            return self;
        }

        template<typename... Ts>
        requires (std::equality_comparable<Ts> && ...)
        friend bool operator==(const Error<Ts...>& lhs, const Error<Ts...>& rhs)
        {
            return lhs.m_Variant == rhs.m_Variant;
        }

        template<typename F>
        decltype(auto) process(F&& function)
        {
            return std::visit(std::forward<F>(function), m_Variant);
        }

    private:
        std::variant<Ts...> m_Variant;
    };

    template<typename T>
    concept ErrorType = requires {
        typename T::__JKTOOLS_ERRORTYPE_FLAG;
    };

    template<typename T, ErrorType E>
    struct Result
    {
        Result() = default;
        Result(const T& result) : m_Exp(result) {}
        Result(T&& result) : m_Exp(std::move(result)) {}
        Result(const E& error) : m_Exp(std::unexpected(error)) {}
        Result(E&& error) : m_Exp(std::unexpected(std::move(error))) {}

        Result(const Result& other) : m_Exp(other.m_Exp) {}
        Result(Result&& other) : m_Exp(std::move(other.m_Exp)) {}

        Result& operator=(this Result& self, const Result& other)
        {
            m_Exp = other.m_Exp;
            return self;
        }

        Result &operator=(this Result &self, Result&& other)
        {
            m_Exp = std::move(other.m_Exp);
            return self;
        }

        // Result's operator== requires both T(unless T is void) and E is comparable.
        template <typename _T, typename _E>
        requires std::equality_comparable<_E> && (std::is_void_v<_T> || std::equality_comparable<_T>)
        friend inline bool operator==(const Result<_T, _E> &lhs, const Result<_T, _E> &rhs)
        {
            return lhs.m_Exp == rhs.m_Exp;
        }

        bool successful() const
        {
            return m_Exp.has_value();
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
                m_Exp.error().process(std::forward<F>(function));

            return *this;
        }

        /** Unwrap the result, if succeeded, return the actual result;
         * if not, return the default value passed in.
         * 
         * @param def_val The default value if unsuccessful.
         */
        T unwrap_or(const T& def_val)
        {
            return m_Exp.value_or(def_val);
        }

        template<typename F>
        T unwrap(F&& function)
        {
            if (successful())
            {
                return m_Exp.value();
            }
            else
            {
                return m_Exp.error().process(function);
            }
        }

    private:
        std::expected<T, E> m_Exp;
    };

    template<typename T, ErrorType E>
    requires std::is_void_v<T>
    struct Result<T, E>
    {
        Result() = default;
        Result(const E& error) : m_Exp(std::unexpected(error)) {}
        Result(E&& error) : m_Exp(std::unexpected(std::move(error))) {}

        Result(const Result &other) : m_Exp(other.m_Exp) {}
        Result(Result &&other) : m_Exp(std::move(other.m_Exp)) {}

        Result& operator=(this Result& self, const Result& other)
        {
            m_Exp = other.m_Exp;
            return self;
        }

        Result& operator=(this Result& self, Result&& other)
        {
            m_Exp = std::move(other.m_Exp);
            return self;
        }

        bool successful() const
        {
            return m_Exp.has_value();
        }

        bool failed() const
        {
            return !successful();
        }

        operator bool() const
        {
            return successful();
        }

        // Result's operator== requires both T(unless T is void) and E is comparable.
        template <typename _T, typename _E>
        requires std::equality_comparable<_E> && (std::is_void_v<_T> || std::equality_comparable<_T>)
        friend bool operator==(const Result<_T, _E> &lhs, const Result<_T, _E> &rhs);

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
                m_Exp.error().process(std::forward<F>(function));

            return *this;
        }

    private:
        std::expected<void, E> m_Exp;
    };

    template<typename... Ts>
    struct ErrorMatcher : Ts... { using Ts::operator()...; };
}

#endif // JKTOOLS_RESULT_H_