#ifndef TRANSLATOR_HPP
#define TRANSLATOR_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "Token.hpp"
#include "DataSets.hpp"
#include "Tools.hpp"
#include "Logger.hpp"
#include "Instruction.hpp"
#include "Tools.hpp"

namespace dl {
  struct VariableInfo {
    std::string value;
    std::string address;
  };

  struct _CommandArgumentInfo {
    Token token;
    bool is_pointer;
  };

  class Translator {
    bool _failed;
    std::unordered_map<std::string, VariableInfo> global_variables;
    std::unordered_map<std::string, std::string> labels_map;
    std::vector<Token>::const_iterator main_start;
    std::vector<Token>::const_iterator main_end;
    std::vector<Instruction> instructions;
    size_t if_counter;
    size_t while_counter;
    bool is_main_defined;

  public:
    Translator()
      : _failed(false)
      , is_main_defined(false)
      , while_counter(0u)
      , if_counter(0u)
    { }

    std::vector<Instruction> translate(const std::vector<Token> &tokens) {
      reset();

      parse_segments(tokens);
      translate_main();
      post_process_instructions();

      return instructions;
    }

    bool failed() {
      return _failed;
    }

    void reset() {
      _failed = false;
      global_variables.clear();
      instructions.clear();
      is_main_defined = false;
      if_counter = 0u;
      while_counter = 0u;
    }

  private:
    void translate_main() {
      if (!is_main_defined) {
        error("main segment is undefined");
        return;
      }

      for (auto x = main_start; x != main_end; x++) {
        handle_command(x, main_end);
      }

      push_instruction("halt", { });
    }

    void handle_command(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator end) {
      if (x->type != Token::Type::IDENTIFIER) {
        error("expected a command at line %tl", *x);
        return;
      }

      if (ds::commands_map.find(x->value) == ds::commands_map.end()) {
        error("unknown command: %tv at line %tl", *x);
        return;
      }
      ds::CommandInfo command = ds::commands_map.at(x->value);

      if (command.special) {
        handle_special_command(x, end, command);
      } else {
        handle_general_command(x, end, command);
      }
    }

    void handle_general_command(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator end, const ds::CommandInfo &command_info) {
      switch (command_info.args) {
        case 0:
          handle_general_no_args_command(x, end, command_info);
          break;
        case 1:
          handle_general_1_arg_command(x, end, command_info);
          break;
        case 2:
          handle_general_2_args_command(x, end, command_info);
          break;
      }
    }

    void handle_general_no_args_command(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator end, const ds::CommandInfo &command_info) {
      push_instruction(command_info.instruction, { });
    }

    void handle_general_1_arg_command(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator end, const ds::CommandInfo &command_info) {
      auto arg = parse_argument(++x, end);

      if (arg.token.type == Token::Type::NUMBER) {
        global_variables["$" + arg.token.value] = VariableInfo { .value = arg.token.value };
        arg.token.value = "$" + arg.token.value;
      }

      if (arg.token.value != "acm") {
        if (arg.is_pointer) {
          push_load_from_pointer(arg);
        } else {
          push_instruction("load", { .code = arg.token.value, .is_variable = true });
        }
      }
      push_instruction(command_info.instruction, { });
    }
    
