#include "matching_service.h"
#include <random>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>


uint32_t hash_lws_ptr_id(void* ptr) {
    uint32_t hash = 2166136261u;
    unsigned char* p = reinterpret_cast<unsigned char*>(&ptr);
    for (size_t i = 0; i < sizeof(void*); ++i) {
        hash ^= p[i];
        hash *= 16777619u;
    }
    return hash;
}

std::string to_base36(uint32_t val, size_t length) {
    static const char base36[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string out;
    while (val > 0 && out.length() < length) {
        out += base36[val % 36];
        val /= 36;
    }
    while (out.length() < length) {
        out += '0';
    }
    return out;
}
