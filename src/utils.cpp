//utils.cpp
#include "utils.h"
std::string int64_to_string(uint64_t value, bool pad16) {
    std::ostringstream oss;
    if (pad16)
        oss << std::setw(16) << std::setfill('0');
    oss << value;
    return oss.str();
}
uint64_t ID16(const std::string& s){
     uint64_t h = std::hash<std::string>{}(s);
     return h % 10000000000000000ULL; // 10^16
} 
std::string CleanBuffer(char* buffer, size_t size) {
    size_t len = strnlen(buffer, size); // find length up to first '\0' or size
    return std::string(buffer, len); // construct string without trailing nulls
}