#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Scope;

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;

    virtual std::shared_ptr<Object> Eval([[maybe_unused]] std::shared_ptr<Scope> scope) const {
        throw std::runtime_error("Eval is not implemented for Object");
    }

    virtual std::string Serialize() const {
        throw std::runtime_error("Serialize is not implemented for Object");
    }

    virtual bool ToBool() const {
        throw std::runtime_error("ToBool is not implemented for Object");
    }
};

using ObjectVector = std::vector<std::shared_ptr<Object>>;

class Scope : public Object {
public:
    explicit Scope(const std::shared_ptr<Scope>& parent);

    Scope(const std::shared_ptr<Scope>& parent,
          const std::unordered_map<std::string, std::shared_ptr<Object>>& objects);

    std::shared_ptr<Object> Eval([[maybe_unused]] std::shared_ptr<Scope> scope) const override {
        throw std::runtime_error("Eval is not implemented for Scope");
    }

    bool ToBool() const override {
        throw std::runtime_error("ToBool is not implemented for Scope");
    }

    std::string Serialize() const override {
        throw std::runtime_error("Serialize is not implemented for Scope");
    }

    std::shared_ptr<Object> Get(const std::string& name) const;

    bool Contains(const std::string& name) const;

    void Define(const std::string& name, std::shared_ptr<Object> object);

    void Set(const std::string& name, std::shared_ptr<Object> object);

    size_t GetDepth() const;

    std::unordered_map<std::string, std::shared_ptr<Object>> GetObjects() const;

    std::shared_ptr<Scope> GetParent() const;

private:
    std::shared_ptr<Scope> parent_;
    std::unordered_map<std::string, std::shared_ptr<Object>> objects_;
};

class Number : public Object {
public:
    Number(int value) : value_(value) {
    }

    std::shared_ptr<Object> Eval([[maybe_unused]] std::shared_ptr<Scope> scope) const override {
        return std::shared_ptr<Object>(new Number{value_});
    }

    std::string Serialize() const override {
        return std::to_string(value_);
    }

    int GetValue() const;

    bool ToBool() const override {
        return true;
    }

private:
    int value_;
};

class Symbol : public Object {
public:
    Symbol(const std::string& name) : name_(name) {
    }

    const std::string& GetName() const;

    std::shared_ptr<Object> Eval([[maybe_unused]] std::shared_ptr<Scope> scope) const override;

    std::string Serialize() const override {
        return name_;
    }

    bool ToBool() const override {
        return true;
    }

private:
    std::string name_;
};

class Boolean : public Object {
public:
    Boolean(bool value) : value_(value) {
    }

    std::string Serialize() const override {
        if (value_) {
            return "#t";
        } else {
            return "#f";
        }
    }

    std::shared_ptr<Object> Eval([[maybe_unused]] std::shared_ptr<Scope> scope) const override {
        return std::shared_ptr<Boolean>(new Boolean{value_});
    }

    bool GetValue() const;

    bool ToBool() const override {
        return value_;
    }

private:
    bool value_;
};

class Cell : public Object {
public:
    std::shared_ptr<Object> GetFirst() const;
    std::shared_ptr<Object> GetSecond() const;

    void SetFirst(std::shared_ptr<Object> first) {
        first_ = first;  // too lazy to do this is .cpp
    }

    void SetSecond(std::shared_ptr<Object> second) {
        second_ = second;
    }

    std::shared_ptr<Object> Eval([[maybe_unused]] std::shared_ptr<Scope> scope) const override;

    std::string Serialize() const override;

    bool ToBool() const override {
        return true;
    }

private:
    std::shared_ptr<Object> first_;
    std::shared_ptr<Object> second_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj) != nullptr;
}
