#include "scheme.h"

std::shared_ptr<Object> Interpreter::Evaluate(std::vector<std::shared_ptr<Object>> nodes) {
    std::deque<std::shared_ptr<Object>> st_op;
    std::deque<std::shared_ptr<Object>> st_res;
    if (nodes.size() == 1 &&
        (Is<Number>(nodes[0]) ||
         (Is<Symbol>(nodes[0]) &&
          operations.find(As<Symbol>(nodes[0])->GetName()) == operations.end()))) {
        return nodes[0];
    }
    for (auto node : nodes) {
        if (Is<Symbol>(node)) {
            if (operations.find(As<Symbol>(node)->GetName()) != operations.end()) {
                if (st_op.empty()) {
                    std::shared_ptr<Object> operation = operations[As<Symbol>(node)->GetName()]();
                    st_op.push_back(operation);
                    if (As<Symbol>(node)->GetName() != "car" &&
                        As<Symbol>(node)->GetName() != "cdr") {
                        st_res.push_back(operation->Apply(nullptr));
                    } else {
                        st_res.push_back(
                            operation->Apply(std::shared_ptr<Object>(new Symbol("nullptr"))));
                    }
                } else {
                    st_res.pop_back();
                    st_res.push_back(st_op.back()->Apply(node));
                }
            } else {
                if (!st_op.empty()) {
                    // проверка на ошибки
                    if (st_res.empty()) {
                        st_res.push_back(st_op.back()->Apply(node));
                    } else {
                        st_res.pop_back();
                        st_res.push_back(st_op.back()->Apply(node));
                    }
                } else {
                    throw RuntimeError{"expression cannot be evaluated"};
                }
            }
        } else {
            if (st_op.empty()) {
                throw RuntimeError{"expression cannot be evaluated"};
            }
            // проверка на ошибки
            if (!st_res.empty()) {
                st_res.pop_back();
                st_res.push_back(st_op.back()->Apply(node));
            }
        }
    }
    if (!st_op.empty()) {
        st_op.back().reset();
        st_op.pop_back();
    }
    while (!st_op.empty()) {
        std::shared_ptr<Object> curr_operation = st_op.back();
        st_op.back().reset();
        st_op.pop_back();
        if (!st_res.empty()) {
            std::shared_ptr<Object> last_res = st_res.back();
            st_res.pop_back();
            st_res.push_back(curr_operation->Apply(last_res));
        } else {
            // обработка ошибок
        }
    }
    if (st_res.empty()) {
        throw RuntimeError{"expression cannot be evaluated"};
    }
    return st_res.back();
}
void Interpreter::AstToVector(std::shared_ptr<Object> node,
                              std::vector<std::shared_ptr<Object>>& result) {
    if (Is<Cell>(node)) {
        if (As<Cell>(node)->GetFirst() == nullptr && As<Cell>(node)->GetSecond() == nullptr) {
            result.push_back(node);
            return;
        }
        if (Is<Symbol>(As<Cell>(node)->GetFirst())) {
            if (As<Symbol>(As<Cell>(node)->GetFirst())->GetName() == "quote") {
                if (As<Cell>(node)->GetFirst() != nullptr) {
                    result.push_back(As<Cell>(node)->GetFirst());
                }
                if (As<Cell>(node)->GetSecond() != nullptr) {
                    result.push_back(As<Cell>(node)->GetSecond());
                }
                return;
            }
        }
        AstToVector(As<Cell>(node)->GetFirst(), result);
        AstToVector(As<Cell>(node)->GetSecond(), result);
        return;
    } else {
        if (node != nullptr) {
            result.push_back(node);
        }
        return;
    }
}

