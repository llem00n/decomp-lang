#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include <istream>
#include <sstream>
#include <algorithm>
#include "Token.hpp"
#include "Logger.hpp"
#include "DataSets.hpp"
#include "Tools.hpp"

namespace dl {
  class Lexer {
    bool _failed;

  public:
    Lexer()
      : _failed(false) 
    { }

    std::vector<Token> parse(std::istream &istream) {
      reset();

      std::vector<Token> result;
      size_t line_number = 1;

      for (std::string line; std::getline(istream, line); line_number++) {
        std::vector<std::string> words = split_line(line);

        for (auto &word : words) {
          if (is_comment(word))
            break;

          std::vector<Token> tokens = tokens_from_word(word);
          for (auto &token : tokens) {
            token.line = line_number;
            if (token.type == Token::Type::ERROR) {
              error(token);

              return result;
            }

            result.push_back(token);
          }
        }
      }

      return result;
    }

    bool failed() {
      return _failed;
    }

    void reset() {
      _failed = false;
    }

  private:
    std::vector<std::string> split_line(std::string line) {
      std::vector<std::string> result;

      for (size_t pos = 0; (pos = line.find_first_of(" \t\b\f\r")) != std::string::npos;) {
        std::string token = line.substr(0, pos);
        if (token.size())
          result.push_back(token);
        line.erase(0, pos + 1);
      }
      if (line.size())
        result.push_back(line);

      return result;
    }

    std::vector<Token> tokens_from_word(std::string word) {
      std::vector<Token> tokens;
      std::transform(word.begin(), word.end(), word.begin(), 
        [] (const char &c) -> char { return std::tolower(c); });

      while (word.size()) {
        Token token;
        size_t offset = 0;
        size_t last_ok_offset = 0;

        do {
          offset++;
          token.type = get_token_type(word.substr(word.size() - offset, offset));
          if (token.type != Token::Type::ERROR)
            last_ok_offset = offset;
        } while (word.size() - offset);

        if (last_ok_offset) {
          token.type = get_token_type(word.substr(word.size() - last_ok_offset, last_ok_offset));
          token.value = word.substr(word.size() - last_ok_offset, last_ok_offset);

          if (token.type == Token::Type::NUMBER)
            token.value = tls::str_to_hex(token.value);
        } else {
          token.value = word;
          token.type = Token::Type::ERROR;
        }
        
        /*token.value = word.substr(word.size() - offset, offset);
        if (token.type == Token::Type::ERROR) {
          offset--;
          if (offset == 0) {
            tokens.push_back(token);
            break;
          }
          token.value = word.substr(word.size() - offset, offset);
          token.type = get_token_type(word.substr(word.size() - offset, offset));
        } else if (token.type == Token::Type::UNSTABLE_NUMBER) {
          token.type = Token::Type::ERROR;
          tokens.push_back(token);
          break;
        } else if (token.type == Token::Type::NUMBER) {
          token.value = tls::str_to_hex(word.substr(word.size() - offset, offset));
        }*/

        word = word.substr(0, word.size() - last_ok_offset);
        tokens.push_back(token);
      }

      std::reverse(tokens.begin(), tokens.end());

      return tokens;
    }

    Token::Type get_token_type(const std::string &word) {
      if (is_number(word)) {
        return Token::Type::NUMBER;
      } else if (is_identifier(word)) {
        return Token::Type::IDENTIFIER;
      } else if (is_operator(word)) {
        return Token::Type::OPERATOR;
      } else if (is_unstable_number(word)) {
        return Token::Type::UNSTABLE_NUMBER;
      } else {
        return Token::Type::ERROR;
      }
    }

    bool is_number(std::string string) {
      if (!string.size())
        return false;

      size_t base = 10;
      if (string.size() >= 2 && string[0] == '0') {
        switch (string[1]) {
          case 'x':
            base = 16;
            string.erase(0, 2);
            break;
          case 'o':
            base = 8;
            string.erase(0, 2);
            break;
          case 'b':
            base = 2;
            string.erase(0, 2);
            break;
        }
      }

      const std::string alphabet = "0123456789abcdef";
      for (size_t index = 0; index < string.size(); index++) {
        char c = std::tolower(string[index]);
        if (alphabet.find(c) >= base)
          return false;
      }

      return true;
    }

    bool is_unstable_number(std::string string) {
      if (!string.size())
        return false;

      const std::string alphabet = "0123456789abcdef";
      for (size_t index = 0; index < string.size(); index++) {
        char c = std::tolower(string[index]);
        if (alphabet.find(c) == std::string::npos)
          return false;
      }

      return true;
    }

    bool is_identifier(const std::string &string) {
      if (!string.size())
        return false;

      if (!((string[0] >= 'a' && string[0] <= 'z') || (string[0] >= 'A' && string[0] <= 'Z') || string[0] == '_'))
        return false;

      for (size_t x = 1; x < string.size(); x++) {
        if (!std::isalpha(string[x]) && string[x] != '_' && !std::isdigit(string[x]))
          return false;
      }

      return true;
    }

    bool is_operator(const std::string &word) {
      return std::find(ds::operators.begin(), ds::operators.end(), word) != ds::operators.end();
    }

    bool is_comment(const std::string &word) {
      return (word.size() && word[0] == '#');
    }

    void error(const Token &token) {
      _failed = true;
      Logger::error("lexer exception at line " + std::to_string(token.line) + ": unexpected token \"" + token.value + "\"\n");
    }
  };
}

#endif // !LEXER_HPP
