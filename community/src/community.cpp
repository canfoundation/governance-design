
#include "../include/community.hpp"
#include "exchange_state.cpp"
#include <eosio/permission.hpp>

#include "code.cpp"
#include "position.cpp"

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

ACTION community::verifyholder(name community_account, uint64_t code_id, uint8_t execution_type, name owner)
{
    require_auth(_self);

    auto com_itr = _communitys.find(community_account.value);
    check(com_itr != _communitys.end(), "ERR::COMMUNITY_NOT_EXIST::Community is not existed.");

    code_table _codes(_self, community_account.value);
    position_table _positions(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    RightHolder right_holder;

    if (execution_type == ExecutionType::SOLE_DECISION) {
        code_sole_decision_table _code_execution_rule(_self, community_account.value);
        auto code_execution_rule_itr = _code_execution_rule.find(code_id);

        check(code_execution_rule_itr != _code_execution_rule.end(), "ERR::CODE_EXECUTION_RULE_NOT_EXIST::Code execution rule has not been initialize yet");

        right_holder = code_execution_rule_itr->right_executor;
    } else {
        code_collective_decision_table _code_vote_rule(_self, community_account.value);
        auto code_vote_rule_itr = _code_vote_rule.find(code_id);

        check(code_vote_rule_itr != _code_vote_rule.end(), "ERR::CODE_EXECUTION_RULE_NOT_EXIST::Code vote rule has not been initialize yet");

        right_holder = code_vote_rule_itr->right_proposer;
    }

    const bool is_set_right_holder = right_holder.accounts.size() != 0 ||
                                     right_holder.required_badges.size() != 0 ||
                                     right_holder.required_positions.size() != 0 ||
                                     right_holder.required_tokens.size() != 0;

    check(is_set_right_holder, "ERR::MISSING_RIGHT_HOLDER::Right holder for this code has not been set yet.");

    // verify right_holder's account
    auto _account_right_holders = right_holder.accounts;
    bool is_right_account = (_account_right_holders.size() == 0);
    auto it = std::find(_account_right_holders.begin(), _account_right_holders.end(), owner);
    if (it != _account_right_holders.end())
        is_right_account = true;

    check(is_right_account, "ERR::VERIFY_FAILED::Owner doesn't belong to code's right accounts.");

    // verify right_holder's badge
    auto _required_badge_ids = right_holder.required_badges;
    bool is_right_badge = (_required_badge_ids.size() == 0);
    for (int i = 0; i < _required_badge_ids.size(); i++)
    {
        ccerts _badges(cryptobadge_contract, owner.value);
        auto owner_badge_itr = _badges.find(_required_badge_ids[i]);
        if (owner_badge_itr != _badges.end())
        {
            is_right_badge = true;
            break;
        }
    }
    check(is_right_badge, "ERR::VERIFY_FAILED::Owner doesn't belong to code's right badges.");

    // verify right_holder's position
    auto _position_right_holder_ids = right_holder.required_positions;
    bool is_right_position = (_position_right_holder_ids.size() == 0);
    for (int i = 0; i < _position_right_holder_ids.size(); i++)
    {
        auto position_itr = _positions.find(_position_right_holder_ids[i]);
        auto _position_holders = position_itr->holders;
        if (std::find(_position_holders.begin(), _position_holders.end(), owner) != _position_holders.end())
        {
            is_right_position = true;
            break;
        }
    }
    check(is_right_position, "ERR::VERIFY_FAILED::Owner doesn't belong to code's right positions.");

    // verify right_holder's token
    auto _required_token_ids = right_holder.required_tokens;
    bool is_right_token = (_required_token_ids.size() == 0);
    for (int i = 0; i < _required_token_ids.size(); i++)
    {
        accounts _acnts(tiger_token_contract, owner.value);
        auto owner_token_itr = _acnts.find(_required_token_ids[i].symbol.code().raw());

        if (owner_token_itr != _acnts.end() && owner_token_itr->balance.amount >= _required_token_ids[i].amount)
        {
            is_right_token = true;
            break;
        }
    }
    check(is_right_token, "ERR::VERIFY_FAILED::Owner doesn't belong to code's right token.");
}

ACTION community::verifyamend(name community_account, uint64_t code_id, uint8_t execution_type, name owner)
{
    require_auth(_self);

    auto com_itr = _communitys.find(community_account.value);
    check(com_itr != _communitys.end(), "ERR::COMMUNITY_NOT_EXIST::Community is not existed.");

    code_table _codes(_self, community_account.value);
    position_table _positions(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    RightHolder right_holder;

    if (execution_type == ExecutionType::SOLE_DECISION) {
        amend_sole_decision_table _amend_execution_rule(_self, community_account.value);
        auto amend_execution_rule_itr = _amend_execution_rule.find(code_id);

        check(amend_execution_rule_itr != _amend_execution_rule.end(), "ERR::CODE_EXECUTION_RULE_NOT_EXIST::Code execution rule has not been initialize yet");

        right_holder = amend_execution_rule_itr->right_executor;
    } else {
        ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);
        auto amend_vote_rule_itr = _amend_vote_rule.find(code_id);

        check(amend_vote_rule_itr != _amend_vote_rule.end(), "ERR::CODE_EXECUTION_RULE_NOT_EXIST::Code vote rule has not been initialize yet");

        right_holder = amend_vote_rule_itr->right_proposer;
    }

    const bool is_set_right_holder = right_holder.accounts.size() != 0 || 
                                     right_holder.required_badges.size() != 0 ||
                                     right_holder.required_positions.size() != 0 ||
                                     right_holder.required_tokens.size() != 0;

    check(is_set_right_holder, "ERR::MISSING_RIGHT_HOLDER::Right holder for this code has not been set yet.");

    // verify right_holder's account
    auto _account_right_holders = right_holder.accounts;
    bool is_right_account = (_account_right_holders.size() == 0);
    auto it = std::find(_account_right_holders.begin(), _account_right_holders.end(), owner);
    if (it != _account_right_holders.end())
        is_right_account = true;

    check(is_right_account, "ERR::VERIFY_FAILED::Owner doesn't belong to code's right accounts.");

    // verify right_holder's badge
    auto _required_badge_ids = right_holder.required_badges;
    bool is_right_badge = (_required_badge_ids.size() == 0);
    for (int i = 0; i < _required_badge_ids.size(); i++)
    {
        ccerts _badges(cryptobadge_contract, owner.value);
        auto owner_badge_itr = _badges.find(_required_badge_ids[i]);
        if (owner_badge_itr != _badges.end())
        {
            is_right_badge = true;
            break;
        }
    }
    check(is_right_badge, "ERR::VERIFY_FAILED::Owner doesn't belong to code's right badges.");

    // verify right_holder's position
    auto _position_right_holder_ids = right_holder.required_positions;
    bool is_right_position = (_position_right_holder_ids.size() == 0);
    for (int i = 0; i < _position_right_holder_ids.size(); i++)
    {
        auto position_itr = _positions.find(_position_right_holder_ids[i]);
        auto _position_holders = position_itr->holders;
        if (std::find(_position_holders.begin(), _position_holders.end(), owner) != _position_holders.end())
        {
            is_right_position = true;
            break;
        }
    }
    check(is_right_position, "ERR::VERIFY_FAILED::Owner doesn't belong to code's right positions.");

    // verify right_holder's token
    auto _required_token_ids = right_holder.required_tokens;
    bool is_right_token = (_required_token_ids.size() == 0);
    for (int i = 0; i < _required_token_ids.size(); i++)
    {
        accounts _acnts(tiger_token_contract, owner.value);
        auto owner_token_itr = _acnts.find(_required_token_ids[i].symbol.code().raw());

        if (owner_token_itr != _acnts.end() && owner_token_itr->balance.amount >= _required_token_ids[i].amount)
        {
            is_right_token = true;
            break;
        }
    }
    check(is_right_token, "ERR::VERIFY_FAILED::Owner doesn't belong to code's right token.");
}

ACTION community::createbadge(
    name community_account,
    uint64_t badge_id,
    uint8_t issue_type,
    name badge_propose_name,
    uint8_t issue_exec_type,
    vector<name> issue_sole_right_accounts,
    vector<uint64_t> issue_sole_right_pos_ids,
    vector<name> issue_proposer_right_accounts,
    vector<uint64_t> issue_proposer_right_pos_ids,
    uint8_t issue_approval_type,
    vector<name> issue_approver_right_accounts,
    vector<uint64_t> issue_approver_right_pos_ids,
    vector<name> issue_voter_right_accounts,
    vector<uint64_t> issue_voter_right_pos_ids,
    double issue_pass_rule,
    uint64_t issue_vote_duration
) {
    // Todo: Verify badge param:
    action(
        permission_level{community_account, "active"_n},
        "eosio.msig"_n,
        "approve"_n,
        std::make_tuple(cryptobadge_contract, badge_propose_name, permission_level{community_account, "active"_n}))
        .send();

    action(
        permission_level{community_account, "active"_n},
        "eosio.msig"_n,
        "exec"_n,
        std::make_tuple(cryptobadge_contract, badge_propose_name, community_account))
        .send();

    code_table _codes(_self, community_account.value);

    code_sole_decision_table _code_execution_rule(_self, community_account.value);
    amend_sole_decision_table _amend_execution_rule(_self, community_account.value);

    code_collective_decision_table _code_vote_rule(_self, community_account.value);
    ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);

    position_table _positions(_self, community_account.value);

    auto getByCodeName = _codes.get_index<"by.code.name"_n>();
    auto ba_create_code = getByCodeName.find(BA_Create.value);

    vector<name> code_actions;
    code_actions.push_back("issuebadge"_n);

    name issue_badge_code_name;
    if (issue_type == BadgeIssueType::WITHOUT_CLAIM) {
        issue_badge_code_name = BA_Issue;
    } else if (issue_type == BadgeIssueType::CLAIM_APPROVE_BY_ISSUER) {
        issue_badge_code_name = BA_Claim;
    } else {
        check(false, "ERR::BADGE_ISSUE_TYPE_INVALID::Badge issue type is invalid");
    }

    // save new code to the table
    auto issue_badge_code = _codes.emplace(community_account, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = issue_badge_code_name;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = issue_exec_type;
        row.code_type = {CodeTypeEnum::BADGE, badge_id};
    });

    if (issue_exec_type != ExecutionType::COLLECTIVE_DECISION) {
        RightHolder _right_holder;
        _right_holder.accounts = issue_sole_right_accounts;
        _right_holder.required_positions = issue_sole_right_pos_ids;
        _code_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = issue_badge_code->code_id;
            row.right_executor = _right_holder;
        });
    }

    if (issue_exec_type != ExecutionType::SOLE_DECISION) {
        RightHolder _right_proposer;
        _right_proposer.accounts = issue_proposer_right_accounts;
        _right_proposer.required_positions = issue_proposer_right_pos_ids;
        RightHolder _right_approver;
        _right_proposer.accounts = issue_approver_right_accounts;
        _right_proposer.required_positions = issue_approver_right_pos_ids;
        RightHolder _right_voter;
        _right_proposer.accounts = issue_voter_right_accounts;
        _right_proposer.required_positions = issue_voter_right_pos_ids;
        _code_vote_rule.emplace(community_account, [&](auto &row) {
            row.code_id = issue_badge_code->code_id;
            row.right_proposer = _right_proposer;
            row.right_approver = _right_approver;
            row.right_voter = _right_voter;
            row.approval_type = issue_approval_type;
            row.pass_rule = issue_pass_rule;
            row.vote_duration = issue_vote_duration;
        });
    }

    code_actions.clear();
    code_actions.push_back("configbadge"_n);

    // save new code to the table
    auto config_badge_code = _codes.emplace(community_account, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = BA_Config;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.amendment_exec_type = ba_create_code->code_exec_type;
        row.code_type = {CodeTypeEnum::BADGE, badge_id};
    });

    if (ba_create_code->code_exec_type != ExecutionType::COLLECTIVE_DECISION) {
        auto ba_create_code_sole_decision = _code_execution_rule.find(ba_create_code->code_id);
        if(ba_create_code_sole_decision != _code_execution_rule.end()) {
            _amend_execution_rule.emplace(community_account, [&](auto &row) {
                row.code_id = config_badge_code->code_id;
                row.right_executor = ba_create_code_sole_decision->right_executor;
            });

            _code_execution_rule.emplace(community_account, [&](auto &row) {
                row.code_id = config_badge_code->code_id;
                row.right_executor = ba_create_code_sole_decision->right_executor;
            });
        }
    }

    if (ba_create_code->code_exec_type != ExecutionType::SOLE_DECISION) {
        auto ba_create_code_collective_decision = _code_vote_rule.find(ba_create_code->code_id);
        if (ba_create_code_collective_decision != _code_vote_rule.end()) {
            _amend_vote_rule.emplace(community_account, [&](auto &row) {
                row.code_id = config_badge_code->code_id;
                row.right_proposer = ba_create_code_collective_decision->right_proposer;
                row.right_approver = ba_create_code_collective_decision->right_approver;
                row.right_voter = ba_create_code_collective_decision->right_voter;
                row.approval_type = ba_create_code_collective_decision->approval_type;
                row.pass_rule = ba_create_code_collective_decision->pass_rule;
                row.vote_duration = ba_create_code_collective_decision->vote_duration;
            });

            _code_vote_rule.emplace(community_account, [&](auto &row) {
                row.code_id = config_badge_code->code_id;
                row.right_proposer = ba_create_code_collective_decision->right_proposer;
                row.right_approver = ba_create_code_collective_decision->right_approver;
                row.right_voter = ba_create_code_collective_decision->right_voter;
                row.approval_type = ba_create_code_collective_decision->approval_type;
                row.pass_rule = ba_create_code_collective_decision->pass_rule;
                row.vote_duration = ba_create_code_collective_decision->vote_duration;
            });
        }
    }
}

