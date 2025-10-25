#include "RedLogger.h"

#include "globals.h"

namespace InfiniteRandomizerFramework {
    void RedLogger::Info(const std::string& message) {
        g_sdk->logger->Info(g_pHandle, message.c_str());
    }

    void RedLogger::Error(const std::string& message) {
        g_sdk->logger->Error(g_pHandle, message.c_str());
    }

    void RedLogger::Debug(const std::string& message) {
        if (!g_isDebug) {
            return;
        }
        g_sdk->logger->Debug(g_pHandle, message.c_str());
    }
}
