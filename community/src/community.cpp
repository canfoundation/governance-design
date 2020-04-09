
#include "../include/community.hpp"
#include "exchange_state.cpp"
#include <eosio/permission.hpp>

const name ram_payer_system = "ram.can"_n;

const symbol ramcore_symbol = symbol(symbol_code("RAMCORE"), 4);
const symbol ram_symbol = symbol(symbol_code("RAM"), 0);
const symbol CORE_SYMBOL = symbol(symbol_code("CAT"), 4);
const uint64_t init_ram_amount = 10 * 1024;
const asset stake_net = asset(1'0000, CORE_SYMBOL);
const asset stake_cpu = asset(1'0000, CORE_SYMBOL);
const asset min_active_contract = asset(10'0000, CORE_SYMBOL);

const name community_name_creator = "c"_n;
const name cryptobadge_contract = "badge"_n;
const name tiger_token_contract = "tiger.token"_n;
const name set_execution_type_action = "setexectype"_n;
const name set_sole_execution_right_holder_action = "setsoleexec"_n;
const name set_approval_type_action = "setapprotype"_n;
const name set_proposer_action = "setproposer"_n;
const name set_approver_action = "setapprover"_n;
const name set_voter_action = "setvoter"_n;
const name set_vote_rule_action = "setvoterule"_n;
const uint64_t pos_admin_id = 1;

const uint64_t seconds_per_day = 24 * 60 * 60;

const string default_admin_position_name = "Admin";
const uint64_t default_admin_position_max_holder = 10;

// list of codes
const name CO_Access = "co.access"_n;
const name CO_Amend = "co.amend"_n;
const name CO_InputMem = "co.members"_n;

const name PO_Create = "po.create"_n;
const name PO_Config = "po.config"_n;
const name PO_Appoint = "po.appoint"_n;
const name PO_Dismiss = "po.dismiss"_n;

const name BA_Create = "ba.create"_n;
const name BA_Issue = "ba.issue"_n;
const name BA_Claim = "ba.claim"_n;
const name BA_Config = "ba.config"_n;
const name BA_Adopt = "ba.adopt"_n;
const name BA_Discard = "ba.discard"_n;

void community::transfer(name from, name to, asset quantity, string memo)
{
    if (from == _self)
    {
        return;
    }
    check(to == _self, "ERR::VERIFY_FAILED::contract is not involved in this transfer");
    check(quantity.symbol.is_valid(), "ERR::VERIFY_FAILED::invalid quantity");
    check(quantity.amount > 0, "ERR::VERIFY_FAILED::only positive quantity allowed");
    check(quantity.amount > 0, "ERR::VERIFY_FAILED::must transfer positive quantity");
    check(get_balance("eosio.token"_n, get_self(), CORE_SYMBOL.code()) >= min_active_contract, "ERR::VERIFY_FAILED::Deposit at least 10 CAT to active creating commnity feature");

    const std::size_t first_break = memo.find("-");
    std::string community_str = memo.substr(0, first_break);

    name community_creator = from;
    if (first_break != std::string::npos){
        std::string creator_str = memo.substr(first_break+1);
        const eosio::name creator_name = eosio::name{creator_str};
        if (creator_name != _self && creator_name != from && is_account(creator_name))
        {
            community_creator = creator_name;
        }
    }
    if ( quantity.symbol == CORE_SYMBOL && community_str != "deposit_core_symbol" )
    {
        name community_acc = name{community_str};
        check(verify_community_account_input(community_acc), "community account name is invalid");
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
            std::make_tuple(community_creator, community_acc))
            .send();
    }
}

ACTION community::createacc(name community_creator, name community_acc)
{
    require_auth(get_self());

    auto com_itr = _communities.find(community_acc.value);
    check(com_itr == _communities.end(), "ERR::CREATEPROP_ALREADY_EXIST::Community already exists.");

    _communities.emplace(_self, [&](auto &row) {
        row.community_account = community_acc;
        row.creator = community_creator;
    });

    permission_level_weight account_permission_level = {permission_level{_self, "eosio.code"_n}, 1};

    authority owner_authority = {1, {}, {account_permission_level}, std::vector<wait_weight>()};
    authority active_authority = {1, {}, {account_permission_level}, std::vector<wait_weight>()};

    action(
        permission_level{community_name_creator, "active"_n},
        "eosio"_n,
        "newaccount"_n,
        std::make_tuple(community_name_creator, community_acc, owner_authority, active_authority))
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

    auto ram_payer = creator;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    check(community_name.length() > 3, "ERR::CREATEPROP_SHORT_TITLE::Name length is too short.");
    check(community_url.length() > 3, "ERR::CREATEPROP_SHORT_URL::Url length is too short.");
    check(description.length() > 3, "ERR::CREATEPROP_SHORT_DESC::Description length is too short.");

    auto com_itr = _communities.find(community_account.value);

    check(com_itr != _communities.end() && com_itr->creator == creator, "ERR::CREATEPROP_NOT_EXIST::Community does not exist.");

    _communities.modify(com_itr, ram_payer, [&](auto &row) {
        row.community_name = community_name;
        row.member_badge = member_badge;
        row.community_url = community_url;
        row.description = description;
    });

    vector<name> accession_account;
    accession_account.push_back(creator);
    accession default_accession;
    default_accession.right_access.is_any_community_member = true;
    default_accession.right_access.accounts = accession_account;
    accession_table _accession(_self, community_account.value);
    _accession.set(default_accession, ram_payer);

    // init template code

    vector<eosio::permission_level> action_permission = {{community_account, "active"_n}};
    if(ram_payer == ram_payer_system)  action_permission.push_back({ram_payer_system, "active"_n});
    action(
        action_permission,
        get_self(),
        "initcode"_n,
        std::make_tuple(community_account, creator, create_default_code))
        .send();
}

ACTION community::setaccess(name community_account, RightHolder right_access)
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    auto com_itr = _communities.find(community_account.value);
    check(com_itr != _communities.end(), "ERR::CREATEPROP_NOT_EXIST::Community does not exist.");

    verify_right_holder_input(community_account, right_access);

    auto it = std::find(right_access.accounts.begin(), right_access.accounts.end(), com_itr->creator);
    if (it == right_access.accounts.end()) {
        right_access.accounts.push_back(com_itr->creator);
    }

    accession setting_accession;
    setting_accession.right_access = right_access;

    accession_table _accession(_self, community_account.value);
    _accession.set(setting_accession, ram_payer);
}

ACTION community::initcode(name community_account, name creator, bool create_default_code)
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;
    
    code_table _codes(_self, community_account.value);

    auto getByCodeId = _codes.get_index<"by.code.name"_n>();
    check(getByCodeId.find(CO_Amend.value) == getByCodeId.end(), "ERR::VERIFY_FAILED::Code already initialize.");

    code_sole_decision_table _code_execution_rule(_self, community_account.value);
    amend_sole_decision_table _amend_execution_rule(_self, community_account.value);

    RightHolder _createcode_right_holder;
    _createcode_right_holder.accounts.push_back(creator);
    vector<name> _init_actions;
    _init_actions.push_back("createcode"_n);

    vector<eosio::permission_level> action_permission = {{community_account, "active"_n}};
    if(ram_payer == ram_payer_system)  action_permission.push_back({ram_payer_system, "active"_n});

    action(
        action_permission,
        get_self(),
        "initadminpos"_n,
        std::make_tuple(community_account, creator))
        .send();

    RightHolder _init_right_holder;

    _init_right_holder.required_positions.push_back(pos_admin_id);

    // initialize createcode code
    auto co_amend_code = _codes.emplace(ram_payer, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = CO_Amend;
        row.contract_name = get_self();
        row.code_actions = _init_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {NORMAL, 0};
    });

    _code_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = co_amend_code->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = co_amend_code->code_id;
            row.right_executor = _init_right_holder;
    });

    if (create_default_code)
    {
        // create inputmembers code
        if (getByCodeId.find(CO_InputMem.value) == getByCodeId.end())
        {
            _init_actions.clear();
            _init_actions.push_back("inputmembers"_n);
            // initialize createcode code
            auto co_inputmem_code = _codes.emplace(ram_payer, [&](auto &row) {
                row.code_id = _codes.available_primary_key();
                row.code_name = CO_InputMem;
                row.contract_name = get_self();
                row.code_actions = _init_actions;
                row.code_exec_type = ExecutionType::SOLE_DECISION;
                row.amendment_exec_type = ExecutionType::SOLE_DECISION;
                row.code_type = {NORMAL, 0};
            });

            _code_execution_rule.emplace(ram_payer, [&](auto &row) {
                    row.code_id = co_inputmem_code->code_id;
                    row.right_executor = _init_right_holder;
            });

            _amend_execution_rule.emplace(ram_payer, [&](auto &row) {
                    row.code_id = co_inputmem_code->code_id;
                    row.right_executor = _init_right_holder;
            });
        }

        if (getByCodeId.find(PO_Create.value) == getByCodeId.end())
        {
            _init_actions.clear();
            _init_actions.push_back("createpos"_n);

            // initialize createcode code
            auto po_create_code = _codes.emplace(ram_payer, [&](auto &row) {
                row.code_id = _codes.available_primary_key();
                row.code_name = PO_Create;
                row.contract_name = get_self();
                row.code_actions = _init_actions;
                row.code_exec_type = ExecutionType::SOLE_DECISION;
                row.amendment_exec_type = ExecutionType::SOLE_DECISION;
                row.code_type = {NORMAL, 0};
            });

            _code_execution_rule.emplace(ram_payer, [&](auto &row) {
                row.code_id = po_create_code->code_id;
                row.right_executor = _init_right_holder;
            });

            _amend_execution_rule.emplace(ram_payer, [&](auto &row) {
                row.code_id = po_create_code->code_id;
                row.right_executor = _init_right_holder;
            });
        }

        if (getByCodeId.find(CO_Access.value) == getByCodeId.end())
        {
            _init_actions.clear();
            _init_actions.push_back("setaccess"_n);

            // initialize createcode code
            auto co_access_code = _codes.emplace(ram_payer, [&](auto &row) {
                row.code_id = _codes.available_primary_key();
                row.code_name = CO_Access;
                row.contract_name = get_self();
                row.code_actions = _init_actions;
                row.code_exec_type = ExecutionType::SOLE_DECISION;
                row.amendment_exec_type = ExecutionType::SOLE_DECISION;
                row.code_type = {NORMAL, 0};
            });

            _code_execution_rule.emplace(ram_payer, [&](auto &row) {
                row.code_id = co_access_code->code_id;
                row.right_executor = _init_right_holder;
            });

            _amend_execution_rule.emplace(ram_payer, [&](auto &row) {
                row.code_id = co_access_code->code_id;
                row.right_executor = _init_right_holder;
            });
        }

        if (getByCodeId.find(BA_Create.value) == getByCodeId.end())
        {
            _init_actions.clear();
            _init_actions.push_back("createbadge"_n);

            // initialize createcode code
            auto ba_create_code = _codes.emplace(ram_payer, [&](auto &row) {
                row.code_id = _codes.available_primary_key();
                row.code_name = BA_Create;
                row.contract_name = get_self();
                row.code_actions = _init_actions;
                row.code_exec_type = ExecutionType::SOLE_DECISION;
                row.amendment_exec_type = ExecutionType::SOLE_DECISION;
                row.code_type = {NORMAL, 0};
            });

            _code_execution_rule.emplace(ram_payer, [&](auto &row) {
                row.code_id = ba_create_code->code_id;
                row.right_executor = _init_right_holder;
            });

            _amend_execution_rule.emplace(ram_payer, [&](auto &row) {
                row.code_id = ba_create_code->code_id;
                row.right_executor = _init_right_holder;
            });
        }
    }
}