ACTION community::configbadge(
    name community_account,
    uint64_t badge_id,
    uint8_t issue_type,
    name update_badge_proposal_name,
    uint8_t issue_exec_type,
    vector<name> issue_sole_right_accounts,
    vector<uint64_t> issue_sole_right_pos_ids,
    vector<name> issue_proposer_right_accounts,
    vector<uint64_t> issue_proposer_right_pos_ids,
    uint8_t issue_approval_type,
    vector<name> issue_approver_right_accounts,
    vector<uint64_t> issue_approver_right_pos_ids,
    vector<name> issue_voter_right_accounts,
    vector<uint64_t> issue_voter_right_pos_ids,
    double issue_pass_rule,
    uint64_t issue_vote_duration
) {
    if (update_badge_proposal_name != name("")) {
        // Todo: Verify badge param:
        action(
            permission_level{community_account, "active"_n},
            "eosio.msig"_n,
            "approve"_n,
            std::make_tuple(cryptobadge_contract, update_badge_proposal_name, permission_level{community_account, "active"_n}))
            .send();

        action(
            permission_level{community_account, "active"_n},
            "eosio.msig"_n,
            "exec"_n,
            std::make_tuple(cryptobadge_contract, update_badge_proposal_name, community_account))
            .send();
    }

    code_table _codes(_self, community_account.value);

    code_sole_decision_table _code_execution_rule(_self, community_account.value);
    amend_sole_decision_table _amend_execution_rule(_self, community_account.value);

    code_collective_decision_table _code_vote_rule(_self, community_account.value);
    ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);

    position_table _positions(_self, community_account.value);

    auto getByCodeName = _codes.get_index<"by.code.name"_n>();
    auto ba_create_code = getByCodeName.find(BA_Create.value);

    name issue_badge_code_name;
    if (issue_type == BadgeIssueType::WITHOUT_CLAIM) {
        issue_badge_code_name = BA_Issue;
    } else if (issue_type == BadgeIssueType::CLAIM_APPROVE_BY_ISSUER){
        issue_badge_code_name = BA_Claim;
    } else {
        check(false, "ERR::BADGE_ISSUE_TYPE_INVALID::Badge issue type is invalid");
    }

    auto getByCodeReferId = _codes.get_index<"by.refer.id"_n>();
    auto issue_badge_code_itr = getByCodeReferId.find(badge_id);

    while (issue_badge_code_itr != getByCodeReferId.end()) {
        if (issue_badge_code_itr->code_type.type == CodeTypeEnum::BADGE && 
            ( issue_badge_code_itr->code_name == BA_Issue || issue_badge_code_itr->code_name == BA_Claim)) {
                break;
        }
        issue_badge_code_itr++;
    }
    
    // save new code to the table
    if (issue_badge_code_itr == getByCodeReferId.end()) {
        vector<name> code_actions;
        code_actions.push_back("issuebadge"_n);
        _codes.emplace(community_account, [&](auto &row) {
            row.code_id = _codes.available_primary_key();
            row.code_name = issue_badge_code_name;
            row.contract_name = get_self();
            row.code_actions = code_actions;
            row.amendment_exec_type = issue_exec_type;
            row.code_type = {CodeTypeEnum::BADGE, badge_id};
        });
    } else {
        getByCodeReferId.modify(issue_badge_code_itr, community_account, [&](auto &row) {
            row.code_name = issue_badge_code_name;
            row.code_exec_type = issue_exec_type;
            row.code_type = {CodeTypeEnum::BADGE, badge_id};
        });
    }

    if (issue_exec_type != ExecutionType::COLLECTIVE_DECISION) {
        RightHolder _right_holder;
        _right_holder.accounts = issue_sole_right_accounts;
        _right_holder.required_positions = issue_sole_right_pos_ids;
        auto code_exec_type_itr = _code_execution_rule.find(issue_badge_code_itr->code_id);
        if (code_exec_type_itr == _code_execution_rule.end()) {
            _code_execution_rule.emplace(community_account, [&](auto &row) {
                row.code_id = issue_badge_code_itr->code_id;
                row.right_executor = _right_holder;
            });
        } else {
            _code_execution_rule.modify(code_exec_type_itr, community_account, [&](auto &row) {
                row.right_executor = _right_holder;
            });
        }
    }

    if (issue_exec_type != ExecutionType::SOLE_DECISION) {
        RightHolder _right_proposer;
        _right_proposer.accounts = issue_proposer_right_accounts;
        _right_proposer.required_positions = issue_proposer_right_pos_ids;
        RightHolder _right_approver;
        _right_approver.accounts = issue_approver_right_accounts;
        _right_approver.required_positions = issue_approver_right_pos_ids;
        RightHolder _right_voter;
        _right_voter.accounts = issue_voter_right_accounts;
        _right_voter.required_positions = issue_voter_right_pos_ids;
        auto code_vote_rule_itr = _code_vote_rule.find(issue_badge_code_itr->code_id);
        if (code_vote_rule_itr == _code_vote_rule.end()) {
            _code_vote_rule.emplace(community_account, [&](auto &row) {
                row.code_id = issue_badge_code_itr->code_id;
                row.right_proposer = _right_proposer;
                row.right_approver = _right_approver;
                row.right_voter = _right_voter;
                row.approval_type = issue_approval_type;
                row.pass_rule = issue_pass_rule;
                row.vote_duration = issue_vote_duration;
            });
        } else {
            _code_vote_rule.modify(code_vote_rule_itr, community_account, [&](auto &row) {
                row.right_proposer = _right_proposer;
                row.right_approver = _right_approver;
                row.right_voter = _right_voter;
                row.approval_type = issue_approval_type;
                row.pass_rule = issue_pass_rule;
                row.vote_duration = issue_vote_duration;
            });
        }
    }
}

