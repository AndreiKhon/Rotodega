
#ifndef HELPERSHPP
#define HELPERSHPP

#include "godot_cpp/core/error_macros.hpp"
#include <source_location>
#include <string>

namespace debug {

inline auto PrintError(
    const std::string &error,
    const std::source_location location = std::source_location::current()) {
  godot::_err_print_error(location.function_name(), location.file_name(),
                          location.line(), error.c_str());
}

} // namespace debug

#endif