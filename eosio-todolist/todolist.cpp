#include "todolist.hpp"

[[eosio::action]] void todolist::insert(std::string content, name user)
{
    require_auth(user);
    todo_table todos(get_self(), user.value);
    todos.emplace(user, [&](auto &row)
                  {
            row.task_ID = todos.available_primary_key();
            row.content = content;
            row.user = user;
            row.completed = false; });
};

[[eosio::action]] void todolist::update(uint64_t task_ID, std::string content, name user, bool completed)
{
    require_auth(user);
    todo_table todos(get_self(), user.value);
    auto itr = todos.find(task_ID);
    check(itr != todos.end(), "No such task existed!");

    todos.modify(itr, user, [&](auto &row)
                 {
            row.content = content;
            row.user = user;
            row.completed = completed; });
};

[[eosio::action]] void todolist::rmvbytask(uint64_t task_ID, name user)
{
    require_auth(user);
    todo_table todos(get_self(), user.value);
    auto itr = todos.find(task_ID);
    check(itr != todos.end(), "No such task existed!");
    todos.erase(itr);
};

[[eosio::action]] void todolist::rmvbyuser(name user)
{
    require_auth(user);
    todo_table todos(get_self(), user.value);
    auto itr = todos.begin();
    while (itr != todos.end())
    {
        itr = todos.erase(itr);
    }
};