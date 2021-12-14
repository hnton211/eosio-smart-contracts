#include "bank.hpp"

void bank::addsptoken(const name &token_contract, const symbol &token_symbol) {
    require_auth(get_self());
    
    config_s c = config.get();
    for (extended_symbol s : c.supported_tokens) {
        check (s.get_symbol() != token_symbol, "This currency symbol already supported !");
    }

    c.supported_tokens.push_back(extended_symbol(token_symbol, token_contract));

    config.set(c, get_self());
};

void bank::deposit(const name &from, const name &to, const asset &quantity, const string &memo) {
    if (from == get_self() || to != get_self()) {
        return;
    }

    bool accepted = false;
    config_s c = config.get();
    for (extended_symbol s : c.supported_tokens) {
        if (s.get_symbol() == quantity.symbol && get_first_receiver() == s.get_contract()) {
            accepted = true;
        } 
    }

    if (!accepted) {
        action (
            permission_level{get_self(), "active"_n},
            get_first_receiver(),
            "transfer"_n,
            std::make_tuple(
                get_self(),
                from, 
                quantity,
                "Deposit refund !"
            )
        ).send();
        return;
    }

    balance_t balances (get_self(), quantity.symbol.code().raw());
    auto iter = balances.find(from.value);
    if (iter == balances.end()) {
        balances.emplace(get_self(), [&] (auto& row) {
            row.owner = from;
            row.quantity = quantity;
        });
    } else {
        balances.modify(iter, get_self(), [&] (auto& row) {
            row.quantity += quantity;
        });
    }
};

void bank::withdraw(const name &to, const asset &quantity) {
    require_auth(to);

    config_s c = config.get();

    extended_symbol sym;
    bool accepted = false;
    
    for (extended_symbol s : c.supported_tokens) {
        if (quantity.symbol == s.get_symbol()) {
            accepted = true;
            sym = s;
        }
    }
    check(accepted, "Withdraw currency error !");

    balance_t balances (get_self(), quantity.symbol.code().raw());
    auto iter = balances.find(to.value);
    check (iter != balances.end(), "No such account existed !");

    uint64_t rate = get_random_number();
    rate = (rate % (max_fee_percent - min_fee_percent + 1)) + min_fee_percent;
    print(rate);

    auto fee = (static_cast<long double>(rate) / static_cast<long double>(100)) * static_cast<long double>(quantity.amount);
    auto total_amount = quantity.amount + static_cast<int64_t>(fee);

    check (iter->quantity >= total_amount, "Withdraw value exceeds available balance !");

    balances.modify(iter, get_self(), [&] (auto& row) {
        row.quantity -= total_amount;
    });

    action(
        permission_level{get_self(), "active"_n},
        sym.get_contract(),
        "transfer"_n,
        std::make_tuple(
            get_self(),
            to,
            quantity,
            "Withdrawal action !"
        )
    ).send();
};

uint64_t bank::get_random_number() {
    size_t size = transaction_size();
    char buf[size];
    size_t read = read_transaction(buf, size);
    check(size == read, "read_transaction() failed!");
    checksum256 res = sha256(buf, read);

    uint64_t rate;
    memcpy(&rate, res.data(), sizeof(rate));

    return rate;
};