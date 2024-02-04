#pragma once
#define SCHEME_FUZZING_2_PRINT_REQUESTS

#include <string>
#include <memory>

class Scope;

class Interpreter {
public:
    Interpreter();

    std::string Run(const std::string&);

private:
    std::shared_ptr<Scope> root_scope_;
};
