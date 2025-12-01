#include "simple_password.h"
#include <iostream>
#include <algorithm>
#include <cstring>

SimplePassword::SimplePassword() {
}

SimplePassword::~SimplePassword() {
}

std::pair<std::string, std::string> SimplePassword::GeneratePasswordHash(const std::string& password) {
    // 生成随机盐
    std::string salt = GenerateSalt();

    // 组合密码和盐
    std::string password_with_salt = password + salt;

    // 计算SHA256哈希
    std::string hash = SHA256(password_with_salt);

    return std::make_pair(hash, salt);
}

bool SimplePassword::VerifyPassword(const std::string& password, const std::string& hash, const std::string& salt) {
    // 组合密码和盐
    std::string password_with_salt = password + salt;

    // 计算SHA256哈希
    std::string computed_hash = SHA256(password_with_salt);

    // 比较哈希值
    return computed_hash == hash;
}

std::string SimplePassword::Base64Encode(const std::string& input) {
    BIO* bio = BIO_new(BIO_f_base64());
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO* b64 = BIO_new_fp(stdout, BIO_NOCLOSE);
    b64 = BIO_push(bio, b64);

    BIO_write(b64, input.c_str(), input.size());
    BIO_flush(b64);

    char buffer[input.size() * 2];
    int len = BIO_read(b64, buffer, sizeof(buffer));

    BIO_free_all(b64);

    return std::string(buffer, len);
}

std::string SimplePassword::Base64Decode(const std::string& input) {
    BIO* bio = BIO_new(BIO_f_base64());
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO* b64 = BIO_new_mem_buf(input.c_str(), input.size());
    b64 = BIO_push(bio, b64);

    char buffer[input.size()];
    int len = BIO_read(b64, buffer, sizeof(buffer));

    BIO_free_all(b64);

    return std::string(buffer, len);
}

std::string SimplePassword::SHA256(const std::string& data) {
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, data.c_str(), data.size());
    EVP_DigestFinal_ex(ctx, digest, &digest_len);
    EVP_MD_CTX_free(ctx);

    char buffer[EVP_MAX_MD_SIZE * 2];
    for (int i = 0; i < digest_len; ++i) {
        sprintf(buffer + i * 2, "%02x", digest[i]);
    }

    return std::string(buffer, digest_len * 2);
}

std::string SimplePassword::GenerateSalt(size_t length) {
    unsigned char* salt = new unsigned char[length];
    RAND_bytes(salt, length);

    char buffer[length * 2];
    for (int i = 0; i < length; ++i) {
        sprintf(buffer + i * 2, "%02x", salt[i]);
    }

    delete[] salt;

    return std::string(buffer, length * 2);
}
