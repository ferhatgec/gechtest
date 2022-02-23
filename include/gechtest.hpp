// MIT License
//
// Copyright (c) 2022 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//
// gechtest - single header unit testing library for C++ (20)
// ----
// * gechtest uses source_location which is implemented in C++20,
//   so there's no any C++17 support yet, work-in-progress.
//

#ifndef GECHTEST_GECHTEST_HPP
#define GECHTEST_GECHTEST_HPP

#include <iostream>
#include <vector>
#include <chrono>

#ifdef __has_include
    #if __has_include(<string_view>)
        #include <string_view>
        using string = std::string_view;
    #else
        using string = std::string;
    #endif
#else
    #include <string>
    using string = std::string;
#endif

#if __cplusplus >= 202002L
    #include <source_location>
#endif

namespace gech {
    using function_test = void(*)();

    enum test_types {
        Eq,
        UnEq,
        Gt,
        Lt,
        GEq,
        LEq,
        MemLeak
    };

    enum test_results {
        Error,
        Success,
        Critical
    };

    class test_rc_node {
    public:
        string name, type;

        std::source_location current_location = std::source_location::current();
    public:
        test_rc_node() = default; ~test_rc_node() = default;
    };

    class test_log_node {
    public:
        unsigned ms_took = 0;
        test_results result;
        string data;
        function_test func;
    public:
        test_log_node() = default; ~test_log_node() = default;
    };

    class test {
    public:
        std::uint_least32_t line, column;

        string file_name, function_name;

        int rc = 0;

        unsigned errors = 0;

        std::vector<test_log_node> infos;
        std::vector<test_rc_node> rc_infos;

        function_test func;

        std::source_location current_location;

        const std::chrono::time_point<
                std::chrono::_V2::system_clock, std::chrono::duration<
                        long int, std::ratio<1, 1000000000>>> main_ms = std::chrono::system_clock::now();
    public:
        test(function_test func) : func(func) {
            this->fill_infos();
        }

        ~test() {
            this->assert_rc();
            this->summary();
        }

        auto since_time() {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - this->main_ms);
        }

        void summary() {
            std::cout << "\n[SUMMARY]\n"
                      << "File: "
                      << this->current_location.file_name()
                      << '\n'
                      << "Error/s: "
                      << this->errors
                      << '\n'
                      << since_time().count()
                      << "ns\n";
        }

