import { loadConfig, Blockchain } from "@klevoya/hydra";
import { initGovernanceContract } from './testHelper'

const config = loadConfig("hydra.yml");

describe("test accession code", () => {
  const blockchain = new Blockchain(config);
  let governanceContract;
  let communityCreator;
  const communityAccountName = 'community1.c';
  let communityAccount;

  beforeAll(async () => {
    const initAccount = await initGovernanceContract(blockchain, communityAccountName);
    governanceContract = initAccount.governanceContract;
    communityCreator = initAccount.communityCreator;
    communityAccount = initAccount.communityAccount;
  });

  beforeEach(async () => {
    // governanceContract.resetTables();
  });

  it("should set access code", async () => {
    const rightAccess = {
      is_anyone: false,
      is_any_community_member: false,
      required_badges: [],
      required_positions: [],
      required_tokens: [],
      required_exp: 0,
      accounts: ['daniel123123'],
    };

    const codeTable = await governanceContract.getTableRowsScoped('v1.code')[communityAccount.accountName];
    const accessCode = codeTable.find(code => code.code_name === 'co.access');

    const serializeActionData = '8040f0d94d2d254500000000000000000000000000013044182244e5a649';
    await governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: accessCode.code_id,
      code_actions: [{
        code_action: 'setaccess',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);

    const communityAccessionItem = governanceContract.getTableRowsScoped(`v1.access`)[communityAccount.accountName][0];

    expect(communityAccessionItem.right_access.is_anyone).toBe(rightAccess.is_anyone);
    expect(communityAccessionItem.right_access.accounts).toContain(rightAccess.accounts[0]);
    expect(communityAccessionItem.right_access.required_positions).toEqual(rightAccess.required_positions);
  });
});
