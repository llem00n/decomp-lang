#include <iostream>
#include <fstream>
#include "Lexer.hpp"
#include "Translator.hpp"
#include "Formatter.hpp"

void print_usage(char *);

int main(int argc, char **argv) {
  if (argc <= 1) {
    print_usage(argv[0]);
    return 1;
  }

  std::ifstream src(argv[1]);
  std::ofstream target(argv[2]);

  dl::Logger::init(std::cout);

  dl::Lexer lexer;
  auto tokens = lexer.parse(src);
  if (lexer.failed())
    return 1;

  dl::Translator translator;
  auto instructions = translator.translate(tokens);
  if (translator.failed())
    return 1;

  dl::Formatter::set_source_instructions(instructions);

  std::cout << dl::Formatter::instructions_format() << "\n\n";
  target << dl::Formatter::decomp_format();
}

void print_usage(char *filename) {
  printf("Usage:\n$ %s <source file> <target file>\n", filename);
}