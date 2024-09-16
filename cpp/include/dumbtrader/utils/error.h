#ifndef DUMBTRADER_UTILS_ERROR_H_
#define DUMBTRADER_UTILS_ERROR_H_

#include <iostream>
#include <stdexcept>
#include <string>

#include <cstring>  // strerror function (errno to errmsg)
#include <cerrno>   // errno macro (`int * __error(void)`)

#if __cplusplus >= 202002L && __has_include(<format>)

#include <format>

#define THROW_CERROR(fmt, ...) \
    throw std::runtime_error(std::format(fmt, ##__VA_ARGS__, errno, std::strerror(errno)))

#define LOG_CERROR(fmt, ...) \
    std::cerr << std::format(fmt, ##__VA_ARGS__, errno, std::strerror(errno)) << std::endl

#else // if __cplusplus < 202002L || !__has_include(<format>)

#include <sstream>

namespace dumbtrader::utils::error {

template<typename T>
inline void substitutePlaceHolder(std::string& fmt, const T& value) {
    std::ostringstream oss;
    oss << value; 

    int pos = fmt.find("{}");
    if (pos != std::string::npos) {
        fmt.replace(pos, 2, oss.str());
    }
}

template<typename... Args>
inline std::string genErrorString(const std::string &fmt, Args... args) {
    std::string s = fmt;
    (..., substitutePlaceHolder(s, args));
    return s;
}

} // namespace dumbtrader::utils::error

#define THROW_CERROR(fmt, ...) \
    throw std::runtime_error(dumbtrader::utils::error::genErrorString(fmt, ##__VA_ARGS__, errno, std::strerror(errno)))

#define LOG_CERROR(fmt, ...) \
    std::cerr << dumbtrader::utils::error::genErrorString(fmt, ##__VA_ARGS__, errno, std::strerror(errno)) << std::endl

#endif // #if __cplusplus >= 202002L && __has_include(<format>)

#endif // #define DUMBTRADER_UTILS_ERROR_H_
