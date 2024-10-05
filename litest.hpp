
#pragma once

#include <cinttypes>
#include <cstdint>
#include <cstdio>


#ifdef _MSC_VER
    #define LITEST_BREAK() __debugbreak()
#elif defined(__has_builtin)
    #if __has_builtin(__builtin_debugtrap)
        #define LITEST_BREAK() __builtin_debugtrap()
    #elif __has_builtin(__builtin_trap)
        #define LITEST_BREAK() __builtin_trap()
    #elif __has_include(<signal.h>)
        #include <signal.h>
        #ifdef (SIGTRAP)
            #define LITEST_BREAK() raise(SIGTRAP)
        #else
            #define LITEST_BREAK() raise(SIGABRT)
        #endif
    #endif
#else
    #define LITEST_BREAK() (void)0 // out of luck
#endif

#ifndef LITEST_BREAK_ON_FAILURE
    #ifndef NDEBUG
        #define LITEST_BREAK_ON_FAILURE 1
    #else
        #define LITEST_BREAK_ON_FAILURE 0
    #endif
#endif

#if !LITEST_BREAK_ON_FAILURE
#undef LITEST_BREAK
#define LITEST_BREAK() (void)0 // do nothing on release
#endif

#ifdef LITEST_RUN_TESTS
namespace litest
{
    struct link_node final
    {
        link_node *next = nullptr;
    };

    struct suite_state final
    {
        std::uint16_t passed{};
        std::uint16_t failed{};
        std::uint16_t skipped{};
    };

    struct suite_node final
    {
        link_node node;
        const char *name;
        const char *file;
        suite_state state;
        void (*suite)(suite_state *) = nullptr;
    };
    
    inline static suite_node *suite_list = nullptr;

    inline int run()
    {
        suite_state global{};
        
        auto visit = [&](auto &&self, suite_node *node, auto &&fn) -> void {
            if (node)
            {
                self(self, (suite_node *)node->node.next, fn);
                fn(node);
            }
        };

        visit(visit, suite_list, [&](suite_node *node) -> void {
            char smallbuf[64];
            auto len = std::snprintf(smallbuf, 64, "Running \"%s\"...", node->name);

            std::printf("%-63s %18s\n", smallbuf, node->file);

            node->suite(&(node->state));
            
            global.failed += node->state.failed;
            global.passed += node->state.passed;
            global.skipped += node->state.skipped;
        });

        std::printf("\nResults:\n========");

        // Dummy node to make printing easier
        suite_node total{
            link_node{&(suite_list->node)}, // .node
            "total", // .name
            "",
            global, // .state
            nullptr, // .suite
        };
        // suite_node total{
        //     .node = {.next = &(suite_list->node)},
        //     .name = "total",
        //     .file = "",
        //     .state = global,
        //     .suite = nullptr,
        // };

        suite_list = &total;
        
        visit(visit, suite_list, [&](suite_node *node) -> void {
            auto color = node->state.failed == 0
                ? "\x1b[32m" // green
                : "\x1b[31m" // red
                ;

            std::printf("%s\n\t%s \x1b[0m", color, node->name);
            std::printf(
                "=> %" PRIu32 " passed", node->state.passed);
            
            if (node->state.failed)
            {
                std::printf(" | %" PRIu32 " failed", node->state.failed);
            }

            if (node->state.skipped)
            {
                std::printf(" | %" PRIu32 " skipped", node->state.skipped);
            }
        });

        return 0;
    }

    namespace impl
    {
        template <std::size_t N>
        constexpr std::size_t string_length(char const (&)[N]) { return N; }
    }
}

#define test_suite(t) \
    static void t(::litest::suite_state *state); \
    inline static int litest_cat(_guard_, __LINE__) = [] { \
        auto constexpr file = [](auto &&file) { \
            for (auto n = ::litest::impl::string_length(file) - 1; n != 0; n--) \
            { \
                if (file[n] == litest_path_sep) \
                    return file + n + 1; \
            } \
            return file + (*file == litest_path_sep); \
        }(__FILE__); \
        \
        static ::litest::suite_node node{ \
            ::litest::link_node{&(::litest::suite_list->node)}, \
            #t, \
            file, \
            ::litest::suite_state{}, \
            t, \
        }; \
        /* static ::litest::suite_node node{ \
            .node = {.next = &(::litest::suite_list->node)}, \
            .name = #t, \
            .file = file, \
            .suite = t, \
        }; */ \
        ::litest::suite_list = &node; \
        return 0; \
    }(); \
    void t(::litest::suite_state *state)
    

#define test(x) \
    int litest_cat(x, _test_); (void)litest_cat(x, _test_); \
    for (bool litest_skip = false; !litest_skip; litest_skip = true)

#define litest_assert_impl(cond, break_cond) \
    do { \
        if (litest_skip) { ++state->skipped; break; } \
         \
        if (!(cond)) \
        { \
            std::printf("\tline %u: Assertion [%s] failed.\n", __LINE__, #cond); \
            ++state->failed; \
            LITEST_BREAK(); \
            break_cond(); \
        } \
        else \
        { \
            ++state->passed; \
        } \
    } while (0)

#define main(...) \
    main() { return ::litest::run(); } \
    \
    int __regular_main(__VA_ARGS__)

// helper macros
#define litest_cat(x, y) litest_cat_impl(x, y)
#define litest_cat_impl(x, y) x##y

#ifdef _WIN32
#define litest_path_sep '\\'
#else
#define litest_path_sep '/'
#endif

#else
#define test_suite(x) void x()
#define test(x) for (; 0;)
#define litest_assert_impl(...) void(x)
#endif

#define litest_break_impl() litest_skip = true
#define ensure(cond) litest_assert_impl(cond, litest_break_impl)

#define litest_no_break_impl() (void)0
#define check(cond) litest_assert_impl(cond, litest_no_break_impl)
