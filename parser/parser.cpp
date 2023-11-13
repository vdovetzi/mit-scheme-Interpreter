#include <parser.h>
#include "deque"

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
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
        return std::shared_ptr<Object>(
            new Cell(std::shared_ptr<Object>(new Symbol("quote")), ReadList(tokenizer)));
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

            tokenizer->Next();
            Token next = tokenizer->GetToken();
            if (ConstantToken* x = std::get_if<ConstantToken>(&next)) {
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