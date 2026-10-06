// test_arg_parser.cpp
#include "arg_parser.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace arg_parser;

// ---------------------------------------------------------------------------
// Minimal test harness
// ---------------------------------------------------------------------------
static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        ++g_checks;                                                            \
        if (!(cond)) {                                                         \
            ++g_failures;                                                      \
            std::cerr << "  FAILED: " #cond "  (line " << __LINE__ << ")\n";   \
        }                                                                      \
    } while (0)

#define CHECK_THROWS_AS(expr, ex)                                              \
    do {                                                                       \
        ++g_checks;                                                            \
        bool threw = false;                                                    \
        try {                                                                  \
            (void)(expr);                                                      \
        } catch (const ex &) {                                                 \
            threw = true;                                                      \
        } catch (...) {                                                        \
        }                                                                      \
        if (!threw) {                                                          \
            ++g_failures;                                                      \
            std::cerr << "  FAILED (expected " #ex "): " #expr << "  (line "   \
                      << __LINE__ << ")\n";                                    \
        }                                                                      \
    } while (0)

static std::vector<std::string> v(std::initializer_list<const char *> init) {
    std::vector<std::string> out;
    for (auto s : init)
        out.emplace_back(s);
    return out;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

static void test_flag_basic() {
    std::cout << "test_flag_basic\n";
    ArgParser p;
    p.add_argument("verbose", "v");

    CHECK(!p.has("verbose"));
    p.parse(v({"--verbose"}));
    CHECK(p.has("verbose"));
    CHECK(p.has("v")); // alias lookup works
    CHECK(p.get<bool>("verbose") == true);
    CHECK(p.get<bool>("v") == true);
}

static void test_flag_alias_short() {
    std::cout << "test_flag_alias_short\n";
    ArgParser p;
    p.add_argument("debug", "d");
    p.parse(v({"-d"}));
    CHECK(p.has("debug"));
    CHECK(p.get<bool>("debug") == true);
}

static void test_flag_default_false() {
    std::cout << "test_flag_default_false\n";
    ArgParser p;
    p.add_argument("debug", "d").set_default("false");
    p.parse(v({}));
    CHECK(!p.has("debug"));
    CHECK(p.get<bool>("debug") == false);
}

static void test_option_equals_form() {
    std::cout << "test_option_equals_form\n";
    ArgParser p;
    p.add_argument("port", "p", ArgType::Option);
    p.parse(v({"--port=8080"}));
    CHECK(p.has("port"));
    CHECK(p.get<int>("port") == 8080);
}

static void test_option_separate_form() {
    std::cout << "test_option_separate_form\n";
    ArgParser p;
    p.add_argument("port", "p", ArgType::Option);
    p.parse(v({"--port", "8080"}));
    CHECK(p.get<int>("port") == 8080);
}

static void test_option_short_alias() {
    std::cout << "test_option_short_alias\n";
    ArgParser p;
    p.add_argument("port", "p", ArgType::Option);
    p.parse(v({"-p", "8080"}));
    CHECK(p.get<int>("port") == 8080);

    ArgParser q;
    q.add_argument("port", "p", ArgType::Option);
    q.parse(v({"-p=8080"}));
    CHECK(q.get<int>("port") == 8080);
}

static void test_option_default_value() {
    std::cout << "test_option_default_value\n";
    ArgParser p;
    p.add_argument("host", "h", ArgType::Option).set_default("localhost");
    p.parse(v({}));
    CHECK(!p.has("host"));
    CHECK(p.get<std::string>("host") == "localhost");

    ArgParser q;
    q.add_argument("host", "h", ArgType::Option).set_default("localhost");
    q.parse(v({"--host=example.com"}));
    CHECK(q.get<std::string>("host") == "example.com");
}

static void test_required_argument() {
    std::cout << "test_required_argument\n";
    ArgParser p;
    p.add_argument("port", "p", ArgType::Option).set_required(true);
    CHECK_THROWS_AS(p.parse(v({})), std::invalid_argument);

    ArgParser q;
    q.add_argument("port", "p", ArgType::Option).set_required(true);
    q.parse(v({"--port=9000"}));
    CHECK(q.get<int>("port") == 9000);
}

static void test_positional_args() {
    std::cout << "test_positional_args\n";
    ArgParser p;
    p.add_argument("verbose", "v");
    p.parse(v({"input.txt", "--verbose", "output.txt"}));
    CHECK(p.has("verbose"));
    CHECK(p.size() == 2);
    CHECK(p.at(0) == "input.txt");
    CHECK(p.at(1) == "output.txt");
}

static void test_end_of_options() {
    std::cout << "test_end_of_options\n";
    ArgParser p;
    p.add_argument("verbose", "v");
    p.parse(v({"--verbose", "--", "--not-a-flag", "file.txt"}));
    CHECK(p.has("verbose"));
    CHECK(p.size() == 2);
    CHECK(p.at(0) == "--not-a-flag");
    CHECK(p.at(1) == "file.txt");
}

static void test_at_out_of_range() {
    std::cout << "test_at_out_of_range\n";
    ArgParser p;
    p.parse(v({"only"}));
    CHECK(p.size() == 1);
    CHECK(p.at(0) == "only");
    CHECK_THROWS_AS(p.at(2), std::out_of_range);
}

static void test_error_unrecognized() {
    std::cout << "test_error_unrecognized\n";
    ArgParser p;
    p.add_argument("verbose", "v");
    CHECK_THROWS_AS(p.parse(v({"--unknown"})), std::invalid_argument);
}

static void test_error_duplicate_provided() {
    std::cout << "test_error_duplicate_provided\n";
    ArgParser p;
    p.add_argument("verbose", "v");
    CHECK_THROWS_AS(p.parse(v({"--verbose", "-v"})), std::invalid_argument);
}

static void test_error_flag_with_value() {
    std::cout << "test_error_flag_with_value\n";
    ArgParser p;
    p.add_argument("verbose", "v");
    CHECK_THROWS_AS(p.parse(v({"--verbose=true"})), std::invalid_argument);
}

static void test_error_option_missing_value() {
    std::cout << "test_error_option_missing_value\n";
    ArgParser p;
    p.add_argument("port", "p", ArgType::Option);
    CHECK_THROWS_AS(p.parse(v({"--port"})), std::invalid_argument);

    ArgParser q;
    q.add_argument("port", "p", ArgType::Option);
    CHECK_THROWS_AS(q.parse(v({"--port="})), std::invalid_argument);
}

static void test_error_option_negative_value() {
    std::cout << "test_error_option_negative_value\n";
    // A value that starts with '-' is rejected in the space-separated form...
    ArgParser p;
    p.add_argument("num", "n", ArgType::Option);
    CHECK_THROWS_AS(p.parse(v({"--num", "-5"})), std::invalid_argument);

    // ...but accepted with the '=' form.
    ArgParser q;
    q.add_argument("num", "n", ArgType::Option);
    q.parse(v({"--num=-5"}));
    CHECK(q.get<int>("num") == -5);
}

static void test_error_registration() {
    std::cout << "test_error_registration\n";
    ArgParser p;
    p.add_argument("foo");
    CHECK_THROWS_AS(p.add_argument("foo"), std::invalid_argument);
    CHECK_THROWS_AS(p.add_argument("bar", "foo"), std::invalid_argument);
    CHECK_THROWS_AS(p.add_argument("--dashed"), std::invalid_argument);
    CHECK_THROWS_AS(p.add_argument("ok", "-x"), std::invalid_argument);
    CHECK_THROWS_AS(p.add_argument(""), std::invalid_argument);
}

static void test_get_unregistered_throws() {
    std::cout << "test_get_unregistered_throws\n";
    ArgParser p;
    p.add_argument("port", "p", ArgType::Option);
    p.parse(v({}));
    CHECK_THROWS_AS(p.get<int>("nope"), std::invalid_argument);
    // get() does not resolve aliases.
    CHECK_THROWS_AS(p.get<int>("p"), std::invalid_argument);
}

static void test_get_not_provided_throws() {
    std::cout << "test_get_not_provided_throws\n";
    ArgParser p;
    p.add_argument("port", "p", ArgType::Option);
    p.add_argument("verbose", "v");
    p.parse(v({}));
    CHECK_THROWS_AS(p.get<int>("port"), std::invalid_argument);
    // NOTE: New feature: set default false automatically
    CHECK(p.get<bool>("verbose") == false);
}

static void test_get_type_conversions() {
    std::cout << "test_get_type_conversions\n";
    ArgParser p;
    p.add_argument("port", "p", ArgType::Option);
    p.add_argument("ratio", "r", ArgType::Option);
    p.add_argument("name", "n", ArgType::Option);

    p.parse(v({"--port=8080", "--ratio=0.5", "--name=alice"}));
    CHECK(p.get<int>("port") == 8080);
    CHECK(p.get<unsigned>("port") == 8080u);
    CHECK(p.get<double>("ratio") == 0.5);
    CHECK(p.get<std::string>("name") == "alice");
}

static void test_get_bad_conversion() {
    std::cout << "test_get_bad_conversion\n";
    ArgParser p;
    p.add_argument("count", "c", ArgType::Option);
    p.parse(v({"--count=abc"}));
    CHECK_THROWS_AS(p.get<int>("count"), std::invalid_argument);

    ArgParser q;
    q.add_argument("count", "c", ArgType::Option);
    q.parse(v({"--count=123abc"}));
    CHECK_THROWS_AS(q.get<int>("count"), std::invalid_argument);
}

static void test_help_text() {
    std::cout << "test_help_text\n";
    ArgParser p;
    p.set_program_name("myprog");
    p.add_argument("verbose", "v").set_description("Enable verbose output");
    p.add_argument("port", "p", ArgType::Option)
        .set_description("Port to bind to");

    auto h = p.help(std::nullopt);
    CHECK(h.find("myprog") != std::string::npos);
    CHECK(h.find("--verbose") != std::string::npos);
    CHECK(h.find("-v") != std::string::npos);
    CHECK(h.find("--port") != std::string::npos);
    CHECK(h.find("<value>") != std::string::npos);
    CHECK(h.find("Enable verbose output") != std::string::npos);
    CHECK(h.find("Port to bind to") != std::string::npos);

    auto h2 = p.help(std::string("myprog [OPTIONS] FILE"));
    CHECK(h2.find("myprog [OPTIONS] FILE") != std::string::npos);
}

// ---------------------------------------------------------------------------
int main() {
    test_flag_basic();
    test_flag_alias_short();
    test_flag_default_false();
    test_option_equals_form();
    test_option_separate_form();
    test_option_short_alias();
    test_option_default_value();
    test_required_argument();
    test_positional_args();
    test_end_of_options();
    test_at_out_of_range();
    test_error_unrecognized();
    test_error_duplicate_provided();
    test_error_flag_with_value();
    test_error_option_missing_value();
    test_error_option_negative_value();
    test_error_registration();
    test_get_unregistered_throws();
    test_get_not_provided_throws();
    test_get_type_conversions();
    test_get_bad_conversion();
    test_help_text();

    std::cout << "\n"
              << (g_checks - g_failures) << "/" << g_checks
              << " checks passed\n";
    return g_failures == 0 ? 0 : 1;
}