    void handle_general_2_args_command(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator end, const ds::CommandInfo &command_info) {
      auto arg1 = parse_argument(++x, end);
      auto arg2 = parse_argument(++x, end);

      if (arg1.token.type == Token::Type::NUMBER) {
        global_variables["$" + arg1.token.value] = VariableInfo { .value = arg1.token.value };
        arg1.token.value = "$" + arg1.token.value;
      }
      if (arg2.token.type == Token::Type::NUMBER) {
        global_variables["$" + arg2.token.value] = VariableInfo { .value = arg2.token.value };
        arg2.token.value = "$" + arg2.token.value;
      }

      if (arg1.token.value == "acm" && arg2.token.value == "acm") {
        push_instruction("store", { .code = "$tmp", .is_variable = true });
        push_instruction(command_info.instruction, { .code = "$tmp", .is_variable = true });
      } else if (arg1.token.value != "acm" && arg2.token.value != "acm") {
        if (arg1.is_pointer) {
          push_load_from_pointer(arg1);
        } else {
          push_instruction("load", { .code = arg1.token.value, .is_variable = true });
        }

        if (arg2.is_pointer) {
          push_instruction_with_pointer_argument(arg2, command_info.instruction);
        } else {
          push_instruction(command_info.instruction, { .code = arg2.token.value, .is_variable = true });
        }
      } else if (arg1.token.value == "acm") {
        if (arg2.is_pointer) {
          push_instruction_with_pointer_argument(arg2, command_info.instruction);
        } else {
          push_instruction(command_info.instruction, { .code = arg2.token.value, .is_variable = true });
        }
      } else if (arg2.token.value == "acm") {
        if (arg1.is_pointer) {
          push_instruction_with_pointer_argument(arg1, command_info.instruction);
        } else {
          push_instruction("store", { .code = "$tmp", .is_variable = true });
          push_instruction("load", { .code = arg1.token.value, .is_variable = true });
          push_instruction(command_info.instruction, { .code = "$tmp", .is_variable = true });
        }
      }
    }

    void handle_special_command(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator end, const ds::CommandInfo &command_info) {
      if (x->value == "goto") {
        handle_goto_command(x, end, command_info);
      } else if (x->value == "if") {
        handle_if_command(x, end, command_info);
      } else if (x->value == "mov") {
        handle_mov_command(x, end, command_info);
      } else if (x->value == "label") {
        handle_label_command(x, end, command_info);
      } else if (x->value == "input") {
        handle_input_command(x, end, command_info);
      } else if (x->value == "while") {
        handle_while_command(x, end, command_info);
      }
    }

    void handle_goto_command(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator end, const ds::CommandInfo &command_info) {
      if (++x == end) {
        error("unexpected end of input: expected a lable name at line %tl", *(x - 1));
        return;
      }

      push_instruction("jmp", { .code = x->value, .is_label = true });
    }

    void handle_label_command(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator end, const ds::CommandInfo &command_info) {
      if (++x == end) {
        error("unexpected end of input: expected a lable name at line %tl", *(x - 1));
        return;
      }

      labels_map.insert(std::make_pair(x->value, tls::str_to_hex(std::to_string(instructions.size()))));
    }

    void handle_input_command(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator end, const ds::CommandInfo &command_info) {
      auto arg = parse_argument(++x, end);

      push_instruction("input", { });
      if (arg.is_pointer) {
        push_store_to_pointer(arg);
      } else if (arg.token.type != Token::Type::IDENTIFIER) {
        error("invalid argument for input at line %tl", arg.token);
        return;
      } else {
        push_instruction("store", { .code = arg.token.value, .is_variable = true });
      }
    }

    void handle_mov_command(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator end, const ds::CommandInfo &command_info) {
      auto arg1 = parse_argument(++x, end);
      auto arg2 = parse_argument(++x, end);

      if (arg1.token.type == Token::Type::NUMBER) {
        global_variables["$" + arg1.token.value] = VariableInfo { .value = arg1.token.value };
        arg1.token.value = "$" + arg1.token.value;
      }
      if (arg2.token.type == Token::Type::NUMBER) {
        global_variables["$" + arg2.token.value] = VariableInfo { .value = arg2.token.value };
        arg2.token.value = "$" + arg2.token.value;
      }

      if (arg1.token.value == "acm" && arg2.token.value == "acm") {
        error("cannot move data from acm to acm at line %tl", *(x - 2));
        return;
      } else if (arg2.token.value == "acm") {
        if (arg1.is_pointer) {
          push_store_to_pointer(arg1);
        } else {
          push_instruction("store", { .code = arg1.token.value, .is_variable = true });
        }
      } else if (arg1.token.value == "acm") {
        if (arg2.is_pointer) {
          push_load_from_pointer(arg2);
        } else {
          push_instruction("load", { .code = arg2.token.value, .is_variable = true });
        }
      } else {
        if (arg2.is_pointer) {
          push_load_from_pointer(arg2);
        } else {
          push_instruction("load", { .code = arg2.token.value, .is_variable = true });
        }

        if (arg1.is_pointer) {
          push_store_to_pointer(arg1);
        } else {
          push_instruction("store", { .code = arg1.token.value, .is_variable = true });
        }
      }
    }

