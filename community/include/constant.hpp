const symbol ramcore_symbol = symbol(symbol_code("RAMCORE"), 4);
const symbol ram_symbol = symbol(symbol_code("RAM"), 0);
const symbol CORE_SYMBOL = symbol(symbol_code("CAT"), 4);
const uint64_t init_ram_amount = 10 * 1024;
const asset stake_net = asset(1'0000, CORE_SYMBOL);
const asset stake_cpu = asset(1'0000, CORE_SYMBOL);

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