void Interpreter::SerializeQuote(std::shared_ptr<Object> node, std::string& result) {
    if (Is<Cell>(node)) {
        std::shared_ptr<Object> left = As<Cell>(node)->GetFirst(),
                                right = As<Cell>(node)->GetSecond();
        if (left != nullptr && !Is<Cell>(right) && right != nullptr) {
            result += left->Serialize() + ". " + right->Serialize();
            if (std::isspace(result.back())) {
                result.pop_back();
            }
            result.push_back(')');
            return;
        } else if (left != nullptr && Is<Cell>(right)) {
            result += left->Serialize();
            if (Is<Symbol>(left)) {
                result += " ";
            }
            SerializeQuote(right, result);
        } else if (left != nullptr && right == nullptr) {
            result += left->Serialize();
            if (std::isspace(result.back())) {
                result.pop_back();
            }
            result.push_back(')');
            return;
        } else {
            result += "())";
            return;
        }
    } else {
        if (node != nullptr) {
            result += node->Serialize() + ' ';
        } else {
            result += ")";
        }
    }
}

void Interpreter::Serialize(std::vector<std::shared_ptr<Object>>& nodes,
                            std::vector<std::shared_ptr<Object>>& original, std::string& result) {
    if (!original.empty() && (Is<Symbol>(original[0]) || Is<Cell>(original[0]))) {
        if (As<Symbol>(original[0])->GetName() == "quote") {
            if (original.size() > 1) {
                if (Is<Cell>(original[1])) {
                    result += '(';
                    if (Is<Cell>(As<Cell>(original[1])->GetFirst())) {
                        SerializeQuote(As<Cell>(original[1])->GetFirst(), result);
                        return;
                    } else {
                        SerializeQuote(original[1], result);
                        return;
                    }
                } else {
                    SerializeQuote(original[1], result);
                    if (std::isspace(result.back())) {
                        result.pop_back();
                    }
                    result += ')';
                    return;
                }
            }
        }
    }

    if (nodes.empty()) {
        result = "()";
        return;
    }
    std::shared_ptr<Object> prev;
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (Is<Cell>(nodes[i])) {
            result += "(())";
        } else if (Is<Symbol>(nodes[i])) {
            if (As<Symbol>(nodes[i])->GetName() == "quote") {
                result += "(quote ";
            } else {
                if (nodes.size() == 1) {
                    result += nodes[i]->Serialize();
                } else if (nodes.size() > 1) {
                    if (result.empty()) {
                        result += "(" + nodes[i]->Serialize() + " ";
                    } else {
                        result += nodes[i]->Serialize() + " ";
                        if (i == nodes.size() - 1) {
                            if (std::isspace(result.back())) {
                                result.pop_back();
                            }
                            result.push_back(')');
                        }
                    }
                }
            }
        } else {
            size_t j = i;
            size_t cnt = 0;
            while (j < nodes.size() && Is<Number>(nodes[j])) {
                ++j;
                ++cnt;
            }
            if (cnt == 1 && Is<Number>(prev)) {
                result += nodes[i]->Serialize();
                if (std::isspace(result.back())) {
                    result.pop_back();
                }
                result.push_back(')');
            } else if (cnt == 1 && !Is<Number>(prev)) {
                result += nodes[i]->Serialize();
                if (std::isspace(result.back())) {
                    result.pop_back();
                }
            } else {
                if (Is<Number>(prev)) {
                    result += nodes[i]->Serialize();
                } else {
                    result += "(" + nodes[i]->Serialize();
                }
            }
        }
        prev = nodes[i];
    }
}

