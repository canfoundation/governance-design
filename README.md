# governance-design

## Version : 3.0.0

This repository is a provided following contracts:

   * [community](./community)


Dependencies:
* [can v1.0.x](https://github.com/canfoundation/CAN/releases/tag/can-v1.0.0)
* [eosio.cdt v1.7.x](https://github.com/EOSIO/eosio.cdt/releases/tag/v1.7.0)
* [crypto-badge v2.0.x](https://github.com/canfoundation/cryptobadge/releases/tag/v2.0.0)

Documents:

* [Example use cleos](https://github.com/canfoundation/governance-design/blob/master/community/Readme.md)

## Build

*NOTE* For production use, make sure that code has been commented out `#define IS_TEST`, line 1, file `community.hpp`. It will not generate init data test action of smart contract. 
```
cd community

eosio-cpp -abigen -I include -contract community -o community.wasm src/community.cpp
```
## How to run unit test


1. Install eosio testing framework [hydra](https://docs.klevoya.com/hydra/about/getting-started) and login into it:

```bash
npm i -g @klevoya/hydra

hydra login
```

2. Install yarn packages:

```bash
yarn install
```

3. Run test:

Run all unit test

```bash
yarn test
```

Run test file:

```bash
yarn test createCommunity.test.ts
```

***WARING*** Because test need to send many request to hydra server, test timeout error may sometime happend. Make sure your network connection is stable to run this test.
