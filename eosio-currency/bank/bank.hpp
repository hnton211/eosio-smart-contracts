#include <eosio.hpp>
#include <asset.hpp>
#include <singleton.hpp>
using namespace eosio;
using std::vector;
using std::string;

class [[eosio::contract("bank")]] bank : public contract {
public:
    bank (name receiver, name code, datastream <const char *> ds) 
    : contract(receiver, code, ds) {} ;

    [[eosio::action]]
    void addsptoken (const name& token_contract, const symbol& token_symbol);

    [[eosio::on_notify("*::transfer")]]
    void deposit (const name& from, const name& to, const asset& quantity, const string& memo);

    [[eosio::action]]
    void withdraw (const name& to, const asset& quantity);

private:
    struct [[eosio::table]] config_s {
        vector <extended_symbol> supported_tokens = {};
    };

    struct [[eosio::table]] balance_s {
        name owner;
        asset quantity;

        uint64_t primary_key() const { return owner.value; }
    };

    using balance_t = multi_index <"balances"_n, balance_s> ;
    using config_t = singleton <"config"_n, config_s> ;

    config_t config = config_t(get_self(), get_self().value) ;

    uint64_t min_fee_percent = 5;
    uint64_t max_fee_percent = 8;

    uint64_t get_random_number();

};