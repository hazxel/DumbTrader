#ifndef DUMBTRADER_UTILS_OPENSSL_H_
#define DUMBTRADER_UTILS_OPENSSL_H_

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

inline void logNonFatalError(unsigned long errorCode) {
    std::cerr << "OpenSSL Non-fatal Error:" << ERR_error_string(errorCode, nullptr) << std::endl;
}

inline OpenSSLException getException(unsigned long errorCode) {
    return OpenSSLException(ERR_error_string(errorCode, nullptr));
}

inline OpenSSLException getException() {
    unsigned long errorCode = ERR_get_error();
    if (errorCode != 0) {
        return OpenSSLException(ERR_error_string(errorCode, nullptr));
    }
    return OpenSSLException("No OpenSSL error in the stack");
}

// inline void ssl_info_callback(const SSL* ssl, int where, int ret) {
//     if (ret == 0) return;

//     if (where & SSL_CB_HANDSHAKE_START) {
//         std::cout << "SSL handshake started\n";
//     }
//     if (where & SSL_CB_HANDSHAKE_DONE) {
//         std::cout << "SSL handshake completed\n";
//     }
//     if (where & SSL_CB_ALERT) {
//         const char* alert_type = (where & SSL_CB_READ) ? "read" : "write";
//         std::cout << "SSL alert: " << alert_type << "(" << SSL_alert_desc_string_long(ret) << ")\n";
//     }
//     if (where & SSL_CB_LOOP) {
//         std::cout << "SSL state: " << SSL_state_string_long(ssl) << "\n";
//     }
// }

} // namespace dumbtrader::utils::openssl

#endif // #define DUMBTRADER_UTILS_OPENSSL_H_