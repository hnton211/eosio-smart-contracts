#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/transaction.hpp> 
#include <eosio/crypto.hpp>
#include "../eosio.token/eosio.token.hpp"
using namespace eosio;

class [[eosio::contract("bank")]] bank : public contract {
public:
    bank (name receiver, name code, datastream <const char *> ds) 
    : contract(receiver, code, ds) {} ;

    [[eosio::on_notify("eosio.token::transfer")]]
    void deposit(const name& from, const name& to, const asset& amount, const std::string& memo);

    [[eosio::action]]
    void withdraw(const name& to, const asset& amount);
private:
    struct [[eosio::table]] balance {
        asset quantity;

        uint64_t primary_key() const { return quantity.symbol.code().raw(); }
    };

    using balance_record = multi_index<"balance"_n, balance>;
    int max_fee_percent = 8;
    int min_fee_percent = 5;
};