void Interpreter::IntegerTypeChecker(std::vector<Token>& tokens) {
    bool have_question_qualifier = false;
    bool quote_flag = false;
    Token prev_token = 0;
    for (size_t i = 0; i < tokens.size(); ++i) {
        bool is_operation = false;
        if (SymbolToken* x = std::get_if<SymbolToken>(&tokens[i])) {
            if (x->name.back() == '?') {
                have_question_qualifier = true;
            }
            if (x->name == "quote") {
                quote_flag = true;
            }
        }
        if (tokens[i] == Token{QuoteToken()}) {
            quote_flag = true;
        }
        if (SymbolToken* x = std::get_if<SymbolToken>(&tokens[i])) {
            if (operations.find(x->name) == operations.end()) {
                if (!have_question_qualifier) {
                    throw RuntimeError{"not an operation cannot be used in arithmetic expression"};
                }
            } else {
                is_operation = true;
            }
        }
        if (ConstantToken* y = std::get_if<ConstantToken>(&tokens[i])) {
            prev_token = tokens[i];
            continue;
        }
        if (!quote_flag && tokens[i] == Token{BracketToken::CLOSE} &&
            prev_token == Token{BracketToken::OPEN}) {
            throw RuntimeError{
                "not an operation or number cannot be used in arithmetic expression"};
        }
        if (!is_operation && !quote_flag && !have_question_qualifier &&
            (tokens[i] != Token{BracketToken::OPEN} && tokens[i] != Token{BracketToken::CLOSE})) {
            throw RuntimeError{"not an operation cannot be used in arithmetic expression"};
        }
        prev_token = tokens[i];
    }
}

void Interpreter::BinaryOperationsChecker(std::vector<std::shared_ptr<Object>>& nodes) {
    for (size_t i = 0; i < nodes.size(); ++i) {
        bool is_binary = false;
        if (Is<Symbol>(nodes[i])) {
            if (As<Symbol>(nodes[i])->GetName() == "/" || As<Symbol>(nodes[i])->GetName() == "-") {
                is_binary = true;
            }
        }
        if (is_binary) {
            size_t j = i + 1;
            size_t cnt = 0;
            while (j < nodes.size() && Is<Number>(nodes[j])) {
                ++cnt;
                ++j;
            }
            if (cnt < 2) {
                throw RuntimeError{"Binary operation has fewer than 2 arguments"};
            }
        }
    }
}

void Interpreter::UnaryOperationChecker(std::vector<std::shared_ptr<Object>>& nodes) {
    for (size_t i = 0; i < nodes.size(); ++i) {
        bool is_unary = false;
        if (Is<Symbol>(nodes[i])) {
            if (As<Symbol>(nodes[i])->GetName() == "max" ||
                As<Symbol>(nodes[i])->GetName() == "min" ||
                As<Symbol>(nodes[i])->GetName() == "abs" ||
                As<Symbol>(nodes[i])->GetName() == "not" ||
                As<Symbol>(nodes[i])->GetName() == "car" ||
                As<Symbol>(nodes[i])->GetName() == "cdr") {
                is_unary = true;
            }
        }
        if (is_unary) {
            size_t j = i + 1;
            size_t cnt = 0;
            while (j < nodes.size() && (Is<Number>(nodes[j]) || Is<Symbol>(nodes[j]))) {
                ++cnt;
                ++j;
            }
            if (cnt < 1) {
                throw RuntimeError{"Unary operation has fewer than 1 arguments"};
            }
        }
    }
}

void Interpreter::RequiresOnlyOneArgumentChecker(std::vector<std::shared_ptr<Object>>& nodes) {
    for (size_t i = 0; i < nodes.size(); ++i) {
        bool requires_only_one = false;
        bool is_not = false;
        if (Is<Symbol>(nodes[i])) {
            if (As<Symbol>(nodes[i])->GetName() == "abs" ||
                As<Symbol>(nodes[i])->GetName() == "not" ||
                As<Symbol>(nodes[i])->GetName() == "car" ||
                As<Symbol>(nodes[i])->GetName() == "cdr") {
                requires_only_one = true;
            }
            if (As<Symbol>(nodes[i])->GetName() == "not") {
                is_not = true;
            }
        }
        if (requires_only_one) {
            size_t j = i + 1;
            size_t cnt = 0;
            while (j < nodes.size() && (Is<Number>(nodes[j]) || Is<Symbol>(nodes[j]))) {
                ++cnt;
                ++j;
                if (Is<Symbol>(nodes[j - 1]) &&
                    (operations.find(As<Symbol>(nodes[j - 1])->GetName()) != operations.end())) {
                    break;
                }
            }
            if (cnt > 1 || (!is_not && cnt == 1 && Is<Symbol>(nodes[j - 1]) &&
                            As<Symbol>(nodes[j - 1])->GetName() == "quote" && j == nodes.size())) {
                throw RuntimeError{"An operation requires only one argument"};
            }
        }
    }
}

