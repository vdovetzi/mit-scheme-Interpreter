#pragma once

#include <variant>
#include <optional>
#include <istream>
#include "error.h"
#include "vector"
#include "string"

struct SymbolToken {
    std::string name;

    SymbolToken() = default;

    SymbolToken(const char* name) : name(name){};
    SymbolToken(std::string& name) : name(name){};

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    QuoteToken() = default;
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    DotToken() = default;
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int64_t value;

    bool is_init = false;

    ConstantToken() = default;

    ConstantToken(int value) : value(value), is_init(true){};

    bool operator==(const ConstantToken& other) const;
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    bool& IsEnd();

    void Next();

    Token GetToken();

    std::istream* GetStream() const;

    std::vector<Token>& GetAllTokens() {
        return tokens_;
    }

    int brackets_cnt = 0;

private:
    bool is_end_ = false;
    std::istream* in_;
    Token token_;
    std::vector<Token> tokens_;
};
