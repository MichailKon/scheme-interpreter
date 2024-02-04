#include "object.h"
#include "functors.h"
#include "error.h"
#include "helpers.h"
#include <memory>

int Number::GetValue() const {
    return value_;
}

const std::string& Symbol::GetName() const {
    return name_;
}

std::shared_ptr<Object> Symbol::Eval([[maybe_unused]] std::shared_ptr<Scope> scope) const {
    if (scope->Contains(name_)) {
        return scope->Get(name_);
    } else {
        throw NameError("Can't find name " + name_ +
                        " in current scope (depth = " + std::to_string(scope->GetDepth()) + ")");
    }
}

std::shared_ptr<Object> Cell::GetFirst() const {
    return first_;
}

std::shared_ptr<Object> Cell::GetSecond() const {
    return second_;
}

bool Boolean::GetValue() const {
    return value_;
}

std::shared_ptr<Object> Cell::Eval([[maybe_unused]] std::shared_ptr<Scope> scope) const {
    if (!first_) {
        throw RuntimeError("shit happened during eval (fucking eval_test)");
    }
    std::shared_ptr<Object> func = first_->Eval(scope);
    if (!Is<IFunctor>(func)) {
        throw RuntimeError("Cell::first is not a functor");
    }
    ObjectVector data = Object2Vector(second_);
    std::shared_ptr<IFunctor> ff = As<IFunctor>(func);
    std::shared_ptr<Object> res = ff->Calc(data, scope);
    return res;
}

std::string Cell::Serialize() const {
    std::string inner;
    std::shared_ptr<Cell> cur(new Cell);
    cur->SetFirst(first_), cur->SetSecond(second_);
    while (true) {
        if (!cur->GetFirst()) {
            inner += "()";
        } else {
            inner += cur->GetFirst()->Serialize();
        }
        if (!cur->GetSecond()) {
            break;
        }
        inner += " ";
        if (!Is<Cell>(cur->GetSecond())) {
            inner += ". " + cur->GetSecond()->Serialize();
            break;
        }
        cur = As<Cell>(cur->GetSecond());
    }
    return "(" + inner + ")";
}

std::shared_ptr<Object> Scope::Get(const std::string& name) const {
    if (objects_.contains(name)) {
        return objects_.at(name);
    } else if (parent_) {
        return parent_->Get(name);
    } else {
        return nullptr;
    }
}

bool Scope::Contains(const std::string& name) const {
    return Get(name) != nullptr;
}

void Scope::Define(const std::string& name, std::shared_ptr<Object> object) {
    objects_[name] = object;
}

void Scope::Set(const std::string& name, std::shared_ptr<Object> object) {
    if (!Contains(name)) {
        throw NameError("Undefined reference to variable " + name);
    }
    if (!objects_.contains(name)) {
        return parent_->Set(name, object);
    }
    Define(name, object);
}

Scope::Scope(const std::shared_ptr<Scope>& parent) : parent_(parent) {
}

size_t Scope::GetDepth() const {
    if (parent_) {
        return 1 + parent_->GetDepth();
    } else {
        return 0;
    }
}

std::unordered_map<std::string, std::shared_ptr<Object>> Scope::GetObjects() const {
    return objects_;
}

std::shared_ptr<Scope> Scope::GetParent() const {
    return parent_;
}

Scope::Scope(const std::shared_ptr<Scope>& parent,
             const std::unordered_map<std::string, std::shared_ptr<Object>>& objects)
    : parent_(parent), objects_(objects) {
}
