# litest

litest is a lightweight open source library for writing unit tests in C++. It aims to make testing your code as easy as possible.

[Documentation](./docs.md)

# Features

- Easy to set up, just drop [litest.hpp](./litest.hpp) to your project, no dependencies needed.
- Easy to use, here's an [example](#usage).
- Easy to integrate, just tell the compiler you want a test build, include the header and write your tests; no need to change your `main` function or anything else.
- Requires a C++17 compiler, which should be easily available.

# Usage

First, just drop [litest.hpp](./litest.hpp) to your project, then test your code as following:

```cpp
#define LITEST_RUN_TESTS // 0. Define this if you want to run tests. Make sure it is before the #include.
#include <litest.hpp> // 1. Include the header.

// Just a class representing numbers, we need to test this.
struct number final
{
    number operator +(number rhs) { return {x + rhs.x}; }
    number operator -(number rhs) { return {x - rhs.x}; }

    bool operator ==(number rhs) const { return x == rhs.x; }

    int x;
};

// 2. Define a suite for your tests with `test_suite`.
test_suite (numbers) {
    // 2.b (Optional). Define test sections just to keep things clean with `test`.
    test (add) {
        number a{10};
        number b{20};

        ensure(a + b == number{30}); // 3. Make sure your code works as intended with `ensure`.
    }

    test (sub) {
        number a{10};
        number b{20};

        ensure(a - b == number{-10});
    }
}

// 4. Make sure you write a `main` function. It should include code the program runs when not running tests.
int main()
{
    // Our program does nothing actually, we're just testing our `number` type.
    return 0;
}
```

Additionally, the library will pause the debugger when any `ensure` check fails on debug builds. The feature can be configured by defining `LITEST_BREAK_ON_FAILURE` to be you desired value (1 to enable or 0 to disable).

That's about all the API of the library. If you follow the given steps you should have a smooth experience with the library.


# License

litest is distributed under the MIT [license](./LICENSE).
