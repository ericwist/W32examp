/*****************
* core filehasher.h
* Platform agnostic file hash
* Author: Eric Wistrand
* Jul 26, 2023
*****************/

#pragma once

#include <string>
#include <cstdint>
#include <cstring>
#ifdef _WIN32
#include "..\inc\openssl\md5.h"
#include "..\inc\openssl\evp.h"
#include <fstream>
#endif
#ifdef __APPLE__
#include <CommonCrypto/CommonDigest.h>
#include <fstream>
#endif
#ifdef __linux__
#include <openssl/md5.h>
#include <fstream>
#endif

// Platform-agnostic hash result structure
struct FileHash {
    uint32_t data[4];  // 128-bit MD5 hash
    
    FileHash() {
        memset(data, 0, sizeof(data));
    }
    
    bool IsValid() const {
        return (data[0] != 0 || data[1] != 0 || data[2] != 0 || data[3] != 0);
    }
};

class FileHasher {
public:
    // Single cross-platform entry point
    // Returns true if successful, fills hash reference with result
    static bool GetFileHash(const std::string& filename, FileHash& hash) {
#ifdef _WIN32
        return GetFileHashWindows(filename, hash);
#elif defined(__APPLE__)
        return GetFileHashMacOS(filename, hash);
#elif defined(__linux__)
        return GetFileHashLinux(filename, hash);
#else
        return false;  // Unsupported platform
#endif
    }

private:
    // Windows implementation using OpenSSL
#ifdef _WIN32
    static bool GetFileHashWindows(const std::string& filename, FileHash& hash) {
        unsigned char digest[16];  // MD5 is always 16 bytes
        EVP_MD_CTX* ctx = EVP_MD_CTX_new();
        if (!ctx) {
            return false;
        }

        const EVP_MD* md = EVP_md5();
        if (!EVP_DigestInit_ex(ctx, md, nullptr)) {
            EVP_MD_CTX_free(ctx);
            return false;
        }

        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            EVP_MD_CTX_free(ctx);
            return false;
        }

        char buffer[4096];
        while (file.read(buffer, sizeof(buffer))) {
            if (!EVP_DigestUpdate(ctx, buffer, file.gcount())) {
                file.close();
                EVP_MD_CTX_free(ctx);
                return false;
            }
        }
        // Handle any remaining bytes
        if (file.gcount() > 0) {
            if (!EVP_DigestUpdate(ctx, buffer, file.gcount())) {
                file.close();
                EVP_MD_CTX_free(ctx);
                return false;
            }
        }
        file.close();

        unsigned int digest_len = 0;
        if (!EVP_DigestFinal_ex(ctx, digest, &digest_len) || digest_len != 16) {
            EVP_MD_CTX_free(ctx);
            return false;
        }
        EVP_MD_CTX_free(ctx);

        memcpy(hash.data, digest, 16);
        return true;
    }
#endif

    // macOS implementation using CommonCrypto
#ifdef __APPLE__
    static bool GetFileHashMacOS(const std::string& filename, FileHash& hash) {
        unsigned char digest[CC_MD5_DIGEST_LENGTH];  // 16 bytes
        CC_MD5_CTX md5;
        CC_MD5_Init(&md5);
        
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        char buffer[4096];
        while (file.read(buffer, sizeof(buffer))) {
            CC_MD5_Update(&md5, (unsigned char*)buffer, file.gcount());
        }
        if (file.gcount() > 0) {
            CC_MD5_Update(&md5, (unsigned char*)buffer, file.gcount());
        }
        file.close();
        
        CC_MD5_Final(digest, &md5);
        
        // Copy 16-byte MD5 to 4 × uint32_t
        memcpy(hash.data, digest, 16);
        return true;
    }
#endif

    // Linux implementation using OpenSSL
#ifdef __linux__
    static bool GetFileHashLinux(const std::string& filename, FileHash& hash) {
        unsigned char digest[MD5_DIGEST_LENGTH];  // 16 bytes
        MD5_CTX md5;
        MD5_Init(&md5);
        
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        char buffer[4096];
        while (file.read(buffer, sizeof(buffer))) {
            MD5_Update(&md5, (unsigned char*)buffer, file.gcount());
        }
        if (file.gcount() > 0) {
            MD5_Update(&md5, (unsigned char*)buffer, file.gcount());
        }
        file.close();
        
        MD5_Final(digest, &md5);
        
        // Copy 16-byte MD5 to 4 × uint32_t
        memcpy(hash.data, digest, 16);
        return true;
    }
#endif
};