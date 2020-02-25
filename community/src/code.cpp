ACTION community::initcode(name community_account, name creator, bool create_default_code)
{
    require_auth(community_account);

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(CO_Amend.value);
    check(code_itr == _codes.end(), "ERR::VERIFY_FAILED::Code already initialize.");

    code_sole_decision_table _code_execution_rule(_self, community_account.value);
    amend_sole_decision_table _amend_execution_rule(_self, community_account.value);

    RightHolder _createcode_right_holder;
    _createcode_right_holder.accounts.push_back(creator);
    vector<name> _init_actions;
    _init_actions.push_back("createcode"_n);

    action(
        permission_level{community_account, "active"_n},
        get_self(),
        "initadminpos"_n,
        std::make_tuple(community_account, creator))
        .send();

    RightHolder _init_right_holder;

    _init_right_holder.required_positions.push_back(pos_admin_id);

    // initialize createcode code
    auto co_amend_code = _codes.emplace(community_account, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = CO_Amend;
        row.contract_name = get_self();
        row.code_actions = _init_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {NORMAL, 0};
    });

    _code_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = co_amend_code->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = co_amend_code->code_id;
            row.right_executor = _init_right_holder;
    });

    if (create_default_code)
    {
        auto getByCodeId = _codes.get_index<"by.code.name"_n>();
        if (getByCodeId.find(PO_Create.value) == getByCodeId.end())
        {
            _init_actions.clear();
            _init_actions.push_back("createpos"_n);

            // initialize createcode code
            auto po_create_code = _codes.emplace(community_account, [&](auto &row) {
                row.code_id = _codes.available_primary_key();
                row.code_name = PO_Create;
                row.contract_name = get_self();
                row.code_actions = _init_actions;
                row.code_exec_type = ExecutionType::SOLE_DECISION;
                row.amendment_exec_type = ExecutionType::SOLE_DECISION;
                row.code_type = {NORMAL, 0};
            });

            _code_execution_rule.emplace(community_account, [&](auto &row) {
                row.code_id = po_create_code->code_id;
                row.right_executor = _init_right_holder;
            });

            _amend_execution_rule.emplace(community_account, [&](auto &row) {
                row.code_id = po_create_code->code_id;
                row.right_executor = _init_right_holder;
            });
        }

        if (getByCodeId.find(CO_Access.value) == getByCodeId.end())
        {
            _init_actions.clear();
            _init_actions.push_back("accesscode"_n);

            // initialize createcode code
            auto co_access_code = _codes.emplace(community_account, [&](auto &row) {
                row.code_id = _codes.available_primary_key();
                row.code_name = CO_Access;
                row.contract_name = get_self();
                row.code_actions = _init_actions;
                row.code_exec_type = ExecutionType::SOLE_DECISION;
                row.amendment_exec_type = ExecutionType::SOLE_DECISION;
                row.code_type = {NORMAL, 0};
            });

            _code_execution_rule.emplace(community_account, [&](auto &row) {
                row.code_id = co_access_code->code_id;
                row.right_executor = _init_right_holder;
            });

            _amend_execution_rule.emplace(community_account, [&](auto &row) {
                row.code_id = co_access_code->code_id;
                row.right_executor = _init_right_holder;
            });
        }

        if (getByCodeId.find(BA_Create.value) == getByCodeId.end())
        {
            _init_actions.clear();
            _init_actions.push_back("createbadge"_n);

            // initialize createcode code
            auto ba_create_code = _codes.emplace(community_account, [&](auto &row) {
                row.code_id = _codes.available_primary_key();
                row.code_name = BA_Create;
                row.contract_name = get_self();
                row.code_actions = _init_actions;
                row.code_exec_type = ExecutionType::SOLE_DECISION;
                row.amendment_exec_type = ExecutionType::SOLE_DECISION;
                row.code_type = {NORMAL, 0};
            });

            _code_execution_rule.emplace(community_account, [&](auto &row) {
                row.code_id = ba_create_code->code_id;
                row.right_executor = _init_right_holder;
            });

            _amend_execution_rule.emplace(community_account, [&](auto &row) {
                row.code_id = ba_create_code->code_id;
                row.right_executor = _init_right_holder;
            });
        }
    }
}

