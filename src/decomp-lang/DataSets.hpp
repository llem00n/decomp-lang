#ifndef DATA_SETS_HPP
#define DATA_SETS_HPP

#include <array>
#include <string>
#include <unordered_map>

namespace dl {
  namespace ds {
    namespace {
      struct CommandInfo {
        std::string instruction;
        unsigned char args = 0;
        bool special = false;
      };

      struct InstructionInfo {
        std::string code;
        std::string name;
      };
    }

    // DeCompLanguage available commands
    const std::unordered_map<std::string, CommandInfo> commands_map = {
      { "add",    { .instruction = "add",    .args = 2 } },
      { "sub",    { .instruction = "sub",    .args = 2 } },
      { "and",    { .instruction = "and",    .args = 2 } },
      { "or",     { .instruction = "or",     .args = 2 } },
      { "xor",    { .instruction = "xor",    .args = 2 } },
      { "not",    { .instruction = "not",    .args = 1 } },
      { "output", { .instruction = "output", .args = 1 } },
      { "lsl",    { .instruction = "lsl",    .args = 1 } },
      { "lsr",    { .instruction = "lsr",    .args = 1 } },
      { "asl",    { .instruction = "asl",    .args = 1 } },
      { "asr",    { .instruction = "asr",    .args = 1 } },
      { "rol",    { .instruction = "rol",    .args = 1 } },
      { "ror",    { .instruction = "ror",    .args = 1 } },
      { "rcl",    { .instruction = "rcl",    .args = 1 } },
      { "rcr",    { .instruction = "rcr",    .args = 1 } },
      { "stop",   { .instruction = "halt" } },

      { "input",  { .special = true } },
      { "label",  { .special = true } },
      { "goto",   { .special = true } },
      { "mov",    { .special = true } },
      { "if",     { .special = true } },
      { "while",  { .special = true } },
    };

    // DeCompLanguage available operators
    const std::array<std::string, 4> operators = {
      "[", "]", ".", "&"
    };

    // DeCompLanguage keywords
    const std::array<std::string, 3> keywords = {
      "if", "end", "do"
    };

    // DeComp instructions with its encodings
    const std::unordered_map<std::string, InstructionInfo> instructions_map = {
      { "load",   { "0", "LOAD" } },
      { "store",  { "1", "STORE" } },
      { "add",    { "2", "ADD" } },
      { "sub",    { "3", "SUB" } },
      { "and",    { "4", "AND" } },
      { "or",     { "5", "OR" } },
      { "xor",    { "6", "XOR" } },
      { "not",    { "70", "NOT" } },
      { "input",  { "74", "INPUT" } },
      { "output", { "78", "OUTPUT" } },
      { "halt",   { "7C", "HALT" } },
      { "jnz",    { "8", "JNZ" } },
      { "jz",     { "9", "JZ" } },
      { "jns",    { "A", "JP" } },
      { "js",     { "B", "JM" } },
      { "jnc",    { "C", "JNC" } },
      { "jc",     { "D", "JC" } },
      { "jmp",    { "E", "JMP" } },
      { "lsl",    { "F0", "LSL" } },
      { "lsr",    { "F2", "LSR" } },
      { "asl",    { "F4", "ASL" } },
      { "asr",    { "F6", "ASR" } },
      { "rol",    { "F8", "ROL" } },
      { "ror",    { "FA", "ROR" } },
      { "rcl",    { "FC", "RCL" } },
      { "rcr",    { "FE", "RCR" } }
    };
  }
}

#endif // !DATA_SETS_HPP
