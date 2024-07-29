#ifndef DUMBTRADER_UTILS_ERROR_H_
#define DUMBTRADER_UTILS_ERROR_H_

#include <format>  // C++20
#include <iostream>
#include <stdexcept>
#include <string>

#include <cstring>  // strerror function (errno to errmsg)
#include <cerrno>   // errno macro (`int * __error(void)`)


#define THROW_RUNTIME_ERROR(fmt, ...) \
    throw std::runtime_error(std::format(fmt, __VA_ARGS__, errno, std::strerror(errno)))

#define LOG_CERROR(fmt, ...) \
    std::cerr << std::format(fmt, __VA_ARGS__, errno, std::strerror(errno)) << std::endl

namespace dumbtrader::utils {


} // namespace dumbtrader

#endif // #define DUMBTRADER_UTILS_ERROR_H_
