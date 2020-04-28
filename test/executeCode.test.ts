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

  it("should throw error if use code of another community to execute code set right holder", async () => {
    const anotherCommunityName = 'another1.c';
    const setExecTypeData = {
      community_account: anotherCommunityName,
      code_id: 3,
      exec_type: 1,
      is_amend_code: 0,
    };
    const serializeActionData = '000002e1aa96e93403000000000000000100';
    await expect(governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: setExecTypeData.code_id,
      code_actions: [{
        code_action: 'setexectype',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::INVALID_PACKED_COMMUNITY_ACCOUNT_PARAM::Specified community account not match with community account in packed params');
  });

  it("should throw error if use code of another amendment code to execute code set right holder", async () => {
    const setExecTypeData = {
      community_account: communityAccount,
      code_id: 3,
      exec_type: 1,
      is_amend_code: 0,
    };

    const serializeActionData = '8040f0d94d2d254503000000000000000100';
    await expect(governanceContract.contract.execcode({
      community_account: setExecTypeData.community_account.accountName,
      exec_account: communityCreator.accountName,
      code_id: 4,
      code_actions: [{
        code_action: 'setexectype',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::INVALID_PACKED_CODE_ID_ACCOUNT_PARAM::Specified code id not match with code id in packed params');
  });

  it("should throw error if use code of another community to execute normal code", async () => {
    const anotherCommunityName = 'another1.c';
    const createCode = {
      community_account: anotherCommunityName,
      code_name: 'testcode',
      contract_name: 'governance',
      code_actions: ['testaction'],
    };

    const serializeActionData = '000002e1aa96e9340000002a5194b1ca008042d3ccab36650100c0a42e2393b1ca';
    await expect(governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: 0,
      code_actions: [{
        code_action: 'createcode',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::INVALID_PACKED_COMMUNITY_ACCOUNT_PARAM::Specified community account not match with community account in packed params');
  });

  it("should throw error if config badge use incorrect badge code", async () => {
    const configBadge = {
      community_account: communityAccount,
      badge_id: 800,
      issue_type: 1,
      update_badge_proposal_name: '',
    };

    const configBadgeCodeOfBadgeId799 = 17;

    const serializeActionData = '8040f0d94d2d25457803000000000000010000000000000000';
    await expect(governanceContract.contract.execcode({
      community_account: configBadge.community_account.accountName,
      exec_account: communityCreator.accountName,
      code_id: configBadgeCodeOfBadgeId799,
      code_actions: [{
        code_action: 'configbadge',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR:INVALID_BADGE_POSITION_CODE::Please use correct code to execute badge/position action');
  });
});
