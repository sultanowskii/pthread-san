#include "backtrace.h"

#include <sstream>
#include <string>

#include <execinfo.h>
#include <stdio.h>
#include <unistd.h>

static const int BACKTRACE_BUFFER_SIZE = 100;

std::string get_formatted_backtrace(void) {
    std::stringstream stream;
    void *backtrace_buffer[BACKTRACE_BUFFER_SIZE];

    size_t actual_size = backtrace(backtrace_buffer, BACKTRACE_BUFFER_SIZE);
    char **result = backtrace_symbols(backtrace_buffer, actual_size);

    for (size_t i = 0; i < actual_size; i++) {
        stream << result[i] << "\n";
    }

    free(result);
    return stream.str();
}
