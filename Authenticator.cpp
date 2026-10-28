#include "Authenticator.h"
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <sstream>
#include <iomanip>

std::string Authenticator::genSalt() const {
    CryptoPP::AutoSeededRandomPool prng;
    uint64_t salt_val = 0;
    prng.GenerateBlock(reinterpret_cast<CryptoPP::byte*>(&salt_val), sizeof(salt_val));
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << salt_val;
    return ss.str();
}

std::string Authenticator::calcHash(const std::string& salt, const std::string& pass) const {
    std::string digest;
    CryptoPP::SHA256 sha256;
    CryptoPP::StringSource(salt + pass, true,
        new CryptoPP::HashFilter(sha256,
            new CryptoPP::HexEncoder(new CryptoPP::StringSink(digest),true)
        )
    );
    return digest;
}

bool Authenticator::checkAuth(const std::string& login, const std::string& clientHash, const std::string& salt, const UserBase& users) const {
    if (!users.checkUser(login)) return false;
    return calcHash(salt, users.getPass(login)) == clientHash;
}
