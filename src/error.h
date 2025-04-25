#pragma once

#include <string>

class SentinelError {
private:
    std::string error_message;
    std::string backtrace;

public:
    SentinelError(std::string error_message);

    std::string get_error_message(void);
    std::string get_backtrace(void);
    std::string format(void);
};
