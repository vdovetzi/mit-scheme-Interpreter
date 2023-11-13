#pragma once

#include <memory>

#include "string"

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
};

class Number : public Object {
public:
    Number(int val) : val_(val){};

    int GetValue() const {
        return val_;
    }

private:
    int val_;
};

class Symbol : public Object {
public:
    Symbol(std::string& name) : name_(name){};

    Symbol(const char* name) : name_(name){};

    const std::string& GetName() const {
        return name_;
    }

private:
    std::string name_;
};

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

private:
    std::shared_ptr<Object> first_ = nullptr;
    std::shared_ptr<Object> second_ = nullptr;
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