ACTION community::issuebadge(name community_account, name badge_propose_name)
{
    // Todo: Verify ceritification param:
    action(
        permission_level{community_account, "active"_n},
        "eosio.msig"_n,
        "approve"_n,
        std::make_tuple(cryptobadge_contract, badge_propose_name, permission_level{community_account, "active"_n}))
        .send();

    action(
        permission_level{community_account, "active"_n},
        "eosio.msig"_n,
        "exec"_n,
        std::make_tuple(cryptobadge_contract, badge_propose_name, community_account))
        .send();
}

bool community::verifyvoter(name community_account, name voter, uint64_t code_id, bool is_amend_code)
{
    bool is_right_holder = false;

    RightHolder right_voter;
    if (is_amend_code) {
        ammend_collective_decision_table _collective_exec(_self, community_account.value);

        auto collective_exec_itr = _collective_exec.find(code_id);

        right_voter = collective_exec_itr->right_voter;
    } else {
        code_collective_decision_table _collective_exec(_self, community_account.value);

        auto collective_exec_itr = _collective_exec.find(code_id);

        right_voter = collective_exec_itr->right_voter;
    }

    auto _account_right_holders = right_voter.accounts;

    is_right_holder = std::find(_account_right_holders.begin(), _account_right_holders.end(), voter) != _account_right_holders.end();

    auto _position_right_holder_ids = right_voter.required_positions;

    position_table _positions(_self, community_account.value);
    for (int i = 0; i < _position_right_holder_ids.size(); i++)
    {
        auto position_itr = _positions.find(_position_right_holder_ids[i]);
        auto _position_holders = position_itr->holders;
        if (std::find(_position_holders.begin(), _position_holders.end(), voter) != _position_holders.end())
        {
            is_right_holder = true;
            break;
        }
    }

    return is_right_holder;
}