    void push_store_to_pointer(const _CommandArgumentInfo &arg) {
      push_instruction_with_pointer_argument(arg, "store");
    }

    void push_load_from_pointer(const _CommandArgumentInfo &arg) {
      push_instruction_with_pointer_argument(arg, "load");
    }

    void push_instruction_with_pointer_argument(const _CommandArgumentInfo &arg, const std::string &instruction) {
      if (ds::instructions_map.find(instruction) == ds::instructions_map.end()) {
        error("unknown instruction: %tv at line %tl", arg.token);
        return;
      }

      push_instruction("store", { .code = "$tmp", .is_variable = true });
      push_instruction("load", { .code = arg.token.value, .is_variable = true });
      push_instruction("add", { .code = "$inst_" + instruction, .is_variable = true });
      std::string address = tls::str_to_hex(std::to_string(instructions.size() + 2));
      address = std::string(3 - address.size(), '0') + address;
      push_instruction("store", { .code = address });
      push_instruction("load", { .code = "$tmp", .is_variable = true });
      instructions.push_back(Instruction { .command = {.code = "00", .name = "NULL" }, .argumnet = {.code = "00" } });

      std::string instruction_code = ds::instructions_map.at(instruction).code;
      instruction_code += std::string(4 - instruction_code.size(), '0');
      global_variables["$inst_" + instruction] = VariableInfo { .value = instruction_code };
    }

    _CommandArgumentInfo parse_argument(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator end) {
      Token command = *x;
      _CommandArgumentInfo argument { };
      if (x == end) {
        error("unexpected end of input: an argument axpected for command %tv at line %tl", command);
        return argument;
      }
      if (x->value == "&") {
        argument.is_pointer = true;
        x++;
      }

      if (x->type != Token::Type::IDENTIFIER && x->type != Token::Type::NUMBER) {
        error("unexpected token \"%tv\" at line %tl. expected an argument", *x);
        return argument;
      }

      argument.token = *x;

      return argument;
    }

    void handle_if_command(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator end, const ds::CommandInfo &command_info) {
      const size_t current_if_count = ++if_counter;
      const Token &if_token = *x;
      if (++x == end) {
        error("unexpected end of input: expected a condition for the \"if\" statement at line %tl", if_token);
        return;
      }

      handle_condition(x, end, "if", current_if_count);
      push_instruction("jmp", { .code = std::to_string(current_if_count) + "_if_else", .is_label = true });

      if (x == end) {
        error("unexpected end of input: do expected at line %tl", *(x - 1));
        return;
      }
      if (x->value != "do") {
        error("unexpected token: do expected at line %tl", *(x - 1));
        return;
      }
      x++;

      labels_map.insert(std::make_pair(std::to_string(current_if_count) + "_if", tls::str_to_hex(std::to_string(instructions.size()))));
      bool end_met = false;
      bool else_met = false;
      for (; x != end; x++) {
        if (x->value == "else") {
          push_instruction("jmp", { .code = std::to_string(current_if_count) + "_if_end", .is_label = true });
          labels_map.insert(std::make_pair(std::to_string(current_if_count) + "_if_else", tls::str_to_hex(std::to_string(instructions.size()))));
          else_met = true; 
        } else if (x->value == "end") {
          if (!else_met)
            labels_map.insert(std::make_pair(std::to_string(current_if_count) + "_if_else", tls::str_to_hex(std::to_string(instructions.size()))));

          labels_map.insert(std::make_pair(std::to_string(current_if_count) + "_if_end", tls::str_to_hex(std::to_string(instructions.size()))));
          end_met = true;
          break;
        } else {
          handle_command(x, end);
        }
      }

      if (!end_met) {
        error("unexpected end of input: \"end\" expected for the \"if\" statement at line %tl", if_token);
      }

      if (x == end) x--;
    }

