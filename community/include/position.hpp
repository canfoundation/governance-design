class [[eosio::contract("eosio.system")]] position : public code
{
  enum FillingType {
    APPOINTMENT = 0,
    ELECTION,
  };
  public:
    position(eosio::name receiver, eosio::name code_name, datastream<const char *> ds) : code(receiver, code_name, ds) {}

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
        double pass_rule,
        vector<name> pos_candidate_accounts,
        vector<name> pos_voter_accounts,
        vector<uint64_t> pos_candidate_positions,
        vector<uint64_t> pos_voter_positions
    );

    ACTION initadminpos(name community_account, name creator);

    ACTION configpos(
        name community_account,
        uint64_t pos_id,
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
    );

    ACTION appointpos(name community_account, uint64_t pos_id, vector<name> holder_accounts, const string& appoint_reason);

    ACTION nominatepos(name community_account, uint64_t pos_id, name owner);

    ACTION voteforpos(name community_account, uint64_t pos_id, name voter, name candidate, bool vote_status);

    ACTION approvepos(name community_account, uint64_t pos_id);

    ACTION dismisspos(name community_account, uint64_t pos_id, name holder, const string& dismissal_reason);
  
  protected:
    bool is_position_exist(name community_account, vector<uint64_t> position_ids);

    bool is_position_holder(name community_account, vector<uint64_t> position_ids, name owner);

    virtual bool is_badge_holder(vector<uint64_t> badge_ids, name owner);

    virtual bool is_token_holder(vector<asset> token_ids, name owner);
  
  private:
    bool is_pos_candidate(name community_account, uint64_t pos_id, name owner);

    bool is_pos_voter(name community_account, uint64_t pos_id, name owner);

    uint64_t get_pos_proposed_id();

    uint64_t getposid();
    
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
        double pass_rule;
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
        double voted_percent;
        map<name, uint64_t> voters_detail;
        uint64_t primary_key() const { return cadidate.value; }
        double by_voted_percent() const { return voted_percent;}
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

};