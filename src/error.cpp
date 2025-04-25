#include "error.h"

#include <sstream>
#include <string>

#include "backtrace.h"

SentinelError::SentinelError(std::string error_message) {
    this->error_message = error_message;
    this->backtrace = get_formatted_backtrace();
}

std::string SentinelError::get_error_message(void) { return error_message; }

std::string SentinelError::get_backtrace(void) { return backtrace; }

std::string SentinelError::format(void) {
    std::stringstream stream;
    stream << error_message << "\n"
           << "---=== BACKTRACE ===---\n"
           << backtrace // NOLINT
           << "---=================---";

    return stream.str();
}