bool community::verifyapprov(name community_account, name approver, uint64_t code_id)
{
    bool is_right_holder = false;

    code_collective_decision_table _collective_exec(_self, community_account.value);
    position_table _positions(_self, community_account.value);

    auto collective_exec_itr = _collective_exec.find(code_id);

    auto _account_right_holders = collective_exec_itr->right_approver.accounts;

    is_right_holder = std::find(_account_right_holders.begin(), _account_right_holders.end(), approver) != _account_right_holders.end();

    auto _position_right_holder_ids = collective_exec_itr->right_approver.required_positions;

    for (int i = 0; i < _position_right_holder_ids.size(); i++)
    {
        auto position_itr = _positions.find(_position_right_holder_ids[i]);
        auto _position_holders = position_itr->holders;
        if (std::find(_position_holders.begin(), _position_holders.end(), approver) != _position_holders.end())
        {
            is_right_holder = true;
            break;
        }
    }

    return is_right_holder;
}

bool community::is_pos_candidate(name community_account, uint64_t pos_id, name owner)
{

    election_table _electionrule(_self, community_account.value);
    auto election_itr = _electionrule.find(pos_id);
    check(election_itr != _electionrule.end(), "ERR::ELECTION_RULE_NOT_EXIST::Position need election rules.");
    auto _pos_candidate_holder = election_itr->pos_candidates.accounts;

    return std::find(_pos_candidate_holder.begin(), _pos_candidate_holder.end(), owner) != _pos_candidate_holder.end();
}

