
Governance Design
----------

Version 2.0.0

This contract provides multiple functionalities:
- Users (creator) can create new community,
- The Community create new code
- The Community set code right holder and amendment right holder
- The Community modify code right holder and amendment right holder
- The Community can create new position, set execution type and set right candidate, right voter for the position.
- The community member can execute the code under community account
- The community member can vote for the code (for election decision code)
- The community member can vote for the position (for election position)
- The community member can nominate themself to a possition

# Actions:

## community::create create new community

```c++
ACTION create(
  name creator, 
  name community_account,
  string & community_name,
  vector<uint64_t> member_badge,
  string & community_url,
  string & description,
  bool create_default_code
);
```

### Description:
- update information of community
- creator must be the owner of this community, the one who transfer money to create community account

### Parameters:
   - **creator** account registered to be a community creator who has permission to create community, the one who transfer money to create community account
   - **community_account** community account which belong to creator
   - **community_name** community's name
   - **member_badge** required badges to become a community member
   - **community_url** community's url
   - **description** short description about the community  
   - **create_default_code** 0: NO, 1: YES, an option to add other default code: `co.amend`, `po.create`, `co.access`, `ba.create`

---

## community::initcode add default code

```c++
    ACTION initcode(name community_account, name creator, bool create_default_code);
```
### Description:
- inline action called when create community
- init default code `co.amend` and `co.access`, `po.create`, `ba.create` if create_default_code is true.

### Parameters:
   - **community_account** community account
   - **creator** who was created the community
   - **create_default_code**  option to add default code: `co.access`, `po.create`, `ba.create`, `ba.issue`

---

## community::RightHolder type

```c++
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
```

### Description:
- Requirement for who can access CiF execute, propose code, nominate, vote for code and position...

### Parameters:
   - **is_anyone** any one have right
   - **is_any_community_member** any members of community have right
   - **required_badges** who have one of these badge will have right
   - **required_positions** who is the holder of one of these position will have right
   - **required_tokens** who amount of one token in these list greater than requirement will have right
   - **required_exp** who have enough experience will have right
   - **accounts** account in this list will have right

---

## community::setaccess set who can access community

```c++
    ACTION setaccess(name community_account, RightHolder right_access);
```

### Description:
- set who can access community
- inline action called by `execcode` action under community account permission

### Parameters:
   - **community_account** community account
   - **right_access** Right holder requirement for who can access CiF
---

## community::create new code

```c++
ACTION createcode(
        name community_account,
        name code_name,
        name contract_name,
        vector<name> code_actions
    );
```

### Description:
- create new code for community
- inline action called by `execcode` action under community account permission

### Parameters:
   - **community_account** community account
   - **code_name** name of the code
   - **contract_name** the contract name this code will call when execute
   - **code_actions** the actions of this code
---

## community::set execution type

```c++
ACTION setexectype(
        name community_account,
        uint64_t code_id,
        uint8_t exec_type,
        bool is_amend_code
    );
```

### Description:
- set execution type for code
- inline action called by `execcode` action under community account permission

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **exec_type** the execution type of code can be SOLE_DECISION or COLLECTIVE_DECISION or BOTH
   - **is_amend_code** is this action set for amend code
---

## community::set right holder for sole execution type of code

```c++
ACTION setsoleexec(
  name community_account,
  uint64_t code_id,
  bool is_amend_code,
  RightHolder right_sole_executor
);
```

### Description:
- set right holder for sole exection type of code
- The one who satisfy the requirement can execute code directly without voting

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **right_sole_executor** Right holder for sole decision
   - **is_amend_code** is this action set for amend code
---

## community::set requirement for the one who can create proposal

```c++
ACTION setproposer(
  name community_account,
  uint64_t code_id,
  bool is_amend_code,
  RightHolder right_proposer
  );
```

### Description:
- set requirement for the one who can create proposal in case code has execution type is COLLECTIVE_DECISION

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **right_proposer** Right Holder for who can propose code
   - **is_amend_code** is this action set for amend code
---

## community::set approval type for code proposal

```c++
ACTION setapprotype(
        name community_account,
        uint64_t code_id,
        bool is_amend_code,
        uint8_t approval_type,
    );
```

### Description:
- if approval_type is SOLE_APPROVAL, the one who meet the requirement can approve and execute code action directly
- if approval_type is APPROVAL_CONSENSUS, the one who meet the requirement can vote for the proposal

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **approval_type** approval type of the proposal, can be SOLE_APPROVAL or APPROVAL_CONSENSUS
   - **is_amend_code** is this action set for amend code
---

## community::set requirement for the one who can create approve for proposal

