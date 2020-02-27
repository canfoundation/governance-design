#include "../include/community.hpp"
#include "exchange_state.cpp"
// #include <eosio/permission.hpp>

void community::transfer(name from, name to, asset quantity, string memo)
{
    if (from == _self)
    {
        return;
    }
    check(to == _self, "ERR::VERIFY_FAILED::contract is not involved in this transfer");
    check(quantity.symbol.is_valid(), "ERR::VERIFY_FAILED::invalid quantity");
    check(quantity.amount > 0, "ERR::VERIFY_FAILED::only positive quantity allowed");
    check(quantity.symbol == CORE_SYMBOL, "ERR::VERIFY_FAILED::only accepts CAT");
    check(quantity.amount > 0, "ERR::VERIFY_FAILED::must transfer positive quantity");

    string community_str = memo.c_str();
    if (community_str.length() == 12)
    {
        name community_acc = name{community_str};
        check(!is_account(community_acc), "ERR::VERIFY_FAILED::account already exist");

        const asset ram_fee = convertbytes2cat(init_ram_amount);
        check(quantity >= stake_cpu + stake_net + ram_fee, "ERR::VERIFY_FAILED::insuffent balance to create new account");
        const asset remain_balance = quantity - stake_cpu - stake_net - ram_fee;

        if (remain_balance.amount > 0)
        {
            action(
                permission_level{_self, "active"_n},
                "eosio.token"_n,
                "transfer"_n,
                std::make_tuple(_self, from, remain_balance, std::string("return remain amount")))
                .send();
        }

        action(
            permission_level{_self, "active"_n},
            get_self(),
            "createacc"_n,
            std::make_tuple(from, community_acc))
            .send();
    }
}

ACTION community::createacc(name community_creator, name community_acc)
{
    require_auth(get_self());

    auto com_itr = _communitys.find(community_acc.value);
    check(com_itr == _communitys.end(), "ERR::CREATEPROP_ALREADY_EXIST::Community already exists.");

    _communitys.emplace(_self, [&](auto &row) {
        row.community_account = community_acc;
        row.creator = community_creator;
    });

    permission_level_weight account_permission_level = {permission_level{_self, "eosio.code"_n}, 1};

    authority owner_authority = {1, {}, {account_permission_level}, std::vector<wait_weight>()};
    authority active_authority = {1, {}, {account_permission_level}, std::vector<wait_weight>()};

    action(
        permission_level{_self, "active"_n},
        "eosio"_n,
        "newaccount"_n,
        std::make_tuple(_self, community_acc, owner_authority, active_authority))
        .send();

    action(
        permission_level{_self, "active"_n},
        "eosio"_n,
        "buyrambytes"_n,
        std::make_tuple(_self, community_acc, init_ram_amount))
        .send();

    action(
        permission_level{_self, "active"_n},
        "eosio"_n,
        "delegatebw"_n,
        std::make_tuple(_self, community_acc, stake_net, stake_cpu, true))
        .send();
}

ACTION community::create(name creator, name community_account, string &community_name, vector<uint64_t> member_badge, string &community_url, string &description, bool create_default_code)
{
    require_auth(creator);

    check(community_name.length() > 3, "ERR::CREATEPROP_SHORT_TITLE::Name length is too short.");
    check(community_url.length() > 3, "ERR::CREATEPROP_SHORT_URL::Url length is too short.");
    check(description.length() > 3, "ERR::CREATEPROP_SHORT_DESC::Description length is too short.");

    auto com_itr = _communitys.find(community_account.value);

    check(com_itr != _communitys.end() && com_itr->creator == creator, "ERR::CREATEPROP_NOT_EXIST::Community does not exist.");

    _communitys.modify(com_itr, creator, [&](auto &row) {
        row.community_name = community_name;
        row.member_badge = member_badge;
        row.community_url = community_url;
        row.description = description;
    });

    // init template code
    action(
        permission_level{community_account, "active"_n},
        get_self(),
        "initcode"_n,
        std::make_tuple(community_account, creator, create_default_code))
        .send();
}

bool community::is_token_holder(vector<asset> token_ids, name owner) {
    bool is_right_token = (token_ids.size() == 0);
    for (int i = 0; i < token_ids.size(); i++)
    {
        accounts _acnts(tiger_token_contract, owner.value);
        auto owner_token_itr = _acnts.find(token_ids[i].symbol.code().raw());

        if (owner_token_itr != _acnts.end() && owner_token_itr->balance.amount >= token_ids[i].amount)
        {
            is_right_token = true;
            break;
        }
    }
    return is_right_token;
}

asset community::convertbytes2cat(uint32_t bytes)
{
    eosiosystem::rammarket _rammarket("eosio"_n, "eosio"_n.value);
    auto itr = _rammarket.find(ramcore_symbol.raw());
    auto tmp = *itr;
    auto eosout = tmp.convert(asset(bytes, ram_symbol), CORE_SYMBOL);
    return eosout;
}

#define EOSIO_ABI_CUSTOM(TYPE, MEMBERS)                                                       \
    extern "C"                                                                                \
    {                                                                                         \
        void apply(uint64_t receiver, uint64_t code, uint64_t action)                         \
        {                                                                                     \
            auto self = receiver;                                                             \
            if (code == self || code == "eosio.token"_n.value || action == "onerror"_n.value) \
            {                                                                                 \
                if (action == "transfer"_n.value)                                             \
                {                                                                             \
                    check(code == "eosio.token"_n.value, "Must transfer Token");              \
                }                                                                             \
                switch (action)                                                               \
                {                                                                             \
                    EOSIO_DISPATCH_HELPER(TYPE, MEMBERS)                                      \
                }                                                                             \
                /* does not allow destructor of thiscontract to run: eosio_exit(0); */        \
            }                                                                                 \
        }                                                                                     \
    }

EOSIO_ABI_CUSTOM(
community, 
(setapprotype)
(setvoter)
(setapprover)
(transfer)
(verifyamend)
(createacc)
(create)
(initcode)
(initadminpos)
(execcode)
(createcode)
(createpos)
(configpos)
(nominatepos)
(approvepos)
(voteforcode)
(voteforpos)
(dismisspos)
(setexectype)
(appointpos)
(proposecode)
(execproposal)
(verifyholder)
(createbadge)
(issuebadge)
(configbadge)
(setsoleexec)
(setproposer)
(setvoterule)
)
