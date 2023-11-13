#pragma once

#include <memory>
#include "error.h"
#include "string"
#include "cstdint"
#include "algorithm"
#include "vector"

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
    virtual std::shared_ptr<Object> Execute() {
        return std::shared_ptr<Object>();
    };
    virtual std::string Serialize() {
        return "";
    };
    virtual std::shared_ptr<Object> Apply(std::shared_ptr<Object>) {
        return std::shared_ptr<Object>();
    };
};

class Number : public Object {
public:
    Number(int64_t val) : val_(val){};

    int64_t GetValue() const {
        return val_;
    }

    std::shared_ptr<Object> Execute() override {
        return std::shared_ptr<Object>(new Number(val_));
    }

    std::string Serialize() override {
        return std::to_string(val_) + " ";
    }

private:
    int64_t val_;
};

class Symbol : public Object {
public:
    Symbol(std::string& name) : name_(name){};

    Symbol(const char* name) : name_(name){};

    const std::string& GetName() const {
        return name_;
    }

    std::shared_ptr<Object> Execute() override {
        return std::shared_ptr<Object>(new Symbol(name_));
    }

    std::string Serialize() override {
        return name_;
    }

private:
    std::string name_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T, Object>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    if (std::dynamic_pointer_cast<T, Object>(obj) == nullptr) {
        return false;
    }
    return true;
}

class Cell : public Object {
public:
    Cell(std::shared_ptr<Object> first, std::shared_ptr<Object> second)
        : first_(first), second_(second){};

    std::shared_ptr<Object> GetFirst() {
        return first_;
    }
    std::shared_ptr<Object> GetSecond() {
        return second_;
    }

    std::string Serialize() override {
        if (first_ != nullptr && second_ != nullptr) {
            return '(' + first_->Serialize() + ' ' + second_->Serialize() + ')';
        } else if (first_ == nullptr && second_ != nullptr) {
            return second_->Serialize();
        } else if (second_ == nullptr && first_ != nullptr) {
            return first_->Serialize();
        } else {
            return "(())";
        }
    }

private:
    std::shared_ptr<Object> first_ = nullptr;
    std::shared_ptr<Object> second_ = nullptr;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class QuoteOperation : public Object {
public:
    QuoteOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) {
        return operand;
    }
};

class AddOperation : public Object {
public:
    AddOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) {
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Number(0));
        }
        if (Is<Number>(operand)) {
            total_sum_ += As<Number>(operand)->GetValue();
        }
        return std::shared_ptr<Object>(new Number(total_sum_));
    }

private:
    int64_t total_sum_ = 0;
};

class CheckIfNumber : public Object {
public:
    CheckIfNumber() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (is_first_) {
            is_first_ = false;
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        if (Is<Number>(operand)) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        return std::shared_ptr<Object>(new Symbol("#f"));
    }

private:
    bool is_first_ = true;
};

class CheckIfEqual : public Object {
public:
    CheckIfEqual() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (ne_) {
            return std::shared_ptr<Object>(new Symbol("#f"));
        }
        if (is_first_ && operand == nullptr) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        } else if (is_first_) {
            is_first_ = false;
            if (operand != nullptr) {
                prev_number_ = As<Number>(operand)->GetValue();
            }
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        if (Is<Number>(operand)) {
            if (prev_number_ == As<Number>(operand)->GetValue()) {
                prev_number_ = As<Number>(operand)->GetValue();
                return std::shared_ptr<Object>(new Symbol("#t"));
            }
            ne_ = true;
            return std::shared_ptr<Object>(new Symbol("#f"));
        }
        return std::shared_ptr<Object>();
    }

private:
    int64_t prev_number_ = 0;
    bool is_first_ = true;
    bool ne_ = false;
};

class CheckIfLess : public Object {
public:
    CheckIfLess() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (nge_) {
            return std::shared_ptr<Object>(new Symbol("#f"));
        }
        if (is_first_ && operand == nullptr) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        } else if (is_first_) {
            is_first_ = false;
            if (operand != nullptr) {
                prev_number_ = As<Number>(operand)->GetValue();
            }
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        if (Is<Number>(operand)) {
            if (prev_number_ < As<Number>(operand)->GetValue()) {
                prev_number_ = As<Number>(operand)->GetValue();
                return std::shared_ptr<Object>(new Symbol("#t"));
            }
            nge_ = true;
            return std::shared_ptr<Object>(new Symbol("#f"));
        }
        return std::shared_ptr<Object>();
    }