void Interpreter::ListsAreNotSelfEvaluating(std::vector<Token>& tokens) {
    bool has_operation = false;
    bool has_brackets = false;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == Token{BracketToken::OPEN}) {
            has_brackets = true;
        }
        if (SymbolToken* x = std::get_if<SymbolToken>(&tokens[i])) {
            if (operations.find(x->name) != operations.end()) {
                has_operation = true;
            }
        }
        if (tokens[i] == Token{QuoteToken()}) {
            has_operation = true;
        }
    }
    if (has_brackets && !has_operation) {
        throw RuntimeError{"Lists Are Not Self Evaluating"};
    }
}

std::shared_ptr<Object> Interpreter::AndEvaluate(std::vector<std::shared_ptr<Object>>& nodes) {
    std::vector<std::shared_ptr<Object>> curr_test;
    std::shared_ptr<Object> last_result;
    size_t i = 1, j = 0;
    while (i < nodes.size()) {
        if (Is<Symbol>(nodes[i])) {
            curr_test.clear();
            if (operations.find(As<Symbol>(nodes[i])->GetName()) != operations.end()) {
                curr_test.push_back(nodes[i]);
                j = i + 1;
                while (j < nodes.size() &&
                       (!Is<Symbol>(nodes[j]) ||
                        (Is<Symbol>(nodes[j]) &&
                         operations.find(As<Symbol>(nodes[j])->GetName()) == operations.end()))) {
                    curr_test.push_back(nodes[j]);
                    ++j;
                }
                i = j;
                std::shared_ptr<Object> result = Evaluate(curr_test);
                if (Is<Symbol>(result)) {
                    if (As<Symbol>(result)->GetName() == "#f") {
                        return result;
                    } else {
                        last_result = result;
                    }
                } else {
                    last_result = result;
                }
            } else {
                curr_test.push_back(nodes[i]);
                std::shared_ptr<Object> result = Evaluate(curr_test);
                ++i;
                if (As<Symbol>(result)->GetName() == "#f") {
                    return result;
                } else {
                    last_result = result;
                }
            }
        } else if (Is<Number>(nodes[i])) {
            curr_test.clear();
            curr_test.push_back(nodes[i]);
            std::shared_ptr<Object> result = Evaluate(curr_test);
            last_result = result;
            ++i;
        }
    }
    if (nodes.size() == 1) {
        return std::shared_ptr<Object>(new Symbol("#t"));
    }
    return last_result;
}

std::shared_ptr<Object> Interpreter::OrEvaluate(std::vector<std::shared_ptr<Object>>& nodes) {
    std::vector<std::shared_ptr<Object>> curr_test;
    std::shared_ptr<Object> last_result;
    size_t i = 1, j = 0;
    while (i < nodes.size()) {
        if (Is<Symbol>(nodes[i])) {
            curr_test.clear();
            if (operations.find(As<Symbol>(nodes[i])->GetName()) != operations.end()) {
                curr_test.push_back(nodes[i]);
                j = i + 1;
                while (j < nodes.size() &&
                       (!Is<Symbol>(nodes[j]) ||
                        (Is<Symbol>(nodes[j]) &&
                         operations.find(As<Symbol>(nodes[j])->GetName()) == operations.end()))) {
                    curr_test.push_back(nodes[j]);
                    ++j;
                }
                i = j;
                std::shared_ptr<Object> result = Evaluate(curr_test);
                if (Is<Symbol>(result)) {
                    if (As<Symbol>(result)->GetName() == "#t") {
                        return result;
                    } else {
                        last_result = result;
                    }
                } else {
                    last_result = result;
                }
            } else {
                curr_test.push_back(nodes[i]);
                std::shared_ptr<Object> result = Evaluate(curr_test);
                ++i;
                if (As<Symbol>(result)->GetName() == "#t") {
                    return result;
                } else {
                    last_result = result;
                }
            }
        } else if (Is<Number>(nodes[i])) {
            curr_test.clear();
            curr_test.push_back(nodes[i]);
            std::shared_ptr<Object> result = Evaluate(curr_test);
            last_result = result;
            ++i;
        }
    }
    if (nodes.size() == 1) {
        return std::shared_ptr<Object>(new Symbol("#f"));
    }
    return last_result;
}

