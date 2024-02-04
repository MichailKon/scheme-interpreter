#include "scheme.h"
#include "parser.h"
#include "tokenizer.h"
#include "error.h"
#include "object.h"
#include "functors.h"
#include <sstream>

std::string Interpreter::Run(const std::string &s) {
    std::stringstream ss{s};
    Tokenizer tokenizer{&ss};
    auto obj = Read(&tokenizer);
    if (!tokenizer.IsEnd()) {
        throw SyntaxError("Didn't read all the tokens for some reason");
    }
    if (!obj) {
        throw RuntimeError("Trying to Eval nullptr");
    }
    auto tmp = obj->Eval(root_scope_);
    if (!tmp) {
        return "()";
    }
    try {
        return tmp->Serialize();
    } catch (...) {
        // shit happened
        throw SyntaxError("Couldn't Serialize result. It's either bug in scheme or in test");
    }
}

Interpreter::Interpreter() {
    root_scope_ = std::shared_ptr<Scope>(new Scope(nullptr));
    root_scope_->Define("+", std::shared_ptr<Object>(new NumberFunctor(std::plus<int>(), 0)));
    root_scope_->Define("-", std::shared_ptr<Object>(new NumberFunctor(std::minus<int>())));
    root_scope_->Define("*", std::shared_ptr<Object>(new NumberFunctor(std::multiplies<int>(), 1)));
    root_scope_->Define("/", std::shared_ptr<Object>(new NumberFunctor(std::divides<int>())));
    root_scope_->Define("abs", std::shared_ptr<Object>(new AbsFunctor));
    auto maxi = [&](long long a, long long b) { return std::max(a, b); };
    root_scope_->Define("max", std::shared_ptr<Object>(new NumberFunctor(maxi)));
    auto mini = [&](long long a, long long b) { return std::min(a, b); };
    root_scope_->Define("min", std::shared_ptr<Object>(new NumberFunctor(mini)));
    root_scope_->Define("<", std::shared_ptr<Object>(new ComparisonFunctor(std::less<int>())));
    root_scope_->Define(">", std::shared_ptr<Object>(new ComparisonFunctor(std::greater<int>())));
    root_scope_->Define("=", std::shared_ptr<Object>(new ComparisonFunctor(std::equal_to<int>())));
    root_scope_->Define("<=",
                        std::shared_ptr<Object>(new ComparisonFunctor(std::less_equal<int>())));
    root_scope_->Define(">=",
                        std::shared_ptr<Object>(new ComparisonFunctor(std::greater_equal<int>())));
    root_scope_->Define("number?", std::shared_ptr<Object>(new CheckTypeFunctor<Number>));
    root_scope_->Define("quote", std::shared_ptr<Object>(new QuoteFunctor));

    root_scope_->Define("and",
                        std::shared_ptr<Object>(new BooleanFunctor(std::logical_and<bool>(), 0)));
    root_scope_->Define("or",
                        std::shared_ptr<Object>(new BooleanFunctor(std::logical_or<bool>(), 1)));
    root_scope_->Define("not", std::shared_ptr<Object>(new NotFunctor));
    root_scope_->Define("boolean?", std::shared_ptr<Object>(new CheckTypeFunctor<Boolean>));

    root_scope_->Define("pair?", std::shared_ptr<Object>(new CheckTypeFunctor<Cell>));
    root_scope_->Define("null?", std::shared_ptr<Object>(new CheckNullFunctor));
    root_scope_->Define("car", std::shared_ptr<Object>(new CarFunctor));
    root_scope_->Define("cdr", std::shared_ptr<Object>(new CdrFunctor));
    root_scope_->Define("cons", std::shared_ptr<Object>(new ConsFunctor));

    root_scope_->Define("list?", std::shared_ptr<Object>(new CheckListFunctor));
    root_scope_->Define("list", std::shared_ptr<Object>(new ListFunctor));
    root_scope_->Define("list-tail", std::shared_ptr<Object>(new ListTailFunctor));
    root_scope_->Define("list-ref", std::shared_ptr<Object>(new ListRefFunctor));

    root_scope_->Define("symbol?", std::shared_ptr<Object>(new CheckTypeFunctor<Symbol>()));
    root_scope_->Define("if", std::shared_ptr<Object>(new IfFunctor));
    root_scope_->Define("define", std::shared_ptr<Object>(new DefineFunctor));
    root_scope_->Define("set!", std::shared_ptr<Object>(new SetFunctor));
    root_scope_->Define("set-car!", std::shared_ptr<Object>(new SetCarFunctor));
    root_scope_->Define("set-cdr!", std::shared_ptr<Object>(new SetCdrFunctor));
    root_scope_->Define("lambda", std::shared_ptr<Object>(new LambdaCreatorFunctor));
}
