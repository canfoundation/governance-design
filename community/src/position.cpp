ACTION community::createpos(
    name community_account,
    name creator,
    string pos_name,
    uint64_t max_holder,
    uint8_t filled_through,
    uint64_t term,
    uint64_t next_term_start_at,
    uint64_t voting_period,
    double pass_rule,
    vector<name> pos_candidate_accounts,
    vector<name> pos_voter_accounts,
    vector<uint64_t> pos_candidate_positions,
    vector<uint64_t> pos_voter_positions
    )
{
    require_auth(community_account);

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

    auto com_itr = _communitys.find(community_account.value);
    check(com_itr != _communitys.end(), "ERR::CREATEPROP_NOT_EXIST::Community does not exist.");

    _init_right_holder.accounts.push_back(creator);

    vector<name> code_actions;
    code_actions.push_back("configpos"_n);

    // TO DO: create configpos with creator (exec_type = SOLE_DESICION) for code_execution_right amendment_execution_right
    auto configCode = _codes.emplace(community_account, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = PO_Config;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {CodeTypeEnum::POSITION, newPositionId};
    });

    _code_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = configCode->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = configCode->code_id;
            row.right_executor = _init_right_holder;
    });

    code_actions.clear();
    code_actions.push_back("appointpos"_n);

    auto appointCode = _codes.emplace(community_account, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = PO_Appoint;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {CodeTypeEnum::POSITION, newPositionId};
    });

    _code_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = appointCode->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = appointCode->code_id;
            row.right_executor = _init_right_holder;
    });

    code_actions.clear();
    code_actions.push_back("dismisspos"_n);

    auto dismissCode = _codes.emplace(community_account, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = PO_Dismiss;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {CodeTypeEnum::POSITION, newPositionId};
    });

    _code_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = dismissCode->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = dismissCode->code_id;
            row.right_executor = _init_right_holder;
    });

    map<name, uint64_t> refer_codes = {
        {PO_Config, configCode->code_id},
        {PO_Appoint, appointCode->code_id},
        {PO_Dismiss, dismissCode->code_id}};

    auto newPosition = _positions.emplace(community_account, [&](auto &row) {
        row.pos_id = _positions.available_primary_key();
        row.pos_name = pos_name;
        row.refer_codes = refer_codes;
    });

    action(
        permission_level{community_account, "active"_n},
        get_self(),
        "configpos"_n,
        std::make_tuple(community_account, newPosition->pos_id, pos_name, max_holder, filled_through, term, next_term_start_at, voting_period, pass_rule, pos_candidate_accounts, pos_voter_accounts, pos_candidate_positions, pos_voter_positions))
        .send();
}

ACTION community::initadminpos(name community_account, name creator)
{
    require_auth(community_account);

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

    auto com_itr = _communitys.find(community_account.value);
    check(com_itr != _communitys.end(), "ERR::CREATEPROP_NOT_EXIST::Community does not exist.");

    _init_right_holder.required_positions.push_back(newPositionId);

    vector<name> code_actions;
    code_actions.push_back("configpos"_n);

    // TO DO: create configpos with creator (exec_type = SOLE_DESICION) for code_execution_right amendment_execution_right
    auto configCode = _codes.emplace(community_account, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = PO_Config;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {CodeTypeEnum::POSITION, newPositionId};
    });

    _code_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = configCode->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = configCode->code_id;
            row.right_executor = _init_right_holder;
    });

    code_actions.clear();
    code_actions.push_back("appointpos"_n);

    auto appointCode = _codes.emplace(community_account, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = PO_Appoint;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {CodeTypeEnum::POSITION, newPositionId};
    });

    _code_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = appointCode->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = appointCode->code_id;
            row.right_executor = _init_right_holder;
    });

    code_actions.clear();
    code_actions.push_back("dismisspos"_n);

    auto dismissCode = _codes.emplace(community_account, [&](auto &row) {
        row.code_id = _codes.available_primary_key();
        row.code_name = PO_Dismiss;
        row.contract_name = get_self();
        row.code_actions = code_actions;
        row.code_exec_type = ExecutionType::SOLE_DECISION;
        row.amendment_exec_type = ExecutionType::SOLE_DECISION;
        row.code_type = {CodeTypeEnum::POSITION, newPositionId};
    });

    _code_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = dismissCode->code_id;
            row.right_executor = _init_right_holder;
    });

    _amend_execution_rule.emplace(community_account, [&](auto &row) {
            row.code_id = dismissCode->code_id;
            row.right_executor = _init_right_holder;
    });

    map<name, uint64_t> refer_codes = {
        {PO_Config, configCode->code_id},
        {PO_Appoint, appointCode->code_id},
        {PO_Dismiss, dismissCode->code_id}};

    _positions.emplace(community_account, [&](auto &row) {
        row.pos_id = newPositionId;
        row.pos_name = default_admin_position_name;
        row.max_holder = default_admin_position_max_holder;
        row.holders = init_admin_holder;
        row.fulfillment_type = FillingType::APPOINTMENT;
        row.refer_codes = refer_codes;
    });
}

