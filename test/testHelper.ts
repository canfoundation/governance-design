export async function initGovernanceContract(blockchain, communityAccountName) {
  const initAccount = await Promise.all([
    blockchain.createAccount(`governance`),
    blockchain.createAccount(`eosio.token`),
    blockchain.createAccount(`creatortest`),
    blockchain.createAccount(`c`),
    blockchain.createAccount(communityAccountName),
  ]);

  const governanceContract = initAccount[0];
  const tokenContract = initAccount[1];
  const communityCreator = initAccount[2];
  const cAccount = initAccount[3];
  const communityAccount = initAccount[4];

  await Promise.all([
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
    }),
    cAccount.updateAuth(`active`, `owner`, {
      accounts: [
        {
          permission: {
            actor: governanceContract.accountName,
            permission: `eosio.code`
          },
          weight: 1
        }
      ]
    }),
    communityAccount.updateAuth(`active`, `owner`, {
      accounts: [
        {
          permission: {
            actor: governanceContract.accountName,
            permission: `eosio.code`
          },
          weight: 1
        }
      ]
    })
  ]);

  governanceContract.setContract(blockchain.contractTemplates[`community`]);

  await governanceContract.contract.setconfig({
    community_creator_name: 'c',
    cryptobadge_contract_name: 'badge',
    token_contract_name: 'eosio.token',
    ram_payer_name: 'ram.can',
    core_symbol: '4,CAT',
    init_ram_amount: 10000,
    min_active_contract: '10.0000 CAT',
    init_net: '1.0000 CAT',
    init_cpu: '1.0000 CAT',
  });

  await governanceContract.loadFixtures();

  return { governanceContract, tokenContract, communityCreator, cAccount, communityAccount };
}
