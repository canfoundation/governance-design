
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

## community::create create new community

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
```c++
    ACTION initcode(name community_account, name creator, bool create_default_code);
```

## community::initcode add default code
### Description:
- inline action called when create community
- init default code `co.amend` and `co.access`, `po.create`, `ba.create` if create_default_code is true.

### Parameters:
   - **community_account** community account
   - **creator** who was created the community
   - **create_default_code**  option to add default code: `co.access`, `po.create`, `ba.create`, `ba.issue`
---
```c++
ACTION createcode(
        name community_account,
        name code_name,
        name contract_name,
        vector<name> code_actions
    );
```

## community::create new code
### Description:
- create new code for community
- inline action called by `execcode` action under community account permission

### Parameters:
   - **community_account** community account
   - **code_name** name of the code
   - **contract_name** the contract name this code will call when execute
   - **code_actions** the actions of this code
---
```c++
ACTION setexectype(
        name community_account,
        uint64_t code_id,
        uint8_t exec_type,
        bool is_amend_code
    );
```

## community::set execution type
### Description:
- set execution type for code
- inline action called by `execcode` action under community account permission

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **exec_type** the execution type of code can be SOLE_DECISION or COLLECTIVE_DECISION or BOTH
   - **is_amend_code** is this action set for amend code
---

```c++
ACTION setsoleexec(
        name community_account, 
        uint64_t code_id, 
        bool is_amend_code,
        vector<name> right_accounts,
        vector<uint64_t> right_pos_ids
    );
```

## community::set right holder for sole execution type of code
### Description:
- set right holder for sole exection type of code
- The one who satisfy the requirement can execute code directly without voting

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **right_accounts** the account right holder
   - **right_pos_ids** the position ids right holder
   - **is_amend_code** is this action set for amend code
---

```c++
ACTION setproposer(
        name community_account,
        uint64_t code_id,
        bool is_amend_code,
        vector<name> right_accounts,
        vector<uint64_t> right_pos_id
    );
```

## community::set requirement for the one who can create proposal
### Description:
- set requirement for the one who can create proposal in case code has execution type is COLLECTIVE_DECISION

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **right_accounts** the account right holder
   - **right_pos_ids** the position ids right holder
   - **is_amend_code** is this action set for amend code
---

```c++
ACTION setapprotype(
        name community_account,
        uint64_t code_id,
        bool is_amend_code,
        uint8_t approval_type,
    );
```

## community::set approval type for code proposal
### Description:
- if approval_type is SOLE_APPROVAL, the one who meet the requirement can approve and execute code action directly
- if approval_type is APPROVAL_CONSENSUS, the one who meet the requirement can vote for the proposal

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **approval_type** approval type of the proposal, can be SOLE_APPROVAL or APPROVAL_CONSENSUS
   - **is_amend_code** is this action set for amend code
---

```c++
ACTION setapprover(
        name community_account,
        uint64_t code_id,
        bool is_amend_code,
        vector<name> right_accounts,
        vector<uint64_t> right_pos_ids
    );
```

## community::set requirement for the one who can create approve for proposal
### Description:
- set requirement for the one who can approve the proposal in case code has execution type is COLLECTIVE_DECISION
- action can only use for code with approval_type is SOLE_APPROVAL, the one who meet the requirement can approve and execute code action directly

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **right_accounts** the account right holder
   - **right_pos_ids** the position ids right holder
   - **is_amend_code** is this action set for amend code
---

```c++
ACTION setapprover(
        name community_account,
        uint64_t code_id,
        bool is_amend_code,
        vector<name> right_accounts,
        vector<uint64_t> right_pos_ids
    );
```

## community::set requirement for the one who can create approve for proposal
### Description:
- set requirement for the one who can vote in case code has execution type is COLLECTIVE_DECISION
- action can only use for code with approval_type is APPROVAL_CONSENSUS, the one who meet the requirement can vote for proposal

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **right_accounts** the account right holder
   - **right_pos_ids** the position ids right holder
   - **is_amend_code** is this action set for amend code
---

```c++
ACTION setvoterule(
        name community_account,
        uint64_t code_id,
        bool is_amend_code,
        double pass_rule,
        uint64_t vote_duration
    ); 
```

## community::set vote rule for code 
### Description:
- set vote rule for code in case that code has execution type is COLLECTIVE_DECISION and 