private:
    int64_t prev_number_ = 0;
    bool is_first_ = true;
    bool nge_ = false;
};

class CheckIfLessOrEqual : public Object {
public:
    CheckIfLessOrEqual() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (ng_) {
            return std::shared_ptr<Object>(new Symbol("#f"));
        }
        if (is_first_ && operand == nullptr) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        } else if (is_first_) {
            is_first_ = false;
            if (operand != nullptr) {
                prev_number_ = As<Number>(operand)->GetValue();
            }
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        if (Is<Number>(operand)) {
            if (prev_number_ <= As<Number>(operand)->GetValue()) {
                prev_number_ = As<Number>(operand)->GetValue();
                return std::shared_ptr<Object>(new Symbol("#t"));
            }
            ng_ = true;
            return std::shared_ptr<Object>(new Symbol("#f"));
        }
        return std::shared_ptr<Object>();
    }

private:
    int64_t prev_number_ = 0;
    bool is_first_ = true;
    bool ng_ = false;
};

class CheckIfGreater : public Object {
public:
    CheckIfGreater() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (nle_) {
            return std::shared_ptr<Object>(new Symbol("#f"));
        }
        if (is_first_ && operand == nullptr) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        } else if (is_first_) {
            is_first_ = false;
            if (operand != nullptr) {
                prev_number_ = As<Number>(operand)->GetValue();
            }
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        if (Is<Number>(operand)) {
            if (prev_number_ > As<Number>(operand)->GetValue()) {
                prev_number_ = As<Number>(operand)->GetValue();
                return std::shared_ptr<Object>(new Symbol("#t"));
            }
            nle_ = true;
            return std::shared_ptr<Object>(new Symbol("#f"));
        }
        return std::shared_ptr<Object>();
    }

private:
    int64_t prev_number_ = 0;
    bool is_first_ = true;
    bool nle_ = false;
};

class CheckIfGreaterOrEqual : public Object {
public:
    CheckIfGreaterOrEqual() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (nl_) {
            return std::shared_ptr<Object>(new Symbol("#f"));
        }
        if (is_first_ && operand == nullptr) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        } else if (is_first_) {
            is_first_ = false;
            if (operand != nullptr) {
                prev_number_ = As<Number>(operand)->GetValue();
            }
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        if (Is<Number>(operand)) {
            if (prev_number_ >= As<Number>(operand)->GetValue()) {
                prev_number_ = As<Number>(operand)->GetValue();
                return std::shared_ptr<Object>(new Symbol("#t"));
            }
            nl_ = true;
            return std::shared_ptr<Object>(new Symbol("#f"));
        }
        return std::shared_ptr<Object>();
    }

private:
    int64_t prev_number_ = 0;
    bool is_first_ = true;
    bool nl_ = false;
};
class MinusOperation : public Object {
public:
    MinusOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) {
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Number(0));
        }
        if (Is<Number>(operand)) {
            if (!is_first) {
                total_sum_ -= As<Number>(operand)->GetValue();
            } else {
                total_sum_ += As<Number>(operand)->GetValue();
                is_first = false;
            }
        }
        return std::shared_ptr<Object>(new Number(total_sum_));
    }

private:
    bool is_first = true;  // NOLINT
    int64_t total_sum_ = 0;
};

class DivisionOperation : public Object {
public:
    DivisionOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) {
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Number(1));
        }
        if (Is<Number>(operand)) {
            if (!is_first) {
                total_sum_ /= As<Number>(operand)->GetValue();
            } else {
                total_sum_ = As<Number>(operand)->GetValue();
                is_first = false;
            }
        }
        return std::shared_ptr<Object>(new Number(total_sum_));
    }

private:
    bool is_first = true;  // NOLINT
    int64_t total_sum_ = 1;
};

class MultiplicationOperation : public Object {
public:
    MultiplicationOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) {
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Number(1));
        }
        if (Is<Number>(operand)) {
            total_sum_ *= As<Number>(operand)->GetValue();
        }
        return std::shared_ptr<Object>(new Number(total_sum_));
    }

private:
    int64_t total_sum_ = 1;
};

class MaxOperation : public Object {
public:
    MaxOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) {
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Number(INT64_MIN));
        }
        if (Is<Number>(operand)) {
            total_max = std::max(As<Number>(operand)->GetValue(), total_max);
        }
        return std::shared_ptr<Object>(new Number(total_max));
    }