bool community::is_pos_voter(name community_account, uint64_t pos_id, name owner)
{
    election_table _electionrule(_self, community_account.value);
    auto election_itr = _electionrule.find(pos_id);
    check(election_itr != _electionrule.end(), "ERR::ELECTION_RULE_NOT_EXIST::Position need election rules.");
    auto _pos_voter_holder = election_itr->pos_voters.accounts;

    return std::find(_pos_voter_holder.begin(), _pos_voter_holder.end(), owner) != _pos_voter_holder.end();
}
/*
* Increment, save and return id for a new position.
*/
uint64_t community::get_pos_proposed_id()
{

    global_table config(_self, _self.value);
    auto _cstate = config.exists() ? config.get() : global{};

    ++_cstate.posproposed_id;

    config.set(_cstate, _self);
    return _cstate.posproposed_id;
}

asset community::convertbytes2cat(uint32_t bytes)
{
    eosiosystem::rammarket _rammarket("eosio"_n, "eosio"_n.value);
    auto itr = _rammarket.find(ramcore_symbol.raw());
    auto tmp = *itr;
    auto eosout = tmp.convert(asset(bytes, ram_symbol), CORE_SYMBOL);
    return eosout;
}

bool community::is_amend_action(name calling_action) {
    return calling_action == set_execution_type_action ||
           calling_action == set_sole_execution_right_holder_action ||
           calling_action == set_approval_type_action ||
           calling_action == set_voter_action ||
           calling_action == set_proposer_action ||
           calling_action == set_approver_action ||
           calling_action == set_vote_rule_action;
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
