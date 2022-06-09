#ifndef FORMATTER_HPP
#define FORMATTER_HPP

#include <vector>
#include <bitset>
#include "Instruction.hpp"

namespace dl {
  class Formatter {
    static std::vector<Instruction> instructions;

  public:
    static void set_source_instructions(const std::vector<Instruction> &instructions) {
      Formatter::instructions = instructions;
    }

    static std::string instructions_format() {
      std::string result;

      std::stringstream ss_for_size;
      ss_for_size << std::hex << instructions.size() - 1;
      unsigned char line_number_size = static_cast<unsigned char>(ss_for_size.str().size());

      for (size_t x = 0; x < instructions.size(); x++) {
        std::stringstream ss;
        ss << std::hex << x;
        std::string line_number = ss.str();
        line_number = std::string(line_number_size - line_number.size(), '0') + line_number;
        result += line_number + ". " + instructions[x].command.name + ' ' + instructions[x].argumnet.code + '\n';
      }
      if (result.size())
        result.pop_back();

      return result;
    }

    static std::string decomp_format() {
      std::string result;
      for (size_t x = 0; x < instructions.size(); x++) {
        std::string line = " ";
        std::string pos = std::bitset<12>(x).to_string();
        line += pos.substr(0, 4) + ' ' + pos.substr(4, 4) + ' ' + pos.substr(8, 4) + "  ";

        std::string full_instruction = instructions[x].command.code + instructions[x].argumnet.code;
        if (full_instruction.size() > 4) {

        }
        std::stringstream ss;
        ss << std::hex << full_instruction;
        unsigned long long int_value;
        ss >> int_value;
        std::string value = std::bitset<16>(int_value).to_string();
        line += value.substr(0, 4) + ' ' + value.substr(4, 4) + ' ' + value.substr(8, 4) + ' ' + value.substr(12, 4) + '\n';

        result += line;
      }

      return result;
    }
  };

  std::vector<Instruction> Formatter::instructions;
}

#endif // !FORMATTER_HPP
