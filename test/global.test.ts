import { loadConfig, Blockchain } from "@klevoya/hydra";

const config = loadConfig("hydra.yml");

describe("test global configuration", () => {
  let blockchain = new Blockchain(config);
  let governanceContract = blockchain.createAccount(`governance`);

  beforeAll(async () => {
    governanceContract.setContract(blockchain.contractTemplates[`community`]);
    governanceContract.updateAuth(`active`, `owner`, {
      accounts: [
        {
          permission: {
            actor: governanceContract.accountName,
            permission: `eosio.code`
          },
          weight: 1
        }
      ]
    });
  });

  beforeEach(async () => {
    governanceContract.resetTables();
  });

  it("should set governance contract configuration", async () => {
    const newConfig = {
      community_creator_name: 'c',
      cryptobadge_contract_name: 'badge',
      token_contract_name: 'eosio.token',
      ram_payer_name: 'ram.can',
      core_symbol: '4,CAT',
      init_ram_amount: 10000,
      min_active_contract: '10.000 CAT',
      init_net: '1.0000 CAT',
      init_cpu: '1.0000 CAT',
    };

    await governanceContract.contract.setconfig(newConfig);

    const contractConfig = governanceContract.getTableRowsScoped(`v1.global`)[governanceContract.accountName][0];

    expect(contractConfig.community_creator_name).toBe(newConfig.community_creator_name);
    expect(contractConfig.cryptobadge_contract_name).toBe(newConfig.cryptobadge_contract_name);
    expect(contractConfig.token_contract_name).toBe(newConfig.token_contract_name);
    expect(contractConfig.init_cpu).toBe(newConfig.init_cpu);
  });
});
