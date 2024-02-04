#include "functors.h"
#include "helpers.h"
#include <optional>

std::shared_ptr<Object> AbsFunctor::Calc(const ObjectVector& values,
                                         [[maybe_unused]] std::shared_ptr<Scope> scope) {
    CheckSize<RuntimeError>(values, 1, "AbsFunctor");
    std::shared_ptr<Object> obj = values[0];
    if (!Is<Number>(obj)) {
        throw RuntimeError("Abs got not Number");
    }
    return std::shared_ptr<Object>(new Number{std::abs(As<Number>(obj)->GetValue())});
}

std::shared_ptr<Object> QuoteFunctor::Calc(const ObjectVector& values,
                                           [[maybe_unused]] std::shared_ptr<Scope> scope) {
    CheckSize<RuntimeError>(values, 1, "QuoteFunctor");
    return values[0];
}

std::shared_ptr<Object> NotFunctor::Calc(const ObjectVector& values,
                                         [[maybe_unused]] std::shared_ptr<Scope> scope) {
    CheckSize<RuntimeError>(values, 1, "NotFunctor");
    return std::shared_ptr<Object>(new Boolean{!values[0]->ToBool()});
}

std::shared_ptr<Object> CheckNullFunctor::Calc(const ObjectVector& values,
                                          [[maybe_unused]] std::shared_ptr<Scope> scope) {
    return std::shared_ptr<Object>(new Boolean{!values[0]->Eval(scope)});
}

std::shared_ptr<Object> CarFunctor::Calc(const ObjectVector& values,
                                         [[maybe_unused]] std::shared_ptr<Scope> scope) {
    CheckSize<RuntimeError>(values, 1, "CarFunctor");
    std::shared_ptr<Object> res = values[0]->Eval(scope);
    if (!Is<Cell>(res)) {
        throw RuntimeError("Got not Cell in CarFunctor");
    }
    return As<Cell>(res)->GetFirst();
}

std::shared_ptr<Object> CdrFunctor::Calc(const ObjectVector& values,
                                         [[maybe_unused]] std::shared_ptr<Scope> scope) {
    CheckSize<RuntimeError>(values, 1, "CdrFunctor");
    std::shared_ptr<Object> res = values[0]->Eval(scope);
    if (!Is<Cell>(res)) {
        throw RuntimeError("Got not Cell in CarFunctor");
    }
    return As<Cell>(res)->GetSecond();
}

std::shared_ptr<Object> ConsFunctor::Calc(const ObjectVector& values,
                                          [[maybe_unused]] std::shared_ptr<Scope> scope) {
    CheckSize<RuntimeError>(values, 2, "ConsFunctor");
    std::shared_ptr<Object> left = values[0]->Eval(scope);
    std::shared_ptr<Object> right = values[1]->Eval(scope);
    std::shared_ptr<Object> res(new Cell);
    As<Cell>(res)->SetFirst(left), As<Cell>(res)->SetSecond(right);
    return res;
}

std::shared_ptr<Object> CheckListFunctor::Calc(const ObjectVector& values,
                                               [[maybe_unused]] std::shared_ptr<Scope> scope) {
    CheckSize<RuntimeError>(values, 1, "CheckListFunctor");
    std::shared_ptr<Object> cur = values[0]->Eval(scope);
    while (cur) {
        if (!Is<Cell>(cur)) {
            break;
        }
        cur = As<Cell>(cur)->GetSecond();
    }
    return std::shared_ptr<Object>(new Boolean{!cur.operator bool()});  // bruh that's funny
}

std::shared_ptr<Object> ListFunctor::Calc(const ObjectVector& values,
                                          [[maybe_unused]] std::shared_ptr<Scope> scope) {
    return Vector2Object(values);
}

std::pair<ObjectVector, long long> ParseListFunctorArguments(
    const ObjectVector& values, [[maybe_unused]] std::shared_ptr<Scope> scope) {
    CheckSize<RuntimeError>(values, 2, "ParseListFunctorArguments");
    std::shared_ptr<Object> list = values[0]->Eval(scope);
    std::shared_ptr<Object> ind = values[1]->Eval(scope);
    if (!Is<Cell>(list)) {
        throw RuntimeError("ParseListFunctorArguments got not Cell in 1st element");
    }
    if (!Is<Number>(ind)) {
        throw RuntimeError("ParseListFunctorArguments got not Number in 2nd element");
    }
    long long res = As<Number>(ind)->GetValue();
    return {Object2Vector(list), res};
}

std::shared_ptr<Object> ListRefFunctor::Calc(const ObjectVector& values,
                                             [[maybe_unused]] std::shared_ptr<Scope> scope) {
    auto [list, ind] = ParseListFunctorArguments(values, scope);
    if (ind < 0 || static_cast<size_t>(ind) >= list.size()) {
        throw RuntimeError("IndexError in ListRefFunctor");
    }
    return list[ind];
}

std::shared_ptr<Object> ListTailFunctor::Calc(const ObjectVector& values,
                                              [[maybe_unused]] std::shared_ptr<Scope> scope) {
    auto [list, ind] = ParseListFunctorArguments(values, scope);
    if (ind < 0 || static_cast<size_t>(ind) > list.size()) {
        throw RuntimeError("IndexError in ListTailFunctor");
    }
    ObjectVector res;
    res.reserve(list.size() - ind);
    for (size_t i = ind; i < list.size(); i++) {
        res.push_back(list[i]);
    }
    return Vector2Object(res);
}