ACTION community::inputmembers(name community_account, vector<name> added_members, vector<name> removed_members)
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    members_table _members(_self, community_account.value);

    for (auto added_member: added_members){
        auto mem_itr = _members.find(added_member.value);
        check(mem_itr == _members.end(), "ERR::MEMBER_ALREADY_EXIST::At least one of the member already exist.");
        _members.emplace(ram_payer, [&](auto &row) {
            row.member = added_member;
        });
    }

    for (auto removed_member: removed_members){
        auto mem_itr = _members.find(removed_member.value);
        check(mem_itr != _members.end(), "ERR::MEMBER_NOT_FOUND::At least one of the member was not found.");
        _members.erase(mem_itr);
    }
}

ACTION community::execcode(name community_account, name exec_account, uint64_t code_id, vector<execution_code_data> code_actions)
{
    require_auth(exec_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    auto com_itr = _communities.find(community_account.value);
    check(com_itr != _communities.end(), "ERR::VERIFY_FAILED::Community doesn't exist.");

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    for (auto execution_data: code_actions) {

        if (!is_amend_action(execution_data.code_action))
        {
            check(code_itr->code_exec_type != ExecutionType::COLLECTIVE_DECISION, "ERR::INVALID_EXEC_TYPE::Can not execute collective decision code, please use proposecode action");
            check(std::find(code_itr->code_actions.begin(), code_itr->code_actions.end(), execution_data.code_action) != code_itr->code_actions.end(), "ERR::VERIFY_FAILED::Action doesn't exist.");
            
            // Verify Right Holder
            action(
                permission_level{get_self(), "active"_n},
                get_self(),
                "verifyholder"_n,
                std::make_tuple(community_account, code_id, uint8_t(ExecutionType::SOLE_DECISION), exec_account, is_amend_action(execution_data.code_action)))
                .send();

            call_action(community_account, ram_payer, code_itr->contract_name, execution_data.code_action, execution_data.packed_params);
        }
        else
        {
            check(code_itr->amendment_exec_type != ExecutionType::COLLECTIVE_DECISION, "ERR::INVALID_EXEC_TYPE::Can not execute collective decision code, please use proposecode action");
            action(
                permission_level{get_self(), "active"_n},
                get_self(),
                "verifyholder"_n,
                std::make_tuple(community_account, code_id, uint8_t(ExecutionType::SOLE_DECISION), exec_account, is_amend_action(execution_data.code_action)))
                .send();

            call_action(community_account, ram_payer, _self, execution_data.code_action, execution_data.packed_params);
        }
    }
}

ACTION community::proposecode(name community_account, name proposer, name proposal_name, uint64_t code_id, vector<execution_code_data> code_actions)
{
    require_auth(proposer);

    auto ram_payer = proposer;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;    

    code_table _codes(_self, community_account.value);

    code_proposals_table _proposals(_self, community_account.value);
    auto proposal_itr = _proposals.find(proposal_name.value);
    check(proposal_itr == _proposals.end(), "ERR::PROPOSAL_NAME_EXIST::The proposal with the this name has already exist");

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    eosio::transaction auto_execute;
    auto_execute.actions.emplace_back(eosio::permission_level{community_account, "active"_n}, _self, "execproposal"_n, std::make_tuple(community_account, proposal_name));
    
    for (auto execution_data: code_actions) {
        if (!is_amend_action(execution_data.code_action))
        {
            check(std::find(code_itr->code_actions.begin(), code_itr->code_actions.end(), execution_data.code_action) != code_itr->code_actions.end(), "ERR::VERIFY_FAILED::Action doesn't exist.");
            check(code_itr->code_exec_type != ExecutionType::SOLE_DECISION, "ERR::INVALID_EXEC_TYPE::Can not create proposal for sole decision code");
            // Verify Right Holder
             action(
                permission_level{get_self(), "active"_n},
                get_self(),
                "verifyholder"_n,
                std::make_tuple(community_account, code_id, uint8_t(ExecutionType::COLLECTIVE_DECISION), proposer, is_amend_action(execution_data.code_action)))
                .send();

            code_collective_decision_table _collective_exec(_self, community_account.value);
            auto collective_exec_itr = _collective_exec.find(code_id);
            check(collective_exec_itr != _collective_exec.end(), "ERR::COLLECTIVE_NOT_EXISTED::The collective rule is not exist, please initialize it before create proposal");

            auto_execute.delay_sec = collective_exec_itr->vote_duration;
        }
        else
        {
            check(code_itr->amendment_exec_type != ExecutionType::SOLE_DECISION, "ERR::INVALID_EXEC_TYPE::Can not create proposal for sole decision code");
            // Verify Right Holder
            action(
                permission_level{get_self(), "active"_n},
                get_self(),
                "verifyholder"_n,
                std::make_tuple(community_account, code_id, uint8_t(ExecutionType::COLLECTIVE_DECISION), proposer, is_amend_action(execution_data.code_action)))
                .send();

            ammend_collective_decision_table _collective_exec(_self, community_account.value);
            auto collective_exec_itr = _collective_exec.find(code_id);
            check(collective_exec_itr != _collective_exec.end(), "ERR::COLLECTIVE_NOT_EXISTED::The collective rule is not exist, please initialize it before create proposal");

            auto_execute.delay_sec = collective_exec_itr->vote_duration + 1;
        }
    }

    _proposals.emplace(ram_payer, [&](auto &row) {
        row.proposal_name = proposal_name;
        row.proposer = proposer;
        row.code_id = code_id;
        row.propose_time = current_time_point();
        row.code_actions = code_actions;
    });

    auto_execute.send(proposal_name.value, _self, true);

}

ACTION community::execproposal(name community_account, name proposal_name)
{
    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    code_proposals_table _proposals(_self, community_account.value);
    auto proposal_itr = _proposals.find(proposal_name.value);
    check(proposal_itr != _proposals.end(), "ERR::VOTE_NOT_FINISH::The voting proposal for this code has not been finished yet");

    code_table _codes(_self, community_account.value);
    auto code_itr = _codes.find(proposal_itr->code_id);

    for (auto action: proposal_itr->code_actions) {
        if (is_amend_action(action.code_action))
        {
            // check the voting proposal for this code has been finished
            ammend_collective_decision_table _collective_exec(_self, community_account.value);
            auto collective_exec_itr = _collective_exec.find(proposal_itr->code_id);
            check(collective_exec_itr != _collective_exec.end(), "ERR::COLLECTIVE_RULE_NOT_EXIST::The collective rule for this code has not been set yet");

            check(collective_exec_itr->approval_type != ApprovalType::SOLE_APPROVAL, "ERR::EXECUTE_SOLE_APPROVAL_PROPOSAL::Can not execute sole approval proposal");

            // check that voting time has been ended
            check(proposal_itr->propose_time.sec_since_epoch() + collective_exec_itr->vote_duration <= current_time_point().sec_since_epoch(), "ERR::VOTE_NOT_FINISH::The voting proposal for this code has not been finished yet");

            // check that code has been accepted by voter or not
            check(proposal_itr->voted_percent >= collective_exec_itr->pass_rule, "ERR::CODE_NOT_ACCEPTED::This code has not been aceepted by voter");

            // check that proposal has been executed or not
            check(proposal_itr->proposal_status != EXECUTED, "ERR::EXECUTED_PROPOSAL::This proposal has been executed");

            // check that proposal has been expired
            check(proposal_itr->proposal_status != EXPIRED, "ERR::EXPIRED_PROPOSAL::This proposal has been expired");

            call_action(community_account, ram_payer, _self, action.code_action, action.packed_params);
        }
        else
        {
            // check the voting proposal for this code has been finished
            code_collective_decision_table _collective_exec(_self, community_account.value);
            auto collective_exec_itr = _collective_exec.find(proposal_itr->code_id);
            check(collective_exec_itr != _collective_exec.end(), "ERR::COLLECTIVE_RULE_NOT_EXIST::The collective rule for this code has not been set yet");

            check(collective_exec_itr->approval_type != ApprovalType::SOLE_APPROVAL, "ERR::EXECUTE_SOLE_APPROVAL_PROPOSAL::Can not execute sole approval proposal");

            // check that voting time has been ended
            check(proposal_itr->propose_time.sec_since_epoch() + collective_exec_itr->vote_duration <= current_time_point().sec_since_epoch(), "ERR::VOTE_NOT_FINISH::The voting proposal for this code has not been finished yet");

            // check that code has been accepted by voter or not
            check(proposal_itr->voted_percent >= collective_exec_itr->pass_rule, "ERR::CODE_NOT_ACCEPTED::This code has not been aceepted by voter");

            // check that proposal has been executed or not
            check(proposal_itr->proposal_status != EXECUTED, "ERR::EXECUTED_PROPOSAL::This proposal has been executed");

            // check that proposal has been expired
            check(proposal_itr->proposal_status != EXPIRED, "ERR::EXPIRED_PROPOSAL::This proposal has been expired");

            call_action(community_account, ram_payer, code_itr->contract_name, action.code_action, action.packed_params);
        }
    }

    _proposals.erase(proposal_itr);
}

ACTION community::verifyholder(name community_account, uint64_t code_id, uint8_t execution_type, name owner, bool is_ammend_holder)
{
    require_auth(_self);

    auto com_itr = _communities.find(community_account.value);
    check(com_itr != _communities.end(), "ERR::COMMUNITY_NOT_EXIST::Community is not existed.");

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    RightHolder right_holder;

    if (execution_type == ExecutionType::SOLE_DECISION) {
        if (is_ammend_holder) {
            amend_sole_decision_table _amend_execution_rule(_self, community_account.value);
            auto amend_execution_rule_itr = _amend_execution_rule.find(code_id);

            check(amend_execution_rule_itr != _amend_execution_rule.end(), "ERR::CODE_EXECUTION_RULE_NOT_EXIST::Code execution rule has not been initialize yet");

            right_holder = amend_execution_rule_itr->right_executor;
        } else {
            code_sole_decision_table _code_execution_rule(_self, community_account.value);
            auto code_execution_rule_itr = _code_execution_rule.find(code_id);

            check(code_execution_rule_itr != _code_execution_rule.end(), "ERR::CODE_EXECUTION_RULE_NOT_EXIST::Code execution rule has not been initialize yet");

            right_holder = code_execution_rule_itr->right_executor;
        }
    } else {
        if (is_ammend_holder) {
            ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);
            auto amend_vote_rule_itr = _amend_vote_rule.find(code_id);

            check(amend_vote_rule_itr != _amend_vote_rule.end(), "ERR::CODE_EXECUTION_RULE_NOT_EXIST::Code vote rule has not been initialize yet");

            right_holder = amend_vote_rule_itr->right_proposer;
        } else {
            code_collective_decision_table _code_vote_rule(_self, community_account.value);
            auto code_vote_rule_itr = _code_vote_rule.find(code_id);

            check(code_vote_rule_itr != _code_vote_rule.end(), "ERR::CODE_EXECUTION_RULE_NOT_EXIST::Code vote rule has not been initialize yet");

            right_holder = code_vote_rule_itr->right_proposer;
        }
    }

    check(verify_account_right_holder(community_account, right_holder, owner), "ERR::VERIFY_FAILED::Owner doesn't belong to code's right holder.");
}

