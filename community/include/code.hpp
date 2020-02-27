#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("community")]] code: public contract
{
  public:
    struct execution_code_data {
      name code_action;
      vector<char> packed_params;
    };
  protected:
    #include "constant.hpp"
    
    enum ExecutionType
    {
        SOLE_DECISION = 0,
        COLLECTIVE_DECISION,
        BOTH
    };

    enum ApprovalType
    {
        SOLE_APPROVAL = 0,
        APPROVAL_CONSENSUS,
        BOTH_TYPE
    };

    enum ProposalStatus
    {
        IN_PROGRESS = 0,
        PROPOSAL_APPROVE,
        EXECUTED,
        EXPIRED
    };

    enum VoteStatus {
        REJECT = 0,
        VOTE,
    };

    enum CodeTypeEnum {
        NORMAL = 0,
        POSITION,
        BADGE,
    };

    struct CodeType {
        uint8_t type;
        uint64_t refer_id;
    };

    struct RightHolder
    {
        bool is_anyone = false;
        bool is_any_community_member = false;
        vector<uint64_t> required_badges;
        vector<uint64_t> required_positions;
        vector<asset> required_tokens;
        uint64_t required_exp;
        vector<name> accounts;
    };

    virtual bool is_position_exist(name community_account, vector<uint64_t> position_ids);

    virtual bool is_position_holder(name community_account, vector<uint64_t> position_ids, name owner);

    virtual bool is_badge_holder(vector<uint64_t> badge_ids, name owner);

    virtual bool is_token_holder(vector<asset> token_ids, name owner);

    // table codes with type sole decision with scope is community_creator
    TABLE codef
    {
        uint64_t code_id;
        name code_name;
        name contract_name;
        vector<name> code_actions;
        uint8_t code_exec_type = ExecutionType::SOLE_DECISION;
        uint8_t amendment_exec_type = ExecutionType::SOLE_DECISION;
        CodeType code_type;

        uint64_t primary_key() const { return code_id; }
        uint64_t by_code_name() const { return code_name.value; }
        uint64_t by_reference_id() const { return code_type.refer_id; }
    };

    typedef eosio::multi_index<"codes"_n, codef, 
        indexed_by< "by.code.name"_n, const_mem_fun<codef, uint64_t, &codef::by_code_name>>,
        indexed_by< "by.refer.id"_n, const_mem_fun<codef, uint64_t, &codef::by_reference_id>>
        > code_table;

    // table code collective rule with scope is community_creator
    TABLE collective_decision
    {
        uint64_t code_id;
        uint64_t vote_duration;
        double pass_rule;
        uint8_t approval_type = ApprovalType::SOLE_APPROVAL;
        RightHolder right_proposer;
        RightHolder right_approver;
        RightHolder right_voter;

        uint64_t primary_key() const { return code_id; }
    };

    typedef eosio::multi_index<"codevoterule"_n, collective_decision> code_collective_decision_table;

    // table amendment collective rule with scope is community_creator to store collective rule of amemdment code
    typedef eosio::multi_index<"amenvoterule"_n, collective_decision> ammend_collective_decision_table;

    // table code collective rule with scope is community_creator
    TABLE sole_decision
    {
        uint64_t code_id;
        RightHolder right_executor;

        uint64_t primary_key() const { return code_id; }
    };

    typedef eosio::multi_index<"codeexecrule"_n, sole_decision> code_sole_decision_table;

    // table amendment collective rule with scope is community_creator to store collective rule of amemdment code
    typedef eosio::multi_index<"amenexecrule"_n, sole_decision> amend_sole_decision_table;

    // table code proposals with scope is community_creator
    TABLE code_proposalf
    {
        name proposal_name;
        name proposer;
        double voted_percent = 0;
        uint64_t code_id;
        vector<execution_code_data> code_actions;
        map<name, int> voters_detail;
        uint8_t proposal_status = IN_PROGRESS;
        time_point propose_time;
        time_point exec_at;

        uint64_t primary_key() const { return proposal_name.value; }
        uint64_t by_proposer() const { return proposer.value; }
        uint64_t by_code_id() const { return code_id; }
    };

    typedef eosio::multi_index<"coproposals"_n, code_proposalf,
        indexed_by< "by.proposer"_n, const_mem_fun<code_proposalf, uint64_t, &code_proposalf::by_proposer>>,
        indexed_by< "by.code.id"_n, const_mem_fun<code_proposalf, uint64_t, &code_proposalf::by_code_id>>
        > code_proposals_table;
  public:
    using eosio::contract::contract;
    // code(eosio::name receiver, eosio::name code_name, datastream<const char *> ds) :contract(receiver, code_name, ds) {}

    ACTION initcode(name community_account, name creator, bool create_default_code);

    ACTION proposecode(name community_account, name proposer, name proposal_name, uint64_t code_id, vector<execution_code_data> code_actions);

    ACTION execcode(name community_account, name exec_account, uint64_t code_id, vector<execution_code_data> code_actions);

    ACTION execproposal(name community_account, name proposal_name);

    // Code Action
    ACTION createcode(name community_account, name code_name, name contract_name, vector<name> code_actions);

    // Set excution type of code
    ACTION setexectype(name community_account, uint64_t code_id, uint8_t exec_type, bool is_amend_code);

    // set right holder for sole decision 
    ACTION setsoleexec(name community_account, uint64_t code_id, bool is_amend_code, vector<name> right_accounts, vector<uint64_t> right_pos_ids);

    // set right for who can proposal code   
    ACTION setproposer(name community_account, uint64_t code_id, bool is_amend_code, vector<name> right_accounts, vector<uint64_t> right_pos_ids);

    // set right for who can proposal code   
    ACTION setapprotype(name community_account, uint64_t code_id, bool is_amend_code, uint8_t approval_type);

    // set right for who can approve or vote for proposal
    ACTION setapprover(name community_account, uint64_t code_id, bool is_amend_code, vector<name> right_accounts, vector<uint64_t> right_pos_ids);

    // set right for who can approve or vote for proposal
    ACTION setvoter(name community_account, uint64_t code_id, bool is_amend_code, vector<name> right_accounts, vector<uint64_t> right_pos_ids);

    // set approval consensus requirements for collective decision
    ACTION setvoterule(name community_account, uint64_t code_id, bool is_amend_code, double pass_rule, uint64_t vote_duration); 

    ACTION voteforcode(name community_account, name proposal_name, name approver, bool vote_status);

    ACTION verifyholder(name community_account, uint64_t code_id, uint8_t execution_type, name owner);

    ACTION verifyamend(name community_account, uint64_t code_id, uint8_t execution_type, name owner);

  private:
    bool verifyapprov(name community_account, name voter, uint64_t code_id);

    bool verifyvoter(name community_account, name voter, uint64_t code_id, bool is_amend_code);

    bool is_amend_action(name calling_action);

    void call_action(name community_account, name contract_name, name action_name, vector<char> packed_params) {
        action sending_action;
        sending_action.authorization.push_back(permission_level{community_account, "active"_n});
        sending_action.account = contract_name;
        sending_action.name = action_name;
        sending_action.data = packed_params;
        sending_action.send();
    }
};