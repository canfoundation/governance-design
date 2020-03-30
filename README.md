# governance-design

## Version : 1.1.0

This repository is a provided following contracts:

   * [community](./community)


Dependencies:
* [can v1.0.x](https://github.com/canfoundation/CAN/releases/tag/can-v1.0.0)
* [eosio.cdt v1.7.x](https://github.com/EOSIO/eosio.cdt/releases/tag/v1.7.0)

To build the contracts 

```
cd community

eosio-cpp -abigen -I include -contract community -o community.wasm src/community.cpp
```
