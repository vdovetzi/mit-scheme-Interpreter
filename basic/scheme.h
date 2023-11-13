#pragma once

#include <string>
#include "unordered_map"
#include "parser.h"
#include "map"
#include "vector"
#include "deque"
#include "sstream"
#include "functional"

class Interpreter {
public:
    std::string Run(const std::string&);

    std::map<const std::string, std::function<std::shared_ptr<Object>()>> operations{
        {"quote", []() { return std::shared_ptr<Object>(new QuoteOperation()); }},
        {"number?", []() { return std::shared_ptr<Object>(new CheckIfNumber()); }},
        {"=", []() { return std::shared_ptr<Object>(new CheckIfEqual()); }},
        {"+", []() { return std::shared_ptr<Object>(new AddOperation()); }},
        {"<", []() { return std::shared_ptr<Object>(new CheckIfLess()); }},
        {"<=", []() { return std::shared_ptr<Object>(new CheckIfLessOrEqual()); }},
        {">", []() { return std::shared_ptr<Object>(new CheckIfGreater()); }},
        {">=", []() { return std::shared_ptr<Object>(new CheckIfGreaterOrEqual()); }},
        {"-", []() { return std::shared_ptr<Object>(new MinusOperation()); }},
        {"/", []() { return std::shared_ptr<Object>(new DivisionOperation()); }},
        {"*", []() { return std::shared_ptr<Object>(new MultiplicationOperation()); }},
        {"max", []() { return std::shared_ptr<Object>(new MaxOperation()); }},
        {"min", []() { return std::shared_ptr<Object>(new MinOperation()); }},
        {"abs", []() { return std::shared_ptr<Object>(new AbsOperation()); }},
        {"boolean?", []() { return std::shared_ptr<Object>(new CheckIfBoolean()); }},
        {"not", []() { return std::shared_ptr<Object>(new NotOperation()); }},
        {"and", []() { return std::shared_ptr<Object>(new AndOperation()); }},
        {"or", []() { return std::shared_ptr<Object>(new OrOperation()); }},
        {"null?", []() { return std::shared_ptr<Object>(new CheckIfNull()); }},
        {"list?", []() { return std::shared_ptr<Object>(new CheckIfList()); }},
        {"pair?", []() { return std::shared_ptr<Object>(new CheckIfPair()); }},
        {"cons", []() { return std::shared_ptr<Object>(new ConstructOperation()); }},
        {"car", []() { return std::shared_ptr<Object>(new CarOperation()); }},
        {"cdr", []() { return std::shared_ptr<Object>(new CdrOperation()); }},
        {"list", []() { return std::shared_ptr<Object>(new ListOperation()); }},
        {"list-ref", []() { return std::shared_ptr<Object>(new ListRefOperation()); }},
        {"list-tail", []() { return std::shared_ptr<Object>(new ListTailOperation()); }}

    };

    void AstToVector(std::shared_ptr<Object>, std::vector<std::shared_ptr<Object>>&);
    std::shared_ptr<Object> Evaluate(std::vector<std::shared_ptr<Object>>);
    void Serialize(std::vector<std::shared_ptr<Object>>&, std::vector<std::shared_ptr<Object>>&,
                   std::string&);
    std::shared_ptr<Object> AndEvaluate(std::vector<std::shared_ptr<Object>>&);
    std::shared_ptr<Object> OrEvaluate(std::vector<std::shared_ptr<Object>>&);

private:
    void IntegerTypeChecker(std::vector<Token>&);
    void BinaryOperationsChecker(std::vector<std::shared_ptr<Object>>&);
    void UnaryOperationChecker(std::vector<std::shared_ptr<Object>>&);
    void RequiresOnlyOneArgumentChecker(std::vector<std::shared_ptr<Object>>&);
    void ListsAreNotSelfEvaluating(std::vector<Token>&);
    void SerializeQuote(std::shared_ptr<Object>, std::string&);
};