    void handle_while_command(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator &end, const ds::CommandInfo &command_info) {
      const size_t current_while_count = ++while_counter;
      const Token &while_token = *x;
      if (x == end) {
        error("unexpected end of input: expected a condition for the \"while\" statement at line %tl", while_token);
        return;
      }

      labels_map.insert(std::make_pair("$" + std::to_string(current_while_count) + "_while_condition", tls::str_to_hex(std::to_string(instructions.size()))));
      handle_condition(++x, end, "while", current_while_count);
      push_instruction("jmp", { .code = "$" + std::to_string(current_while_count) + "_while_end", .is_label = true });
      labels_map.insert(std::make_pair("$" + std::to_string(current_while_count) + "_while", tls::str_to_hex(std::to_string(instructions.size()))));

      if (x == end) {
        error("unexpected end of input: do expected at line %tl", *(x - 1));
        return;
      }
      if (x->value != "do") {
        error("unexpected token: do expected at line %tl", *(x - 1));
        return;
      }
      x++;

      bool end_met = false;
      for (; x != end; x++) {
        if (x->value == "end") {
          push_instruction("jmp", { .code = "$" + std::to_string(current_while_count) + "_while_condition", .is_label = true });
          labels_map.insert(std::make_pair("$" + std::to_string(current_while_count) + "_while_end", tls::str_to_hex(std::to_string(instructions.size()))));
          end_met = true;
          break;
        } else {
          handle_command(x, end);
        }
      }

      if (!end_met) {
        error("unexpected end of input: \"end\" expected for the \"while\" statement at line %tl", while_token);
      }

      if (x == end) x--;
    }

    void handle_condition(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator &end, const std::string &statement, size_t counter) {
      handle_conditions_query(x, end, statement, counter);
    }

    void handle_conditions_query(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator &end, const std::string &statement, size_t counter) {
      const Token &starting_token = *(x - 1);
      size_t and_counter = 0;
      bool after_logic_operator = false;

      for (; x != end; x++) {
        if (x->value == "do") {
          if (after_logic_operator) {
            error("unexpected token \"do\": expected a condition at line %tl");
          }
          break;
        }

        if (x->value == "and") {
          labels_map.insert(std::make_pair(
            "$" + statement + "_" + std::to_string(counter) + "_" + std::to_string(++and_counter),
            tls::str_to_hex(std::to_string(instructions.size()))
          ));
          after_logic_operator = true;
          continue;
        }

        if (x->value == "or") {
          after_logic_operator = true;
          continue;
        }

        handle_single_condition(x, end, "$" + statement + "_" + std::to_string(counter) + "_" + std::to_string(and_counter + 1));
        after_logic_operator = false;
      }

      labels_map.insert(std::make_pair(
        "$" + statement + "_" + std::to_string(counter) + "_" + std::to_string(++and_counter),
        tls::str_to_hex(std::to_string(instructions.size() + 1))
      ));

      if (x == end) {
        x--;
        error("unexpected end of input: \"do\" expected for \"if\" at line %tl", starting_token);
      }
    }

    void handle_single_condition(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator &end, const std::string &label_name) {
      const Token &starting_token = *x;
      if (x->value == "[") {
        handle_condition_expression(x, end);
        x++;
      }

      if (x == end) {
        error("unexpected end of input: condition expected for expression at line %tl", starting_token);
      }

      if (ds::instructions_map.find("j" + x->value) != ds::instructions_map.end()) {
        push_instruction("j" + x->value, { .code = label_name, .is_label = true });
      } else {
        error("unexpected token \"%tv\": unknown condition", *x);
      }
    }

