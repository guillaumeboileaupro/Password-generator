#include "password_core.h"

#include <openssl/sha.h>

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

string build_charset(const PasswordOptions& options, vector<string>& subsets) {
    string charset;

    if (options.lower) {
        subsets.emplace_back("abcdefghijklmnopqrstuvwxyz");
        charset += subsets.back();
    }
    if (options.upper) {
        subsets.emplace_back("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        charset += subsets.back();
    }
    if (options.digits) {
        subsets.emplace_back("0123456789");
        charset += subsets.back();
    }
    if (options.symbols) {
        subsets.emplace_back("!@#$%^&*()-_=+[]{};:,.?/");
        charset += subsets.back();
    }

    return charset;
}

void ensure_stream_capacity(
    const vector<unsigned char>& seed,
    vector<unsigned char>& stream,
    size_t index,
    size_t minimum_remaining
) {
    if (stream.size() >= index + minimum_remaining) {
        return;
    }

    const size_t grown_size = max(stream.size() * 2, index + minimum_remaining + 64);
    stream = derive_stream(seed, grown_size);
}

char draw_without_bias(
    const string& charset,
    const vector<unsigned char>& seed,
    vector<unsigned char>& stream,
    size_t& index
) {
    const unsigned int charset_size = static_cast<unsigned int>(charset.size());
    const unsigned int limit = 256 - (256 % charset_size);

    while (true) {
        ensure_stream_capacity(seed, stream, index, 1);

        const unsigned char value = stream[index++];
        if (value < limit) {
            return charset[value % charset_size];
        }
    }
}

void shuffle_password(
    string& password,
    const vector<unsigned char>& seed,
    vector<unsigned char>& stream,
    size_t& index
) {
    if (password.size() < 2) {
        return;
    }

    for (size_t i = password.size() - 1; i > 0; --i) {
        const unsigned int bound = static_cast<unsigned int>(i + 1);
        const unsigned int limit = 256 - (256 % bound);

        while (true) {
            ensure_stream_capacity(seed, stream, index, 1);

            const unsigned char value = stream[index++];
            if (value < limit) {
                swap(password[i], password[value % bound]);
                break;
            }
        }
    }
}

}  // namespace

vector<unsigned char> sha256_bytes(const vector<unsigned char>& data) {
    vector<unsigned char> hash(SHA256_DIGEST_LENGTH);
    SHA256(data.data(), data.size(), hash.data());
    return hash;
}

vector<unsigned char> derive_stream(const vector<unsigned char>& seed, size_t needed) {
    vector<unsigned char> output;
    output.reserve(needed);
    uint64_t counter = 0;

    while (output.size() < needed) {
        vector<unsigned char> block = seed;

        for (int i = 0; i < 8; ++i) {
            block.push_back(static_cast<unsigned char>((counter >> (i * 8)) & 0xFF));
        }

        vector<unsigned char> digest = sha256_bytes(block);
        output.insert(output.end(), digest.begin(), digest.end());
        ++counter;
    }

    output.resize(needed);
    return output;
}

string generate_password_from_seed(
    int length,
    const PasswordOptions& options,
    const vector<unsigned char>& seed
) {
    vector<string> subsets;
    const string charset = build_charset(options, subsets);

    if (charset.empty()) {
        throw runtime_error("Select at least one character type. / Choisis au moins un type de caractere.");
    }

    if (length <= 0) {
        throw runtime_error("Password length must be positive. / La longueur doit etre positive.");
    }

    if (length < static_cast<int>(subsets.size())) {
        throw runtime_error("Length must cover every selected character type. / La longueur doit couvrir chaque type de caractere choisi.");
    }

    vector<unsigned char> stream = derive_stream(seed, max(static_cast<size_t>(length), static_cast<size_t>(16)));
    size_t index = 0;

    string password;
    password.reserve(length);

    for (const string& subset : subsets) {
        password += draw_without_bias(subset, seed, stream, index);
    }

    while (static_cast<int>(password.size()) < length) {
        password += draw_without_bias(charset, seed, stream, index);
    }

    shuffle_password(password, seed, stream, index);
    return password;
}
