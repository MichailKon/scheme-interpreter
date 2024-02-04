#pragma once

#include <variant>
#include <optional>
#include <istream>
#include <string>

struct NoToken {
    bool operator==(const NoToken& other) const;
};

struct SymbolToken {
    std::string name;

    bool operator==(const SymbolToken& other) const;
};

struct QuoteToken {
    bool operator==(const QuoteToken&) const;
};

struct DotToken {
    bool operator==(const DotToken&) const;
};

enum class BracketToken { OPEN, CLOSE };

struct ConstantToken {
    int value;

    bool operator==(const ConstantToken& other) const;
};

using Token = std::variant<ConstantToken, BracketToken, SymbolToken, QuoteToken, DotToken, NoToken>;

class Tokenizer {
public:
    Tokenizer(std::istream* in);

    bool IsEnd();

    void Next();

    Token GetToken();

private:
    std::istream* in_;
    Token last_token_;

    void SkipSpaces();

    static bool IsFirstSymbolChar(char c);

    static bool IsSymbolChar(char c);
};

template <class T>
bool IsTokenT(const Token& obj) {
    try {
        (void)std::get<T>(obj);
    } catch (std::bad_variant_access) {
        return false;
    }
    return true;
}
