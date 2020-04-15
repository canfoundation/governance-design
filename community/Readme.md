
Governance Design
----------

Version 1.1.0

code hash b1b99cb027707a00a8e4457483dfb56fca9dc71c4730ad70d00636407a5e5703

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


* account to deploy smart contract:  governance23
 
* Private key: 5K168UAEkfQNZbjGDF7MvFPRW94fN5yCMwGWAcCuN6C1uXTUNrv
* Public key: EOS5W5LFjcWbih3ETW9nWzoBqXdYoz57zvSNzH58i4D9knt4FeYED

* CAN Testnet endpoint network: http://18.182.95.163:8888

* Contract name: governance23

### Set smart contract to governance22 account
````bash
// complie contract
eosio-cpp -abigen -I include -contract community -o community.wasm src/community.cpp

// set contract
cleos -u http://18.182.95.163:8888 set contract governance23 . community.wasm community.abi -p governance23

// set contract account permission
cleos -u http://18.182.95.163:8888 set account permission governance23 active '{"threshold": 1,"keys": [{"key": "EOS5W5LFjcWbih3ETW9nWzoBqXdYoz57zvSNzH58i4D9knt4FeYED","weight": 1}],"accounts": [{"permission":{"actor":"governance23","permission":"eosio.code"},"weight":1}]}' owner -p governance23
````

## Sample of community follow using cleos command
### create community account and delegate permission code to contract

- Transfer token to governance contract account with memo is the community account name
- The token will be used for create new account, initial ram and bandwidth for new account
- The remain token will be returned back to creator if have.

````bash
cleos -u http://18.182.95.163:8888 transfer quocleplayer governance23 "10.0000 CAT" "community232" -p quocleplayer

# allow 3rd party pay CAT in creating community account instead of community creator
cleos -u http://18.182.95.163:8888 transfer quyvoplayer governance23 "10.0000 CAT" "community232-quocleplayer" -p quyvoplayer

$ cleos -u http://18.182.95.163:8888 get account community225
created: 2019-11-29T10:43:04.500
permissions: 
     owner     1:    1 governance22@eosio.code
        active     1:    1 governance22@eosio.code
````

### create community and init default code

````bash

$ cleos -u http://18.182.95.163:8888 push action governance23 create '[quocleplayer, community232, "Vingle Group", [], "vingle_group.com", "Testing", 1]' -p quocleplayer

$ cleos -u http://18.182.95.163:8888 get table governance23 governance23 community
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

// initialize co.amend, co.access, po.create, ba.create when create new  community
// by default all initial codes have execution type is sole decision with right account is admin
$ cleos get table governance23 community241 codes
{
  "rows": [{
      "code_id": 0,
      "code_name": "co.amend",
      "contract_name": "governance23",
      "code_actions": [
        "createcode"
      ],
      "code_exec_type": 0,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
      }
    },{
      "code_id": 1,
      "code_name": "po.create",
      "contract_name": "governance23",
      "code_actions": [
        "createpos"
      ],
      "code_exec_type": 0,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
      }
    },{
      "code_id": 2,
      "code_name": "co.access",
      "contract_name": "governance23",
      "code_actions": [
        "accesscode"
      ],
      "code_exec_type": 0,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
      }
    },{
      "code_id": 3,
      "code_name": "ba.create",
      "contract_name": "governance23",
      "code_actions": [
        "createbadge"
      ],
      "code_exec_type": 0,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
      }
    }
  ],
  "more": false
}

// right holder for sole execution code store in codeexecrule table
$ cleos get table governance23 community241 codeexecrule
{
  "rows": [{
      "code_id": 0,
      "right_executor": {
        "is_anyone": 0,
        "is_any_community_member": 0,
        "required_badges": [],
        "required_positions": [],
        "required_tokens": [],
        "required_exp": 0,
        "accounts": [
          "quocleplayer"
        ]
      }
    },{
      "code_id": 1,
      "right_executor": {
        "is_anyone": 0,
        "is_any_community_member": 0,
        "required_badges": [],
        "required_positions": [],
        "required_tokens": [],
        "required_exp": 0,
        "accounts": [
          "quocleplayer"
        ]
      }
    },{
      "code_id": 2,
      "right_executor": {
        "is_anyone": 0,
        "is_any_community_member": 0,
        "required_badges": [],
        "required_positions": [],
        "required_tokens": [],
        "required_exp": 0,
        "accounts": [
          "quocleplayer"
        ]
      }
    },{
      "code_id": 3,
      "right_executor": {
        "is_anyone": 0,
        "is_any_community_member": 0,
        "required_badges": [],
        "required_positions": [],
        "required_tokens": [],
        "required_exp": 0,
        "accounts": [
          "quocleplayer"
        ]
      }
    }
  ],
  "more": false
}
````