ACTION community::execcode(name community_account, name exec_account, uint64_t code_id, vector<execution_code_data> code_actions)
{
    require_auth(exec_account);

    auto com_itr = _communitys.find(community_account.value);
    check(com_itr != _communitys.end(), "ERR::VERIFY_FAILED::Community doesn't exist.");

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
                std::make_tuple(community_account, code_id, uint8_t(ExecutionType::SOLE_DECISION), exec_account))
                .send();

            call_action(community_account, code_itr->contract_name, execution_data.code_action, execution_data.packed_params);
        }
        else
        {
            check(code_itr->amendment_exec_type != ExecutionType::COLLECTIVE_DECISION, "ERR::INVALID_EXEC_TYPE::Can not execute collective decision code, please use proposecode action");
            action(
                permission_level{get_self(), "active"_n},
                get_self(),
                "verifyamend"_n,
                std::make_tuple(community_account, code_id, uint8_t(ExecutionType::SOLE_DECISION), exec_account))
                .send();

            call_action(community_account, _self, execution_data.code_action, execution_data.packed_params);
        }
    }
}

ACTION community::proposecode(name community_account, name proposer, name proposal_name, uint64_t code_id, vector<execution_code_data> code_actions)
{
    require_auth(proposer);

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
            check(code_itr->code_exec_type == ExecutionType::COLLECTIVE_DECISION, "ERR::INVALID_EXEC_TYPE::Can not create proposal for sole decision code");
            // Verify Right Holder
             action(
                permission_level{get_self(), "active"_n},
                get_self(),
                "verifyholder"_n,
                std::make_tuple(community_account, code_id, uint8_t(ExecutionType::COLLECTIVE_DECISION), proposer))
                .send();

            code_collective_decision_table _collective_exec(_self, community_account.value);
            auto collective_exec_itr = _collective_exec.find(code_id);
            check(collective_exec_itr != _collective_exec.end(), "ERR::COLLECTIVE_NOT_EXISTED::The collective rule is not exist, please initialize it before create proposal");

            auto_execute.delay_sec = collective_exec_itr->vote_duration;
        }
        else
        {
            check(code_itr->amendment_exec_type == ExecutionType::COLLECTIVE_DECISION, "ERR::INVALID_EXEC_TYPE::Can not create proposal for sole decision code");
            // Verify Right Holder
            action(
                permission_level{get_self(), "active"_n},
                get_self(),
                "verifyamend"_n,
                std::make_tuple(community_account, code_id, uint8_t(ExecutionType::COLLECTIVE_DECISION), proposer))
                .send();

            ammend_collective_decision_table _collective_exec(_self, community_account.value);
            auto collective_exec_itr = _collective_exec.find(code_id);
            check(collective_exec_itr != _collective_exec.end(), "ERR::COLLECTIVE_NOT_EXISTED::The collective rule is not exist, please initialize it before create proposal");

            auto_execute.delay_sec = collective_exec_itr->vote_duration + 1;
        }
    }

    _proposals.emplace(proposer, [&](auto &row) {
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
            check(proposal_itr->voted_percent > collective_exec_itr->pass_rule, "ERR::CODE_NOT_ACCEPTED::This code has not been aceepted by voter");

            // check that proposal has been executed or not
            check(proposal_itr->proposal_status != EXECUTED, "ERR::EXECUTED_PROPOSAL::This proposal has been executed");

            // check that proposal has been expired
            check(proposal_itr->proposal_status != EXPIRED, "ERR::EXPIRED_PROPOSAL::This proposal has been expired");

            call_action(community_account, _self, action.code_action, action.packed_params);
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
            check(proposal_itr->voted_percent > collective_exec_itr->pass_rule, "ERR::CODE_NOT_ACCEPTED::This code has not been aceepted by voter");

            // check that proposal has been executed or not
            check(proposal_itr->proposal_status != EXECUTED, "ERR::EXECUTED_PROPOSAL::This proposal has been executed");

            // check that proposal has been expired
            check(proposal_itr->proposal_status != EXPIRED, "ERR::EXPIRED_PROPOSAL::This proposal has been expired");

            call_action(community_account, code_itr->contract_name, action.code_action, action.packed_params);
        }
    }

    _proposals.erase(proposal_itr);
}

