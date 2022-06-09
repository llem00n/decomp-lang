#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

namespace dl {
  struct Token {
    enum class Type {
      UNSTABLE_NUMBER,
      NUMBER,
      IDENTIFIER,
      OPERATOR,
      UNDEFINED,
      ERROR,
    };

    std::string value;
    Type type;
    size_t line;

    Token() 
      : type(Type::UNDEFINED)
      , line(0u)
    { }

    Token(const Token& token)
      : value(token.value)
      , type(token.type)
      , line(token.line)
    { }

    Token(Token &&token) noexcept
      : value(std::move(token.value))
      , type(std::exchange(token.type, Type::UNDEFINED))
      , line(std::exchange(token.line, 0u))
    { }

    Token &operator =(const Token &token) {
      value = token.value;
      type = token.type;
      line = token.line;

      return *this;
    }

    Token &operator =(Token &&token) noexcept {
      value = std::move(token.value);
      type = std::exchange(token.type, Type::UNDEFINED);
      line = std::exchange(token.line, 0u);

      return *this;
    }
  };
}

#endif // !TOKEN_HPP