### set who is community members
1. pack action data of `inputmembers` action
```bash
cleos convert pack_action_data governance inputmembers '["community111",  ["quocle", "can"], ["eosio"]]'                                            
00004020a888a8b603000000000000a6410000000000ea305500000000a888a8b6010000000000ea3055
```
2. get id of `co.members` code
```bash
cleos get table governance community111 codes --index 2 --key-type i64 -L co.members -U co.members
{
  "rows": [{
      "code_id": 1,
      "code_name": "co.members",
      "contract_name": "governance33",
      "code_actions": [
        "inputmembers"
      ],
      "code_exec_type": 0,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
      }
    }
  ],
  "more": false,
  "next_key": ""
}
```

3. execute `inputmembers` action of `co.members` code:
```bash
cleos push action governance execcode '["community111", "sally", 1, [["inputmembers", "00004020a888a8b603000000000000a6410000000000ea305500000000a888a8b6010000000000ea3055"]]]' -p quocle
```

4. get table `members` to verify
```bash
cleos get table governance community111 members
{
  "rows": [{
      "member": "can"
    },{
      "member": "eosio"
    }
  ],
  "more": false,
  "next_key": ""
}
```

### set who can access community
1. pack action data of `setaccess` action
```bash
cleos convert pack_action_data governance setaccess '["community111", [0, 0, [], [], [], 0, ["sally", "chen", "corona"]]]'                                                
1042f0d94d2d2545000000000000000000000000000300000000001fa3c100000000003055430000000098492f45
```
2. get id of `co.access` code
```bash
cleos get table governance community111 codes --index 2 --key-type i64 -L co.access -U co.access
{
  "rows": [{
      "code_id": 2,
      "code_name": "co.access",
      "contract_name": "governance24",
      "code_actions": [
        "setaccess"
      ],
      "code_exec_type": 0,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
      }
    }
  ],
  "more": false
}
```

3. execute `setaccess` action of `co.access` code:
```bash
cleos push action governance execcode '["community111", "sally", 2, [["setaccess", "1042f0d94d2d2545000000000000000000000000000300000000001fa3c100000000003055430000000098492f45"]]]' -p quocleplayer
```

4. get table `accession` to verify
```bash
cleos get table governance24 community314 accession                                                                                                                     130 ↵
{
  "rows": [{
      "right_access": {
        "is_anyone": 0,
        "is_any_community_member": 0,
        "required_badges": [],
        "required_positions": [],
        "required_tokens": [],
        "required_exp": 0,
        "accounts": [
          "sally",
          "chen",
          "corona"
        ]
      }
    }
  ],
  "more": false,
  "next_key": ""
}
```

### use amend code to config code code_execution_right

*Note*: Each code created will has two execution right:

  - code execution right: is the requirement for the one who has right to execute this code store in `codevoterule` and `codeexecrule` table
  - amendment execution right: is the requirement for the one who has right to set right for code_execution_right and amendment_execution_right store in `amenvoterule` and `amenexecrule` table

#### set right holder for code execution right

1. pack setexectype action data to set execution type of `co.access` with code_id 3 to collective decision (1):

```bash
// last parameter is identify set for amendment code (1 is set for amendment code, 0 is set for code)
$ cleos convert pack_action_data governance setexectype '["community111", 2, 1, 0]'
1042f0d94d2d254502000000000000000100
```

2. pack `setapprotype` action data to set approval type of code proposal (approval type 1 is approval consensus):
```bash
$ cleos convert pack_action_data governance setapprotype '["community111", 2, 0, 1]'
1042f0d94d2d254502000000000000000001
```

