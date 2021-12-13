#include <eosio/eosio.hpp>
using namespace eosio;

class [[eosio::contract("todolist")]] todolist : public contract
{
public:
    todolist(name receiver, name code, datastream<const char *> ds)
        : contract(receiver, code, ds) {};

    [[eosio::action]] void insert(std::string content, name user);

    [[eosio::action]] void update(uint64_t task_id, std::string content, name user, bool completed);

    [[eosio::action]] void rmvbytask(uint64_t task_id, name user);

    [[eosio::action]] void rmvbyuser(name user);

private:
    struct [[eosio::table]] task
    {
        uint64_t task_id;
        std::string content;
        name user;
        bool completed;

        uint64_t primary_key() const { return task_id; }
    };

    using todo_table = multi_index<"todolist"_n, task>;
};