ACTION community::createcode(name community_account, name code_name, name contract_name, vector<name> code_actions)
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    code_table _codes(_self, community_account.value);

    code_sole_decision_table _code_execution_rule(_self, community_account.value);
    amend_sole_decision_table _amend_execution_rule(_self, community_account.value);

    code_collective_decision_table _code_vote_rule(_self, community_account.value);
    ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);

    position_table _positions(_self, community_account.value);

    RightHolder _init_right_holder;

    auto getByCodeName = _codes.get_index<"by.code.name"_n>();
    auto co_amend_code = getByCodeName.find(CO_Amend.value);

    // save new code to the table
    auto new_codes = _codes.emplace(ram_payer, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = code_name;
        row.contract_name = contract_name;
        row.code_actions = code_actions;
        row.code_exec_type = co_amend_code->code_exec_type;
        row.amendment_exec_type = co_amend_code->amendment_exec_type;
        row.code_type = {NORMAL, 0};
    });

    if (co_amend_code->code_exec_type != ExecutionType::COLLECTIVE_DECISION) {
        auto co_amend_code_sole_decision = _code_execution_rule.find(co_amend_code->code_id);
        if(co_amend_code_sole_decision != _code_execution_rule.end()) {
            _amend_execution_rule.emplace(ram_payer, [&](auto &row) {
                row.code_id = new_codes->code_id;
                row.right_executor = co_amend_code_sole_decision->right_executor;
            });
        }
    }

    if (co_amend_code->code_exec_type != ExecutionType::SOLE_DECISION) {
        auto co_amend_code_collective_decision = _code_vote_rule.find(co_amend_code->code_id);
        if (co_amend_code_collective_decision != _code_vote_rule.end()) {
            _amend_vote_rule.emplace(ram_payer, [&](auto &row) {
                row.code_id = new_codes->code_id;
                row.right_proposer = co_amend_code_collective_decision->right_proposer;
                row.right_approver = co_amend_code_collective_decision->right_approver;
                row.right_voter = co_amend_code_collective_decision->right_voter;
                row.approval_type = co_amend_code_collective_decision->approval_type;
                row.pass_rule = co_amend_code_collective_decision->pass_rule;
                row.vote_duration = co_amend_code_collective_decision->vote_duration;
            });
        }
    }
}

ACTION community::setexectype(name community_account, uint64_t code_id, uint8_t exec_type, bool is_amend_code) {
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code does not exist.");

    if (!is_amend_code) {
        _codes.modify(code_itr, ram_payer, [&](auto &row) {
            row.code_exec_type = exec_type;
        });

        if (exec_type == ExecutionType::COLLECTIVE_DECISION) {
            code_sole_decision_table _code_execution_rule(_self, community_account.value);

            auto code_execution_rule_itr = _code_execution_rule.find(code_id);
            if (code_execution_rule_itr != _code_execution_rule.end()) {
                _code_execution_rule.erase(code_execution_rule_itr);
            }
        }
    } else {
        _codes.modify(code_itr, ram_payer, [&](auto &row) {
            row.amendment_exec_type = exec_type;
        });

        if (exec_type == ExecutionType::COLLECTIVE_DECISION) {
            amend_sole_decision_table _amend_execution_rule(_self, community_account.value);

            auto amend_execution_rule_itr = _amend_execution_rule.find(code_id);
            if (amend_execution_rule_itr != _amend_execution_rule.end()) {
                _amend_execution_rule.erase(amend_execution_rule_itr);
            }
        }
    }
}

