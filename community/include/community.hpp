#include <eosio/eosio.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/print.hpp>
#include <eosio/asset.hpp>
#include <eosio/symbol.hpp>
#include <eosio/singleton.hpp>
#include <eosio/crypto.hpp>
#include <math.h>
// #include "exchange_state.hpp"

using namespace eosio;
using namespace std;

// #include "code.hpp"
// #include "position.hpp"
// #include "badge.hpp"

class [[eosio::contract("eosio.system")]] community : public badge
{
  struct permission_level_weight {
    permission_level  permission;
    uint16_t          weight;

    // explicit serialization macro is not necessary, used here only to improve compilation time
    EOSLIB_SERIALIZE( permission_level_weight, (permission)(weight) )
  };

  struct key_weight {
    eosio::public_key  key;
    uint16_t           weight;

    // explicit serialization macro is not necessary, used here only to improve compilation time
    EOSLIB_SERIALIZE( key_weight, (key)(weight) )
  };

  struct wait_weight {
    uint32_t           wait_sec;
    uint16_t           weight;

    // explicit serialization macro is not necessary, used here only to improve compilation time
    EOSLIB_SERIALIZE( wait_weight, (wait_sec)(weight) )
  };

  struct authority {
    uint32_t                              threshold = 0;
    std::vector<key_weight>               keys;
    std::vector<permission_level_weight>  accounts;
    std::vector<wait_weight>              waits;

    // explicit serialization macro is not necessary, used here only to improve compilation time
    EOSLIB_SERIALIZE( authority, (threshold)(keys)(accounts)(waits) )
  };

  public:
    community(eosio::name receiver, eosio::name code_name, datastream<const char *> ds) : badge(receiver, code_name, ds), _communitys(_self, _self.value) {}

    void transfer(name from, name to, asset quantity, string memo);

    ACTION createacc(name community_creator, name community_acc);

    ACTION create(name creator, name community_account, string & community_name, vector<uint64_t> member_badge, string & community_url, string & description, bool create_default_code);
  
  private:
    eosio::asset convertbytes2cat(uint32_t bytes);

    bool is_token_holder(vector<asset> token_ids, name owner);

    TABLE communityf
    {
        name community_account;
        name creator;
        string community_name;
        vector<uint64_t> member_badge;
        string community_url;
        string description;

        uint64_t by_creator() const { return creator.value; }
        uint64_t primary_key() const { return community_account.value; }
    };

    typedef eosio::multi_index<"community"_n, communityf, indexed_by< "by.creator"_n, const_mem_fun<communityf, uint64_t, &communityf::by_creator>>> community_table;

    // refer from tiger token contract
    TABLE account {
        asset    balance;

        uint64_t primary_key()const { return balance.symbol.code().raw(); }
    };

    typedef eosio::multi_index< "accounts"_n, account > accounts;

    community_table _communitys;
};