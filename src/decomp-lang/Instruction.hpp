#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <string>

namespace dl {
  namespace {
    struct _Command {
      std::string code;
      std::string name;
    };

    struct _Argument {
      std::string code;
      bool is_variable = false;
      bool is_label = false;
    };
  }

  struct Instruction {
    _Command command;
    _Argument argumnet;

    bool is_variable = false;
  };
}

#endif // !INSTRUCTION_HPP