ACTION community::configpos(name community_account, uint64_t pos_id, string pos_name, uint64_t max_holder, uint8_t filled_through, uint64_t term, uint64_t next_term_start_at, uint64_t voting_period, double pass_rule, vector<name> pos_candidate_accounts, vector<name> pos_voter_accounts, vector<uint64_t> pos_candidate_positions, vector<uint64_t> pos_voter_positions)
{
    require_auth(community_account);

    check(pos_name.length() > 3, "ERR::CREATEPROP_SHORT_TITLE::Name length is too short.");
    check(max_holder > 0, "ERR::MAXHOLDER_INVALID::Max holder should be a positive value.");
    check(filled_through == FillingType::APPOINTMENT || filled_through == FillingType::ELECTION, "ERR::FILLEDTHROUGH_INVALID::Filled through should be 0 or 1.");

    position_table _positions(_self, community_account.value);
    auto pos_itr = _positions.find(pos_id);
    check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::Position id doesn't exist.");

    _positions.modify(pos_itr, community_account, [&](auto &row) {
        row.pos_name = pos_name;
        row.max_holder = max_holder;
        row.fulfillment_type = filled_through;
    });

    if (filled_through == FillingType::ELECTION)
    {
        uint64_t votting_start_date = next_term_start_at - seconds_per_day - voting_period;
        uint64_t votting_end_date = next_term_start_at - seconds_per_day;

        check(votting_start_date > current_time_point().sec_since_epoch(), "ERR::START_TIME_INVALID::Voting start date must greater than now.");
        RightHolder _pos_candidates;
        _pos_candidates.accounts = pos_candidate_accounts;
        _pos_candidates.required_positions = pos_candidate_positions;

        RightHolder _pos_voters;
        _pos_voters.accounts = pos_voter_accounts;
        _pos_voters.required_positions = pos_voter_positions;

        election_table _electionrule(_self, community_account.value);
        auto election_rule_itr = _electionrule.find(pos_id);

        if (election_rule_itr == _electionrule.end())
        {
            _electionrule.emplace(community_account, [&](auto &row) {
                row.pos_id = pos_id;
                row.term = term;
                row.next_term_start_at = time_point_sec(next_term_start_at);
                row.voting_period = voting_period;
                row.pass_rule = pass_rule;
                row.pos_candidates = _pos_candidates;
                row.pos_voters = _pos_voters;
            });
        }
        else
        {
            _electionrule.modify(election_rule_itr, community_account, [&](auto &row) {
                row.term = term;
                row.next_term_start_at = time_point_sec(next_term_start_at);
                row.voting_period = voting_period;
                row.pass_rule = pass_rule;
                row.pos_candidates = _pos_candidates;
                row.pos_voters = _pos_voters;
            });
        }

        posproposal_table _pos_proposal(_self, community_account.value);
        auto posproposal_itr = _pos_proposal.find(pos_id);

        if (posproposal_itr == _pos_proposal.end()) {
            _pos_proposal.emplace(community_account, [&](auto &row) {
                row.pos_id = pos_id;
                row.pos_proposal_id = get_pos_proposed_id();
                row.pos_proposal_status = ProposalStatus::IN_PROGRESS;
            });
        } else {
            _pos_proposal.modify(posproposal_itr, community_account, [&](auto &row) {
                row.pos_proposal_status = ProposalStatus::IN_PROGRESS;
            });
        }

        cancel_deferred(pos_id);
        eosio::transaction auto_execute;
        auto_execute.actions.emplace_back(eosio::permission_level{community_account, "active"_n}, _self, "approvepos"_n, std::make_tuple(community_account, pos_id));
        auto_execute.delay_sec = votting_end_date - current_time_point().sec_since_epoch();
        auto_execute.send(pos_id, _self, true);
    }
}