ACTION community::setsoleexec(name community_account, uint64_t code_id, bool is_amend_code, RightHolder right_sole_executor)
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    verify_right_holder_input(community_account, right_sole_executor);

    if (is_amend_code) {
        amend_sole_decision_table _execution_rule(_self, community_account.value);
        // check(code_itr->amendment_exec_type != ExecutionType::COLLECTIVE_DECISION, "ERR::VERIFY_FAILED::Can not set execution rule for collective decision code");
        auto amend_execution_rule_itr = _execution_rule.find(code_id);

        if (amend_execution_rule_itr != _execution_rule.end()) {
            _execution_rule.modify(amend_execution_rule_itr, ram_payer, [&](auto &row) {
                    row.right_executor = right_sole_executor;
            });
        } else {
            _execution_rule.emplace(ram_payer, [&](auto &row) {
                    row.code_id = code_id;
                    row.right_executor = right_sole_executor;
            });
        }
    } else {
        code_sole_decision_table _execution_rule(_self, community_account.value);
        // check(code_itr->code_exec_type != ExecutionType::COLLECTIVE_DECISION, "ERR::VERIFY_FAILED::Can not set execution rule for collective decision code");

        auto code_execution_rule_itr = _execution_rule.find(code_id);

        if (code_execution_rule_itr != _execution_rule.end()) {
            _execution_rule.modify(code_execution_rule_itr, ram_payer, [&](auto &row) {
                    row.right_executor = right_sole_executor;
            });
        } else {
            _execution_rule.emplace(ram_payer, [&](auto &row) {
                    row.code_id = code_id;
                    row.right_executor = right_sole_executor;
            });
        }
    }
}

ACTION community::setproposer(name community_account, uint64_t code_id, bool is_amend_code, RightHolder right_proposer)
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    verify_right_holder_input(community_account, right_proposer);

    if (is_amend_code) {
        // check(code_itr->amendment_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set proposer for sole decision code");

        ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);

        auto amend_vote_rule_itr = _amend_vote_rule.find(code_id);

        if (amend_vote_rule_itr != _amend_vote_rule.end()) {
            _amend_vote_rule.modify(amend_vote_rule_itr, ram_payer, [&](auto &row) {
                    row.right_proposer = right_proposer;
            });
        } else {
            _amend_vote_rule.emplace(ram_payer, [&](auto &row) {
                    row.code_id = code_id;
                    row.right_proposer = right_proposer;
            });
        }
    } else {
        // check(code_itr->code_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set proposer for sole decision code");

        code_collective_decision_table _code_vote_rule(_self, community_account.value);

        auto code_vote_rule_itr = _code_vote_rule.find(code_id);

        if (code_vote_rule_itr != _code_vote_rule.end()) {
            _code_vote_rule.modify(code_vote_rule_itr, ram_payer, [&](auto &row) {
                    row.right_proposer = right_proposer;
            });
        } else {
            _code_vote_rule.emplace(ram_payer, [&](auto &row) {
                    row.code_id = code_id;
                    row.right_proposer = right_proposer;
            });
        }
    }
}

ACTION community::setapprotype(name community_account, uint64_t code_id, bool is_amend_code, uint8_t approval_type)
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    if (is_amend_code) {
        // check(code_itr->amendment_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set approval type for sole decision code");

        ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);

        auto amend_vote_rule_itr = _amend_vote_rule.find(code_id);

        if (amend_vote_rule_itr != _amend_vote_rule.end()) {
            _amend_vote_rule.modify(amend_vote_rule_itr, ram_payer, [&](auto &row) {
                    row.approval_type = approval_type;
            });
        } else {
            _amend_vote_rule.emplace(ram_payer, [&](auto &row) {
                    row.code_id = code_id;
                    row.approval_type = approval_type;
            });
        }
    } else {
        // check(code_itr->code_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set approval type for sole decision code");

        code_collective_decision_table _code_vote_rule(_self, community_account.value);

        auto code_vote_rule_itr = _code_vote_rule.find(code_id);

        if (code_vote_rule_itr != _code_vote_rule.end()) {
            _code_vote_rule.modify(code_vote_rule_itr, ram_payer, [&](auto &row) {
                    row.approval_type = approval_type;
            });
        } else {
            _code_vote_rule.emplace(ram_payer, [&](auto &row) {
                    row.code_id = code_id;
                    row.approval_type = approval_type;
            });
        }
    }
}

ACTION community::setapprover(name community_account, uint64_t code_id, bool is_amend_code, RightHolder right_approver) 
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    verify_right_holder_input(community_account, right_approver);

    if (is_amend_code) {
        // check(code_itr->amendment_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set approver for sole decision code");

        ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);

        auto amend_vote_rule_itr = _amend_vote_rule.find(code_id);

        check(amend_vote_rule_itr != _amend_vote_rule.end(), "ERR::APPROVAL_TYPE_NOT_SET::Please use action setapprotype set approval type for this code first");
        check(amend_vote_rule_itr->approval_type != ApprovalType::APPROVAL_CONSENSUS, "ERR::SET_APPROVER_FOR_CONSENSUS::Can not set approver for approval consensus code");

        _amend_vote_rule.modify(amend_vote_rule_itr, ram_payer, [&](auto &row) {
                row.right_approver = right_approver;
        });
    } else {
        // check(code_itr->code_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set approver for sole decision code");

        code_collective_decision_table _code_vote_rule(_self, community_account.value);

        auto code_vote_rule_itr = _code_vote_rule.find(code_id);

        check(code_vote_rule_itr != _code_vote_rule.end(), "ERR::APPROVAL_TYPE_NOT_SET::Please use action setapprotype set approval type for this code first");
        check(code_vote_rule_itr->approval_type != ApprovalType::APPROVAL_CONSENSUS, "ERR::SET_APPROVER_FOR_CONSENSUS::Can not set approver for approval consensus code");

        _code_vote_rule.modify(code_vote_rule_itr, ram_payer, [&](auto &row) {
                row.right_approver = right_approver;
        });
    }
}

ACTION community::setvoter(name community_account, uint64_t code_id, bool is_amend_code, RightHolder right_voter)
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    verify_right_holder_input(community_account, right_voter);

    if (is_amend_code) {
        // check(code_itr->amendment_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set voter for sole decision code");

        ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);

        auto amend_vote_rule_itr = _amend_vote_rule.find(code_id);

        check(amend_vote_rule_itr != _amend_vote_rule.end(), "ERR::APPROVAL_TYPE_NOT_SET::Please use action setapprotype set approval type for this code first");
        check(amend_vote_rule_itr->approval_type != ApprovalType::SOLE_APPROVAL, "ERR::SET_APPROVER_FOR_CONSENSUS::Can not set voter for sole approval code");

        _amend_vote_rule.modify(amend_vote_rule_itr, ram_payer, [&](auto &row) {
                row.right_voter = right_voter;
        });
    } else {
        // check(code_itr->code_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set voter for sole decision code");

        code_collective_decision_table _code_vote_rule(_self, community_account.value);

        auto code_vote_rule_itr = _code_vote_rule.find(code_id);

        check(code_vote_rule_itr != _code_vote_rule.end(), "ERR::APPROVAL_TYPE_NOT_SET::Please use action setapprotype set approval type for this code first");
        check(code_vote_rule_itr->approval_type != ApprovalType::SOLE_APPROVAL, "ERR::SET_APPROVER_FOR_CONSENSUS::Can not set voter for sole approval code");

        _code_vote_rule.modify(code_vote_rule_itr, ram_payer, [&](auto &row) {
                row.right_voter = right_voter;
        });
    }
}

ACTION community::setvoterule(name community_account, uint64_t code_id, bool is_amend_code, double pass_rule, uint64_t vote_duration) 
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    check(0 < pass_rule && pass_rule <= 100, "ERR::INVALID_PASS_RULE::Pass rule percent is invalid");

    if (is_amend_code) {
        // check(code_itr->amendment_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set collective rule for sole decision code");

        ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);

        auto amend_vote_rule_itr = _amend_vote_rule.find(code_id);
        check(amend_vote_rule_itr != _amend_vote_rule.end(), "ERR::VERIFY_FAILED::Please initialize approval type first");
        // check(amend_vote_rule_itr->approval_type != ApprovalType::SOLE_APPROVAL, "ERR::VERIFY_FAILED::Can not set voter for sole approval code");

        _amend_vote_rule.modify(amend_vote_rule_itr, ram_payer, [&](auto &row) {
                row.vote_duration = vote_duration;
                row.pass_rule = pass_rule;
        });
    } else {
        // check(code_itr->code_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set collective rule for sole decision code");

        code_collective_decision_table _code_vote_rule(_self, community_account.value);

        auto code_vote_rule_itr = _code_vote_rule.find(code_id);
        check(code_vote_rule_itr != _code_vote_rule.end(), "ERR::VERIFY_FAILED::Please initialize approval type first");
        // check(code_vote_rule_itr->approval_type != ApprovalType::SOLE_APPROVAL, "ERR::VERIFY_FAILED::Can not set voter for sole approval code");

        _code_vote_rule.modify(code_vote_rule_itr, ram_payer, [&](auto &row) {
                row.vote_duration = vote_duration;
                row.pass_rule = pass_rule;
        });
    }
}

