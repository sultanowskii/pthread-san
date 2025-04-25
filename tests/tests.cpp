#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <sanitizer.h>

#include <regex>
#include <sstream>

#include <unistd.h>

struct Result {
    std::string stdout;
    std::string stderr;
    int exit_code;
};

std::string read_file(const char *path) {
    auto fp = std::fopen(path, "rb");
    std::fseek(fp, 0u, SEEK_END);
    auto size = std::ftell(fp);
    std::fseek(fp, 0u, SEEK_SET);
    std::string s(size, '\0');
    std::fread(&s[0], 1u, size, fp);
    std::fclose(fp);
    return s;
}

Result run(const char *name, const char *stdout_log, const char *stderr_log) {
    std::stringstream cmdstream;
    cmdstream << "LD_PRELOAD=./sanitizer.so ./cases/" << name << ".elf 1>" << stdout_log << " 2>" << stderr_log;
    auto ret = system(cmdstream.str().c_str());

    return Result{
        read_file(stdout_log),
        read_file(stderr_log),
        WEXITSTATUS(ret),
    };
}

#define CHECK_CONTAINS(s, substr) CHECK(s.find(substr) != std::string::npos)
#define CHECK_CONTAINS_C(s, substr) CHECK_CONTAINS(s, std::string(substr))

#define RUN_EXE(name, result_var)                                                                                      \
    char stdout_log[] = "tmp/sanitizer." #name ".out.XXXXXX";                                                          \
    char stderr_log[] = "tmp/sanitizer." #name ".err.XXXXXX";                                                          \
    close(mkstemp(stdout_log));                                                                                        \
    close(mkstemp(stderr_log));                                                                                        \
    result_var = run(#name, stdout_log, stderr_log);

TEST_CASE("sanitizer") {
    system("make");
    system("mkdir tmp");
    system("make -C cases");

    SUBCASE("no_pthread") {
        Result result;
        RUN_EXE(no_pthread, result)

        CHECK_EQ("hey\n", result.stdout);
        CHECK_EQ("", result.stderr);
        CHECK_EQ(0, result.exit_code);
    }
    SUBCASE("normal") {
        Result result;
        RUN_EXE(normal, result)

        CHECK_EQ("", result.stdout);
        CHECK_EQ("", result.stderr);
        CHECK_EQ(0, result.exit_code);
    }
    SUBCASE("double_lock") {
        Result result;
        RUN_EXE(double_lock, result)

        CHECK_EQ("", result.stdout);

        std::regex error_message_regex(
            "Thread #\\d+ acquires a mutex M\\d+ "
            "\\[0x[0-9a-f]+\\] while already "
            "holding it"
        );
        CHECK(std::regex_search(result.stderr, error_message_regex));
        CHECK_CONTAINS_C(result.stderr, "---=== BACKTRACE ===---");
        CHECK_CONTAINS_C(result.stderr, "---=================---");

        CHECK_EQ(1, result.exit_code);
    }
    SUBCASE("cycle") {
        Result result;
        RUN_EXE(cycle, result)

        bool stdout_is_one_of_messages =
            (result.stdout == "hi from f1()!\n") || (result.stdout == "hello from f2()!\n");
        CHECK(stdout_is_one_of_messages);

        std::regex error_message_regex(
            "Cycle in lock order graph: M\\d+ \\[0x[0-9a-f]+\\]( => M\\d+ "
            "\\[0x[0-9a-f]+\\]){2}"
        );
        CHECK(std::regex_search(result.stderr, error_message_regex));
        CHECK_CONTAINS_C(result.stderr, "---=== BACKTRACE ===---");
        CHECK_CONTAINS_C(result.stderr, "---=================---");

        CHECK_EQ(1, result.exit_code);
    }
    SUBCASE("multiple_mutexes_cycle") {
        Result result;
        RUN_EXE(multiple_mutexes_cycle, result)

        bool stdout_is_one_of_messages = (result.stdout == "f1 says: hi\n") || (result.stdout == "f2 says: hi\n");
        CHECK(stdout_is_one_of_messages);

        std::regex error_message_regex(
            "Cycle in lock order graph: M\\d+ \\[0x[0-9a-f]+\\]( => M\\d+ "
            "\\[0x[0-9a-f]+\\]){2}"
        );
        CHECK(std::regex_search(result.stderr, error_message_regex));
        CHECK_CONTAINS_C(result.stderr, "---=== BACKTRACE ===---");
        CHECK_CONTAINS_C(result.stderr, "---=================---");

        CHECK_EQ(1, result.exit_code);
    }
    SUBCASE("complex_cycle") {
        Result result;
        RUN_EXE(complex_cycle, result)

        CHECK_EQ("", result.stdout);

        std::regex error_message_regex(
            "Cycle in lock order graph: M\\d+ \\[0x[0-9a-f]+\\]( => M\\d+ "
            "\\[0x[0-9a-f]+\\]){4}"
        );
        CHECK(std::regex_search(result.stderr, error_message_regex));
        CHECK_CONTAINS_C(result.stderr, "---=== BACKTRACE ===---");
        CHECK_CONTAINS_C(result.stderr, "---=================---");

        CHECK_EQ(1, result.exit_code);
    }

    system("make clean");
    system("rm -rf tmp");
    system("make -C cases clean");
}
