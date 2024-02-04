#pragma once

#include "error.h"
#include "object.h"
#include "helpers.h"
#include <vector>
#include <memory>
#include <optional>

class IFunctor : public Object {
public:
    virtual std::shared_ptr<Object> Calc([[maybe_unused]] const ObjectVector& values,
                                         [[maybe_unused]] std::shared_ptr<Scope> scope) {
        throw std::runtime_error("Calc is not implemented for IFunctor");
    }

    std::string Serialize() const override {
        throw std::runtime_error("Can't Serialize IFunctor");
    }

    std::shared_ptr<Object> Eval([[maybe_unused]] std::shared_ptr<Scope> scope) const override {
        throw std::runtime_error("Can't Eval IFunctor");
    }
};

template <class T>
class NumberFunctor : public IFunctor {
public:
    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override {
        if (!CheckForNoNulls(values)) {
            throw RuntimeError("null in vector during Object2Vector");
        }
        std::optional<long long> res = init_;
        for (auto& i : values) {
            std::shared_ptr<Object> cur = i->Eval(scope);
            if (!Is<Number>(cur)) {
                throw RuntimeError("Functor in NumberFunctor got non-Number argument");
            }
            if (res.has_value()) {
                res = functor_(res.value(), As<Number>(cur)->GetValue());
            } else {
                res = As<Number>(cur)->GetValue();
            }
        }
        if (!res.has_value()) {
            throw RuntimeError("can't calc value without init");
        }
        return std::shared_ptr<Object>(new Number{static_cast<int>(res.value())});
    }

    explicit NumberFunctor(T functor) : functor_(functor) {
    }

    NumberFunctor(T functor, long long int init) : functor_(functor), init_(init) {
    }

    std::shared_ptr<Object> Eval([[maybe_unused]] std::shared_ptr<Scope> scope) const override {
        throw SyntaxError("blyat");  // real blyat
    }

private:
    T functor_;
    std::optional<long long> init_;
};

class AbsFunctor : public IFunctor {
public:
    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override;
};

template <class T>
class ComparisonFunctor : public IFunctor {
public:
    explicit ComparisonFunctor(T functor) : functor_(functor) {
    }

    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override {
        bool res = true;
        std::optional<int> cock;
        for (auto& i : values) {
            std::shared_ptr<Object> huy = i->Eval(scope);
            if (!Is<Number>(huy)) {
                throw RuntimeError("ComparisonFunctor needs only numbers");
            }
            int val = As<Number>(huy)->GetValue();
            if (cock.has_value()) {
                res &= functor_(cock.value(), val);
            } else {
                cock = val;
            }
            if (!res) {
                break;
            }
        }
        return std::shared_ptr<Object>(new Boolean{res});
    }

private:
    T functor_;
};

template <class T>
class CheckTypeFunctor : public IFunctor {
public:
    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override {
        if (values.size() != 1) {
            throw RuntimeError("CheckTypeFunctor needs only one element");
        }
        return std::shared_ptr<Object>(new Boolean{Is<T>(values[0]->Eval(scope))});
    }
};

class QuoteFunctor : public IFunctor {
public:
    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override;
};

template <class T>
class BooleanFunctor : public IFunctor {
public:
    BooleanFunctor(T functor, bool stop_value) : functor_(functor), stop_value_(stop_value) {
    }

    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override {
        if (!CheckForNoNulls(values)) {
            throw RuntimeError("null in vector during Object2Vector");
        }
        bool res = !stop_value_;
        std::shared_ptr<Object> last_eval;
        for (auto& i : values) {
            last_eval = i->Eval(scope);
            res = functor_(res, last_eval->ToBool());
            if (res == stop_value_) {
                return last_eval;
            }
        }
        if (res) {
            if (values.empty()) {
                return std::shared_ptr<Object>(new Boolean{true});
            } else {
                return last_eval;
            }
        }
        return std::shared_ptr<Object>(new Boolean{res});
    }

private:
    T functor_;
    bool stop_value_;
};

class NotFunctor : public IFunctor {
public:
    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override;
};

class CheckNullFunctor : public IFunctor {
public:
    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override;
};

class CarFunctor : public IFunctor {
public:
    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override;
};

class CdrFunctor : public IFunctor {
public:
    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override;
};

class ConsFunctor : public IFunctor {
    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override;
};

class CheckListFunctor : public IFunctor {
public:
    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override;
};

class ListFunctor : public IFunctor {
    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override;
};

class ListRefFunctor : public IFunctor {
    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override;
};

class ListTailFunctor : public IFunctor {
    std::shared_ptr<Object> Calc(const ObjectVector& values,
                                 [[maybe_unused]] std::shared_ptr<Scope> scope) override;
};

class IfFunctor : public IFunctor {
    std::shared_ptr<Object> Calc(const ObjectVector& values, std::shared_ptr<Scope> scope) override;
};

class DefineFunctor : public IFunctor {
    std::shared_ptr<Object> Calc(const ObjectVector& values, std::shared_ptr<Scope> scope) override;
};

class SetFunctor : public IFunctor {
    std::shared_ptr<Object> Calc(const ObjectVector& values, std::shared_ptr<Scope> scope) override;
};

class SetCarFunctor : public IFunctor {
    std::shared_ptr<Object> Calc(const ObjectVector& values, std::shared_ptr<Scope> scope) override;
};

class SetCdrFunctor : public IFunctor {
    std::shared_ptr<Object> Calc(const ObjectVector& values, std::shared_ptr<Scope> scope) override;
};

class LambdaFunctor : public IFunctor {
public:
    LambdaFunctor(const std::vector<std::string>& args, const ObjectVector& body,
                  const std::shared_ptr<Scope>& parent_scope);

    std::shared_ptr<Object> Calc(const ObjectVector& values, std::shared_ptr<Scope> scope) override;

private:
    std::vector<std::string> args_;
    ObjectVector body_;
    std::shared_ptr<Scope> parent_scope_;
};

class LambdaCreatorFunctor : public IFunctor {
    std::shared_ptr<Object> Calc(const ObjectVector& values, std::shared_ptr<Scope> scope) override;
};