private:
    int64_t total_max = INT64_MIN;  // NOLINT
};

class MinOperation : public Object {
public:
    MinOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) {
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Number(INT64_MAX));
        }
        if (Is<Number>(operand)) {
            total_min = std::min(As<Number>(operand)->GetValue(), total_min);
        }
        return std::shared_ptr<Object>(new Number(total_min));
    }

private:
    int64_t total_min = INT64_MAX;  // NOLINT
};

class AbsOperation : public Object {
public:
    AbsOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) {
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Number(0));
        }
        if (Is<Number>(operand)) {
            total_abs = std::abs(As<Number>(operand)->GetValue());
        }
        return std::shared_ptr<Object>(new Number(total_abs));
    }

private:
    int64_t total_abs = 0;  // NOLINT
};

class CheckIfBoolean : public Object {
public:
    CheckIfBoolean() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (is_first_) {
            is_first_ = false;
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        if (Is<Symbol>(operand) &&
            (As<Symbol>(operand)->GetName() == "#t" || As<Symbol>(operand)->GetName() == "#f")) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        return std::shared_ptr<Object>(new Symbol("#f"));
    }

private:
    bool is_first_ = true;
};

class NotOperation : public Object {
public:
    NotOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) {
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        if (Is<Symbol>(operand)) {
            if (As<Symbol>(operand)->GetName() == "#t") {
                return std::shared_ptr<Object>(new Symbol("#f"));
            }
            if (As<Symbol>(operand)->GetName() == "#f") {
                return std::shared_ptr<Object>(new Symbol("#t"));
            } else {
                return std::shared_ptr<Object>(new Symbol("#f"));
            }
        } else {
            return std::shared_ptr<Object>(new Symbol("#f"));
        }
    }
};

class AndOperation : public Object {
public:
    AndOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) {
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        return std::shared_ptr<Object>();
    }
};

class OrOperation : public Object {
public:
    OrOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) {
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        return std::shared_ptr<Object>();
    }
};

class CheckIfNull : public Object {
public:
    CheckIfNull() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (is_first_) {
            is_first_ = false;
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        } else if (Is<Symbol>(operand) && As<Symbol>(operand)->GetName() == "quote") {
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        return std::shared_ptr<Object>(new Symbol("#f"));
    }

private:
    bool is_first_ = true;
};

class CheckIfList : public Object {
public:
    CheckIfList() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        } else if (Is<Symbol>(operand) && As<Symbol>(operand)->GetName() == "quote") {
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        bool flag = false;
        Helper(operand, flag);
        if (flag) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        return std::shared_ptr<Object>(new Symbol("#f"));
    }

private:
    void Helper(std::shared_ptr<Object> node, bool& flag) {
        if (node == nullptr) {
            flag = true;
            return;
        }
        if (Is<Cell>(node)) {
            Helper(As<Cell>(node)->GetSecond(), flag);
        }
    }
};

class CheckIfPair : public Object {
public:
    CheckIfPair() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (operand == nullptr) {
            return std::shared_ptr<Object>(new Symbol("#f"));
        } else if (Is<Symbol>(operand) && As<Symbol>(operand)->GetName() == "quote") {
            return std::shared_ptr<Object>(new Symbol("#f"));
        }
        if (Is<Cell>(operand) && !Is<Cell>(As<Cell>(operand)->GetFirst()) &&
            (!Is<Cell>(As<Cell>(operand)->GetSecond()) ||
             (Is<Cell>(As<Cell>(operand)->GetSecond()) &&
              As<Cell>(As<Cell>(operand)->GetSecond())->GetSecond() == nullptr))) {
            return std::shared_ptr<Object>(new Symbol("#t"));
        }
        return std::shared_ptr<Object>(new Symbol("#t"));
    }
};

class ConstructOperation : public Object {
public:
    ConstructOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (operand == nullptr) {
            return nullptr;
        }
        if (is_first_) {
            first_ = operand;
            is_first_ = false;
            return first_;
        }
        second_ = operand;
        return std::shared_ptr<Object>(new Cell(first_, second_));
    }

private:
    bool is_first_ = true;
    std::shared_ptr<Object> first_, second_;
};

class CarOperation : public Object {
public:
    CarOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (operand == nullptr) {
            throw RuntimeError{"WTF after car"};
        }
        if (Is<Cell>(operand)) {
            return As<Cell>(operand)->GetFirst();
        }
        return operand;
    }
};