ACTION community::voteforcode(name community_account, name proposal_name, name approver, bool vote_status)
{
    require_auth(approver);
    
    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    code_proposals_table _proposals(_self, community_account.value);
    auto proposal_itr = _proposals.find(proposal_name.value);

    check(proposal_itr != _proposals.end(), "ERR::PROPOSAL_NOT_EXISTED::The proposal is not exist");

    code_table _codes(_self, community_account.value);
    auto code_itr = _codes.find(proposal_itr->code_id);

    bool is_executed = false;

    for (auto action: proposal_itr->code_actions) {
        const bool amend_action = is_amend_action(action.code_action);
        if (!amend_action)
        {
            code_collective_decision_table _collective_exec(_self, community_account.value);
            auto collective_exec_itr = _collective_exec.find(proposal_itr->code_id);
            check(collective_exec_itr != _collective_exec.end(), "ERR::COLLECTIVE_RULE_NOT_EXIST::The collective rule for this code has not been set yet");

            check(collective_exec_itr->vote_duration + proposal_itr->propose_time.sec_since_epoch() > current_time_point().sec_since_epoch(), "ERR::VOTING_ENDED::Voting for this proposal has ben ended");

            if (collective_exec_itr->approval_type != ApprovalType::APPROVAL_CONSENSUS && verify_approver(community_account, approver, proposal_itr->code_id, amend_action)) {
                call_action(community_account, ram_payer, code_itr->contract_name, action.code_action, action.packed_params);
                is_executed = true;
            } else {
                check(verify_voter(community_account, approver, proposal_itr->code_id, amend_action), "ERR::VERIFY_FAILED::You do not have permission to vote for this action.");
            }
        } else {
            ammend_collective_decision_table _collective_exec(_self, community_account.value);
            auto collective_exec_itr = _collective_exec.find(proposal_itr->code_id);
            check(collective_exec_itr != _collective_exec.end(), "ERR::COLLECTIVE_RULE_NOT_EXIST::The collective rule for this code has not been set yet");

            check(collective_exec_itr->vote_duration + proposal_itr->propose_time.sec_since_epoch() > current_time_point().sec_since_epoch(), "ERR::VOTING_ENDED::Voting for this proposal has ben ended");

            if (collective_exec_itr->approval_type != ApprovalType::APPROVAL_CONSENSUS && verify_approver(community_account, approver, proposal_itr->code_id, amend_action)) {
                call_action(community_account, ram_payer, code_itr->contract_name, action.code_action, action.packed_params);
                is_executed = true;
            } else {
                check(verify_voter(community_account, approver, proposal_itr->code_id, amend_action), "ERR::VERIFY_FAILED::You do not have permission to vote for this action.");
            }
        }
    }

    double voted_percent;
    map<name, int> new_voters_detail = proposal_itr->voters_detail;
    uint64_t total_participation = proposal_itr->voters_detail.size();
    uint64_t total_voted = round((proposal_itr->voted_percent * total_participation) / 100);
    map<name, int>::const_iterator voter_detail_itr = proposal_itr->voters_detail.find(approver);
    if (voter_detail_itr == proposal_itr->voters_detail.end())
    {
        new_voters_detail.insert(pair<name, int>(approver, vote_status));
        voted_percent = double((vote_status == VOTE ? total_voted + 1 : total_voted) * 100) / (total_participation + 1);
    } else {
        // check(voter_detail_itr->second != vote_status, "ERR::VOTE_DUPLICATE::The voter has already voted for this code with the same status");
        voted_percent = double((vote_status == VOTE ? total_voted + 1 : total_voted - 1) * 100) / total_participation;
        new_voters_detail[approver] = vote_status;
    }

    _proposals.modify(proposal_itr, approver, [&](auto &row) {
        row.voted_percent = voted_percent;
        row.voters_detail = new_voters_detail;
    });

    if (is_executed) {
        _proposals.erase(proposal_itr);
    }
}

ACTION community::createpos(
    name community_account,
    name creator,
    string pos_name,
    uint64_t max_holder,
    uint8_t filled_through,
    uint64_t term,
    uint64_t next_term_start_at,
    uint64_t voting_period,
    RightHolder right_candidate,
    RightHolder right_voter
) {
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    check(pos_name.length() > 3, "ERR::CREATEPROP_SHORT_TITLE::Name length is too short.");
    check(max_holder > 0, "ERR::MAXHOLDER_INVALID::Max holder should be a positive value.");
    check(filled_through == FillingType::APPOINTMENT || filled_through == FillingType::ELECTION, "ERR::FILLEDTHROUGH_INVALID::Filled through should be 0 or 1.");

    code_table _codes(_self, community_account.value);
    position_table _positions(_self, community_account.value);

    code_sole_decision_table _code_execution_rule(_self, community_account.value);
    amend_sole_decision_table _amend_execution_rule(_self, community_account.value);

    auto getByCodeId = _codes.get_index<"by.code.name"_n>();
    auto positionCode = getByCodeId.find(PO_Create.value);

    auto newPositionId = _positions.available_primary_key();

    RightHolder _init_right_holder;

    auto com_itr = _communities.find(community_account.value);
    check(com_itr != _communities.end(), "ERR::CREATEPROP_NOT_EXIST::Community does not exist.");

    _init_right_holder.accounts.push_back(creator);

    vector<name> code_actions;
    code_actions.push_back("configpos"_n);

    // TO DO: create configpos with creator (exec_type = SOLE_DESICION) for code_execution_right amendment_execution_right
    auto configCode = _codes.emplace(ram_payer, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = PO_Config;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {CodeTypeEnum::POSITION, newPositionId};
    });

    _code_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = configCode->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = configCode->code_id;
            row.right_executor = _init_right_holder;
    });

    code_actions.clear();
    code_actions.push_back("appointpos"_n);

    auto appointCode = _codes.emplace(ram_payer, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = PO_Appoint;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {CodeTypeEnum::POSITION, newPositionId};
    });

    _code_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = appointCode->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = appointCode->code_id;
            row.right_executor = _init_right_holder;
    });

    code_actions.clear();
    code_actions.push_back("dismisspos"_n);

    auto dismissCode = _codes.emplace(ram_payer, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = PO_Dismiss;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {CodeTypeEnum::POSITION, newPositionId};
    });

    _code_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = dismissCode->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = dismissCode->code_id;
            row.right_executor = _init_right_holder;
    });

    map<name, uint64_t> refer_codes = {
        {PO_Config, configCode->code_id},
        {PO_Appoint, appointCode->code_id},
        {PO_Dismiss, dismissCode->code_id}};

    auto newPosition = _positions.emplace(ram_payer, [&](auto &row) {
        row.pos_id = _positions.available_primary_key();
        row.pos_name = pos_name;
        row.refer_codes = refer_codes;
    });

    vector<eosio::permission_level> action_permission = {{community_account, "active"_n}};
    if(ram_payer == ram_payer_system)  action_permission.push_back({ram_payer_system, "active"_n});

    action(
        action_permission,
        get_self(),
        "configpos"_n,
        std::make_tuple(community_account, newPosition->pos_id, pos_name, max_holder, filled_through, term, next_term_start_at, voting_period, right_candidate, right_voter))
        .send();
}

ACTION community::initadminpos(name community_account, name creator)
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    code_table _codes(_self, community_account.value);
    position_table _positions(_self, community_account.value);

    code_sole_decision_table _code_execution_rule(_self, community_account.value);
    amend_sole_decision_table _amend_execution_rule(_self, community_account.value);

    auto getByCodeId = _codes.get_index<"by.code.name"_n>();
    auto positionCode = getByCodeId.find(PO_Create.value);

    auto newPositionId = pos_admin_id;

    RightHolder _init_right_holder;

    vector<name> init_admin_holder;
    init_admin_holder.push_back(creator);

    auto com_itr = _communities.find(community_account.value);
    check(com_itr != _communities.end(), "ERR::CREATEPROP_NOT_EXIST::Community does not exist.");

    _init_right_holder.required_positions.push_back(newPositionId);

    vector<name> code_actions;
    code_actions.push_back("configpos"_n);

    // TO DO: create configpos with creator (exec_type = SOLE_DESICION) for code_execution_right amendment_execution_right
    auto configCode = _codes.emplace(ram_payer, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = PO_Config;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {CodeTypeEnum::POSITION, newPositionId};
    });

    _code_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = configCode->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = configCode->code_id;
            row.right_executor = _init_right_holder;
    });

    code_actions.clear();
    code_actions.push_back("appointpos"_n);

    auto appointCode = _codes.emplace(ram_payer, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = PO_Appoint;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {CodeTypeEnum::POSITION, newPositionId};
    });

    _code_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = appointCode->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = appointCode->code_id;
            row.right_executor = _init_right_holder;
    });

    code_actions.clear();
    code_actions.push_back("dismisspos"_n);

    auto dismissCode = _codes.emplace(ram_payer, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = PO_Dismiss;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {CodeTypeEnum::POSITION, newPositionId};
    });

    _code_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = dismissCode->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = dismissCode->code_id;
            row.right_executor = _init_right_holder;
    });

    map<name, uint64_t> refer_codes = {
        {PO_Config, configCode->code_id},
        {PO_Appoint, appointCode->code_id},
        {PO_Dismiss, dismissCode->code_id}};

    _positions.emplace(ram_payer, [&](auto &row) {
        row.pos_id = newPositionId;
        row.pos_name = default_admin_position_name;
        row.max_holder = default_admin_position_max_holder;
        row.holders = init_admin_holder;
        row.fulfillment_type = FillingType::APPOINTMENT;
        row.refer_codes = refer_codes;
    });
}

