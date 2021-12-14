#include <eosio/eosio.hpp>
#include <asset.hpp>
#include <singleton.hpp>
using namespace eosio;
using std::string;

class [[eosio::contract("vnd")]] vnd : public contract
{
public:
    vnd(name receiver, name code, datastream<const char *> ds) : contract(receiver, code, ds){};

    [[eosio::action]] void create(const asset &max_supply);

    [[eosio::action]] void issue(const name &to, const asset &quantity, const string &memo);

    [[eosio::action]] void transfer(const name &from, const name &to, const asset &quantity, const string &memo);

    using create_action = action_wrapper<"create"_n, &vnd::create>;
    using issue_action = action_wrapper<"issue"_n, &vnd::issue>;
    using transfer_action = action_wrapper<"transfer"_n, &vnd::transfer>;

private:
    struct [[eosio::table]] balance
    {
        name owner;
        int64_t amount;

        uint64_t primary_key() const { return owner.value; }
    };

    struct [[eosio::table]] info
    {
        symbol_code sym = symbol_code(name("vnd").value);
        int64_t max_supply = 0;
        int64_t current_supply = 0;
    };

    using balance_table = multi_index<"balances"_n, balance>;
    using stat_table = singleton<"stats"_n, info>;

    balance_table blc = balance_table(get_self(), get_self().value);
    stat_table stat = stat_table(get_self(), get_self().value);
};
