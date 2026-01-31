#include <string>
#include <cstdint>
#include <cstring>
#include <iomanip>
std::string int64_to_string(uint64_t value, bool pad16 = true);
uint64_t ID16(const std::string& s);
std::string CleanBuffer(char* buffer,size_t size);