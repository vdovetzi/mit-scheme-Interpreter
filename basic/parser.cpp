#include <parser.h>
#include "deque"
#include "scheme.h"
#include "cstdint"

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (std::count(tokenizer->GetAllTokens().begin(), tokenizer->GetAllTokens().end(),
                   Token{BracketToken::CLOSE}) > std::count(tokenizer->GetAllTokens().begin(),
                                                            tokenizer->GetAllTokens().end(),
                                                            Token{BracketToken::OPEN})) {
        throw SyntaxError{"Brackets, bruuuuh"};
    }
    Token curr_token = tokenizer->GetToken();
    tokenizer->Next();
    if (curr_token == Token{BracketToken::OPEN}) {
        ++tokenizer->brackets_cnt;
        std::shared_ptr<Object> node = ReadList(tokenizer);
        tokenizer->Next();
        if (tokenizer->IsEnd()) {
            std::vector<Token> tokens = tokenizer->GetAllTokens();
            int closed = std::count(tokens.begin(), tokens.end(), Token{BracketToken::CLOSE});
            if (closed != std::count(tokens.begin(), tokens.end(), Token{BracketToken::OPEN})) {
                throw SyntaxError{"Brackets, bruuuuh"};
            }
            if (std::count(tokens.begin(), tokens.end(), Token{QuoteToken()}) != 0) {
                int cnt = 0;
                bool flag = false;
                int i = 0;
                int count_of_opened = 0;
                while (tokens[i] != Token{QuoteToken()}) {
                    if (tokens[i] == Token{BracketToken::OPEN}) {
                        ++count_of_opened;
                    } else if (tokens[i] == Token{BracketToken::CLOSE}) {
                        --count_of_opened;
                    }
                    ++i;
                }
                for (size_t j = i; j < tokens.size() - count_of_opened; ++j) {
                    if (tokens[j] == Token{BracketToken::OPEN}) {
                        ++cnt;
                    } else if (tokens[j] == Token{BracketToken::CLOSE}) {
                        --cnt;
                        if (cnt == 0) {
                            flag = true;
                        }
                    }
                    if (cnt != 0 && flag) {
                        throw RuntimeError{"A shit after quote"};
                    }
                }
            }
        }
        return node;
    } else if (ConstantToken* x = std::get_if<ConstantToken>(&curr_token)) {
        if (!x->is_init) {
            throw SyntaxError{"Error"};
        }
        return std::shared_ptr<Object>(new Number(x->value));
    } else if (curr_token == Token{DotToken()}) {
        Token next = tokenizer->GetToken();
        if (next == Token{BracketToken::CLOSE}) {
            throw SyntaxError{".)"};
        }
        if (tokenizer->IsEnd() && tokenizer->GetToken() == Token{DotToken()}) {
            throw SyntaxError{"......."};
        }
        if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
            --tokenizer->brackets_cnt;
        }
        return Read(tokenizer);
    } else if (curr_token == Token{QuoteToken()}) {
        if (tokenizer->IsEnd() && tokenizer->GetToken() == Token{QuoteToken()}) {
            throw SyntaxError{"Nothing to quote"};
        }
        if (tokenizer->IsEnd()) {
            return nullptr;
        }
        return std::shared_ptr<Object>(
            new Cell(std::shared_ptr<Object>(new Symbol("quote")), Read(tokenizer)));
    } else if (SymbolToken* x3 = std::get_if<SymbolToken>(&curr_token)) {
        if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
            --tokenizer->brackets_cnt;
        }
        return std::shared_ptr<Object>(new Symbol(x3->name));
    } else if (curr_token == Token{BracketToken::CLOSE}) {
        --tokenizer->brackets_cnt;
        Token next = tokenizer->GetToken();
        if (ConstantToken* x4 = std::get_if<ConstantToken>(&next)) {
            return std::shared_ptr<Object>(new Number(x4->value));
        }
        return nullptr;
    }
    return std::shared_ptr<Object>();
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    while (true) {
        if (tokenizer->IsEnd() && tokenizer->GetToken() != Token{BracketToken::CLOSE}) {
            throw SyntaxError{"Error!"};
        }
        if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
            std::istream* ss = tokenizer->GetStream();
            char curr = ss->peek();
            int i = 0;
            while (i < 100 && curr != ')') {
                curr = ss->peek();
                ++i;
                if (curr == EOF) {
                    break;
                }
            }
            if (curr == ')') {
                --tokenizer->brackets_cnt;
            }
            auto ptr = Read(tokenizer);
            return ptr;
        }
        if (tokenizer->GetToken() == Token{DotToken()}) {
            int pos = static_cast<int>(tokenizer->GetAllTokens().size()) - 3;
            if (pos < 0) {
                throw SyntaxError{".num"};
            }
            tokenizer->Next();
            Token next = tokenizer->GetToken();
            if (std::get_if<ConstantToken>(&next)) {
                std::istream* ss = tokenizer->GetStream();
                char curr = ss->peek();
                int i = 0;
                while (i < 100 && curr == ' ') {
                    curr = ss->peek();
                    ++i;
                    if (curr == EOF) {
                        break;
                    }
                }
                if (curr != ')') {
                    throw SyntaxError{".num num"};
                }
            }
            if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
                throw SyntaxError{".)"};
            }
            auto ptr = Read(tokenizer);
            if (tokenizer->GetToken() == Token{BracketToken::CLOSE}) {
                --tokenizer->brackets_cnt;
            }
            return ptr;
        } else {
            return std::shared_ptr<Object>(new Cell(Read(tokenizer), ReadList(tokenizer)));
        }
    }
}