ACTION community::appointpos(name community_account, uint64_t pos_id, vector<name> holder_accounts, const string &appoint_reason)
{
    require_auth(community_account);

    position_table _positions(_self, community_account.value);
    auto pos_itr = _positions.find(pos_id);
    check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::Position id doesn't exist.");
    check(pos_itr->fulfillment_type == FillingType::APPOINTMENT, "ERR::FAILED_FILLING_TYPE::Only fulfillment equal appoinment need to appoint");
    check(pos_itr->max_holder >= holder_accounts.size(), "ERR::VERIFY_FAILED::The holder accounts exceed the maximum number.");

    _positions.modify(pos_itr, community_account, [&](auto &row) {
        row.holders = holder_accounts;
    });
}

ACTION community::nominatepos(name community_account, uint64_t pos_id, name owner)
{
    require_auth(owner);

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

    _poscandidate.emplace(owner, [&](auto &row) {
        row.cadidate = owner;
        row.voted_percent = 0;
    });
}

ACTION community::voteforpos(name community_account, uint64_t pos_id, name voter, name candidate, bool vote_status)
{
    require_auth(voter);

    position_table _positions(_self, community_account.value);
    auto pos_itr = _positions.find(pos_id);
    check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::Position id doesn't exist.");
    check(pos_itr->fulfillment_type == FillingType::ELECTION, "ERR::FAILED_FILLING_TYPE::Only election postion need to nominate");

    check(is_pos_candidate(community_account, pos_id, candidate), "ERR::VERIFY_FAILED::accounts does not belong position right holder");
    check(is_pos_voter(community_account, pos_id, voter), "ERR::VERIFY_FAILED::accounts does not belong filling rule right holder");

    election_table _electionrule(_self, community_account.value);
    auto election_itr = _electionrule.find(pos_id);
    check(election_itr != _electionrule.end(), "ERR::FILLING_RULE_NOT_EXIST::Position need filling rules.");

    uint64_t votting_start_date = election_itr->next_term_start_at.sec_since_epoch() - seconds_per_day - election_itr->voting_period;
    uint64_t votting_end_date = election_itr->next_term_start_at.sec_since_epoch() - seconds_per_day;

    check(votting_start_date <= current_time_point().sec_since_epoch() && votting_end_date >= current_time_point().sec_since_epoch(), "ERR::START_END_TIME_INVALID::Voting for this position have been expired.");

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

    _pos_candidate.modify(candidate_itr, voter, [&](auto &row) {
        row.voted_percent = voted_percent;
        row.voters_detail = new_voters_detail;
    });
}

ACTION community::approvepos(name community_account, uint64_t pos_id)
{
    require_auth(community_account);

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
        if (it->voted_percent >= election_itr->pass_rule)
            top_candidates.emplace_back(it->cadidate);
    }

    if (top_candidates.size() == 0)
    {
        return;
    }

    /// sort by candidate name
    std::sort(top_candidates.begin(), top_candidates.end());

    _pos_proposal.modify(posproposal_itr, community_account, [&](auto &row) {
        row.pos_proposal_status = ProposalStatus::PROPOSAL_APPROVE;
        row.approved_at = current_time_point();
    });

    _positions.modify(pos_itr, community_account, [&](auto &row) {
        row.holders = top_candidates;
    });
}

ACTION community::dismisspos(name community_account, uint64_t pos_id, name holder, const string &dismissal_reason)
{
    require_auth(community_account);

    position_table _positions(_self, community_account.value);
    auto pos_itr = _positions.find(pos_id);
    check(pos_itr != _positions.end(), "ERR::VERIFY_FAILED::Position id doesn't exist.");
    check(pos_itr->holders.size() > 0, "ERR::VERIFY_FAILED::There are no holders for this position.");

    auto _holders = pos_itr->holders;
    _holders.erase(std::find(_holders.begin(), _holders.end(), holder));

    _positions.modify(pos_itr, community_account, [&](auto &row) {
        row.holders = _holders;
    });
}