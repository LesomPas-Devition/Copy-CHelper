//
// Created by Yancey666 on 2023/11/6.
//

#ifndef CHELPER_LEXER_H
#define CHELPER_LEXER_H

#include <vector>
#include "StringReader.h"
#include "Token.h"

class Lexer {

    StringReader stringReader;

public:

    [[maybe_unused]] explicit Lexer(std::string content, std::string filename);

    [[maybe_unused]] explicit Lexer(StringReader stringReader);

    std::vector<Token> lex();

    TokenType nextTokenType();

    Token nextTokenNumber(bool whiteSpace);

    Token nextTokenSymbol(bool whiteSpace);

    Token nextTokenString(bool whiteSpace);
};

bool isNum(char ch);

bool isEndChar(char ch);

bool isSymbol(char ch);

#endif //CHELPER_LEXER_H
