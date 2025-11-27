# jktools

A set(not for now?) of header-only cpp tools that may be of some help.

---

## Result (C++23 and above)

A class that can be used to return a result of a function call, similar to `Result<T, E>` in Rust. It allows convenient error processing without using exception.

**It requires C++23 and above since it uses `std::expected`.**

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

// General error type, used as E in Result<T, E>.
// Inherit Error<Ts...> so it can be constructed from types in Ts.
struct FunctionErr : jktools::Error<ErrorA, ErrorB> {};
//Or: using FunctionErr = jktools::Error<ErrorA, ErrorB>;

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
* Otherwise, use the given default value(-1), and process the error
* with the given function(process_func).
*/
int number = foo(1).unwrap_or(-1, process_func);

// Or use another version of unwrap_or:
int another_number = foo(0).unwrap_or(process_func);
/* This version requires process_func to return a value
* which is used as default value if no valid value.
*/
```

Or you can first check if the result has a valid value:

```c++
int number;
auto result = foo(1);
if (result.successful())
	number = result.unwrap();
```

Note that `unwrap` can only be called when the `Result` has a valid value, or an exception may be thrown.

Or better still, use the helper class `ErrorMatcher` to simplify the setup of the process function and avoid annoying type manipulations:

```c++
auto process_func = jktools::ErrorMatcher{
	[](const ErrorA& a) { std::println("ErrorA"); },
    [](const ErrorB& b) { std::println("ErrorB"); },
    [](const auto& u) { std::println("Unknown error."); }
};

auto result = foo(1).unwrap_or(-1, process_func);
```

This method benefits from overload resolution.

You can read the source to check how `Result` works.