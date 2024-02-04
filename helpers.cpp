#include "helpers.h"
#include "error.h"

ObjectVector Object2Vector(std::shared_ptr<Object> obj) {
    if (!obj) {
        return {};
    }
    if (!Is<Cell>(obj)) {
        return {obj};
    }
    ObjectVector res;
    while (obj != nullptr) {
        std::shared_ptr<Cell> cur = As<Cell>(obj);
        res.push_back(cur->GetFirst());
        if (!Is<Cell>(cur->GetSecond())) {
            if (cur->GetSecond()) {
                res.push_back(cur->GetSecond());
            }
            break;
        }
        obj = cur->GetSecond();
    }
    return res;
}

bool CheckForNoNulls(const ObjectVector &obj) {
    for (auto &i : obj) {
        if (i == nullptr) {
            return false;
        }
    }
    return true;
}

std::shared_ptr<Object> Vector2Object(const ObjectVector &obj) {
    //    if (!CheckForNoNulls(obj)) {
    //        throw RuntimeError("Vector2Object's argument contains nullptr");
    //    }
    if (obj.empty()) {
        return {};
    }
    std::shared_ptr<Object> res(new Cell);
    std::shared_ptr<Object> cur(res);
    for (auto &i : obj) {
        if (As<Cell>(cur)->GetFirst() == nullptr) {
            As<Cell>(cur)->SetFirst(i);
        } else {
            std::shared_ptr<Object> next(new Cell);
            As<Cell>(next)->SetFirst(i);
            As<Cell>(cur)->SetSecond(next);
            cur = next;
        }
    }
    return res;
}

std::pair<std::shared_ptr<Cell>, std::shared_ptr<Object>> PrepareForCarCdr(
    const ObjectVector &values, std::shared_ptr<Scope> scope) {
    if (values.size() != 2) {
        throw RuntimeError("SetCar and SetCdr functions needs exactly two arguments");
    }
    std::shared_ptr<Object> first_arg = values[0]->Eval(scope);
    if (!Is<Cell>(first_arg)) {
        throw RuntimeError("SetCar ot SetCdr got not cell in its first arguments");
    }
    std::shared_ptr<Object> second_arg = values[1]->Eval(scope);
    return {As<Cell>(first_arg), second_arg};
}
