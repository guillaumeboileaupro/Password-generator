#include "password_core.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {

void fail(const string& message) {
    cerr << "TEST FAILURE: " << message << '\n';
    exit(1);
}

void expect(bool condition, const string& message) {
    if (!condition) {
        fail(message);
    }
}

void expect_equal(const string& actual, const string& expected, const string& message) {
    if (actual != expected) {
        fail(message + " | actual=" + actual + " expected=" + expected);
    }
}

string to_hex(const vector<unsigned char>& data) {
    static const char* hex = "0123456789abcdef";
    string output;
    output.reserve(data.size() * 2);

    for (unsigned char value : data) {
        output.push_back(hex[value >> 4]);
        output.push_back(hex[value & 0x0F]);
    }

    return output;
}

bool contains_any(const string& value, const string& charset) {
    for (char ch : value) {
        if (charset.find(ch) != string::npos) {
            return true;
        }
    }
    return false;
}

void test_sha256_bytes() {
    const vector<unsigned char> input = {'a', 'b', 'c'};
    const string digest = to_hex(sha256_bytes(input));
    expect_equal(
        digest,
        "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
        "sha256 digest mismatch"
    );
}

void test_derive_stream_is_stable() {
    const vector<unsigned char> seed = {1, 2, 3, 4};
    const vector<unsigned char> short_stream = derive_stream(seed, 32);
    const vector<unsigned char> long_stream = derive_stream(seed, 80);

    expect(short_stream.size() == 32, "short derive_stream size mismatch");
    expect(long_stream.size() == 80, "long derive_stream size mismatch");
    expect(equal(short_stream.begin(), short_stream.end(), long_stream.begin()), "derive_stream prefix must be stable");
}

void test_generate_password_from_seed_with_all_groups() {
    const PasswordOptions options{true, true, true, true};
    const vector<unsigned char> seed = {'m', 'i', 'c', 'r', 'o'};
    const string password = generate_password_from_seed(24, options, seed);

    expect(password.size() == 24, "password length mismatch");
    expect(contains_any(password, "abcdefghijklmnopqrstuvwxyz"), "missing lowercase");
    expect(contains_any(password, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"), "missing uppercase");
    expect(contains_any(password, "0123456789"), "missing digit");
    expect(contains_any(password, "!@#$%^&*()-_=+[]{};:,.?/"), "missing symbol");
}

void test_generate_password_single_charset() {
    const PasswordOptions options{false, false, true, false};
    const vector<unsigned char> seed = {9, 8, 7, 6, 5, 4, 3, 2};
    const string password = generate_password_from_seed(40, options, seed);

    expect(password.size() == 40, "single charset password length mismatch");
    expect(password.find_first_not_of("0123456789") == string::npos, "single charset password contains unexpected characters");
}

void test_generate_password_length_one() {
    const PasswordOptions options{true, false, false, false};
    const vector<unsigned char> seed = {4, 2, 4, 2};
    const string password = generate_password_from_seed(1, options, seed);

    expect(password.size() == 1, "length-one password mismatch");
    expect(contains_any(password, "abcdefghijklmnopqrstuvwxyz"), "length-one password must stay in charset");
}

void test_generate_password_throws_without_charset() {
    const PasswordOptions options{false, false, false, false};
    const vector<unsigned char> seed = {1};

    try {
        static_cast<void>(generate_password_from_seed(10, options, seed));
        fail("expected exception when no charset is selected");
    } catch (const runtime_error&) {
    }
}

void test_generate_password_throws_when_length_is_too_short() {
    const PasswordOptions options{true, true, true, false};
    const vector<unsigned char> seed = {1, 2, 3};

    try {
        static_cast<void>(generate_password_from_seed(2, options, seed));
        fail("expected exception when length is too short");
    } catch (const runtime_error&) {
    }
}

void test_generate_password_throws_when_length_is_non_positive() {
    const PasswordOptions options{true, false, false, false};
    const vector<unsigned char> seed = {1, 2, 3};

    try {
        static_cast<void>(generate_password_from_seed(0, options, seed));
        fail("expected exception when length is non positive");
    } catch (const runtime_error&) {
    }
}

}  // namespace

int main() {
    test_sha256_bytes();
    test_derive_stream_is_stable();
    test_generate_password_from_seed_with_all_groups();
    test_generate_password_single_charset();
    test_generate_password_length_one();
    test_generate_password_throws_without_charset();
    test_generate_password_throws_when_length_is_too_short();
    test_generate_password_throws_when_length_is_non_positive();

    cout << "All password core tests passed." << '\n';
    return 0;
}
