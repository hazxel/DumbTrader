#include <openssl/ssl.h>
#include <openssl/err.h>

#include <stdexcept>
#include <string>

namespace dumbtrader::utils::openssl {

class OpenSSLException : public std::runtime_error {
public:
    explicit OpenSSLException(const std::string& message)
        : std::runtime_error(message) {}
};

inline OpenSSLException getOpenSSLError(unsigned long errorCode) {
    std::string error_message(ERR_error_string(errorCode, nullptr));
    return OpenSSLException(error_message);
}

inline OpenSSLException getOpenSSLError() {
    unsigned long err_code;
    while ((err_code = ERR_get_error()) != 0) {
        std::string error_message(ERR_error_string(err_code, nullptr));
        return OpenSSLException(error_message);
    }
    return OpenSSLException("No OpenSSL error in the stack");
}
}