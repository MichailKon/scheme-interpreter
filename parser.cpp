#include <parser.h>
#include <vector>
#include "error.h"

enum DotInfo { kNoDots, kWasDot, kAfterDot };

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("No tokens");
    }
    std::shared_ptr<Cell> res(new Cell);
    std::shared_ptr<Cell> cur(res);
    DotInfo dot_info = kNoDots;
    bool non_empty_cell = false;

    while (true) {
        if (tokenizer->IsEnd()) {
            throw SyntaxError("Unexpected EOF");
        }
        if (IsTokenT<BracketToken>(tokenizer->GetToken()) &&
            std::get<BracketToken>(tokenizer->GetToken()) == BracketToken::CLOSE) {
            tokenizer->Next();
            break;
        }
        if (dot_info == kAfterDot) {
            throw SyntaxError("smth after dot twice");
        }
        if (IsTokenT<DotToken>(tokenizer->GetToken())) {
            if (!cur->GetFirst()) {
                throw SyntaxError("bad dots (1)");
            }
            tokenizer->Next();
            dot_info = kWasDot;
            continue;
        }
        if (dot_info == kWasDot) {
            dot_info = kAfterDot;
            cur->SetSecond(Read(tokenizer));
        } else {
            if (non_empty_cell) {
                std::shared_ptr<Cell> next(new Cell);
                cur->SetSecond(next);
                cur = next;
            }
            cur->SetFirst(Read(tokenizer));
        }
        non_empty_cell = true;
    }
    if (dot_info == kWasDot) {
        throw SyntaxError("nothing after a dot");
    }
    if (!non_empty_cell) {
        return nullptr;
    }
    return res;
}

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("No tokens");
    }
    Token cur_token = tokenizer->GetToken();
    if (IsTokenT<BracketToken>(cur_token)) {
        if (std::get<BracketToken>(tokenizer->GetToken()) == BracketToken::CLOSE) {
            throw SyntaxError("unexpected close bracket");
        }
        tokenizer->Next();
        return ReadList(tokenizer);
    }
    if (IsTokenT<ConstantToken>(cur_token)) {
        int value = std::get<ConstantToken>(cur_token).value;
        tokenizer->Next();
        return std::shared_ptr<Object>(new Number(value));
    }
    if (IsTokenT<SymbolToken>(cur_token)) {
        std::string symbol = std::get<SymbolToken>(cur_token).name;
        if (symbol.starts_with('#')) {
            if (symbol.size() < 2) {
                throw SyntaxError("nothing after #");
            } else if (symbol[1] == 't') {
                tokenizer->Next();
                return std::shared_ptr<Object>(new Boolean(true));
            } else if (symbol[1] == 'f') {
                tokenizer->Next();
                return std::shared_ptr<Object>(new Boolean(false));
            } else {
                throw SyntaxError(std::string("illegal symbol: ") + symbol);
            }
        }
        tokenizer->Next();
        return std::shared_ptr<Object>(new Symbol(symbol));
    }
    if (IsTokenT<DotToken>(cur_token)) {
        throw SyntaxError("unexpected dot");
    }
    if (IsTokenT<QuoteToken>(cur_token)) {
        tokenizer->Next();
        std::shared_ptr<Object> quote_obj = Read(tokenizer);
        std::shared_ptr<Object> res(new Cell);
        As<Cell>(res)->SetFirst(std::shared_ptr<Object>(new Symbol("quote")));
        std::shared_ptr<Object> tmp(new Cell);

        As<Cell>(tmp)->SetFirst(quote_obj);
        As<Cell>(tmp)->SetSecond(nullptr);
        As<Cell>(res)->SetSecond(tmp);
        return res;
    }
    throw SyntaxError("shit happened");
}
