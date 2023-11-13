#pragma once

#include <variant>
#include <optional>
#include <istream>
#include "error.h"
#include "vector"
#include "string"

struct SymbolToken {
    std::string name;

    SymbolToken(const char* name) : name(name){};
    SymbolToken(std::string& name) : name(name){};

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    ConstantToken(int value) : value(value){};

    bool operator==(const ConstantToken& other) const;
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    bool IsEnd();

    void Next();

    Token GetToken();

private:
    Token token_;
    bool is_end_ = false;
    std::istream* in_;
};