3. pack setproposer action data to set proposer right holder of `co.access` with code_id 3:

```bash
$ cleos convert pack_action_data governance setproposer '["community111", 2, 0, [0, 0, [], [], [], 0, ["chen"]]]'
1042f0d94d2d254502000000000000000000000000000000000000000000010000000000305543
```

4. pack action data `setvoter` to set voter of proposal:

```bash
cleos convert pack_action_data governance setvoter '["community111", 2, 0, [0, 0, [], [], [], 0, ["corona"]]]'
1042f0d94d2d254502000000000000000000000000000000000000000000010000000098492f45
```

5. pack action data `setvoterule` to set vote rule of proposal:
```bash
cleos convert pack_action_data governance setvoterule '["community111", 2, 0, 60, 600]'
1042f0d94d2d25450200000000000000000000000000004e405802000000000000
```

6. use action `execcode` to execute above action, amend execution type is the special case, even `setexectype`, `setproposer`, `setvoter`, `setvoterule` action is not exist in the code, but `execcode` action with automatically know it is use for changing the execiton type:

```bash
cleos push action governance execcode '["community111", "sally", 2, [["setexectype", "1042f0d94d2d254502000000000000000100"], ["setapprotype", "1042f0d94d2d254502000000000000000001"], ["setproposer", "1042f0d94d2d254502000000000000000000000000000000000000000000010000000000305543"], ["setvoter", "1042f0d94d2d254502000000000000000000000000000000000000000000010000000098492f45"], ["setvoterule", "1042f0d94d2d25450200000000000000000000000000004e405802000000000000"]]]' -p sally
```


7. get code and codevoterule table to check:

```bash
$ cleos get table governance community111 codes
{
      "code_id": 2,
      "code_name": "co.access",
      "contract_name": "governance23",
      "code_actions": [
        "accesscode"
      ],
      "code_exec_type": 1,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
}

$ cleos get table governance community111 codevoterule
{
  "rows": [{
      "code_id": 3,
      "vote_duration": 600,
      "pass_rule": "60.00000000000000000",
      "approval_type": 1,
      "right_proposer": {
        "is_anyone": 0,
        "is_any_community_member": 0,
        "required_badges": [],
        "required_positions": [],
        "required_tokens": [],
        "required_exp": 0,
        "accounts": [
          "quyvoplayers"
        ]
      },
      "right_approver": {
        "is_anyone": 0,
        "is_any_community_member": 0,
        "required_badges": [],
        "required_positions": [],
        "required_tokens": [],
        "required_exp": 0,
        "accounts": []
      },
      "right_voter": {
        "is_anyone": 0,
        "is_any_community_member": 0,
        "required_badges": [],
        "required_positions": [],
        "required_tokens": [],
        "required_exp": 0,
        "accounts": [
          "quyvoplayers"
        ]
      }
    }
  ],
  "more": false
}
```

##### set right holder for amendment execution right

1. pack setexectype action data to set execution type of `co.access` with code_id 3 to collective decision (1):

```bash
// last parameter is identify set for amendment code (1 is set for amendment code, 0 is set for code)
$ cleos convert pack_action_data governance setexectype '["community111", 2, 1, 1]'
1042f0d94d2d254502000000000000000101
```

2. pack `setapprotype` action data to set approval type of code proposal (approval type 1 is approval consensus):
```bash
$ cleos convert pack_action_data governance setapprotype '["community111", 2, 1, 1]'
1042f0d94d2d254502000000000000000101
```

3. pack setproposer action data to set proposer right holder of `co.access` with code_id 3:

```bash
$ cleos convert pack_action_data governance setproposer '["community111", 2, 1, [0, 0, [], [], [], 0, ["chen"]]]'
1042f0d94d2d254502000000000000000100000000000000000000000000010000000000305543
```

4. pack action data `setvoter` to set voter of proposal:

```bash
cleos convert pack_action_data governance setvoter '["community111", 2, 1, [0, 0, [], [], [], 0, ["corona"]]]'
1042f0d94d2d254502000000000000000100000000000000000000000000010000000098492f45
```

5. pack action data `setvoterule` to set vote rule of proposal:
```bash
cleos convert pack_action_data governance setvoterule '["community111", 2, 1, 60, 600]'
1042f0d94d2d25450200000000000000010000000000004e405802000000000000
```

