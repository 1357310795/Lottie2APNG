#pragma once
#include <string>
#include <string.h>

struct api_result
{
    bool success;
    char message[256];
};

inline api_result ApiResult(bool success, const char* message) {
    api_result res;
    strncpy_s(res.message, message, 255);
    res.success = success;
    return res;
}