class CdrOperation : public Object {
public:
    CdrOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (operand == nullptr) {
            throw RuntimeError{"WTF after cdr"};
        }
        if (Is<Cell>(operand)) {
            return As<Cell>(operand)->GetSecond();
        }
        return nullptr;
    }
};

class ListOperation : public Object {
public:
    ListOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (operand == nullptr) {
            return nullptr;
        }
        list_.push_back(operand);
        return MakeList(list_, 0);
    }

private:
    std::shared_ptr<Object> MakeList(std::vector<std::shared_ptr<Object>>& nodes, size_t i) {
        if (i == nodes.size()) {
            return nullptr;
        }
        ++i;
        return std::shared_ptr<Object>(new Cell(list_[i - 1], MakeList(nodes, i)));
    }
    std::vector<std::shared_ptr<Object>> list_;
};
class ListRefOperation : public Object {
public:
    ListRefOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (operand == nullptr) {
            return nullptr;
        }
        if (Is<Cell>(operand)) {
            MakeList(operand, list_);
            if (list_.empty()) {
                return nullptr;
            }
            if (As<Number>(list_.back())->GetValue() >= static_cast<int64_t>(list_.size()) - 1) {
                throw RuntimeError{"index-error in list"};
            }
            return list_[As<Number>(list_.back())->GetValue()];
        } else {
            return nullptr;
        }
    }

private:
    void MakeList(std::shared_ptr<Object> node, std::vector<std::shared_ptr<Object>>& result) {
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
            MakeList(As<Cell>(node)->GetFirst(), result);
            MakeList(As<Cell>(node)->GetSecond(), result);
            return;
        } else {
            if (node != nullptr) {
                result.push_back(node);
            }
            return;
        }
    }
    std::vector<std::shared_ptr<Object>> list_;
};

class ListTailOperation : public Object {
public:
    ListTailOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand) override {
        if (operand == nullptr) {
            return nullptr;
        }
        if (Is<Cell>(operand)) {
            std::shared_ptr<Object> normal = Normilize(operand);
            MakeList(normal, list_);
            ToVector(operand, v_);
            if (list_.empty()) {
                return nullptr;
            }
            if (As<Number>(v_.back())->GetValue() >= static_cast<int64_t>(v_.size())) {
                throw RuntimeError{"index-error in list"};
            }
            return list_[As<Number>(v_.back())->GetValue()];
        } else {
            return nullptr;
        }
    }

private:
    std::shared_ptr<Object> Normilize(std::shared_ptr<Object> node) {
        if (Is<Cell>(node)) {
            std::shared_ptr<Object> right = As<Cell>(node)->GetSecond();
            if (Is<Cell>(right) || right == nullptr) {
                return std::shared_ptr<Object>(
                    new Cell(As<Cell>(node)->GetFirst(), Normilize(As<Cell>(node)->GetSecond())));
            } else if (Is<Number>(right)) {
                return std::shared_ptr<Object>(new Cell(As<Cell>(node)->GetFirst(), nullptr));
            }
        }
        return nullptr;
    }
    void MakeList(std::shared_ptr<Object> node, std::vector<std::shared_ptr<Object>>& result) {
        if (node == nullptr) {
            result.push_back(nullptr);
            return;
        }
        if (Is<Cell>(node)) {
            result.push_back(node);
            std::shared_ptr<Object> right = As<Cell>(node)->GetSecond();
            if (Is<Cell>(right) || right == nullptr) {
                MakeList(As<Cell>(node)->GetSecond(), result);
                return;
            } else if (Is<Number>(node)) {
                result.push_back(As<Number>(As<Cell>(node)->GetFirst()));
                result.push_back(nullptr);
                return;
            }
        } else {
            result.push_back(node);
            return;
        }
    }
    void ToVector(std::shared_ptr<Object> node, std::vector<std::shared_ptr<Object>>& result) {
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
            ToVector(As<Cell>(node)->GetFirst(), result);
            ToVector(As<Cell>(node)->GetSecond(), result);
            return;
        } else {
            if (node != nullptr) {
                result.push_back(node);
            }
            return;
        }
    }
    std::vector<std::shared_ptr<Object>> list_;
    std::vector<std::shared_ptr<Object>> v_;
};

class DefineOperation{
public:
    DefineOperation() = default;

    std::shared_ptr<Object> Apply(std::shared_ptr<Object> operand){
        if (is_first){

            is_first = false;
        }
    }

private:
    bool is_first = true;

};