### Parameters:
   - **community_account** community account
   - **code_id** name of the code
   - **vote_duration** the duration for voting
   - **pass_rule** the percentage of voting to be accepted
   - **is_amend_code** is this action set for amend code
---

```c++
ACTION execcode(name community_account, name exec_account, uint64_t code_id, name code_action, vector<char> packed_params);
```

## community::execcode execute a code
### Description:
- the middle action to execute the code
- action will check the right holder of executor, and call inline action to the contract and action of executing code under community account permission
- ram fee of this action will be paid by community account

### Parameters:
   - **community_account** community account
   - **exec_account** right holder account (account who sign to execute this action)
   - **code_id** the code's id
   - **code_action** the code's action
   - **packed_params** the code's action which was convered to binary 

```c++
ACTION execproposal(name community_account, name proposal_name);
```
## community::execproposal execute a code with type election decision
### Description:
- execute code proposal
- voting for this proposal must be ended
- propopsal must be execute before execution expire time
- this action will check the proposal status and call inline action of this code

### Parameters:
   - **codeid** id of code
   - **proposal_name** the proposal name

---

```c++
ACTION voteforcode(name community_account, name proposal_name, name approver, bool vote_status);
```

## community::voteforcode vote or approve for the proposal of code
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
```c++
ACTION community::createpos(name community_account, name creator, string pos_name, uint64_t max_holder, uint8_t filled_through)
```

## community::createpos create new position
### Description:

### Parameters:
   - **community_account** community account
   - **creator** community account
   - **pos_name** position's name
   - **max_holder** the maximum holder for position
   - **filled_through** how to fill the holder for this position, 0 APOINTMENT, 1 ELECTION
   
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
  double pass_rule, vector<name> pos_candidate_accounts, 
  vector<name> pos_voter_accounts, 
  vector<uint64_t> pos_candidate_positions, 
  vector<uint64_t> pos_voter_positions);
```

```c++
ACTION appointpos(
  name community_account,
  uint64_t pos_id,
  vector<name> holder_accounts,
  const string& appoint_reason);
```

## community::appointpos 
   - executor: community
   - **community_account** community account
   - **pos_id** the pos's id
   - **holder_accounts** the candidate list

```c++
ACTION nominatepos(name community_account, uint64_t pos_id, name owner);
```

## community::nominatepos 
   - executor: Position right holder
   - **community_account** community account
   - **pos_id** the poisition's id
   - **owner** nominate themself to an position.

```c++
ACTION voteforpos(name community_account, uint64_t pos_id, name voter, name candidate, bool vote_status);
```

## community::voteforpos 
   - executor: voter refer community::setfillrule:right_accounts
   - **community_account** community account
   - **pos_id** the poisition's id
   - **candidate** the position right holder
   - **vote_status** 0: UNVOTE, 1: VOTE

```c++
ACTION approvepos(name community_account, uint64_t pos_id);
```

## community::approvepos The action should be executed after the election
   - executor: community
   - **community_account** community account
   - **pos_id** the code's id
  
```c++
ACTION dismisspos(name community_account, uint64_t pos_id, name holder, const string& dismissal_reason);
```

## ACTION dissmisspos(name community_account, uint64_t pos_id, name holder);
   - executor: community
   - **community_account** community account
   - **pos_id** the poisition's id
   - **holder** the holder's account

```c++
ACTION createbadge(name community_account, name badge_propose_name);
```

```c++
ACTION issuebadge(name community_account, name badge_propose_name);
```
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


### use amend code to config code code_execution_right

*Note*: Each code created will has two execution right:

  - code execution right: is the requirement for the one who has right to execute this code store in `codevoterule` and `codeexecrule` table
  - amendment execution right: is the requirement for the one who has right to set right for code_execution_right and amendment_execution_right store in `amenvoterule` and `amenexecrule` table

#### set right holder for code execution right

1. pack setexectype action data to set execution type of `co.access` with code_id 3 to collective decision (1):

```bash
// last parameter is identify set for amendment code (1 is set for amendment code, 0 is set for code)
$ cleos convert pack_action_data governance23 setexectype '["community241", 2, 1, 0]'
2088f0d94d2d254501000000000000000100
```

2. pack `setapprotype` action data to set approval type of code proposal (approval type 1 is approval consensus):
```bash
$ cleos convert pack_action_data governance23 setapprotype '["community241", 2, 0, 1]'
2088f0d94d2d254501000000000000000001
```

3. pack setproposer action data to set proposer right holder of `co.access` with code_id 3:

```bash
$ cleos convert pack_action_data governance23 setproposer '["community241", 2, 0, ["quyvoplayers"], []]'
2088f0d94d2d25450100000000000000000180aff22656babdb600
```

4. pack action data `setvoter` to set voter of proposal:

```bash
cleos convert pack_action_data governance23 setvoter '["community241", 2, 0, ["quyvoplayers"], []]'
2088f0d94d2d25450100000000000000000180aff22656babdb600
```

5. pack action data `setvoterule` to set vote rule of proposal:
```bash
cleos convert pack_action_data governance23 setvoterule '["community241", 2, 0, 60, 600]'
2088f0d94d2d25450100000000000000000000000000004e405802000000000000
```

6. use action `execcode` to execute above action, amend execution type is the special case, even `setexectype`, `setproposer`, `setvoter`, `setvoterule` action is not exist in the code, but `execcode` action with automatically know it is use for changing the execiton type:

```bash
cleos push action governance23 execcode '["community241", "quocleplayer", 2, [["setexectype", "2088f0d94d2d254501000000000000000100"], ["setapprotype", "2088f0d94d2d254501000000000000000001"], ["setproposer", "2088f0d94d2d25450100000000000000000180aff22656babdb600"], ["setvoter", "2088f0d94d2d25450100000000000000000180aff22656babdb600"], ["setvoterule", "2088f0d94d2d25450100000000000000000000000000004e405802000000000000"]]]' -p quocleplayer
```


7. get code and codevoterule table to check:

```bash
$ cleos get table governance23 community241 codes
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