ACTION community::createcode(name community_account, name code_name, name contract_name, vector<name> code_actions)
{
    require_auth(community_account);

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
    auto new_codes = _codes.emplace(community_account, [&](auto &row) {
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
            _amend_execution_rule.emplace(community_account, [&](auto &row) {
                row.code_id = new_codes->code_id;
                row.right_executor = co_amend_code_sole_decision->right_executor;
            });
        }
    }

    if (co_amend_code->code_exec_type != ExecutionType::SOLE_DECISION) {
        auto co_amend_code_collective_decision = _code_vote_rule.find(co_amend_code->code_id);
        if (co_amend_code_collective_decision != _code_vote_rule.end()) {
            _amend_vote_rule.emplace(community_account, [&](auto &row) {
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

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code does not exist.");

    if (!is_amend_code) {
        _codes.modify(code_itr, community_account, [&](auto &row) {
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
        _codes.modify(code_itr, community_account, [&](auto &row) {
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

ACTION community::setsoleexec(name community_account, uint64_t code_id, bool is_amend_code, vector<name> right_accounts, vector<uint64_t> right_pos_ids)
{
    require_auth(community_account);

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    RightHolder _right_holder;
    _right_holder.accounts = right_accounts;

    position_table _positions(_self, community_account.value);
    for (auto pos_id : right_pos_ids)
    {
        auto pos_itr = _positions.find(pos_id);
        check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::One or more position ids doesn't exist.");
    }
    _right_holder.required_positions = right_pos_ids;

    if (is_amend_code) {
        amend_sole_decision_table _execution_rule(_self, community_account.value);
        // check(code_itr->amendment_exec_type != ExecutionType::COLLECTIVE_DECISION, "ERR::VERIFY_FAILED::Can not set execution rule for collective decision code");
        auto amend_execution_rule_itr = _execution_rule.find(code_id);

        if (amend_execution_rule_itr != _execution_rule.end()) {
            _execution_rule.modify(amend_execution_rule_itr, community_account, [&](auto &row) {
                    row.right_executor = _right_holder;
            });
        } else {
            _execution_rule.emplace(community_account, [&](auto &row) {
                    row.code_id = code_id;
                    row.right_executor = _right_holder;
            });
        }
    } else {
        code_sole_decision_table _execution_rule(_self, community_account.value);
        // check(code_itr->code_exec_type != ExecutionType::COLLECTIVE_DECISION, "ERR::VERIFY_FAILED::Can not set execution rule for collective decision code");

        auto code_execution_rule_itr = _execution_rule.find(code_id);

        if (code_execution_rule_itr != _execution_rule.end()) {
            _execution_rule.modify(code_execution_rule_itr, community_account, [&](auto &row) {
                    row.right_executor = _right_holder;
            });
        } else {
            _execution_rule.emplace(community_account, [&](auto &row) {
                    row.code_id = code_id;
                    row.right_executor = _right_holder;
            });
        }
    }
}

ACTION community::setproposer(name community_account, uint64_t code_id, bool is_amend_code, vector<name> right_accounts, vector<uint64_t> right_pos_ids) {
    require_auth(community_account);

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    RightHolder _right_holder;
    _right_holder.accounts = right_accounts;

    position_table _positions(_self, community_account.value);
    for (auto pos_id : right_pos_ids)
    {
        auto pos_itr = _positions.find(pos_id);
        check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::One or more position ids doesn't exist.");
    }
    _right_holder.required_positions = right_pos_ids;

    if (is_amend_code) {
        // check(code_itr->amendment_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set proposer for sole decision code");

        ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);

        auto amend_vote_rule_itr = _amend_vote_rule.find(code_id);

        if (amend_vote_rule_itr != _amend_vote_rule.end()) {
            _amend_vote_rule.modify(amend_vote_rule_itr, community_account, [&](auto &row) {
                    row.right_proposer = _right_holder;
            });
        } else {
            _amend_vote_rule.emplace(community_account, [&](auto &row) {
                    row.code_id = code_id;
                    row.right_proposer = _right_holder;
            });
        }
    } else {
        // check(code_itr->code_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set proposer for sole decision code");

        code_collective_decision_table _code_vote_rule(_self, community_account.value);

        auto code_vote_rule_itr = _code_vote_rule.find(code_id);

        if (code_vote_rule_itr != _code_vote_rule.end()) {
            _code_vote_rule.modify(code_vote_rule_itr, community_account, [&](auto &row) {
                    row.right_proposer = _right_holder;
            });
        } else {
            _code_vote_rule.emplace(community_account, [&](auto &row) {
                    row.code_id = code_id;
                    row.right_proposer = _right_holder;
            });
        }
    }
}

ACTION community::setapprotype(name community_account, uint64_t code_id, bool is_amend_code, uint8_t approval_type) {
    require_auth(community_account);

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    if (is_amend_code) {
        // check(code_itr->amendment_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set approval type for sole decision code");

        ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);

        auto amend_vote_rule_itr = _amend_vote_rule.find(code_id);

        if (amend_vote_rule_itr != _amend_vote_rule.end()) {
            _amend_vote_rule.modify(amend_vote_rule_itr, community_account, [&](auto &row) {
                    row.approval_type = approval_type;
            });
        } else {
            _amend_vote_rule.emplace(community_account, [&](auto &row) {
                    row.code_id = code_id;
                    row.approval_type = approval_type;
            });
        }
    } else {
        // check(code_itr->code_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set approval type for sole decision code");

        code_collective_decision_table _code_vote_rule(_self, community_account.value);

        auto code_vote_rule_itr = _code_vote_rule.find(code_id);

        if (code_vote_rule_itr != _code_vote_rule.end()) {
            _code_vote_rule.modify(code_vote_rule_itr, community_account, [&](auto &row) {
                    row.approval_type = approval_type;
            });
        } else {
            _code_vote_rule.emplace(community_account, [&](auto &row) {
                    row.code_id = code_id;
                    row.approval_type = approval_type;
            });
        }
    }
}

ACTION community::setapprover(name community_account, uint64_t code_id, bool is_amend_code, vector<name> right_accounts, vector<uint64_t> right_pos_ids) {
    require_auth(community_account);

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    RightHolder _right_holder;
    _right_holder.accounts = right_accounts;

    position_table _positions(_self, community_account.value);
    for (auto pos_id : right_pos_ids)
    {
        auto pos_itr = _positions.find(pos_id);
        check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::One or more position ids doesn't exist.");
    }
    _right_holder.required_positions = right_pos_ids;


    if (is_amend_code) {
        // check(code_itr->amendment_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set approver for sole decision code");

        ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);

        auto amend_vote_rule_itr = _amend_vote_rule.find(code_id);

        check(amend_vote_rule_itr != _amend_vote_rule.end(), "ERR::APPROVAL_TYPE_NOT_SET::Please use action setapprotype set approval type for this code first");
        check(amend_vote_rule_itr->approval_type != ApprovalType::APPROVAL_CONSENSUS, "ERR::SET_APPROVER_FOR_CONSENSUS::Can not set approver for approval consensus code");

        _amend_vote_rule.modify(amend_vote_rule_itr, community_account, [&](auto &row) {
                row.right_approver = _right_holder;
        });
    } else {
        // check(code_itr->code_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set approver for sole decision code");

        code_collective_decision_table _code_vote_rule(_self, community_account.value);

        auto code_vote_rule_itr = _code_vote_rule.find(code_id);

        check(code_vote_rule_itr != _code_vote_rule.end(), "ERR::APPROVAL_TYPE_NOT_SET::Please use action setapprotype set approval type for this code first");
        check(code_vote_rule_itr->approval_type != ApprovalType::APPROVAL_CONSENSUS, "ERR::SET_APPROVER_FOR_CONSENSUS::Can not set approver for approval consensus code");

        _code_vote_rule.modify(code_vote_rule_itr, community_account, [&](auto &row) {
                row.right_approver = _right_holder;
        });
    }
}

ACTION community::setvoter(name community_account, uint64_t code_id, bool is_amend_code, vector<name> right_accounts, vector<uint64_t> right_pos_ids) {
    require_auth(community_account);

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    RightHolder _right_holder;
    _right_holder.accounts = right_accounts;

    position_table _positions(_self, community_account.value);
    for (auto pos_id : right_pos_ids)
    {
        auto pos_itr = _positions.find(pos_id);
        check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::One or more position ids doesn't exist.");
    }
    _right_holder.required_positions = right_pos_ids;


    if (is_amend_code) {
        // check(code_itr->amendment_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set voter for sole decision code");

        ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);

        auto amend_vote_rule_itr = _amend_vote_rule.find(code_id);

        check(amend_vote_rule_itr != _amend_vote_rule.end(), "ERR::APPROVAL_TYPE_NOT_SET::Please use action setapprotype set approval type for this code first");
        check(amend_vote_rule_itr->approval_type != ApprovalType::SOLE_APPROVAL, "ERR::SET_APPROVER_FOR_CONSENSUS::Can not set voter for sole approval code");

        _amend_vote_rule.modify(amend_vote_rule_itr, community_account, [&](auto &row) {
                row.right_voter = _right_holder;
        });
    } else {
        // check(code_itr->code_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set voter for sole decision code");

        code_collective_decision_table _code_vote_rule(_self, community_account.value);

        auto code_vote_rule_itr = _code_vote_rule.find(code_id);

        check(code_vote_rule_itr != _code_vote_rule.end(), "ERR::APPROVAL_TYPE_NOT_SET::Please use action setapprotype set approval type for this code first");
        check(code_vote_rule_itr->approval_type != ApprovalType::SOLE_APPROVAL, "ERR::SET_APPROVER_FOR_CONSENSUS::Can not set voter for sole approval code");

        _code_vote_rule.modify(code_vote_rule_itr, community_account, [&](auto &row) {
                row.right_voter = _right_holder;
        });
    }
}

ACTION community::setvoterule(name community_account, uint64_t code_id, bool is_amend_code, double pass_rule, uint64_t vote_duration) {
    require_auth(community_account);

    code_table _codes(_self, community_account.value);

    auto code_itr = _codes.find(code_id);
    check(code_itr != _codes.end(), "ERR::VERIFY_FAILED::Code doesn't exist.");

    if (is_amend_code) {
        // check(code_itr->amendment_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set collective rule for sole decision code");

        ammend_collective_decision_table _amend_vote_rule(_self, community_account.value);

        auto amend_vote_rule_itr = _amend_vote_rule.find(code_id);
        check(amend_vote_rule_itr != _amend_vote_rule.end(), "ERR::VERIFY_FAILED::Please initialize approval type first");
        check(amend_vote_rule_itr->approval_type != ApprovalType::SOLE_APPROVAL, "ERR::VERIFY_FAILED::Can not set voter for sole approval code");

        _amend_vote_rule.modify(amend_vote_rule_itr, community_account, [&](auto &row) {
                row.vote_duration = vote_duration;
                row.pass_rule = pass_rule;
        });
    } else {
        // check(code_itr->code_exec_type != ExecutionType::SOLE_DECISION, "ERR::VERIFY_FAILED::Can not set collective rule for sole decision code");

        code_collective_decision_table _code_vote_rule(_self, community_account.value);

        auto code_vote_rule_itr = _code_vote_rule.find(code_id);
        check(code_vote_rule_itr != _code_vote_rule.end(), "ERR::VERIFY_FAILED::Please initialize approval type first");
        check(code_vote_rule_itr->approval_type != ApprovalType::SOLE_APPROVAL, "ERR::VERIFY_FAILED::Can not set voter for sole approval code");

        _code_vote_rule.modify(code_vote_rule_itr, community_account, [&](auto &row) {
                row.vote_duration = vote_duration;
                row.pass_rule = pass_rule;
        });
    }
}

ACTION community::voteforcode(name community_account, name proposal_name, name approver, bool vote_status)
{
    require_auth(approver);

    code_proposals_table _proposals(_self, community_account.value);
    auto proposal_itr = _proposals.find(proposal_name.value);

    check(proposal_itr != _proposals.end(), "ERR::PROPOSAL_NOT_EXISTED::The proposal is not exist");

    code_table _codes(_self, community_account.value);
    auto code_itr = _codes.find(proposal_itr->code_id);

    bool is_executed = false;

    for (auto action: proposal_itr->code_actions) {
        if (!is_amend_action(action.code_action))
        {
            code_collective_decision_table _collective_exec(_self, community_account.value);
            auto collective_exec_itr = _collective_exec.find(proposal_itr->code_id);
            check(collective_exec_itr != _collective_exec.end(), "ERR::COLLECTIVE_RULE_NOT_EXIST::The collective rule for this code has not been set yet");

            if (collective_exec_itr->approval_type != ApprovalType::APPROVAL_CONSENSUS && verifyapprov(community_account, approver, proposal_itr->code_id)) {
                call_action(community_account, code_itr->contract_name, action.code_action, action.packed_params);
                is_executed = true;
            } else {
                check(verifyvoter(community_account, approver, proposal_itr->code_id, false), "ERR::VERIFY_FAILED::You do not have permission to vote for this action.");
                check(collective_exec_itr->vote_duration + proposal_itr->propose_time.sec_since_epoch() > current_time_point().sec_since_epoch(), "ERR::VOTING_ENDED::Voting for this proposal has ben ended");
            }
        } else {
            ammend_collective_decision_table _collective_exec(_self, community_account.value);
            auto collective_exec_itr = _collective_exec.find(proposal_itr->code_id);
            check(collective_exec_itr != _collective_exec.end(), "ERR::COLLECTIVE_RULE_NOT_EXIST::The collective rule for this code has not been set yet");

            if (collective_exec_itr->approval_type != ApprovalType::APPROVAL_CONSENSUS && verifyapprov(community_account, approver, proposal_itr->code_id)) {
                call_action(community_account, code_itr->contract_name, action.code_action, action.packed_params);
                is_executed = true;
            } else {
                check(verifyvoter(community_account, approver, proposal_itr->code_id, true), "ERR::VERIFY_FAILED::You do not have permission to vote for this action.");
                check(collective_exec_itr->vote_duration + proposal_itr->propose_time.sec_since_epoch() > current_time_point().sec_since_epoch(), "ERR::VOTING_ENDED::Voting for this proposal has ben ended");
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