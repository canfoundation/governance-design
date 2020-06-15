import { loadConfig, Blockchain } from '@klevoya/hydra';
import { initGovernanceContract } from './testHelper';
import { ExecutionType } from './type/smart-contract-enum'

const config = loadConfig("hydra.yml");

describe("test verify holder", () => {
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

  it("should allow to execute code if right holder is any one", async () => {
    // code is initialized in /initDataTest/v1.code.json
    const anyOneCodeId = 10;
    const anotherAccount = blockchain.createAccount('abcabcabc123');
    await governanceContract.contract.verifyholder({
      community_account: communityAccount.accountName,
      code_id: anyOneCodeId,
      execution_type: ExecutionType.SOLE_DECISION,
      owner: anotherAccount.accountName,
      is_ammend_holder: 0,
    });
  });

  it("should allow to execute code if right holder is any community member", async () => {
    // code is initialized in /initDataTest/v1.code.json
    const anyMemberCodeId = 11;
    const memberAccount = blockchain.createAccount('member123123');

    // add account to community member
    const codeTable = await governanceContract.getTableRowsScoped('v1.code')[communityAccount.accountName];
    const memberCode = codeTable.find(code => code.code_name === 'co.members');

    const serializeActionData = '8040f0d94d2d254501304418225c75a49200';
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

    // verify holder
    await governanceContract.contract.verifyholder({
      community_account: communityAccount.accountName,
      code_id: anyMemberCodeId,
      execution_type: ExecutionType.SOLE_DECISION,
      owner: memberAccount.accountName,
      is_ammend_holder: 0,
    });
  });

  it("should throw error if execute code with right of not a community member", async () => {
    // code is initialized in /initDataTest/v1.code.json
    const anyMemberCodeId = 11;
    const notMemberAccount = blockchain.createAccount('notmember123');

    // verify holder
    await expect(governanceContract.contract.verifyholder({
      community_account: communityAccount.accountName,
      code_id: anyMemberCodeId,
      execution_type: ExecutionType.SOLE_DECISION,
      owner: notMemberAccount.accountName,
      is_ammend_holder: 0,
    })).rejects.toThrowError('ERR::VERIFY_FAILED::Owner doesn\'t belong to code\'s right holder');
  });

  it("should throw error if execute code with actor does not have required badges", async () => {
    // code is initialized in /initDataTest/v1.code.json
    const requiredBadgeCodeId = 12;
    const notBadgeAccount = blockchain.createAccount('notbadge1234');

    // verify holder
    await expect(governanceContract.contract.verifyholder({
      community_account: communityAccount.accountName,
      code_id: requiredBadgeCodeId,
      execution_type: ExecutionType.SOLE_DECISION,
      owner: notBadgeAccount.accountName,
      is_ammend_holder: 0,
    })).rejects.toThrowError('ERR::VERIFY_FAILED::Owner doesn\'t belong to code\'s right holder');
  });

  it("should allow to execute code if actor is holder of required position", async () => {
    // code is initialized in /initDataTest/v1.code.json
    const requiredPositionCodeId = 13;

    // verify holder
    await governanceContract.contract.verifyholder({
      community_account: communityAccount.accountName,
      code_id: requiredPositionCodeId,
      execution_type: ExecutionType.SOLE_DECISION,
      owner: communityCreator.accountName, // creator is admin position holder
      is_ammend_holder: 0,
    });
  });

  it("should throw error if execute code with actor is not holder of required position", async () => {
    // code is initialized in /initDataTest/v1.code.json
    const requiredPositionCodeId = 13;
    const notAdminPositionAccount = blockchain.createAccount('notadmin1234');

    // verify holder
    await expect(governanceContract.contract.verifyholder({
      community_account: communityAccount.accountName,
      code_id: requiredPositionCodeId,
      execution_type: ExecutionType.SOLE_DECISION,
      owner: notAdminPositionAccount.accountName, // creator is admin position holder
      is_ammend_holder: 0,
    })).rejects.toThrowError('ERR::VERIFY_FAILED::Owner doesn\'t belong to code\'s right holder');
  });

  it("should allow to execute code if actor is one of account in required account", async () => {
    // code is initialized in /init-data-test/v1.code.json
    const requireAccountCodeId = 14;
    // right account is initialized in /init-data-test/v1.codeexec.json
    const rightAccount = blockchain.createAccount('daniel111111');

    // verify holder
    await governanceContract.contract.verifyholder({
      community_account: communityAccount.accountName,
      code_id: requireAccountCodeId,
      execution_type: ExecutionType.SOLE_DECISION,
      owner: rightAccount.accountName, // creator is admin position holder
      is_ammend_holder: 0,
    });
  });

  it("should throw error if execute code with actor is not required account", async () => {
    // code is initialized in /initDataTest/v1.code.json
    const requireAccountCodeId = 14;
    const notRequiredAccount = blockchain.createAccount('notaccount12');

    // verify holder
    await expect(governanceContract.contract.verifyholder({
      community_account: communityAccount.accountName,
      code_id: requireAccountCodeId,
      execution_type: ExecutionType.SOLE_DECISION,
      owner: notRequiredAccount.accountName, // creator is admin position holder
      is_ammend_holder: 0,
    })).rejects.toThrowError('ERR::VERIFY_FAILED::Owner doesn\'t belong to code\'s right holder');
  });
});
