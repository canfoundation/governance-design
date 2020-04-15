
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
cleos -u http://18.163.57.224:8888 transfer quocle governance33 "10.0000 CAT" "quocle1.c" -p quocle

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