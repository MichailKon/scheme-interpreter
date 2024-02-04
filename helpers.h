#pragma once

#include "error.h"
#include "object.h"
#include <memory>
#include <vector>

ObjectVector Object2Vector(std::shared_ptr<Object> obj);

bool CheckForNoNulls(const ObjectVector &obj);

std::shared_ptr<Object> Vector2Object(const ObjectVector &obj);

std::pair<std::shared_ptr<Cell>, std::shared_ptr<Object>> PrepareForCarCdr(
    const ObjectVector &values, std::shared_ptr<Scope> scope);

template <class T>
void CheckSize(const ObjectVector &values, size_t need, const std::string &func) {
    if (values.size() != need) {
        throw T(func + std::string(" needs exactly ") + std::to_string(need) + " values");
    }
}
