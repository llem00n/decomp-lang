#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <ostream>

namespace dl {
  class Logger {
    static std::ostream *ostream;

  public:
    static void init(std::ostream &ostream) {
      Logger::ostream = &ostream;
    }

    static void error(const std::string &message) {
      std::string result_message = "ERROR: " + message + "\n";
      *ostream << result_message.c_str();
    }
  };

  std::ostream *Logger::ostream = nullptr;
}

#endif // !LOGGER_HPP
