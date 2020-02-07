# governance-design

## Version : 1.0.0

This repository is a provided following contracts:

   * [community](./community)


Dependencies:
* [eosio v1.8.x](https://github.com/EOSIO/eos/releases/tag/v1.8.0-rc2)
* [eosio.cdt v1.7.x](https://github.com/EOSIO/eosio.cdt/releases/tag/v1.7.0)

To build the contracts 

```
cd community

eosio-cpp -abigen -I include -contract community -o community.wasm src/community.cpp
```
