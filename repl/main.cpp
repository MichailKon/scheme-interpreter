#include <iostream>
#include <scheme.h>

int main() {
    Interpreter interpreter;
    std::cout << interpreter.Run("(define slow-add (lambda (x y) (if (= x 0) y (slow-add (- x 1) (+ y 1)))))") << '\n';
    std::cout << interpreter.Run("(slow-add 3 3)");
    return 0;
}
