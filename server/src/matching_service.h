#ifndef MATCHING_SERVICE
#define MATCHING_SERVICE

#include <string>
#include <cstdint>

#define CODE_LENGTH 5


extern uint32_t hash_lws_ptr_id(void* ptr);
extern std::string to_base36(uint32_t val, size_t length);


#endif