```c++
ACTION setapprover(
  name community_account,
  uint64_t code_id,
  bool is_amend_code,
  RightHolder right_approver
);
```

### Description:
- set requirement for the one who can approve the proposal in case code has execution type is COLLECTIVE_DECISION
- action can only use for code with approval_type is SOLE_APPROVAL, the one who meet the requirement can approve and execute code action directly

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **right_approver** Right Holder for who can approve proposal
   - **is_amend_code** is this action set for amend code
---

## community::set requirement for the one who can create approve for proposal

```c++
ACTION setvoter(
  name community_account,
  uint64_t code_id,
  bool is_amend_code,
  RightHolder right_voter
);
```

### Description:
- set requirement for the one who can vote in case code has execution type is COLLECTIVE_DECISION
- action can only use for code with approval_type is APPROVAL_CONSENSUS, the one who meet the requirement can vote for proposal

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **right_voter** Right Holder for the one who can vote for proposal
   - **is_amend_code** is this action set for amend code
---

## community::set vote rule for code 

```c++
ACTION setvoterule(
        name community_account,
        uint64_t code_id,
        bool is_amend_code,
        double pass_rule,
        uint64_t vote_duration
    ); 
```

### Description:
- set vote rule for code in case that code has execution type is COLLECTIVE_DECISION and 

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **vote_duration** the duration for voting
   - **pass_rule** the percentage of voting to be accepted
   - **is_amend_code** is this action set for amend code
---

## community::execcode execute a code

```c++
ACTION execcode(name community_account, name exec_account, uint64_t code_id, vector<execution_code_data> code_actions);
```

### Description:
- the middle action to execute the code
- action will check the right holder of executor, and call inline action to the contract and action of executing code under community account permission
- ram fee of this action will be paid by community account

### Parameters:
   - **community_account** community account
   - **exec_account** right holder account (account who sign to execute this action)
   - **code_id** the code's id
   - **code_actions** list of actions and packed parameters to execute action
      - **code_action** name of the action
      - **packed_params**: packed parameters to exectute action

---

## community::create the proposal

```c++
ACTION proposecode(name community_account, name proposer, name proposal_name, uint64_t code_id, vector<execution_code_data> code_actions);
```

### Description:
- In case execution type of code is COLLECTIVE_DECISSION, use this action to create proposal of code
- action will check the right holder of proposer, and save proposal to table
- ram fee of this action will be paid proposer

### Parameters:
   - **community_account** community account
   - **proposer** right proposer account (account who sign to execute this action)
   - **code_id** the code's id
   - **code_actions** list of actions and packed parameters to execute action
      - **code_action** name of the action
      - **packed_params**: packed parameters to exectute action

---

## community::execproposal execute a code with type election decision

```c++
ACTION execproposal(name community_account, name proposal_name);
```

### Description:
- execute code proposal
- voting for this proposal must be ended
- propopsal must be execute before execution expire time
- this action will check the proposal status and call inline action of this code

### Parameters:
   - **codeid** id of code
   - **proposal_name** the proposal name

---

## community::voteforcode vote or approve for the proposal of code

```c++
ACTION voteforcode(name community_account, name proposal_name, name approver, bool vote_status);
```

### Description:
- voter or approver call this action to vote or approve for proposal of code
- execute under voter/approver permission
- voter/approver must be has the right to executer this code, define in right_voter/righ_approver of collective rule of code (`codevoterule` and `amendvoterule` table)
- vote_status 0 is disagree and 1 is agree
- if voter has right to approve the proposal, proposal will be executed directly
- if voter just has right to vote the proposal, voting status will be stored.
- if voter does not have right to do anything, throw error.

### Paramters:
   - **community_account** community account
   - **proposal_id** the code's id
   - **voter** the name of voter or approver
   - **vote_status** 0: UNVOTE, 1: VOTE
   
---

## community::createpos create new position

```c++
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
```

### Description:
   - create new position

### Parameters:
   - **community_account** community account
   - **creator** community account
   - **pos_name** position's name
   - **max_holder** the maximum holder for position
   - **filled_through** how to fill the holder for this position, 0 APOINTMENT, 1 ELECTION
   - **term** term of position
   - **next_term_start_at** start time of next term
   - **voting_period** voting period
   - **right_candidate** Right Holder for the one who can nominate for position
   - **right_voter** Right Holder for the one who can vote for candidate

```bash
        position_created_time      voting_start_day          voting_end_date          next_term_start_at
                |---------------------->|-------------------------->|------------------------>|
                |                       |                           |                         |
                |                       |<---- vote_duration  ----->|<-------- 1 day -------->|
                |                       |                           |                         |
                |<---------------- nomination duration ------------>|                         |

```
---

