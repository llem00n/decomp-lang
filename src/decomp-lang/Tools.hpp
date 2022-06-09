#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <string>
#include <algorithm>

namespace tls {
  std::string to_lower(std::string src) {
    std::transform(src.begin(), src.end(), src.begin(), [] (unsigned char c) { return std::tolower(c); });
    return src;
  }

  std::string str_to_hex(std::string value) {
    std::stringstream ss;

    if (!value.find("0x")) {
      value.erase(0, 2);
      return value;
    } else if (!value.find("0o")) {
      value.erase(0, 2);
      size_t decimal = std::stoull(value, 0, 8);
      std::stringstream ss;
      ss << std::hex << decimal;
      return ss.str();
    } else if (!value.find("0b")) {
      value.erase(0, 2);
      size_t decimal = std::stoull(value, 0, 2);
      ss << std::hex << decimal;
      return ss.str();
    } else {
      size_t decimal = std::stoull(value);
      ss << std::hex << decimal;
      return ss.str();
    }
  }
}

#endif // !TOOLS_HPP
