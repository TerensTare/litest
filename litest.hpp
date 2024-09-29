#pragma once

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef LITEST_RUN_TESTS
namespace litest
{
    struct link_node final
    {
        link_node *next = nullptr;
    };

    struct suite_state final
    {
        std::uint32_t passed{};
        std::uint32_t failed{};
    };

    struct suite_node final
    {
        link_node node;
        const char *name;
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
            std::printf("Running \"%s\"...\n", node->name);

            node->suite(&(node->state));
            
            global.failed += node->state.failed;
            global.passed += node->state.passed;
        });

        std::printf("\nResults:\n========");

        // Dummy node to make printing easier
        suite_node total{
            link_node{&(suite_list->node)}, // .node
            "total", // .name
            global, // .state
            nullptr, // .suite
        };
        // suite_node total{
        //     .node = {.next = &(suite_list->node)},
        //     .name = "total",
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
                "=> %" PRIu32 " passed | %" PRIu32 " failed",
                node->state.passed, node->state.failed
            );
        });

        return 0;
    }
}

#define test_suite(t) \
    static void t(::litest::suite_state *state); \
    inline static int litest_cat(_guard_, __LINE__) = [] { \
        static ::litest::suite_node node{ \
            ::litest::link_node{&(::litest::suite_list->node)}, \
            #t, \
            ::litest::suite_state{}, \
            t, \
        }; \
        /* static ::litest::suite_node node{ \
            .node = {.next = &(::litest::suite_list->node)}, \
            .name = #t, \
            .suite = t, \
        }; */ \
        ::litest::suite_list = &node; \
        return 0; \
    }(); \
    void t(::litest::suite_state *state)
    

#define test(x) int litest_cat(x, _test_); (void)litest_cat(x, _test_);

#define ensure(cond) \
    do { \
        if (!(cond)) \
        { \
            auto file = strrchr(__FILE__, litest_path_sep) + 1; \
            std::printf("\t%s:%u:FAILED: [%s]\n", file, __LINE__, #cond); \
            ++state->failed; \
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
#define ensure(x) void(x)
#endif