ACTION community::configpos(
        name community_account,
        uint64_t pos_id,
        string pos_name,
        uint64_t max_holder,
        uint8_t filled_through,
        uint64_t term,
        uint64_t next_term_start_at,
        uint64_t voting_period,
        RightHolder right_candidate,
        RightHolder right_voter
) {
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    check(pos_name.length() > 3, "ERR::CREATEPROP_SHORT_TITLE::Name length is too short.");
    check(max_holder > 0, "ERR::MAXHOLDER_INVALID::Max holder should be a positive value.");
    check(filled_through == FillingType::APPOINTMENT || filled_through == FillingType::ELECTION, "ERR::FILLEDTHROUGH_INVALID::Filled through should be 0 or 1.");

    position_table _positions(_self, community_account.value);
    auto pos_itr = _positions.find(pos_id);
    check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::Position id doesn't exist.");

    _positions.modify(pos_itr, ram_payer, [&](auto &row) {
        row.pos_name = pos_name;
        row.max_holder = max_holder;
        row.fulfillment_type = filled_through;
    });

    if (filled_through == FillingType::ELECTION)
    {
        uint64_t votting_start_date = next_term_start_at - seconds_per_day - voting_period;
        uint64_t votting_end_date = next_term_start_at - seconds_per_day;

        check(votting_start_date > current_time_point().sec_since_epoch(), "ERR::START_TIME_INVALID::Voting start date must greater than now.");
        verify_right_holder_input(community_account, right_candidate);
        verify_right_holder_input(community_account, right_voter);

        election_table _electionrule(_self, community_account.value);
        auto election_rule_itr = _electionrule.find(pos_id);

        if (election_rule_itr == _electionrule.end())
        {
            _electionrule.emplace(ram_payer, [&](auto &row) {
                row.pos_id = pos_id;
                row.term = term;
                row.next_term_start_at = time_point_sec(next_term_start_at);
                row.voting_period = voting_period;
                row.pos_candidates = right_candidate;
                row.pos_voters = right_voter;
            });
        }
        else
        {
            _electionrule.modify(election_rule_itr, ram_payer, [&](auto &row) {
                row.term = term;
                row.next_term_start_at = time_point_sec(next_term_start_at);
                row.voting_period = voting_period;
                row.pos_candidates = right_candidate;
                row.pos_voters = right_voter;
            });
        }

        posproposal_table _pos_proposal(_self, community_account.value);
        auto posproposal_itr = _pos_proposal.find(pos_id);

        if (posproposal_itr == _pos_proposal.end()) {
            _pos_proposal.emplace(ram_payer, [&](auto &row) {
                row.pos_id = pos_id;
                row.pos_proposal_id = get_pos_proposed_id();
                row.pos_proposal_status = ProposalStatus::IN_PROGRESS;
            });
        } else {
            _pos_proposal.modify(posproposal_itr, ram_payer, [&](auto &row) {
                row.pos_proposal_status = ProposalStatus::IN_PROGRESS;
            });
        }

        cancel_deferred(pos_id);
        eosio::transaction auto_execute;
        if(ram_payer == ram_payer_system){
            auto_execute.actions.emplace_back(vector<eosio::permission_level>{{community_account, "active"_n}, {ram_payer_system, "active"_n}}, _self, "approvepos"_n, std::make_tuple(community_account, pos_id));
        }else{
            auto_execute.actions.emplace_back(eosio::permission_level{community_account, "active"_n}, _self, "approvepos"_n, std::make_tuple(community_account, pos_id));
        }
        auto_execute.delay_sec = votting_end_date - current_time_point().sec_since_epoch();
        auto_execute.send(pos_id, _self, true);
    }
}

ACTION community::appointpos(name community_account, uint64_t pos_id, vector<name> holder_accounts, const string &appoint_reason)
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    position_table _positions(_self, community_account.value);
    auto pos_itr = _positions.find(pos_id);
    check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::Position id doesn't exist.");
    check(pos_itr->fulfillment_type == FillingType::APPOINTMENT, "ERR::FAILED_FILLING_TYPE::Only fulfillment equal appoinment need to appoint");
    check(pos_itr->max_holder >= pos_itr->holders.size() + holder_accounts.size(), "ERR::VERIFY_FAILED::The holder accounts exceed the maximum number.");

    holder_accounts.insert(holder_accounts.end(), pos_itr->holders.begin(), pos_itr->holders.end());
    _positions.modify(pos_itr, ram_payer, [&](auto &row) {
        row.holders = holder_accounts;
    });
}

ACTION community::nominatepos(name community_account, uint64_t pos_id, name owner)
{
    require_auth(owner);

    auto ram_payer = owner;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;
    
    position_table _positions(_self, community_account.value);
    auto pos_itr = _positions.find(pos_id);
    check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::Position id doesn't exist.");
    check(pos_itr->fulfillment_type == FillingType::ELECTION, "ERR::FAILED_FILLING_TYPE::Only election postion need to nominate");

    check(is_pos_candidate(community_account, pos_id, owner), "ERR::VERIFY_FAILED::accounts does not belong position candidates");

    election_table _electionrule(_self, community_account.value);
    auto election_itr = _electionrule.find(pos_id);
    check(election_itr != _electionrule.end(), "ERR::FILLING_RULE_NOT_EXIST::Position need filling rules.");
    check(election_itr->next_term_start_at.sec_since_epoch() - seconds_per_day > current_time_point().sec_since_epoch(), "ERR::PROPOSED_HAS_END::The end time of proposal must greater than current time.");

    // Todo: check right holder for candidates
    posproposal_table _pos_proposal(_self, community_account.value);

    auto posproposal_itr = _pos_proposal.find(pos_id);
    check(posproposal_itr != _pos_proposal.end() && posproposal_itr->pos_proposal_status == ProposalStatus::IN_PROGRESS, "ERR::PROPOSED_NOT_EXIST::Position proposed does not exist");

    poscandidate_table _poscandidate(_self, posproposal_itr->pos_proposal_id);
    auto candidate_itr = _poscandidate.find(owner.value);
    check(candidate_itr == _poscandidate.end(), "ERR::CANDIDATE_EXIST::The candidate already exist");

    _poscandidate.emplace(ram_payer, [&](auto &row) {
        row.cadidate = owner;
        row.voted_percent = 0;
    });
}

ACTION community::voteforpos(name community_account, uint64_t pos_id, name voter, name candidate, bool vote_status)
{
    require_auth(voter);

    auto ram_payer = voter;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    position_table _positions(_self, community_account.value);
    auto pos_itr = _positions.find(pos_id);
    check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::Position id doesn't exist.");
    check(pos_itr->fulfillment_type == FillingType::ELECTION, "ERR::FAILED_FILLING_TYPE::Only election postion need to nominate");

    check(is_pos_voter(community_account, pos_id, voter), "ERR::VERIFY_FAILED::accounts does not belong to position right voters");

    election_table _electionrule(_self, community_account.value);
    auto election_itr = _electionrule.find(pos_id);
    check(election_itr != _electionrule.end(), "ERR::FILLING_RULE_NOT_EXIST::Position need filling rules.");

    uint64_t votting_start_date = election_itr->next_term_start_at.sec_since_epoch() - seconds_per_day - election_itr->voting_period;
    uint64_t votting_end_date = election_itr->next_term_start_at.sec_since_epoch() - seconds_per_day;

    check(votting_end_date >= current_time_point().sec_since_epoch(), "ERR::START_END_TIME_INVALID::Voting for this position have been expired.");
    check(votting_start_date <= current_time_point().sec_since_epoch(), "ERR::START_END_TIME_INVALID::Voting for this position have not been started yet.");

    posproposal_table _posproposal(_self, community_account.value);
    auto posproposal_itr = _posproposal.find(pos_id);
    check(posproposal_itr != _posproposal.end(), "ERR::PROPOSAL_NOT_EXIST::Proposal does not exist.");

    poscandidate_table _pos_candidate(_self, posproposal_itr->pos_proposal_id);
    auto candidate_itr = _pos_candidate.find(candidate.value);
    check(candidate_itr != _pos_candidate.end(), "ERR::CANDIDATE_NOT_ESIXT::The candidate does not exist");

    auto new_voters_detail = candidate_itr->voters_detail;
    uint64_t total_participation = candidate_itr->voters_detail.size();
    uint64_t total_voted = round((candidate_itr->voted_percent * total_participation) / 100);
    auto voter_detail_itr = candidate_itr->voters_detail.find(voter);

    double voted_percent = 0;
    if (voter_detail_itr == candidate_itr->voters_detail.end())
    {
        new_voters_detail.insert(pair<name, int>(voter, vote_status));
        voted_percent = double((vote_status == VOTE ? total_voted + 1 : total_voted) * 100) / (total_participation + 1);
    }
    else
    {
        check(voter_detail_itr->second != vote_status, "ERR::VOTE_DUPLICATE::The voter has already voted for this code with the same status");
        voted_percent = double((vote_status == VOTE ? total_voted + 1 : total_voted - 1) * 100) / total_participation;
        new_voters_detail[voter] = vote_status;
    }

    _pos_candidate.modify(candidate_itr, ram_payer, [&](auto &row) {
        row.voted_percent = voted_percent;
        row.voters_detail = new_voters_detail;
    });
}

