#include "bank.hpp"

void bank::deposit(const name &from, const name &to, const asset &amount, const std::string &memo) {
    if (from == get_self() || to != get_self()) {
        return;
    }

    auto sym = amount.symbol.code();
    balance_record table (get_self(), from.value);
    auto itr = table.find(sym.raw());
    if (itr == table.end()) {
        table.emplace(get_self(), [&] (auto& row) {
            row.quantity = amount;
        });
    } else {
        table.modify(itr, get_self(), [&] (auto& row) {
            row.quantity += amount;
        });
    }
};

void bank::withdraw(const name &to, const asset &amount) {
    require_auth(to);
    
    auto sym = amount.symbol.code();
    balance_record table (get_self(), to.value);
    
    auto itr = table.find(sym.raw());
    check(itr != table.end(), "No such currency deposited!");

    size_t size = transaction_size();
    char buf[size];
    size_t read = read_transaction(buf, size);
    check(size == read, "read_transaction() failed!");
    checksum256 res = sha256(buf, read);

    uint64_t rate;
    memcpy(&rate, res.data(), sizeof(rate));
    rate = (rate % (max_fee_percent - min_fee_percent + 1)) + min_fee_percent;
    print(rate);

    auto fee = (static_cast<long double>(rate) / static_cast<long double>(100))
        * static_cast<long double>(amount.amount);
    auto total_amount = amount.amount + static_cast<int64_t>(fee);
    
    const auto& item = table.get(sym.raw());
    check(item.quantity.amount >= total_amount, "Not enough value for withdrawal!");

    table.modify(itr, get_self(), [&] (auto& row) {
        row.quantity.amount -= total_amount;
    });

    token::transfer_action transfer("eosio.token"_n, {get_self(), "active"_n});
    transfer.send(get_self(), to, amount, "Transfer withdrawal!");
}
