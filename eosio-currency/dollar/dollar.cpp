#include "dollar.hpp"

void dollar::create(const asset& max_supply) {
    require_auth(get_self());

    info i = stat.get();
    check (max_supply.symbol.code() == i.sym, "Wrong currency !");
    check (max_supply.is_valid(), "Invalid amount !");
    check (i.max_supply == 0, "Currency can only be created once !");

    i.current_supply = 0;
    i.max_supply = max_supply.amount;

    stat.set(i, get_self());
};

void dollar::issue(const name &to, const asset &quantity, const string &memo) {
    require_auth(get_self());

    info i = stat.get();

    check (is_account(to), "Account does not exist !");
    check (quantity.symbol.code() == i.sym, "Wrong currency !");
    check (quantity.amount > 0, "Issue amount must be positive !");
    check (quantity.amount <= (i.max_supply - i.current_supply), "Issue amount exceeds limit !");

    i.current_supply += quantity.amount;
    stat.set(i, get_self());

    auto itr = blc.find(to.value);
    if (itr == blc.end()) {
        blc.emplace(get_self(), [&] (auto& row) {
            row.owner = to;
            row.amount = quantity.amount;
        });
    } else {
        blc.modify(itr, get_self(), [&] (auto& row) {
            row.amount += quantity.amount;
        });
    };
};

void dollar::transfer(const name &from, const name &to, const asset &quantity, const string &memo) {
    require_auth(from);

    check (from != to, "Cannot transfer to self !");
    check (is_account(to), "Invalid receiver account !");
    check (quantity.symbol.code() == stat.get().sym, "Wrong currency !");
    check (quantity.amount > 0, "Transfer amount must be positive !");
    check (sizeof(memo) <= 256, "Memo must be <= 256 characters long !");

    auto from_itr = blc.find(from.value);
    check (from_itr != blc.end(), "Sender account has no money !");
    check (from_itr->amount >= quantity.amount, "Sender account has not enough money !");

    blc.modify (from_itr, get_self(), [&] (auto& row) {
        row.amount -= quantity.amount;
    });

    auto to_itr = blc.find(to.value); 
    if (to_itr == blc.end()) {
        blc.emplace(get_self(), [&] (auto& row) {
            row.owner = to;
            row.amount = quantity.amount;
        });
    } else {
        blc.modify(to_itr, get_self(), [&] (auto& row) {
            row.amount += quantity.amount;
        });
    }

    require_recipient (from);
    require_recipient (to);
};