## community::configpos 

```c++
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
    )
```

### Description:
   - configure for position

### Parameters:
   - **community_account** community account
   - **pos_name** position's name
   - **max_holder** the maximum holder for position
   - **filled_through** how to fill the holder for this position, 0 APOINTMENT, 1 ELECTION
   - **term** term of position
   - **next_term_start_at** start time of next term
   - **voting_period** voting period
   - **right_candidate** Right Holder for the one who can nominate for position
   - **right_voter** Right Holder for the one who can vote for candidate
---

## community::appointpos 

```c++
ACTION appointpos(
  name community_account,
  uint64_t pos_id,
  vector<name> holder_accounts,
  const string& appoint_reason);
```

   - executor: community
   - **community_account** community account
   - **pos_id** the pos's id
   - **holder_accounts** the candidate list

---

## community::nominatepos 

```c++
ACTION nominatepos(name community_account, uint64_t pos_id, name owner);
```

   - executor: Position right holder
   - **community_account** community account
   - **pos_id** the poisition's id
   - **owner** nominate themself to an position.

---

## community::voteforpos

```c++
ACTION voteforpos(name community_account, uint64_t pos_id, name voter, name candidate, bool vote_status);
```

   - executor: voter refer community::setfillrule:right_accounts
   - **community_account** community account
   - **pos_id** the poisition's id
   - **candidate** the position right holder
   - **vote_status** 0: UNVOTE, 1: VOTE

---

## community::approvepos The action should be executed after the election

```c++
ACTION approvepos(name community_account, uint64_t pos_id);
```

   - executor: community
   - **community_account** community account
   - **pos_id** the code's id
  
---

## ACTION dissmisspos(name community_account, uint64_t pos_id, name holder);

```c++
ACTION dismisspos(name community_account, uint64_t pos_id, name holder, const string& dismissal_reason);
```

   - executor: community
   - **community_account** community account
   - **pos_id** the poisition's id
   - **holder** the holder's account

---

## community::createbadge create new badge

