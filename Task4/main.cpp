#include "lexer.h"
#include <iostream>
#include <regex>
#include <stdexcept>

using namespace std;

int main() {
    parser parser1("parser.in");
    parser1.create();
    lexer lexer1("lexer.in", parser1.token_types);
    lexer1.tokenize();
    parser1.define_type = lexer1.define_type;
    lexer1.make_file();
    parser1.make_file();
    return 0;
}