ACTION community::approvepos(name community_account, uint64_t pos_id)
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    position_table _positions(_self, community_account.value);
    auto pos_itr = _positions.find(pos_id);
    check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::Position id doesn't exist.");
    check(pos_itr->max_holder > 0 && pos_itr->max_holder < 100, "ERR::VERIFY_FAILED::The number position holdlder should be from 1 to 100");
    check(pos_itr->fulfillment_type == FillingType::ELECTION, "ERR::FAILED_FILLING_TYPE::Only election postion need to nominate");

    election_table _electionrule(_self, community_account.value);
    auto election_itr = _electionrule.find(pos_id);
    check(election_itr != _electionrule.end(), "ERR::FILLING_RULE_NOT_EXIST::Position need filling rules.");
    uint64_t votting_start_date = election_itr->next_term_start_at.sec_since_epoch() - seconds_per_day - election_itr->voting_period;
    uint64_t votting_end_date = election_itr->next_term_start_at.sec_since_epoch() - seconds_per_day;

    check(votting_end_date <= current_time_point().sec_since_epoch(), "ERR::END_TIME_INVALID::End voting date should be expired.");

    posproposal_table _pos_proposal(_self, community_account.value);
    auto posproposal_itr = _pos_proposal.find(pos_id);
    check(posproposal_itr != _pos_proposal.end() && posproposal_itr->pos_proposal_status == ProposalStatus::IN_PROGRESS, "ERR::NOMINATION_NOT_FOUND::Nomination does not exist.");

    poscandidate_table _pos_candidate(_self, posproposal_itr->pos_proposal_id);

    auto idx = _pos_candidate.get_index<"byvoted"_n>();

    std::vector<name> top_candidates;
    top_candidates.reserve(pos_itr->max_holder);

    for (auto it = idx.cbegin(); it != idx.cend() && top_candidates.size() <= pos_itr->max_holder; ++it)
    {
        if (it->voted_percent > 0)
            top_candidates.emplace_back(it->cadidate);
    }

    if (top_candidates.size() == 0)
    {
        return;
    }

    /// sort by candidate name
    std::sort(top_candidates.begin(), top_candidates.end());

    _pos_proposal.modify(posproposal_itr, ram_payer, [&](auto &row) {
        row.pos_proposal_status = ProposalStatus::PROPOSAL_APPROVE;
        row.approved_at = current_time_point();
    });

    _positions.modify(pos_itr, ram_payer, [&](auto &row) {
        row.holders = top_candidates;
    });
}

ACTION community::dismisspos(name community_account, uint64_t pos_id, name holder, const string &dismissal_reason)
{
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

    position_table _positions(_self, community_account.value);
    auto pos_itr = _positions.find(pos_id);
    check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::Position id doesn't exist.");
    check(pos_itr->holders.size() > 0, "ERR::VERIFY_FAILED::There are no holders for this position.");

    auto _holders = pos_itr->holders;
    _holders.erase(std::find(_holders.begin(), _holders.end(), holder));

    _positions.modify(pos_itr, ram_payer, [&](auto &row) {
        row.holders = _holders;
    });
}

