import { loadConfig, Blockchain } from '@klevoya/hydra';
import { initGovernanceContract } from './testHelper';

const config = loadConfig("hydra.yml");

describe("test verify holder", () => {
  let blockchain = new Blockchain(config);
  let governanceContract;
  let communityCreator;
  const communityAccountName = 'community1.c';
  let communityAccount;
  const memberAccount1 = blockchain.createAccount('member123123');
  const memberAccount2 = blockchain.createAccount('member345345');

  beforeAll(async () => {
    const initAccount = await initGovernanceContract(blockchain, communityAccountName);
    governanceContract = initAccount.governanceContract;
    communityCreator = initAccount.communityCreator;
    communityAccount = initAccount.communityAccount;
  });

  beforeEach(async () => {
    // governanceContract.resetTables();
  });

  it("should input new member to community", async () => {
    // add account to community member
    const codeTable = await governanceContract.getTableRowsScoped('v1.code')[communityAccount.accountName];
    const memberCode = codeTable.find(code => code.code_name === 'co.members');

    const inputActionData = {
      community_account: communityAccount.accountName,
      added_members: [memberAccount1.accountName, memberAccount2.accountName],
      removed_members: [],
    };

    const serializeActionData = '8040f0d94d2d254502304418225c75a49250c828645c75a49200';
    await governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: memberCode.code_id,
      code_actions: [{
        code_action: 'inputmembers',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);

    const communityMembers = governanceContract.getTableRowsScoped(
      `v1.member`
    )[communityAccount.accountName];

    expect(communityMembers).toContainEqual({ member: inputActionData.added_members[0] });
    expect(communityMembers).toContainEqual({ member: inputActionData.added_members[1] });
  });

  it("should throw error if input members already existed", async () => {
    // add account to community member
    const codeTable = await governanceContract.getTableRowsScoped('v1.code')[communityAccount.accountName];
    const memberCode = codeTable.find(code => code.code_name === 'co.members');

    const inputActionData = {
      community_account: communityAccount.accountName,
      added_members: [memberAccount1.accountName, memberAccount2.accountName],
      removed_members: [],
    };

    const serializeActionData = '8040f0d94d2d254502304418225c75a49250c828645c75a49200';
    await expect(governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: memberCode.code_id,
      code_actions: [{
        code_action: 'inputmembers',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::MEMBER_ALREADY_EXIST::At least one of the member already exist.');
  });

  it("should remove member of community", async () => {
    // add account to community member
    const codeTable = await governanceContract.getTableRowsScoped('v1.code')[communityAccount.accountName];
    const memberCode = codeTable.find(code => code.code_name === 'co.members');

    const inputActionData = {
      community_account: communityAccount.accountName,
      removed_members: [memberAccount1.accountName],
    };
    const serializeActionData = '8040f0d94d2d25450001304418225c75a492';
    await governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: memberCode.code_id,
      code_actions: [{
        code_action: 'inputmembers',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);

    const communityMembers = governanceContract.getTableRowsScoped(
      `v1.member`
    )[communityAccount.accountName];

    // check that removed_members is remove from table
    expect(communityMembers).not.toContainEqual({ member: inputActionData.removed_members[0] });
    // check that others user is still remain in table
    expect(communityMembers).toContainEqual({ member: memberAccount2.accountName });
  });

  it("should throw error if remove member that does not exist", async () => {
    // add account to community member
    const codeTable = await governanceContract.getTableRowsScoped('v1.code')[communityAccount.accountName];
    const memberCode = codeTable.find(code => code.code_name === 'co.members');

    const inputActionData = {
      community_account: communityAccount.accountName,
      removed_members: [memberAccount1.accountName],
    };
    const serializeActionData = '8040f0d94d2d25450001304418225c75a492';
    await expect(governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: memberCode.code_id,
      code_actions: [{
        code_action: 'inputmembers',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::MEMBER_NOT_FOUND::At least one of the member was not found.');
  });
});
