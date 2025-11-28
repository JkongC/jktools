# jktools

A set(not for now?) of header-only cpp tools that may be of some help.

---

## Result (C++23 and above)

A class that can be used to return a result of a function call, similar to `Result<T, E>` in Rust. It allows convenient error processing without using exception.

**It requires C++23 and above since it uses `std::expected` as its base class.**

Its equivalent declaration is like:

```c++
template<typename T, ErrorType E>
struct Result;
```

`ErrorType` is a C++20 concept that requires E to be `Error` type or inherit `Error` type, whose equivalent declaration is like:

```c++
template<typename... Ts>
struct Error;
```

To use `Result` in a function, you simply do this:

```c++
// Specific Error types.
struct ErrorA {};
struct ErrorB {};

using FunctionErr = jktools::Error<ErrorA, ErrorB>;
// Or: struct FunctionErr : jktools::Error<ErrorA, ErrorB> {}
// Inherit Error<Ts...> so it can be constructed from types in Ts.

jktools::Result<int, FunctionErr> foo(int param)
{
    if (param < 0)
        return FunctionErr(ErrorA{});
    else if (param == 0)
        return FunctionErr(ErrorB{});
    else
        return param + 1;
}
```

**Note:** If you choose to inherit `Error` instead of creating a type alias(for additional information in error class or other reason), it is important to know that `Error`'s constructor is explicit and also a function template, which may requires special treatment:

```C++
// To support expressions like "return FunctionErr(ErrorA{});"
// You may need do a constructor forwarding.
struct FunctionErr : jktools::Error<ErrorA, ErrorB>
{
	FunctionErr(auto&& err) : jktools::Error<ErrorA, ErrorB>(std::forward<decltype(err)>(err)) {}
}

// Or if you like, directly use Error's constructor.
struct FunctionErr : jktools::Error<ErrorA, ErrorB>
{
	using Base = jktools::Error<ErrorA, ErrorB>;
	using Base::Base;
}
```

To process the result:

```c++
auto process_func = [](auto&& val) {
	using ValType = std::remove_cvref_t<decltype(val)>;
    if constexpr (std::is_same_v<ValType, ErrorA>)
    	std::println("ErrorA");
    else if constexpr (std::is_same_v<ValType, ErrorB>)
    	std::println("ErrorB");
};

/* This statement calls foo, and unwrap the returned Result.
* If it has valid value, return the value.
* Otherwise, process the error with the given function (in if_failed)
* then return the given default value(-1)
*/
int number = foo(1).if_failed(process_func).unwrap_or(-1);
```

Or better still, use the helper class `ErrorMatcher` to simplify the setup of the process function and avoid annoying type manipulations:

```c++
auto process_func = jktools::ErrorMatcher{
	[](const ErrorA& a) { std::println("ErrorA"); },
	[](const ErrorB& b) { std::println("ErrorB"); },
	[](auto&& u) { std::println("Unknown error."); }
};

auto result = foo(1).if_failed(process_func).unwrap_or(-1);
```

Or you can first check if the result has a valid value:

```c++
int number;
auto result = foo(1);
if (result.successful())
	number = result.value();
else
	result.error().process([](auto&&) -> { std::println("Error occurred."); });
```

If you want to decide the final returned value using the error information, you can use the `unwrap` method:

```c++
// Arbitrary infomation
// Using enum here for convenience
enum ErrorInfo
{
    fail, warning, reject, success
};

struct ErrorWithInfo : jktools::Error<ErrorInfo>
{
	explicit ErrorWithInfo(int) = default;  
};

Result<int, ErrorWithInfo> foo(int input)
{
    if (input == -1)
        return ErrorWithInfo{fail};
    else if (input == -2)
        return ErrorWithInfo{warning};
    else if (input == -3)
        return ErrorWithInfo{reject};
    else
        return input;
}
```

```c++
auto process_func = [](ErrorInfo info) -> std::string {
	switch (info)
    {
        case fail:
            return -10;
            break;
        case warning:
            return -20;
            break;
        case reject:
            return -30;
            break;
        default:
            return 0;
            break;
    }
}

int result = foo(<Any Input>).unwrap(process_func);
```

You can read the source to further check how `Result` works.