6. use action `execcode` to execute above action, amend execution type is the special case, even `setexectype`, `setproposer`, `setvoter`, `setvoterule` action is not exist in the code, but `execcode` action with automatically know it is use for changing the execiton type:

```bash
cleos push action governance execcode '["community111", "sally", 2, [["setapprotype", "1042f0d94d2d254502000000000000000101"], ["setproposer", "1042f0d94d2d254502000000000000000100000000000000000000000000010000000000305543"], ["setvoter", "1042f0d94d2d254502000000000000000100000000000000000000000000010000000098492f45"], ["setvoterule", "1042f0d94d2d25450200000000000000010000000000004e405802000000000000"], ["setexectype", "1042f0d94d2d254502000000000000000101"]]]' -p quocleplayer
```

7. get code and amenvoterule table to check:

```bash
$ cleos get table governance community111 codes
{
      "code_id": 2,
      "code_name": "co.access",
      "contract_name": "governance23",
      "code_actions": [
        "accesscode"
      ],
      "code_exec_type": 1,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
}

$ cleos get table governance community111 amenvoterule
{
  "rows": [{
      "code_id": 2,
      "vote_duration": 600,
      "pass_rule": "60.00000000000000000",
      "approval_type": 1,
      "right_proposer": {
        "is_anyone": 0,
        "is_any_community_member": 0,
        "required_badges": [],
        "required_positions": [],
        "required_tokens": [],
        "required_exp": 0,
        "accounts": [
          "chen"
        ]
      },
      "right_approver": {
        "is_anyone": 0,
        "is_any_community_member": 0,
        "required_badges": [],
        "required_positions": [],
        "required_tokens": [],
        "required_exp": 0,
        "accounts": []
      },
      "right_voter": {
        "is_anyone": 0,
        "is_any_community_member": 0,
        "required_badges": [],
        "required_positions": [],
        "required_tokens": [],
        "required_exp": 0,
        "accounts": [
          "corona"
        ]
      }
    }
  ],
  "more": false,
  "next_key": ""
}
```
---

### create new code

1. pack action data to create new code
```bash
cleos convert pack_action_data governance createcode '["community111", "testcode", "governance", ["testaction"]]'
1042f0d94d2d25450000002a5194b1ca008042d3ccab36650100c0a42e2393b1ca
```

2. find the code id of co.amend code, use secondary index to find code by code name:

```bash
cleos get table governance community111 codes --index 2 --key-type i64 -L co.amend -U co.amend
{
  "rows": [{
      "code_id": 0,
      "code_name": "co.amend",
      "contract_name": "governance23",
      "code_actions": [
        "createcode"
      ],
      "code_exec_type": 0,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
      }
    }
  ],
  "more": false
}
```

3. execute create code
```bash
cleos push action governance execcode '[community111, sally, 0, [[createcode, "1042f0d94d2d25450000002a5194b1ca008042d3ccab36650100c0a42e2393b1ca"]]]' -p sally
```

4. get table to check again:
```bash
cleos get table governance community111 codes --index 2 --key-type i64 -L testcode -U testcode
{
  "rows": [{
      "code_id": 5,
      "code_name": "testcode",
      "contract_name": "governance23",
      "code_actions": [
        "testaction"
      ],
      "code_exec_type": 0,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
      }
    }
  ],
  "more": false
}
```


### create new position

1. Find creation of Position code, use secondary index, get the code id is 1:

```bash
cleos get table governance community111 codes --index 2 --key-type i64 -L po.create -U po.create
{
  "rows": [{
      "code_id": 1,
      "code_name": "po.create",
      "contract_name": "governance23",
      "code_actions": [
        "createpos"
      ],
      "code_exec_type": 0,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
      }
    }
  ],
  "more": false
}
```

2. Pack above action data, `Appointment` filling type coresspond to number 0:
```bash
cleos convert pack_action_data governance createpos '["community111", "sally", "Badge Manager", 3, 1, 9000, 1584513948, 500, [0, 0, [], [], [], 0, ["chen", "corona"]], [0, 0, [], [], [], 0, ["daniel", "sally"]]]' 
1042f0d94d2d254500000000001fa3c10d4261646765204d616e6167657203000000000000000128230000000000009cc3715e00000000f401000000000000000000000000000000000000000200000000003055430000000098492f4500000000000000000000000000020000000044e5a64900000000001fa3c1
```