```c++
ACTION createbadge(
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
    );
```
- **community_account**: community account name,
- **badge_id**: id of badge,
- **issue_type**: badge issue type, refer to [document](https://docs.google.com/document/edit?hgd=1&id=1ZRQLixZ1_r-8xgYnkyWP0WfHBbSpcIKbOJ6RYg6XMXc#)
- **badge_propose_name**: multisig proposal name to create badge,
- **issue_exec_type**: SOLE_EXECUTION or COLLECTIVE_EXECUTION, execution type of issue badge code of this badge,
- **right_issue_sole_executor**: in case of issue_exec_type is SOLE_EXECUTION, Right Holder for sole decision of issue code
- **right_issue_proposer**: in case of issue_exec_type is COLLECTIVE_EXECUTION, Right Holder for who can propose issue badge
- **issue_approval_type**: SOLE_APPROVAL - approver can approve and execute code right a way or APPROVAL_CONSENSUS - voter vote for proposal, if majority of voter voted, proposal can be executed,
- **right_issue_approver**: in case of issue_approval_type is SOLE_APPROVAL, Right Holder for who can approve issue badge proposal
- **right_issue_voter**: in case of issue_approval_type is APPROVAL_CONSENSUS, Right Holder for who can vote for issue badge proposal
- **issue_pass_rule**: perent of voted for proposal to be pass,
- **issue_vote_duration**: duration to vote for proposal

---

## community::configbadge config or update badge

```c++
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
```
- **community_account**: community account name,
- **badge_id**: id of badge,
- **issue_type**: badge issue type, refer to [document](https://docs.google.com/document/edit?hgd=1&id=1ZRQLixZ1_r-8xgYnkyWP0WfHBbSpcIKbOJ6RYg6XMXc#)
- **update_badge_proposal_name**: multisig proposal name to update badge,
- **issue_exec_type**: SOLE_EXECUTION or COLLECTIVE_EXECUTION, execution type of issue badge code of this badge,
- **right_issue_sole_executor**: in case of issue_exec_type is SOLE_EXECUTION, Right Holder for sole decision of issue code
- **right_issue_proposer**: in case of issue_exec_type is COLLECTIVE_EXECUTION, Right Holder for who can propose issue badge
- **issue_approval_type**: SOLE_APPROVAL - approver can approve and execute code right a way or APPROVAL_CONSENSUS - voter vote for proposal, if majority of voter voted, proposal can be executed,
- **right_issue_approver**: in case of issue_approval_type is SOLE_APPROVAL, Right Holder for who can approve issue badge proposal
- **right_issue_voter**: in case of issue_approval_type is APPROVAL_CONSENSUS, Right Holder for who can vote for issue badge proposal
- **issue_pass_rule**: perent of voted for proposal to be pass,
- **issue_vote_duration**: duration to vote for proposal

---

## community::issuebadge issue badge

```c++
ACTION issuebadge(name community_account, name badge_propose_name);
```

- **community_account**: community account name,
- **badge_propose_name**: multisig proposal name to issue badge,
-------
## Prerequisites


* account to deploy smart contract:  governance33
 
* Private key: 5K168UAEkfQNZbjGDF7MvFPRW94fN5yCMwGWAcCuN6C1uXTUNrv
* Public key: EOS5W5LFjcWbih3ETW9nWzoBqXdYoz57zvSNzH58i4D9knt4FeYED

* CAN Testnet endpoint network: http://18.163.57.224:8888

* Contract name: governance33

### Set smart contract to governance22 account
````bash
// complie contract
eosio-cpp -abigen -I include -contract community -o community.wasm src/community.cpp

// set contract
cleos -u http://18.163.57.224:8888 set contract governance33 . community.wasm community.abi -p governance33

// set contract account permission
cleos -u http://18.163.57.224:8888 set account permission governance33 active '{"threshold": 1,"keys": [{"key": "EOS7UAP5KoSD58dEejXNihF4nAqQjSgu9wvUwemtm4op72jL7hBUs","weight": 1}],"accounts": [{"permission":{"actor":"governance33","permission":"eosio.code"},"weight":1}]}' owner -p governance33

cleos -u http://18.163.57.224:8888 set account permission c active '{"threshold": 1,"keys": [{"key": "EOS6yfoREUrCWa1MZkjnfhLyG2cBk9spkayth6NKPBCmpLkzEK7NG","weight": 1}],"accounts": [{"permission":{"actor":"governance33","permission":"eosio.code"},"weight":1}]}' owner -p c

cleos -u http://18.163.57.224:8888 set account permission ram.can active '{"threshold": 1,"keys": [{"key": "EOS7UAP5KoSD58dEejXNihF4nAqQjSgu9wvUwemtm4op72jL7hBUs","weight": 1}],"accounts": [{"permission":{"actor":"governance33","permission":"eosio.code"},"weight":1}]}' owner -p ram.can

````

## Sample of community follow using cleos command
### create community account and delegate permission code to contract

- Transfer token to governance contract account with memo is the community account name
- The token will be used for create new account, initial ram and bandwidth for new account
- The remain token will be returned back to creator if have.

````bash
# avtive creating community account feature
cleos -u http://18.163.57.224:8888 transfer quocle governance33 "10.0000 CAT" "community232" "deposit_core_symbol" -p quocle

# create community account
cleos -u http://18.163.57.224:8888 transfer quocle governance33 "10.0000 CAT" "community232" "quocle1.c" -p quocle

cleos -u http://18.163.57.224:8888 get account quocle1.c
created: 2020-04-09T05:57:18.000
permissions: 
     owner     1:    1 governance33@eosio.code
        active     1:    1 governance33@eosio.code
````

### Inluce RAM payer signatur in creating community and other action

````bash

# To support RAM for users/communites. It need the signature from ram payer

cleos -u http://18.163.57.224:8888 multisig propose createcom \
'[{"actor": "quocle", "permission": "active"},{"actor": "ram.can", "permission": "active"}]' \
'[{"actor": "quocle", "permission": "active"},{"actor": "ram.can", "permission": "active"}]' \
governance33 create '{"creator":"quocle", "community_account":"quocle1.c", "community_name":"QuocLe", "member_badge":[], "community_url":"community_url", "description":"description application", "create_default_code":1}' quocle 12

cleos -u http://18.163.57.224:8888 multisig review quocle createcom

cleos -u http://18.163.57.224:8888 get table eosio.msig quocle approvals2 -U createcom -L createcom

cleos -u http://18.163.57.224:8888 multisig approve quocle createcom '{"actor": "quocle", "permission": "active"}' -p quocle

cleos -u http://18.163.57.224:8888 multisig approve quocle createcom '{"actor": "ram.can", "permission": "active"}' -p ram.can

cleos -u http://18.163.57.224:8888 multisig exec quocle createcom ram.can

$ cleos -u http://18.163.57.224:8888 get table governance33 governance33 community
{
  "rows": [{
      "community_account": "community232",
      "creator": "quocleplayer",
      "community_name": "Vingle Group",
      "member_badge": [],
      "community_url": "vingle_group.com",
      "description": "Testing"
    }
  ]
}

Check result at [here](https://local.bloks.io/account/governance33?nodeUrl=history.stagenet.canfoundation.io&coreSymbol=CAT&systemDomain=eosio)

````