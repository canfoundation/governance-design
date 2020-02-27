class [[eosio::contract("eosio.system")]] badge : public position
{
  enum BadgeIssueType {
        WITHOUT_CLAIM = 0,
        CLAIM_APPROVE_BY_ISSUER,
  };

  public:
    badge(eosio::name receiver, eosio::name code, datastream<const char *> ds) : position(receiver, code, ds) {}

    ACTION createbadge(
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
    );

    ACTION configbadge(
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
    );

    ACTION issuebadge(name community_account, name badge_propose_name);

  protected:
    bool is_badge_holder(vector<uint64_t> badge_ids, name owner);

    virtual bool is_token_holder(vector<asset> token_ids, name owner);

  private:
    // refer from crypto-badge contract
    TABLE ccert
    {
      uint64_t                id;
      name                    owner;
      name                    issuer;
      uint64_t                badgeid;
      checksum256        idata;

      auto primary_key() const { return id;}
      uint64_t by_issuer() const { return issuer.value;}
    };
      typedef eosio::multi_index< "ccerts"_n, ccert,
        eosio::indexed_by< "issuer"_n, eosio::const_mem_fun<ccert, uint64_t, &ccert::by_issuer> >
        > ccerts;
};