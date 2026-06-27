#ifndef PASSWORD_CORE_H
#define PASSWORD_CORE_H

#include <cstddef>
#include <string>
#include <vector>

struct PasswordOptions {
    bool lower;
    bool upper;
    bool digits;
    bool symbols;
};

std::vector<unsigned char> sha256_bytes(const std::vector<unsigned char>& data);
std::vector<unsigned char> derive_stream(const std::vector<unsigned char>& seed, std::size_t needed);
std::string generate_password_from_seed(
    int length,
    const PasswordOptions& options,
    const std::vector<unsigned char>& seed
);

#endif