    void handle_condition_expression(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator &end) {
      const Token &starting_token = *x;
      x++;
      for (; x != end; x++) {
        if (x->value == "]") {
          break;
        } else {
          handle_command(x, end);
        }
      }

      if (x == end) {
        x--;
        error("unexpected end of input: \"]\" was expected for the block of commands started at line %tl", starting_token);
        return;
      }
    }

    void parse_segments(const std::vector<Token> &tokens) {
      auto end = tokens.end();
      for (auto x = tokens.begin(); x != end;) {
        parse_segment(x, end);
      }
    }

    void parse_segment(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator &end) {
      if (x->value != ".") {
        error("operator \".\" expected at line %tl", *x);
        return;
      }
      x++;
      if (x == end) {
        error("segment name expected at line %tl", *x);
        return;
      }

      const std::string segment_name = x->value;
      x++;

      if (segment_name == "data") {
        parse_data_segment(x, end);
      } else if (segment_name == "main") {
        parse_main_segment(x, end);
      }
    }

    void parse_data_segment(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator &end) {
      for (; x != end; x++) {
        if (x->value == ".") {
          return;
        }

        const std::string name = x->value;
        x++;
        if (x == end) {
          error("unexpected end of input: variable value was expected at line %tl", *(x - 1));
          return;
        }

        const std::string value = x->value;

        global_variables.insert(std::make_pair(name, VariableInfo { .value = value }));
      }
    }

    void parse_main_segment(std::vector<Token>::const_iterator &x, const std::vector<Token>::const_iterator &end) {
      if (is_main_defined) {
        error("main segment was already defined at line %tl", *main_start);
      }
      is_main_defined = true;
      main_start = x;

      for (; x != end; x++) {
        if (x->value == ".") {
          break;
        }
      }

      main_end = x;
    }

    void post_process_instructions() {
      post_process_labels();
      post_process_variables();
      post_process_commands();
    }

    void post_process_labels() {
      for (auto &[key, value] : labels_map) {
        value = std::string(3 - value.size(), '0') + value;
      }
    }

    void post_process_variables() {
      global_variables.insert(std::make_pair("$tmp", "00"));

      for (auto &[name, value] : global_variables) {
        value.address = tls::str_to_hex(std::to_string(instructions.size()));
        value.address = std::string(3 - value.address.size(), '0') + value.address;

        instructions.push_back(Instruction { .command { .code = "", .name = name }, .argumnet { .code = value.value }, .is_variable = true });
      }
    }

    void post_process_commands() {
      for (auto &instruction : instructions) {
        if (instruction.argumnet.is_variable) {
          instruction.argumnet.code = global_variables[instruction.argumnet.code].address;
        } else if (instruction.argumnet.is_label) {
          instruction.argumnet.code = labels_map[instruction.argumnet.code];
        }

        if (instruction.command.code.size() < 2)
          instruction.command.code = std::string(2 - instruction.command.code.size(), '0') + instruction.command.code;

        if (instruction.argumnet.code.size() < 2)
          instruction.argumnet.code = std::string(2 - instruction.argumnet.code.size(), '0') + instruction.argumnet.code;
      }
    }

    void push_instruction(const std::string name, const _Argument &argument) {
      instructions.push_back(Instruction {
        .command = _Command { .code = ds::instructions_map.at(name).code, .name = ds::instructions_map.at(name).name },
        .argumnet = argument
      });
    }

    void error(std::string message, const Token &token = Token()) {
      _failed = true;

      for (size_t pos = message.find("%tv"); pos != std::string::npos; pos = message.find("%tv"))
        message.replace(pos, 3, token.value);
      for (size_t pos = message.find("%tl"); pos != std::string::npos; pos = message.find("%tv"))
        message.replace(pos, 3, std::to_string(token.line));

      Logger::error("translator exception: " + message);

      throw std::runtime_error("translator exception: " + message);
    }
  };
}

#endif // !TRANSLATOR_HPP
