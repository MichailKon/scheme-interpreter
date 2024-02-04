#include <tokenizer.h>
#include "error.h"

bool SymbolToken::operator==(const SymbolToken& other) const {
    return name == other.name;
}

bool QuoteToken::operator==(const QuoteToken&) const {
    return true;
}

bool DotToken::operator==(const DotToken&) const {
    return true;
}

bool ConstantToken::operator==(const ConstantToken& other) const {
    return value == other.value;
}

Tokenizer::Tokenizer(std::istream* in) : in_(in) {
    Next();
}

bool Tokenizer::IsEnd() {
    try {
        return std::get<NoToken>(last_token_) == NoToken();
    } catch (...) {
        return false;
    }
}

void Tokenizer::SkipSpaces() {
    while (isspace(in_->peek())) {
        (void)in_->get();
    }
}

void Tokenizer::Next() {
    SkipSpaces();
    if (in_->peek() == EOF) {
        last_token_ = NoToken();
        return;
    }
    if (in_->peek() == '+' || in_->peek() == '-') {
        char c = in_->get();
        // operator or number
        if (!isdigit(in_->peek())) {
            last_token_ = SymbolToken{std::string(1, c)};
            return;
        }
        // number
        long long num = 0;
        while (in_->peek() != EOF && isdigit(in_->peek())) {
            num = num * 10 + (in_->get() - '0');
        }
        if (c == '-') {
            num *= -1;
        }
        last_token_ = ConstantToken{static_cast<int>(num)};
        return;
    }
    if (isdigit(in_->peek())) {
        long long num = 0;
        while (isdigit(in_->peek())) {
            num = num * 10 + (in_->get() - '0');
        }
        last_token_ = ConstantToken{static_cast<int>(num)};
        return;
    }
    if (in_->peek() == '(') {
        (void)in_->get();
        last_token_ = BracketToken::OPEN;
        return;
    }
    if (in_->peek() == ')') {
        (void)in_->get();
        last_token_ = BracketToken::CLOSE;
        return;
    }
    if (in_->peek() == '\'') {
        (void)in_->get();
        last_token_ = QuoteToken();
        return;
    }
    if (in_->peek() == '.') {
        (void)in_->get();
        last_token_ = DotToken();
        return;
    }
    if (IsFirstSymbolChar(in_->peek())) {
        std::string res = std::string(1, in_->get());
        while (IsSymbolChar(in_->peek())) {
            res.push_back(in_->get());
        }
        last_token_ = SymbolToken{res};
        return;
    }

    throw SyntaxError(std::string("found strange symbol '") + std::string(1, in_->peek()) +
                      std::string("'"));
}

Token Tokenizer::GetToken() {
    return last_token_;
}

bool Tokenizer::IsFirstSymbolChar(char c) {
    if (isalpha(c)) {
        return true;
    }
    return c == '<' || c == '=' || c == '>' || c == '*' || c == '/' || c == '#';
}

bool Tokenizer::IsSymbolChar(char c) {
    if (IsFirstSymbolChar(c)) {
        return true;
    }
    return isdigit(c) || c == '?' || c == '!' || c == '-';
}

bool NoToken::operator==([[maybe_unused]] const NoToken& other) const {
    return true;
}
