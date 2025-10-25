
#pragma once
#include <string>

namespace InfiniteRandomizerFramework {
    struct RedLogger {
        static void Info(const std::string& message);
        static void Error(const std::string& message);
        static void Debug(const std::string& message);
    };
}