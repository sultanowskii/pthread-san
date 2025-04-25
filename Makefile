SHELL := /bin/bash

SRC_DIR        := src
TEST_DIR       := tests
TEST_CASES_DIR := cases
DOCTEST_DIR    := tests/doctest/doctest
EXAMPLES_DIR   := examples

TARGET := sanitizer.so
SRCS := $(shell find src/ -type f -name '*.cpp')
HDRS := $(shell find src/ -type f -name '*.h')
OBJS := $(SRCS:.cpp=.o)

TEST_TARGET := tests/tests.elf
TEST_SRCS   := $(wildcard tests/*.cpp)

TEST_CASE_SRCS := $(shell find cases/ -type f -name '*.c')

FILES_TO_LINT := $(SRCS) $(HDRS) $(TEST_SRCS) $(TEST_CASE_SRCS)

.PHONY: build
build: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $^ -std=c++17 -shared -fPIC -ldl -g -O3 -Werror -Wall -Wextra -pthread -pedantic -o $(TARGET)

src/%.o: src/%.cpp
	$(CXX) -std=c++17 -c -fPIC -o $@ $<

.PHONY: run-tests
run-tests:
	./$(TEST_TARGET) -s -d

.PHONY: build-tests
build-tests:
	$(CXX) $(TEST_SRCS) -I $(SRC_DIR) -I $(TEST_DIR) -I $(DOCTEST_DIR) -std=c++17 -ldl -g -O3 -Werror -Wall -Wextra -pthread -pedantic -o $(TEST_TARGET)

.PHONY: clang-format
clang-format:
	diff -u <(cat $(FILES_TO_LINT)) <(clang-format $(FILES_TO_LINT))

.PHONY: build-examples
build-examples:
	$(CXX) -rdynamic -o $(EXAMPLES_DIR)/cpp-cycle-example.elf $(EXAMPLES_DIR)/cpp-cycle-example.cpp
	$(CC) -rdynamic -o $(EXAMPLES_DIR)/c-cycle-example.elf $(EXAMPLES_DIR)/c-cycle-example.c

.PHONY: clang-format-fix
clang-format-fix:
	clang-format -i $(FILES_TO_LINT)

.PHONY: clang-tidy
clang-tidy:
	clang-tidy \
		$(FILES_TO_LINT) \
		-checks=-*,clang-analyzer-*,-clang-analyzer-cplusplus* \
		-- \
		-xc++ \
		-I $(SRC_DIR) \
		-I $(TEST_DIR) \
		-I $(DOCTEST_DIR) \
		-std=c++17 \
		-g \
		-O3 \
		-Werror \
		-Wall \
		-Wextra \
		-pthread \
		-pedantic

.PHONY:
clean:
	rm -f $(TARGET)
	rm -f $(OBJS)
	rm -f $(EXAMPLES_DIR)/*.elf
	rm -f $(CASES_DIR)/*.elf
