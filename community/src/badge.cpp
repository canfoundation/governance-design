#include "../include/badge.hpp"

ACTION badge::createbadge(
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

ACTION badge::configbadge(
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

ACTION badge::issuebadge(name community_account, name badge_propose_name)
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

bool badge::is_badge_holder(vector<uint64_t> badge_ids, name owner) {
    bool is_right_badge = (badge_ids.size() == 0);
    for (int i = 0; i < badge_ids.size(); i++)
    {
        ccerts _badges(cryptobadge_contract, owner.value);
        auto owner_badge_itr = _badges.find(badge_ids[i]);
        if (owner_badge_itr != _badges.end())
        {
            is_right_badge = true;
            break;
        }
    }
    return is_right_badge;
}