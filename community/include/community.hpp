#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/print.hpp>
#include <eosio/asset.hpp>
#include <eosio/symbol.hpp>
#include <eosio/singleton.hpp>
#include <eosio/crypto.hpp>
#include <math.h>
// #include "exchange_state.hpp"

using namespace eosio;
using namespace std;
CONTRACT community : public contract
{
    enum CodeRightHolder
    {
        CODE = 0,
        AMENDMENT,
    };

    enum ExecutionType
    {
        SOLE_DECISION = 0,
        COLLECTIVE_DECISION,
        BOTH
    };

    enum ApprovalType{
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

    enum FillingType {
        APPOINTMENT = 0,
        ELECTION,
    };

    enum VoteStatus {
        REJECT = 0,
        VOTE,
    };

    enum CodeTypeEnum {
        NORMAL = 0,
        POSITION_CONFIG,
        POSITION_APPOINT,
        POSITION_DISMISS,
        BADGE_CONFIG,
        BADGE_ISSUE,
    };

    enum BadgeIssueType {
        WITHOUT_CLAIM = 0,
        CLAIM_APPROVE_BY_ISSUER,
    };

   struct permission_level_weight {
      permission_level  permission;
      uint16_t          weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( permission_level_weight, (permission)(weight) )
   };

   struct key_weight {
      eosio::public_key  key;
      uint16_t           weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( key_weight, (key)(weight) )
   };

   struct wait_weight {
      uint32_t           wait_sec;
      uint16_t           weight;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( wait_weight, (wait_sec)(weight) )
   };

   struct authority {
      uint32_t                              threshold = 0;
      std::vector<key_weight>               keys;
      std::vector<permission_level_weight>  accounts;
      std::vector<wait_weight>              waits;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( authority, (threshold)(keys)(accounts)(waits) )
   };

   struct CodeType {
        uint8_t type;
        uint64_t refer_id;
    };

    static asset get_balance( const name& token_contract_account, const name& owner, const symbol_code& sym_code )
    {
        accounts accountstable( token_contract_account, owner.value );
        const auto& ac = accountstable.get( sym_code.raw() );
        return ac.balance;
    }

public:

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

    using contract::contract;
    community(eosio::name receiver, eosio::name code, datastream<const char *> ds) : contract(receiver, code, ds), _communities(_self, _self.value) {}

    struct execution_code_data {
      name code_action;
      vector<char> packed_params;
   };

    void transfer(name from, name to, asset quantity, string memo);

    ACTION createacc(name community_creator, name community_acc);

    ACTION create(name creator, name community_account, string & community_name, vector<uint64_t> member_badge, string & community_url, string & description, bool create_default_code);

    ACTION setaccess(name community_account,RightHolder right_access);

    ACTION initcode(name community_account, name creator, bool create_default_code);

    ACTION inputmembers(name community_account, vector<name> added_members, vector<name> removed_members);

    ACTION initadminpos(name community_account, name creator);

    ACTION proposecode(name community_account, name proposer, name proposal_name, uint64_t code_id, vector<execution_code_data> code_actions);

    ACTION execcode(name community_account, name exec_account, uint64_t code_id, vector<execution_code_data> code_actions);

    ACTION execproposal(name community_account, name proposal_name);

    // Verify owner has right to execute or propose code
    ACTION verifyholder(name community_account, uint64_t code_id, uint8_t execution_type, name owner, bool is_ammend_holder);

    // Code Action
    ACTION createcode(name community_account, name code_name, name contract_name, vector<name> code_actions);

    // Set excution type of code
    ACTION setexectype(name community_account, uint64_t code_id, uint8_t exec_type, bool is_amend_code);

    // set right holder for sole decision 
    ACTION setsoleexec(name community_account, uint64_t code_id, bool is_amend_code, RightHolder right_sole_executor);

    // set right for who can proposal code   
    ACTION setproposer(name community_account, uint64_t code_id, bool is_amend_code, RightHolder right_proposer);

    // set right for who can proposal code   
    ACTION setapprotype(name community_account, uint64_t code_id, bool is_amend_code, uint8_t approval_type);

    // set right for who can approve or vote for proposal
    ACTION setapprover(name community_account, uint64_t code_id, bool is_amend_code, RightHolder right_approver);

    // set right for who can approve or vote for proposal
    ACTION setvoter(name community_account, uint64_t code_id, bool is_amend_code, RightHolder right_voter);

    // set approval consensus requirements for collective decision
    ACTION setvoterule(name community_account, uint64_t code_id, bool is_amend_code, double pass_rule, uint64_t vote_duration); 

    ACTION voteforcode(name community_account, name proposal_name, name approver, bool vote_status);

    // Code Position
    ACTION createpos(
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
    );

    ACTION configpos(
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
    );

    ACTION appointpos(name community_account, uint64_t pos_id, vector<name> holder_accounts, const string& appoint_reason);

    ACTION nominatepos(name community_account, uint64_t pos_id, name owner);

    ACTION voteforpos(name community_account, uint64_t pos_id, name voter, name candidate, bool vote_status);

    ACTION approvepos(name community_account, uint64_t pos_id);

    ACTION dismisspos(name community_account, uint64_t pos_id, name holder, const string& dismissal_reason);

    ACTION createbadge(
        name community_account,
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
    );

    ACTION configbadge(
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
    );

    ACTION issuebadge(name community_account, name badge_propose_name);

private:
    bool verify_approver(name community_account, name voter, uint64_t code_id, bool is_ammnend_code);

    bool verify_voter(name community_account, name voter, uint64_t code_id, bool is_amend_code);

    bool verify_account_right_holder(name community_account, RightHolder right_holder, name owner);

    bool is_pos_candidate(name community_account, uint64_t pos_id, name owner);

    bool is_pos_voter(name community_account, uint64_t pos_id, name owner);

    uint64_t get_pos_proposed_id();

    void call_action(name community_account, name ram_payer, name contract_name, name action_name, vector<char> packed_params);

    uint64_t getposid();

    bool is_amend_action(name calling_action);

    bool verify_community_account_input(name community_account);

    eosio::asset convertbytes2cat(uint32_t bytes);

    void verify_right_holder_input(name community_account, RightHolder rightHolder);

    static inline uint128_t build_reference_id(uint64_t reference_id, uint64_t type) {
        return static_cast<uint128_t>(type)  | static_cast<uint128_t>(reference_id) << 64;
    }

    TABLE communityf
    {
        name community_account;
        name creator;
        string community_name;
        vector<uint64_t> member_badge;
        string community_url;
        string description;

        uint64_t by_creator() const { return creator.value; }
        uint64_t primary_key() const { return community_account.value; }
    };

    typedef eosio::multi_index<"community"_n, communityf, indexed_by< "by.creator"_n, const_mem_fun<communityf, uint64_t, &communityf::by_creator>>> community_table;

    TABLE comunity_member
    {
        name member;

        uint64_t primary_key() const { return member.value; }
    };

    typedef eosio::multi_index<"members"_n, comunity_member> members_table;

    TABLE accession
    {
        RightHolder right_access;
    };

    typedef eosio::singleton<"accession"_n, accession> accession_table;

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
        uint128_t by_reference_id() const { return build_reference_id(code_type.refer_id, code_type.type); }
    };

    typedef eosio::multi_index<"codes"_n, codef, 
        indexed_by< "by.code.name"_n, const_mem_fun<codef, uint64_t, &codef::by_code_name>>,
        indexed_by< "by.refer.id"_n, const_mem_fun<codef, uint128_t, &codef::by_reference_id>>
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

    // table positions with scope is community_account
    TABLE positionf {
        uint64_t pos_id;
        string pos_name;
        uint64_t max_holder = 0;
        vector<name> holders;
        uint8_t fulfillment_type;
        map<name, uint64_t> refer_codes;

        uint64_t primary_key() const { return pos_id; }
    };
    typedef eosio::multi_index<"positions"_n, positionf> position_table;

    // table executed rule with scope is community_account
    TABLE election_rule {
        uint64_t pos_id;
        uint64_t term;
        time_point next_term_start_at;
        uint64_t voting_period;
        RightHolder pos_candidates;
        RightHolder pos_voters;

        uint64_t primary_key() const { return pos_id; }
    };
        
    typedef eosio::multi_index<"fillingrule"_n, election_rule> election_table;    

    // position proposals with scope is community_account
    TABLE pos_proposal {
        uint64_t pos_id;
        uint64_t pos_proposal_id;
        uint8_t pos_proposal_status;
        time_point approved_at;
        uint64_t primary_key() const { return pos_id; }
    };
    typedef eosio::multi_index<"posproposal"_n, pos_proposal> posproposal_table;

    // position proposals with scope is community_account
    TABLE pos_candidate {
        name cadidate;
        vector<name> voters;
        uint64_t primary_key() const { return cadidate.value; }
        double by_voted_percent() const { return static_cast<double>(-voters.size()); }
    };
    typedef eosio::multi_index<"poscandidate"_n, pos_candidate,indexed_by<"byvoted"_n, const_mem_fun<pos_candidate, double, &pos_candidate::by_voted_percent>>> poscandidate_table;
	/*
	* global singelton table, used for position id building
	* Scope: self
	*/
	TABLE global{
		  
		global(){}
		uint64_t posproposed_id	= 0;

		EOSLIB_SERIALIZE( global, (posproposed_id) )
	};
    typedef eosio::singleton< "global"_n, global> global_table;

    // refer from tiger token contract
    TABLE account {
        asset    balance;

        uint64_t primary_key()const { return balance.symbol.code().raw(); }
    };

    typedef eosio::multi_index< "accounts"_n, account > accounts;

    // refer from crypto-badge contract
	TABLE ccert
	{
		uint64_t id;
		uint64_t badge_id;
		uint64_t badge_revision;
		name owner;
		uint64_t state;
		uint64_t expire_at;

		auto primary_key() const
		{
			return id;
		}
		uint64_t by_badge_id() const
		{
			return badge_id;
		}
		uint64_t by_owner() const
		{
			return owner.value;
		}
	};

	typedef eosio::multi_index<"v1.certs"_n, ccert,
							   eosio::indexed_by<"badgeid"_n, eosio::const_mem_fun<ccert, uint64_t, &ccert::by_badge_id>>,
							   eosio::indexed_by<"owner"_n, eosio::const_mem_fun<ccert, uint64_t, &ccert::by_owner>>> ccerts;

    TABLE proposal {
        name                            proposal_name;
        std::vector<char>               packed_transaction;

        uint64_t primary_key()const { return proposal_name.value; }
    };

    typedef eosio::multi_index< "proposal"_n, proposal > multisig_proposals;

    community_table _communities;
};