3. Because the code is `Collective decision` type we need to use `proposecode` to create proposal ( in case of `sole decision` we can call `execcode` to execute it directly), the proposal need to pay the ram for this proposal:

create proposal:
```bash
cleos push action governance proposecode '["community111", "sally", "newpos", 1, [["createpos", "1042f0d94d2d254500000000001fa3c10d4261646765204d616e6167657203000000000000000128230000000000009cc3715e00000000f401000000000000000000000000000000000000000200000000003055430000000098492f4500000000000000000000000000020000000044e5a64900000000001fa3c1"]]]' -p quocleplayer
```

4. Voter vote for proposal

To get all proposals of community:

```bash
cleos get table governance community111 coproposals
{
  "rows": [{
      "proposal_name": "newpos",
      "proposer": "quocleplayer",
      "code_id": 1,
      "code_action": "createpos",
      "voted_percent": "0.00000000000000000",
      "data": "1088f0d94d2d2545709537b1aa88a8b60d4261646765204d616e61676572030000000000000000",
      "voters_detail": [],
      "proposal_status": 0,
      "propose_time": "2020-02-05T04:07:02.500",
      "exec_at": "1970-01-01T00:00:00.000"
    }
  ],
  "more": false
}
```

User `voteforcode` action to vote for code proposal
```bash
// quocleplayer agree proposal
cleos push action governance voteforcode '["community111", "newpos", quocleplayer, 1]' -p quocleplayer

// creator.can agree proposal
cleos push action governance voteforcode '["community111", "newpos", creator.can, 1]' -p creator.can
```

5. After voting has been finished, call execproposal action to execute proposal:

To get all proposals of community:

```bash
cleos get table governance community111 coproposals
{
  "rows": [{
      "proposal_name": "newpos",
      "proposer": "quyvoplayers",
      "voted_percent": "100.00000000000000000",
      "code_id": 1,
      "code_actions": [{
          "code_action": "createpos",
          "packed_params": "2088f0d94d2d2545709537b1aa88a8b60d4261646765204d616e61676572030000000000000000"
        }
      ],
      "voters_detail": [{
          "key": "quyvoplayers",
          "value": 1
        }
      ],
      "proposal_status": 0,
      "propose_time": "2020-02-06T07:21:19.000",
      "exec_at": "1970-01-01T00:00:00.000"
    }
  ],
  "more": false
}
```

To execute proposal
```bash
cleos push action governance execproposal '[community111, newpos]' -p sally
```

6. Get position table to check:

Three corresponding code of positions have been created. `po.appoint` with code id 7, `po.config` with code id 7, `po.dismiss` with code id 6 and `po.dissmiss` with code id 8.

```bash
cleos get table governance community111 positions
{
  "rows": [{
      "pos_id": 0,
      "pos_name": "Badge Manager",
      "max_holder": 3,
      "holders": [],
      "fulfillment_type": 0,
      "refer_codes": [{
          "key": "po.appoint",
          "value": 7
        },{
          "key": "po.config",
          "value": 6
        },{
          "key": "po.dismiss",
          "value": 8
        }
      ]
    }
  ],
  "more": false
}

cleos get table governance community111 codes -L 6 -U 8
{
  "rows": [{
      "code_id": 6,
      "code_name": "po.config",
      "contract_name": "governance23",
      "code_actions": [
        "configpos"
      ],
      "code_exec_type": 0,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
      }
    },{
      "code_id": 7,
      "code_name": "po.appoint",
      "contract_name": "governance23",
      "code_actions": [
        "appointpos"
      ],
      "code_exec_type": 0,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
      }
    },{
      "code_id": 8,
      "code_name": "po.dismiss",
      "contract_name": "governance23",
      "code_actions": [
        "dismisspos"
      ],
      "code_exec_type": 0,
      "amendment_exec_type": 0,
      "code_type": {
        "type": 0,
        "refer_id": 0
      }
    }
  ],
  "more": false
}
```

##### use po.create to create new election position