ACTION community::createbadge(
        name community_account,
        uint64_t badge_id,
        uint8_t issue_type,
        name badge_propose_name,
        uint8_t issue_exec_type,
        RightHolder right_issue_sole_executor,
        RightHolder right_issue_proposer,
        uint8_t issue_approval_type,
        RightHolder right_issue_approver,
        RightHolder right_issue_voter,
        double issue_pass_rule,
        uint64_t issue_vote_duration
) {
    require_auth(community_account);

    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

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
    auto issue_badge_code = _codes.emplace(ram_payer, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = issue_badge_code_name;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = issue_exec_type;
        row.code_type = {CodeTypeEnum::BADGE, badge_id};
    });

    if (issue_exec_type != ExecutionType::COLLECTIVE_DECISION) {
        verify_right_holder_input(community_account, right_issue_sole_executor);
        _code_execution_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = issue_badge_code->code_id;
            row.right_executor = right_issue_sole_executor;
        });
    }

    if (issue_exec_type != ExecutionType::SOLE_DECISION) {
        verify_right_holder_input(community_account, right_issue_proposer);
        verify_right_holder_input(community_account, right_issue_approver);
        verify_right_holder_input(community_account, right_issue_voter);
        _code_vote_rule.emplace(ram_payer, [&](auto &row) {
            row.code_id = issue_badge_code->code_id;
            row.right_proposer = right_issue_proposer;
            row.right_approver = right_issue_approver;
            row.right_voter = right_issue_voter;
            row.approval_type = issue_approval_type;
            row.pass_rule = issue_pass_rule;
            row.vote_duration = issue_vote_duration;
        });
    }

    code_actions.clear();
    code_actions.push_back("configbadge"_n);

    // save new code to the table
    auto config_badge_code = _codes.emplace(ram_payer, [&](auto &row) {
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
            _amend_execution_rule.emplace(ram_payer, [&](auto &row) {
                row.code_id = config_badge_code->code_id;
                row.right_executor = ba_create_code_sole_decision->right_executor;
            });

            _code_execution_rule.emplace(ram_payer, [&](auto &row) {
                row.code_id = config_badge_code->code_id;
                row.right_executor = ba_create_code_sole_decision->right_executor;
            });
        }
    }

    if (ba_create_code->code_exec_type != ExecutionType::SOLE_DECISION) {
        auto ba_create_code_collective_decision = _code_vote_rule.find(ba_create_code->code_id);
        if (ba_create_code_collective_decision != _code_vote_rule.end()) {
            _amend_vote_rule.emplace(ram_payer, [&](auto &row) {
                row.code_id = config_badge_code->code_id;
                row.right_proposer = ba_create_code_collective_decision->right_proposer;
                row.right_approver = ba_create_code_collective_decision->right_approver;
                row.right_voter = ba_create_code_collective_decision->right_voter;
                row.approval_type = ba_create_code_collective_decision->approval_type;
                row.pass_rule = ba_create_code_collective_decision->pass_rule;
                row.vote_duration = ba_create_code_collective_decision->vote_duration;
            });

            _code_vote_rule.emplace(ram_payer, [&](auto &row) {
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
        RightHolder right_issue_sole_executor,
        RightHolder right_issue_proposer,
        uint8_t issue_approval_type,
        RightHolder right_issue_approver,
        RightHolder right_issue_voter,
        double issue_pass_rule,
        uint64_t issue_vote_duration
) {
    require_auth(community_account);
    
    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

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

    // find issue badge code of this configuring badge
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
        _codes.emplace(ram_payer, [&](auto &row) {
            row.code_id = _codes.available_primary_key();
            row.code_name = issue_badge_code_name;
            row.contract_name = get_self();
            row.code_actions = code_actions;
            row.amendment_exec_type = issue_exec_type;
            row.code_type = {CodeTypeEnum::BADGE, badge_id};
        });
    } else {
        getByCodeReferId.modify(issue_badge_code_itr, ram_payer, [&](auto &row) {
            row.code_name = issue_badge_code_name;
            row.code_exec_type = issue_exec_type;
            row.code_type = {CodeTypeEnum::BADGE, badge_id};
        });
    }

    if (issue_exec_type != ExecutionType::COLLECTIVE_DECISION) {
        verify_right_holder_input(community_account, right_issue_sole_executor);
        auto code_exec_type_itr = _code_execution_rule.find(issue_badge_code_itr->code_id);
        if (code_exec_type_itr == _code_execution_rule.end()) {
            _code_execution_rule.emplace(ram_payer, [&](auto &row) {
                row.code_id = issue_badge_code_itr->code_id;
                row.right_executor = right_issue_sole_executor;
            });
        } else {
            _code_execution_rule.modify(code_exec_type_itr, ram_payer, [&](auto &row) {
                row.right_executor = right_issue_sole_executor;
            });
        }
    }

    if (issue_exec_type != ExecutionType::SOLE_DECISION) {
        verify_right_holder_input(community_account, right_issue_proposer);
        verify_right_holder_input(community_account, right_issue_approver);
        verify_right_holder_input(community_account, right_issue_voter);
        auto code_vote_rule_itr = _code_vote_rule.find(issue_badge_code_itr->code_id);
        if (code_vote_rule_itr == _code_vote_rule.end()) {
            _code_vote_rule.emplace(ram_payer, [&](auto &row) {
                row.code_id = issue_badge_code_itr->code_id;
                row.right_proposer = right_issue_proposer;
                row.right_approver = right_issue_approver;
                row.right_voter = right_issue_voter;
                row.approval_type = issue_approval_type;
                row.pass_rule = issue_pass_rule;
                row.vote_duration = issue_vote_duration;
            });
        } else {
            _code_vote_rule.modify(code_vote_rule_itr, ram_payer, [&](auto &row) {
                row.right_proposer = right_issue_proposer;
                row.right_approver = right_issue_approver;
                row.right_voter = right_issue_voter;
                row.approval_type = issue_approval_type;
                row.pass_rule = issue_pass_rule;
                row.vote_duration = issue_vote_duration;
            });
        }
    }
}

ACTION community::issuebadge(name community_account, name badge_propose_name)
{
    require_auth(community_account);
    
    auto ram_payer = community_account;
	if(has_auth(ram_payer_system)) ram_payer = ram_payer_system;

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

bool community::verify_voter(name community_account, name voter, uint64_t code_id, bool is_amend_code)
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

    return verify_account_right_holder(community_account, right_voter, voter);
}

bool community::verify_approver(name community_account, name approver, uint64_t code_id, bool is_ammnend_code)
{
    RightHolder right_holder;
    if (is_ammnend_code) {
        ammend_collective_decision_table _collective_exec(_self, community_account.value);
        position_table _positions(_self, community_account.value);

        auto collective_exec_itr = _collective_exec.find(code_id);

        RightHolder right_holder = collective_exec_itr->right_approver;
    } else {
        code_collective_decision_table _collective_exec(_self, community_account.value);
        position_table _positions(_self, community_account.value);

        auto collective_exec_itr = _collective_exec.find(code_id);

        RightHolder right_holder = collective_exec_itr->right_approver;
    }

    return verify_account_right_holder(community_account, right_holder, approver);
}

bool community::is_pos_candidate(name community_account, uint64_t pos_id, name owner)
{
    election_table _electionrule(_self, community_account.value);
    auto election_itr = _electionrule.find(pos_id);
    check(election_itr != _electionrule.end(), "ERR::ELECTION_RULE_NOT_EXIST::Position need election rules.");
    auto _pos_candidate_holder = election_itr->pos_candidates.accounts;

    auto it = std::find(_pos_candidate_holder.begin(), _pos_candidate_holder.end(), owner);
    if (it != _pos_candidate_holder.end())
        return true;

    auto _position_right_holder_ids = election_itr->pos_candidates.required_positions;

    position_table _positions(_self, community_account.value);
    for (int i = 0; i < _position_right_holder_ids.size(); i++)
    {
        auto position_itr = _positions.find(_position_right_holder_ids[i]);
        auto _position_holders = position_itr->holders;
        if (std::find(_position_holders.begin(), _position_holders.end(), owner) != _position_holders.end())
        {
            return true;
        }
    }

    // verify right_holder's badge
    auto _required_badge_ids = election_itr->pos_candidates.required_badges;
    for (int i = 0; i < _required_badge_ids.size(); i++)
    {
        ccerts _badges(cryptobadge_contract, owner.value);
        auto owner_badge_itr = _badges.find(_required_badge_ids[i]);
        if (owner_badge_itr != _badges.end())
        {
            return true;
        }
    }

    return false;
}

bool community::verify_community_account_input(name community_account) {
    if (community_account.length() < 6) return false;

    if (community_account.suffix() != community_name_creator) {
        return false;
    }

    return true;
}

bool community::is_pos_voter(name community_account, uint64_t pos_id, name owner)
{
    election_table _electionrule(_self, community_account.value);
    auto election_itr = _electionrule.find(pos_id);
    check(election_itr != _electionrule.end(), "ERR::ELECTION_RULE_NOT_EXIST::Position need election rules.");
    auto _pos_voter_holder = election_itr->pos_voters.accounts;

    auto it = std::find(_pos_voter_holder.begin(), _pos_voter_holder.end(), owner);
    if (it != _pos_voter_holder.end())
        return true;

    auto _position_right_holder_ids = election_itr->pos_voters.required_positions;

    position_table _positions(_self, community_account.value);
    for (int i = 0; i < _position_right_holder_ids.size(); i++)
    {
        auto position_itr = _positions.find(_position_right_holder_ids[i]);
        auto _position_holders = position_itr->holders;
        if (std::find(_position_holders.begin(), _position_holders.end(), owner) != _position_holders.end())
        {
            return true;
        }
    }

    auto _required_badge_ids = election_itr->pos_voters.required_badges;
    for (int i = 0; i < _required_badge_ids.size(); i++)
    {
        ccerts _badges(cryptobadge_contract, owner.value);
        auto owner_badge_itr = _badges.find(_required_badge_ids[i]);
        if (owner_badge_itr != _badges.end())
        {
            return true;
        }
    }

    return false;
}

bool community::verify_account_right_holder(name community_account, RightHolder right_holder, name owner) {
    const bool is_set_right_holder = right_holder.is_anyone ||
                                     right_holder.is_any_community_member || 
                                     right_holder.accounts.size() != 0 ||
                                     right_holder.required_badges.size() != 0 ||
                                     right_holder.required_positions.size() != 0 ||
                                     right_holder.required_tokens.size() != 0 ||
                                     right_holder.required_exp;

    if (!is_set_right_holder) return false;

    if (right_holder.is_anyone) {
        return true;
    }

    if (right_holder.is_any_community_member) {
        members_table _members(_self, community_account.value);
        auto mem_itr = _members.find(owner.value);
        if(mem_itr != _members.end()) return true;
    }

    // TODO verify account right exp
    // if (required_exp) {

    // }

    // verify right_holder's account
    auto _account_right_holders = right_holder.accounts;
    auto it = std::find(_account_right_holders.begin(), _account_right_holders.end(), owner);
    if (it != _account_right_holders.end())
        return true;

    // verify right_holder's badge
    auto _required_badge_ids = right_holder.required_badges;
    for (int i = 0; i < _required_badge_ids.size(); i++)
    {
        ccerts _badges(cryptobadge_contract, owner.value);
        auto owner_badge_itr = _badges.find(_required_badge_ids[i]);
        if (owner_badge_itr != _badges.end())
        {
            return true;
        }
    }

    position_table _positions(_self, community_account.value);
    // verify right_holder's position
    auto _position_right_holder_ids = right_holder.required_positions;
    for (int i = 0; i < _position_right_holder_ids.size(); i++)
    {
        auto position_itr = _positions.find(_position_right_holder_ids[i]);
        auto _position_holders = position_itr->holders;
        if (std::find(_position_holders.begin(), _position_holders.end(), owner) != _position_holders.end())
        {
            return true;
        }
    }

    // verify right_holder's token
    auto _required_token_ids = right_holder.required_tokens;
    for (int i = 0; i < _required_token_ids.size(); i++)
    {
        accounts _acnts(tiger_token_contract, owner.value);
        auto owner_token_itr = _acnts.find(_required_token_ids[i].symbol.code().raw());

        if (owner_token_itr != _acnts.end() && owner_token_itr->balance.amount >= _required_token_ids[i].amount)
        {
            return true;
        }
    }

    return false;
}

// verify right holder input's logic is valid
void community::verify_right_holder_input(name community_account, RightHolder right_holder) {
    if ((right_holder.is_anyone) ) {
        check(right_holder.is_any_community_member == 0 || 
              right_holder.required_badges.size() == 0 ||
              right_holder.required_positions.size() == 0 ||
              right_holder.required_tokens.size() == 0 ||
              right_holder.accounts.size() == 0,
              "can not set another option if is anyone is true"
              );
    }

    if (right_holder.is_any_community_member) {
        check(right_holder.required_badges.size() == 0 ||
              right_holder.required_positions.size() == 0 ||
              right_holder.required_tokens.size() == 0 ||
              right_holder.accounts.size() == 0,
              "can not set another option if is any community member is true"
              );
    }

    if (right_holder.required_positions.size()) {
        position_table _positions(_self, community_account.value);
        for (auto pos_id : right_holder.required_positions)
        {
            auto pos_itr = _positions.find(pos_id);
            check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::One or more position ids in right holder doesn't exist.");
        }
    } 
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

void community::call_action(name community_account, name ram_payer, name contract_name, name action_name, vector<char> packed_params) {
    action sending_action;
    sending_action.authorization.push_back(permission_level{community_account, "active"_n});
    if(ram_payer == ram_payer_system) sending_action.authorization.push_back(permission_level{ram_payer_system, "active"_n});
    sending_action.account = contract_name;
    sending_action.name = action_name;
    sending_action.data = packed_params;
    sending_action.send();
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

EOSIO_ABI_CUSTOM(community, 
(setapprotype)
(setvoter)
(setapprover)
(setaccess)
(transfer)
(createacc)
(create)
(initcode)
(inputmembers)
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