std::string Interpreter::Run(const std::string& str) {
    std::stringstream ss{str};
    Tokenizer tokenizer{&ss};
    std::string result;
    std::shared_ptr<Object> ast = Read((&tokenizer));
    std::vector<Token>& tokens = tokenizer.GetAllTokens();
    std::vector<std::shared_ptr<Object>> v;
    AstToVector(ast, v);
    if (v.size() == 1 && Is<Symbol>(v[0])) {
        if (As<Symbol>(v[0])->GetName() != "#t" && As<Symbol>(v[0])->GetName() != "#f") {
            if (operations.find(As<Symbol>(v[0])->GetName()) == operations.end()) {
                throw RuntimeError{"Symbol cannot be evaluated"};
            }
        }
    } else {
        if (!v.empty() && Is<Symbol>(v[0])) {
            std::string operation = As<Symbol>(v[0])->GetName();
            if (operation != "not" && operation != "boolean?" && operation != "number?" &&
                operation != "and" && operation != "or") {
                IntegerTypeChecker(tokens);
            }
        } else {
            IntegerTypeChecker(tokens);
        }
    }
    BinaryOperationsChecker(v);
    UnaryOperationChecker(v);
    RequiresOnlyOneArgumentChecker(v);
    ListsAreNotSelfEvaluating(tokens);
    std::shared_ptr<Object> evaluated_ast;
    // Integer evaluation or smth else evaluation
    if (v.empty() ||
        (!v.empty() && Is<Symbol>(v[0]) &&
         (As<Symbol>(v[0])->GetName() != "and" && As<Symbol>(v[0])->GetName() != "or"))) {
        evaluated_ast = Evaluate(v);
    }
    // And / Or evaluation
    else {
        if (Is<Symbol>(v[0])) {
            std::string operation = As<Symbol>(v[0])->GetName();
            if (operation == "and") {
                evaluated_ast = AndEvaluate(v);
            } else if (operation == "or") {
                evaluated_ast = OrEvaluate(v);
            }
        } else {
            evaluated_ast = Evaluate(v);
        }
    }
    std::vector<std::shared_ptr<Object>> v_evaluated;
    AstToVector(evaluated_ast, v_evaluated);
    if (Is<Cell>(evaluated_ast) && As<Cell>(evaluated_ast)->GetSecond() != nullptr) {
        std::shared_ptr<CheckIfList> checker = std::shared_ptr<CheckIfList>(new CheckIfList());
        std::shared_ptr<Object> res = checker->Apply(evaluated_ast);
        if (Is<Symbol>(res) && As<Symbol>(res)->GetName() == "#f") {
            result.push_back('(');
            SerializeQuote(evaluated_ast, result);
            return result;
        }
    }
    std::shared_ptr<CheckIfList> checker = std::shared_ptr<CheckIfList>(new CheckIfList());
    std::shared_ptr<Object> res = checker->Apply(evaluated_ast);
    if (!v.empty() && Is<Symbol>(v[0]) && As<Symbol>(v[0])->GetName() != "quote" &&
        Is<Symbol>(res) && As<Symbol>(res)->GetName() == "#t") {
        result.push_back('(');
        SerializeQuote(evaluated_ast, result);
        return result;
    }
    Serialize(v_evaluated, v, result);
    return result;
}