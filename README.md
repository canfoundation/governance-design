# governance-design

## Version : 2.0.0

This repository is a provided following contracts:

   * [community](./community)


Dependencies:
* [can v1.0.x](https://github.com/canfoundation/CAN/releases/tag/can-v1.0.0)
* [eosio.cdt v1.7.x](https://github.com/EOSIO/eosio.cdt/releases/tag/v1.7.0)
* [crypto-badge v2.0.x](https://github.com/canfoundation/cryptobadge/releases/tag/v2.0.0)

## Build

```
cd community

eosio-cpp -abigen -I include -contract community -o community.wasm src/community.cpp
```