        unsigned calculate_time(function_test func) {
            auto ms = std::chrono::high_resolution_clock::now();
            func();
            return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - ms).count();
        }

        void run_tests() {
            this->infos.back().ms_took = this->calculate_time(this->func);

            for(auto& test : this->infos) {
                if(test.func != nullptr)
                    test.ms_took = this->calculate_time(test.func);
            }
        }

        void test_function(function_test test) {
            gech::test_log_node val;
            val.func = test;
            this->infos.push_back(val);
        }

        void fill_infos(const std::source_location location = std::source_location::current()) {
            this->line = location.line();
            this->column = location.column();
            this->file_name = location.file_name();
            this->function_name = location.function_name();
        }

        gech::test_log_node put_log(const gech::test_results& result, const string message) noexcept {
            gech::test_log_node val;
            val.result = result;
            val.data = message;
            this->infos.push_back(val);
            return val;
        }

        void draw_case(const gech::test_log_node& node) {
            if(node.result == Critical)
                std::cout << "[CRITICAL]: ";
            else if(node.result == Success)
                std::cout << "[SUCCESS]: ";
            else {
                std::cout << "[FAILED]: ";
                ++this->errors;
            }
        }

        template <typename... Val>
        void put(const Val... message) noexcept {
            auto info = this->infos.back();

            this->draw_case(info);

            std::cout << "("
                      << this->current_location.file_name()
                      << ", "
                      << this->current_location.line()
                      << ":"
                      << this->current_location.column()
                      << ":"
                      << info.ms_took
                      << "ns"
                      << ") ["
                      << this->current_location.function_name()
                      << "] -> "
                      << info.data << '\n';
        }

        template <typename Arg1, typename Arg2>
        void assert(const gech::test_types& type, Arg1& val, Arg2& val2,
                    const std::source_location location = std::source_location::current()) {
            this->current_location = location;
            switch(type) {
                case Eq:
                    if(val == val2) {
                        this->put(this->put_log(Success, "OK").data);
                        break;
                    }

                    this->put(this->put_log(Error, "Given values are not equal, expected equal").data);
                    break;

                case UnEq:
                    if(val != val2) {
                        this->put(this->put_log(Success, "OK").data);
                        break;
                    }

                    this->put(this->put_log(Error, "Given values are equal, expected not equal").data);
                    break;

                case Gt:
                    if(val > val2) {
                        this->put(this->put_log(Success, "OK").data);
                        break;
                    }

                    this->put(this->put_log(Error, "Given values are not greater, expected greater").data);
                    break;

                case Lt:
                    if(val < val2) {
                        this->put(this->put_log(Success, "OK").data);
                        break;
                    }

                    this->put(this->put_log(Error, "Given values are greater, expected not greater").data);
                    break;

                case GEq:
                    if(val >= val2) {
                        this->put(this->put_log(Success, "OK").data);
                        break;
                    }

                    this->put(this->put_log(Error, "Given values are not greater or equal, expected greater or equal").data);
                    break;

                case LEq:
                    if(val <= val2) {
                        this->put(this->put_log(Success, "OK").data);
                        break;
                    }

                    this->put(this->put_log(Error, "Given values are greater or equal, expected not greater or equal").data);
            }
        }

        void assert_rc(const std::source_location location = std::source_location::current()) {
            if(this->rc < 0) {
                this->put(this->put_log(Critical, "(RC < 0) Deallocating not allocated value").data, location);
                this->summary();
                std::abort();
            }
        }

        template <typename Arg1, typename Arg2>
        void assert_eq(Arg1 val, Arg2 val2, const std::source_location location = std::source_location::current()) {
            this->assert(Eq, val, val2, location);
        }

        template <typename Arg1, typename Arg2>
        void assert_uneq(Arg1 val, Arg2 val2, const std::source_location location = std::source_location::current()) {
            this->assert(UnEq, val, val2, location);
        }

        template <typename Arg1, typename Arg2>
        void assert_gt(Arg1 val, Arg2 val2, const std::source_location location = std::source_location::current()) {
            this->assert(Gt, val, val2, location);
        }

        template <typename Arg1, typename Arg2>
        void assert_lt(Arg1 val, Arg2 val2, const std::source_location location = std::source_location::current()) {
            this->assert(Lt, val, val2, location);
        }

        template <typename Arg1, typename Arg2>
        void assert_geq(Arg1 val, Arg2 val2, const std::source_location location = std::source_location::current()) {
            this->assert(GEq, val, val2, location);
        }

        template <typename Arg1, typename Arg2>
        void assert_leq(Arg1 val, Arg2 val2, const std::source_location location = std::source_location::current()) {
            this->assert(LEq, val, val2, location);
        }
    };
}

#define TEST(case_name) \
    void case_name();  \
    gech::test test_reg(case_name);\
    void case_name()


#define TEST_MAIN \
    int main(int argc, char** argv) { \
        test_reg.run_tests(); \
    }

#define ALLOC(name, type) \
    test_reg.assert_rc(); \
    ++test_reg.rc;        \
    type* name = new type;

#define DEALLOC(name) \
    --test_reg.rc;    \
    test_reg.assert_rc(); \
    delete name;

#define ASSERT_EQ(val, val2) \
    test_reg.assert_eq(val, val2);

#define ASSERT_UNEQ(val, val2) \
    test_reg.assert_uneq(val, val2);

#define ASSERT_GT(val, val2) \
    test_reg.assert_gt(val, val2);

#define ASSERT_LT(val, val2) \
    test_reg.assert_lt(val, val2);

#define ASSERT_GEQ(val, val2) \
    test_reg.assert_geq(val, val2);

#define ASSERT_LEQ(val, val2) \
    test_reg.assert_leq(val, val2);

#endif // GECHTEST_GECHTEST_HPP