$ cleos get table governance23 community241 codevoterule
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

---

### create new code

1. pack action data to create new code
```bash
cleos convert pack_action_data governance23 createcode '["community241", "testcode", "governance23", ["testaction"]]'
2088f0d94d2d25450000002a5194b1ca308442d3ccab36650100c0a42e2393b1ca
```

2. find the code id of co.amend code, use secondary index to find code by code name:

```bash
cleos get table governance23 community241 codes --index 2 --key-type i64 -L co.amend -U co.amend
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
cleos push action governance23 execcode '[community241, quocleplayer, 0, [[createcode, "2088f0d94d2d25450000002a5194b1ca308442d3ccab36650100c0a42e2393b1ca"]]]' -p quocleplayer
```

4. get table to check again:
```bash
cleos get table governance23 community241 codes --index 2 --key-type i64 -L testcode -U testcode
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
cleos get table governance23 community241 codes --index 2 --key-type i64 -L po.create -U po.create
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
cleos convert pack_action_data governance23 createpos '["community241", "quocleplayer", "Badge Manager", 3, 0]' 
2088f0d94d2d2545709537b1aa88a8b60d4261646765204d616e61676572030000000000000000
```

3. Because the code is `Collective decision` type we need to use `proposecode` to create proposal ( in case of `sole decision` we can call `execcode` to execute it directly), the proposal need to pay the ram for this proposal:

create proposal:
```bash
cleos push action governance23 proposecode '["community241", "quocleplayer", "newpos", 1, [["createpos", "2088f0d94d2d2545709537b1aa88a8b60d4261646765204d616e61676572030000000000000000"]]]' -p quocleplayer
```

4. Voter vote for proposal

To get all proposals of community:

```bash
cleos get table governance23 community241 coproposals
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
cleos push action governance23 voteforcode '["community241", "newpos", quocleplayer, 1]' -p quocleplayer

// creator.can agree proposal
cleos push action governance23 voteforcode '["community241", "newpos", creator.can, 1]' -p creator.can
```

5. After voting has been finished, call execproposal action to execute proposal:

To get all proposals of community:

```bash
cleos get table governance23 community241 coproposals
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
cleos push action governance23 execproposal '[community241, newpos]' -p quocleplayer
```

6. Get position table to check:

Three corresponding code of positions have been created. `po.appoint` with code id 7, `po.config` with code id 7, `po.dismiss` with code id 6 and `po.dissmiss` with code id 8.

```bash
cleos get table governance23 community241 positions
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

cleos get table governance23 community233 codes -L 6 -U 8
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
