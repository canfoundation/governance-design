# governance-design

## Version : 1.0.0

The design of the EOSIO blockchain calls for a number of smart contracts that are run at a privileged permission level in order to support functions such as block producer registration and voting, token staking for CPU and network bandwidth, RAM purchasing, multi-sig, etc.  These smart contracts are referred to as the bios, system, msig, wrap (formerly known as sudo) and token contracts.

They are provided for reference purposes:

   * [community](./community)


Dependencies:
* [eosio v1.8.x](https://github.com/EOSIO/eos/releases/tag/v1.8.0-rc2)
* [eosio.cdt v1.7.x](https://github.com/EOSIO/eosio.cdt/releases/tag/v1.7.0)

To build the contracts 

cd community
eosio-cpp -abigen -I include -contract community -o community.wasm src/community.cpp
