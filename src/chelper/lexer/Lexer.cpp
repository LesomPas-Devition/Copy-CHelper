//
// Created by Yancey on 2023/11/6.
//

#include "Lexer.h"

//字符串的结束字符
const std::string endChars = " ,@~^/$&\"'!#%+*=[{]}\\|<>`\n";
//可以被识别成符号的字符，这些字符不一定是字符串的结束字符
const std::string symbols = ",@~^/$&'!#%+*=[{]}\\|<>`+-=:";

bool isNum(char ch) {
    return (ch >= '0' && ch <= '9') || ch == '.';
}

bool isEndChar(char ch) {
    return std::any_of(endChars.begin(), endChars.end(), [&ch](const char &endChar) {
        return ch == endChar;
    });
}

bool isSymbol(char ch) {
    return std::any_of(symbols.begin(), symbols.end(), [&ch](const char &endChar) {
        return ch == endChar;
    });
}

CHelper::Lexer::Lexer(const std::string &content,
                      const std::string &filename)
        : stringReader(content, filename) {}

CHelper::Lexer::Lexer(StringReader stringReader)
        : stringReader(stringReader) {}

std::vector<CHelper::Token> CHelper::Lexer::lex() {
    Profile::push("start lex: " + stringReader.content);
    std::vector<Token> tokenList = std::vector<Token>();
    while (true) {
        if (stringReader.peek() == EOF) {
            break;
        }
        switch (nextTokenType()) {
            case CHelper::TokenType::NUMBER:
                tokenList.push_back(nextTokenNumber());
                break;
            case CHelper::TokenType::SYMBOL:
                tokenList.push_back(nextTokenSymbol());
                break;
            case CHelper::TokenType::STRING:
                tokenList.push_back(nextTokenString());
                break;
            case CHelper::TokenType::WHITE_SPACE:
                tokenList.push_back(nextTokenWhiteSpace());
                break;
            case TokenType::LF:
                tokenList.push_back(nextTokenLF());
                break;
        }
    }
    Profile::pop();
    return tokenList;
}

CHelper::TokenType::TokenType CHelper::Lexer::nextTokenType() {
    char ch = stringReader.peek();
    if (ch == '\n') {
        return CHelper::TokenType::LF;
    } else if (ch == ' ') {
        return CHelper::TokenType::WHITE_SPACE;
    } else if (isNum(ch)) {
        return CHelper::TokenType::NUMBER;
    } else if (ch == '+' || ch == '-') {
        stringReader.mark();
        ch = stringReader.next();
        stringReader.reset();
        if (isNum(ch)) {
            return CHelper::TokenType::NUMBER;
        } else {
            return CHelper::TokenType::SYMBOL;
        }
    } else if (isSymbol(ch)) {
        return CHelper::TokenType::SYMBOL;
    } else {
        return CHelper::TokenType::STRING;
    }
}

CHelper::Token CHelper::Lexer::nextTokenNumber() {
    stringReader.mark();
    char ch = stringReader.peek();
    if (ch == '+' || ch == '-') {
        ch = stringReader.next();
    }
    while (isNum(ch)) {
        ch = stringReader.next();
    }
    return {CHelper::TokenType::NUMBER, stringReader.posBackup, stringReader.collect()};
}

CHelper::Token CHelper::Lexer::nextTokenSymbol() {
    char ch = stringReader.peek();
    std::string str = std::string();
    str.push_back(ch);
    Token result = {CHelper::TokenType::SYMBOL, stringReader.pos, str};
    stringReader.skip();
    return result;
}

CHelper::Token CHelper::Lexer::nextTokenString() {
    stringReader.mark();
    char ch = stringReader.peek();
    bool containSpace = ch == '"';
    if (containSpace) {
        ch = stringReader.next();
    }
    while (true) {
        if (ch == EOF) {
            break;
        }
        if (containSpace) {
            if (ch == '"') {
                stringReader.skip();
                break;
            }
        } else if (isEndChar(ch)) {
            break;
        }
        ch = stringReader.next();
    }
    return {CHelper::TokenType::STRING, stringReader.posBackup, stringReader.collect()};
}

CHelper::Token CHelper::Lexer::nextTokenWhiteSpace() {
    char ch = stringReader.peek();
    std::string str = std::string();
    str.push_back(ch);
    Token result = {CHelper::TokenType::WHITE_SPACE, stringReader.pos, str};
    stringReader.skip();
    return result;
}

CHelper::Token CHelper::Lexer::nextTokenLF() {
    char ch = stringReader.peek();
    std::string str = std::string();
    str.push_back(ch);
    Token result = {CHelper::TokenType::LF, stringReader.pos, str};
    stringReader.skip();
    return result;
}