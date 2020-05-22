import { loadConfig, Blockchain } from "@klevoya/hydra";
import { initGovernanceContract } from './testHelper'

const config = loadConfig("hydra.yml");

describe("test execute code", () => {
  let blockchain = new Blockchain(config);
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

  it("should not allow to execute code if user does not have permission", async () => {
    const anotherAccount = blockchain.createAccount('anotheracc');
    const codeTable = await governanceContract.getTableRowsScoped('v1.code')[communityAccountName];
    const accessCode = codeTable.find(code => code.code_name === 'co.access');

    const serializeActionData = '8040f0d94d2d254500000000000000000000000000013044182244e5a649';
    await expect(governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: anotherAccount.accountName,
      code_id: accessCode.code_id,
      code_actions: [{
        code_action: 'setaccess',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: anotherAccount.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::VERIFY_FAILED::Owner doesn\'t belong to code\'s right holder.');
  });

  it("should throw error if execute code with code id is not exist", async () => {
    const serializeActionData = '8040f0d94d2d254500000000000000000000000000013044182244e5a649';
    await expect(governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: '9999999',
      code_actions: [{
        code_action: 'setaccess',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::VERIFY_FAILED::Code doesn\'t exist.');
  });

  it("should throw error if execute code if action is not exist", async () => {
    const codeTable = await governanceContract.getTableRowsScoped('v1.code')[communityAccountName];
    const accessCode = codeTable.find(code => code.code_name === 'co.access');

    const serializeActionData = '8040f0d94d2d254500000000000000000000000000013044182244e5a649';
    await expect(governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: accessCode.code_id,
      code_actions: [{
        code_action: 'notaction',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::VERIFY_FAILED::Action doesn\'t exist');
  });
});
