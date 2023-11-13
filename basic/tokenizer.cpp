#include <tokenizer.h>
#define CHECK_IF_SYMBOL                                                                            \
    (std::isalnum(symbol) || (symbol <= '>' && symbol >= '<') || symbol == '*' || symbol == '#' || \
     symbol == '/' || symbol == '-' || symbol == '!' || symbol == '?') &&                          \
        symbol != EOF

Tokenizer::Tokenizer(std::istream *in) : in_(in) {
    Next();
}

bool &Tokenizer::IsEnd() {
    return is_end_;
}

Token Tokenizer::GetToken() {
    return token_;
}
void Tokenizer::Next() {
    char symbol = in_->get();
    while (std::isspace(symbol)) {
        symbol = in_->get();
    }
    if (symbol == EOF) {
        is_end_ = true;
        return;
    }
    if (symbol == '(') {
        token_ = BracketToken::OPEN;
    } else if (symbol == ')') {
        token_ = BracketToken::CLOSE;
    } else if (symbol == '+' || symbol == '-') {
        std::string num;
        num.push_back(symbol);
        symbol = in_->peek();
        if (symbol <= '9' && symbol >= '0') {
            symbol = in_->get();
        }
        while (symbol <= '9' && symbol >= '0') {
            num.push_back(symbol);
            symbol = in_->peek();
            if (symbol <= '9' && symbol >= '0') {
                symbol = in_->get();
            }
        }
        if (num.length() > 1) {
            token_ = ConstantToken(std::stoi(num));  // NOLINT
        } else {
            token_ = SymbolToken(num);
        }
    } else if ((symbol <= '9' && symbol >= '0')) {
        std::string num;
        while (symbol <= '9' && symbol >= '0') {
            num.push_back(symbol);
            symbol = in_->peek();
            if (symbol <= '9' && symbol >= '0') {
                symbol = in_->get();
            }
        }
        if (symbol != EOF && !CHECK_IF_SYMBOL && symbol != '(' && symbol != ')' && symbol != '.' &&
            symbol != '-' && symbol != '+' && !std::isspace(symbol)) {
            throw SyntaxError{"undefined symbol"};
        }
        token_ = ConstantToken(std::stoi(num));  // NOLINT
    } else if (symbol == '\'') {
        token_ = QuoteToken();
    } else if (symbol == '.') {
        token_ = DotToken();
    } else if (std::isalpha(symbol) || (symbol <= '>' && symbol >= '<') || symbol == '*' ||
               symbol == '#' || symbol == '/') {
        std::string name;
        while (CHECK_IF_SYMBOL) {
            name.push_back(symbol);
            symbol = in_->peek();
            if (CHECK_IF_SYMBOL) {
                symbol = in_->get();
            }
        }
        if (symbol != EOF && !CHECK_IF_SYMBOL && symbol != '(' && symbol != ')' && symbol != '.' &&
            symbol != '-' && symbol != '+' && !std::isspace(symbol)) {
            throw SyntaxError{"undefined symbol"};
        }
        token_ = SymbolToken(name);
    } else {
        throw SyntaxError("undefined symbol");
    }
    tokens_.push_back(token_);
}
bool SymbolToken::operator==(const SymbolToken &other) const {
    return other.name == name;
}
bool QuoteToken::operator==(const QuoteToken &) const {
    return true;
}
bool DotToken::operator==(const DotToken &) const {
    return true;
}
bool ConstantToken::operator==(const ConstantToken &other) const {
    return other.value == value;
}

std::istream *Tokenizer::GetStream() const {
    return in_;
}