std::shared_ptr<Object> IfFunctor::Calc(const ObjectVector& values, std::shared_ptr<Scope> scope) {
    if (values.size() != 2 && values.size() != 3) {
        throw SyntaxError("Wrong if syntax (got " + std::to_string(values.size()) + " values)");
    }
    std::shared_ptr<Object> if_res = values[0]->Eval(scope);
    if (!Is<Boolean>(if_res)) {
        throw RuntimeError("If statement returned not boolean");
    }
    if (if_res->ToBool()) {
        return values[1]->Eval(scope);
    } else {
        if (values.size() == 2) {
            return nullptr;
        } else {
            return values[2]->Eval(scope);
        }
    }
}

std::shared_ptr<Object> DefineFunctor::Calc(const ObjectVector& values,
                                            std::shared_ptr<Scope> scope) {
    if (values.empty()) {
        throw SyntaxError("Empty define operation");
    }
    if (Is<Symbol>(values[0])) {
        CheckSize<SyntaxError>(values, 2, "DefineVariableFunctor");
        if (values.size() != 2) {
            throw RuntimeError("Attempt to define a variable with not 2 items");
        }
        std::string varname = As<Symbol>(values[0])->GetName();
        scope->Define(varname, values[1]->Eval(scope));
        return std::shared_ptr<Object>(new Symbol(varname));
    }
    if (!Is<Cell>(values[0])) {
        throw SyntaxError("Define got not variable or list/cell");
    }
    if (values.size() < 2) {
        throw RuntimeError("Lambda sugar got < 2 arguments");
    }
    ObjectVector body = values;
    std::string func_name;
    std::vector<std::string> args;
    {
        ObjectVector tmp = Object2Vector(body[0]);
        body.erase(body.begin());
        if (tmp.empty()) {
            throw SyntaxError("Lambda sugar first argument is empty");
        }
        if (!Is<Symbol>(tmp[0])) {
            throw SyntaxError("Lambda sugar first argument first argument must be a Symbol");
        }
        func_name = As<Symbol>(tmp[0])->GetName();
        tmp.erase(tmp.begin());
        for (auto& i : tmp) {
            if (!Is<Symbol>(i)) {
                throw SyntaxError("Lambda sugar arguments must be Symbols");
            }
        }
        for (auto& i : tmp) {
            args.push_back(As<Symbol>(i)->GetName());
        }
        args.reserve(tmp.size());
    }
    scope->Define(func_name, std::shared_ptr<Object>(new LambdaFunctor(args, body, scope)));
    return std::shared_ptr<Object>(new Symbol(func_name));
}

std::shared_ptr<Object> SetFunctor::Calc(const ObjectVector& values, std::shared_ptr<Scope> scope) {
    CheckSize<SyntaxError>(values, 2, "SetFunctor");
    if (!Is<Symbol>(values[0])) {
        throw RuntimeError("First element in SetFunctor is not Symbol");
    }
    std::string varname = As<Symbol>(values[0])->GetName();
    scope->Set(varname, values[1]->Eval(scope));
    return values[0]->Eval(scope);
}

std::shared_ptr<Object> SetCarFunctor::Calc(const ObjectVector& values,
                                            std::shared_ptr<Scope> scope) {
    auto [obj, val] = PrepareForCarCdr(values, scope);
    obj->SetFirst(val);
    return nullptr;
}

std::shared_ptr<Object> SetCdrFunctor::Calc(const ObjectVector& values,
                                            std::shared_ptr<Scope> scope) {
    auto [obj, val] = PrepareForCarCdr(values, scope);
    obj->SetSecond(val);
    return nullptr;
}

std::shared_ptr<Object> LambdaFunctor::Calc(const ObjectVector& values,
                                            std::shared_ptr<Scope> scope) {
    if (values.size() != args_.size()) {
        throw RuntimeError("Expected " + std::to_string(args_.size()) +
                           " arguments in lambda, but got " + std::to_string(values.size()));
    }
    std::shared_ptr<Scope> cur(new Scope(parent_scope_));
    for (size_t i = 0; i < values.size(); i++) {
        cur->Define(args_[i], values[i]->Eval(scope));
    }
    for (size_t i = 0; i + 1 < body_.size(); i++) {
        body_[i]->Eval(cur);
    }
    return body_.back()->Eval(cur);
}

LambdaFunctor::LambdaFunctor(const std::vector<std::string>& args, const ObjectVector& body,
                             const std::shared_ptr<Scope>& parent_scope)
    : args_(args), body_(body), parent_scope_(parent_scope) {
}

std::shared_ptr<Object> LambdaCreatorFunctor::Calc(const ObjectVector& values,
                                                   std::shared_ptr<Scope> scope) {
    if (values.empty()) {
        throw SyntaxError("LambdaCreatorFunctor needs at least 1 argument");
    }
    if (values.size() == 1) {
        throw SyntaxError("LambdaCreatureFunctor got empty body");
    }
    std::vector<std::string> args;
    for (auto& i : Object2Vector(values[0])) {
        if (!Is<Symbol>(i)) {
            throw SyntaxError("LambdaCreatorFunctor needs only Symbol as args");
        }
        args.push_back(As<Symbol>(i)->GetName());
    }
    ObjectVector body = values;
    body.erase(body.begin());
    return std::shared_ptr<Object>(new LambdaFunctor(args, body, scope));
}
