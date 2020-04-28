import { loadConfig, Blockchain } from "@klevoya/hydra";
import { initGovernanceContract } from './testHelper'
import { ApprovalType, ExecutionType } from './type/smart-contract-enum';

const config = loadConfig("hydra.yml");

describe("test code", () => {
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

  it("should test create new code", async () => {
    const codeInfo = {
      community_account: communityAccount.accountName,
      code_name: 'codetest',
      contract_name: 'contracttest',
      code_actions: ['testaction1', 'testaction2'],
    };

    let codeTable = await governanceContract.getTableRowsScoped('v1.code')[communityAccount.accountName];
    const amendmentCode = codeTable.find(code => code.code_name === 'co.amend');

    const serializeActionData = '8040f0d94d2d254500000019abac124590b1ca19999b27450200c2a42e2393b1ca00c4a42e2393b1ca';
    await governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: amendmentCode.code_id,
      code_actions: [{
        code_action: 'createcode',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);

    codeTable = await governanceContract.getTableRowsScoped('v1.code')[communityAccount.accountName];
    const newCreatedCode = codeTable.find(code => code.code_name === codeInfo.code_name);

    expect(newCreatedCode).toBeTruthy();
    expect(newCreatedCode.contract_name).toBe(codeInfo.contract_name);
    expect(newCreatedCode.code_actions).toEqual(codeInfo.code_actions);
  });

  it("should test set sole execution right holder for code", async () => {
    const setSoleExecution = {
      community_account: communityAccount.accountName,
      code_id: 14,
      right_sole_executor: {
        is_anyone: false,
        is_any_community_member: false,
        required_badges: ['10'],
        required_positions: ['1', '99'],
        required_tokens: [],
        required_exp: 0,
        accounts: [
          'daniel111111',
          'danieltest12'
        ]
      },
      is_amend_code: false,
    };

    const serializeActionData = '8040f0d94d2d25450e00000000000000000000010a000000000000000201000000000000006300000000000000000000000000000000021042082144e5a6492042c62a47e5a649';
    await governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: setSoleExecution.code_id,
      code_actions: [{
        code_action: 'setsoleexec',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);

    const codeExecutionRuleTable = await governanceContract.getTableRowsScoped('v1.codeexec')[communityAccount.accountName];
    const changedCode = codeExecutionRuleTable.find(code => Number(code.code_id) === setSoleExecution.code_id);

    expect(changedCode).toBeTruthy();
    expect(changedCode.right_executor.is_anyone).toBe(setSoleExecution.right_sole_executor.is_anyone);
    expect(changedCode.right_executor.required_badges).toEqual(setSoleExecution.right_sole_executor.required_badges);
    expect(changedCode.right_executor.required_positions).toEqual(setSoleExecution.right_sole_executor.required_positions);
    expect(changedCode.right_executor.accounts).toEqual(setSoleExecution.right_sole_executor.accounts);
  });

  it("should test set proposer right holder for code", async () => {
    const setProposer = {
      community_account: communityAccount.accountName,
      code_id: 14,
      right_proposer: {
        is_anyone: false,
        is_any_community_member: false,
        required_badges: ['10'],
        required_positions: ['1', '99'],
        required_tokens: [],
        required_exp: 0,
        accounts: [
          'daniel111111',
          'danieltest12'
        ]
      },
      is_amend_code: false,
    };

    const serializeActionData = '8040f0d94d2d25450e00000000000000000000010a000000000000000201000000000000006300000000000000000000000000000000021042082144e5a6492042c62a47e5a649';
    await governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: setProposer.code_id,
      code_actions: [{
        code_action: 'setproposer',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);

    const codeExecutionRuleTable = await governanceContract.getTableRowsScoped('v1.codevote')[communityAccount.accountName];
    const changedCode = codeExecutionRuleTable.find(code => Number(code.code_id) === setProposer.code_id);

    expect(changedCode).toBeTruthy();
    expect(changedCode.right_proposer.is_anyone).toBe(setProposer.right_proposer.is_anyone);
    expect(changedCode.right_proposer.required_badges).toEqual(setProposer.right_proposer.required_badges);
    expect(changedCode.right_proposer.required_positions).toEqual(setProposer.right_proposer.required_positions);
    expect(changedCode.right_proposer.accounts).toEqual(setProposer.right_proposer.accounts);
  });

  it("should test set approval type of collective rule for code", async () => {
    const setApprotype = {
      community_account: communityAccount.accountName,
      code_id: 14,
      approval_type: ApprovalType.SOLE_APPROVAL,
      is_amend_code: false,
    };

    const serializeActionData = '8040f0d94d2d25450e000000000000000000';
    await governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: setApprotype.code_id,
      code_actions: [{
        code_action: 'setapprotype',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);

    const codeExecutionRuleTable = await governanceContract.getTableRowsScoped('v1.codevote')[communityAccount.accountName];
    const changedCode = codeExecutionRuleTable.find(code => Number(code.code_id) === setApprotype.code_id);

    expect(changedCode).toBeTruthy();
    expect(changedCode.approval_type).toBe(setApprotype.approval_type);
  });

  it("should test set set approver of collective rule for code", async () => {
    const setApprover = {
      community_account: communityAccount.accountName,
      code_id: 14,
      right_approver: {
        is_anyone: false,
        is_any_community_member: false,
        required_badges: ['10'],
        required_positions: ['1', '99'],
        required_tokens: [],
        required_exp: 0,
        accounts: [
          'daniel111111',
          'danieltest12'
        ]
      },
      is_amend_code: false,
    };

    const serializeActionData = '8040f0d94d2d25450e00000000000000000000010a000000000000000201000000000000006300000000000000000000000000000000021042082144e5a6492042c62a47e5a649';
    await governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: setApprover.code_id,
      code_actions: [{
        code_action: 'setapprover',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);

    const codeExecutionRuleTable = await governanceContract.getTableRowsScoped('v1.codevote')[communityAccount.accountName];
    const changedCode = codeExecutionRuleTable.find(code => Number(code.code_id) === setApprover.code_id);

    expect(changedCode).toBeTruthy();
    expect(changedCode.right_approver.is_anyone).toBe(setApprover.right_approver.is_anyone);
    expect(changedCode.right_approver.required_badges).toEqual(setApprover.right_approver.required_badges);
    expect(changedCode.right_approver.required_positions).toEqual(setApprover.right_approver.required_positions);
    expect(changedCode.right_approver.accounts).toEqual(setApprover.right_approver.accounts);
  });

  it("should test set set voter of collective rule for code", async () => {
    const setVoter = {
      community_account: communityAccount.accountName,
      code_id: 14,
      right_voter: {
        is_anyone: false,
        is_any_community_member: false,
        required_badges: ['10'],
        required_positions: ['1', '99'],
        required_tokens: [],
        required_exp: 0,
        accounts: [
          'daniel111111',
          'danieltest12'
        ]
      },
      is_amend_code: false,
    };

    // because of just approval consensus code can set voter, we need to set code to approval consensus first
    let serializeActionData = '8040f0d94d2d25450e000000000000000001';
    await governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: setVoter.code_id,
      code_actions: [{
        code_action: 'setapprotype',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);

    serializeActionData = '8040f0d94d2d25450e00000000000000000000010a000000000000000201000000000000006300000000000000000000000000000000021042082144e5a6492042c62a47e5a649';
    await governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: setVoter.code_id,
      code_actions: [{
        code_action: 'setvoter',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);

    const codeExecutionRuleTable = await governanceContract.getTableRowsScoped('v1.codevote')[communityAccount.accountName];
    const changedCode = codeExecutionRuleTable.find(code => Number(code.code_id) === setVoter.code_id);

    expect(changedCode).toBeTruthy();
    expect(changedCode.right_approver.is_anyone).toBe(setVoter.right_voter.is_anyone);
    expect(changedCode.right_approver.required_badges).toEqual(setVoter.right_voter.required_badges);
    expect(changedCode.right_approver.required_positions).toEqual(setVoter.right_voter.required_positions);
    expect(changedCode.right_approver.accounts).toEqual(setVoter.right_voter.accounts);
  });

  it("should test set vote rule of collective rule for code", async () => {
    const setVoteRule = {
      community_account: communityAccount.accountName,
      code_id: 14,
      pass_rule: 99,
      vote_duration: 60000,
    };

    const serializeActionData = '8040f0d94d2d25450e00000000000000000000000000c0584060ea000000000000';
    await governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: setVoteRule.code_id,
      code_actions: [{
        code_action: 'setvoterule',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);

    const codeExecutionRuleTable = await governanceContract.getTableRowsScoped('v1.codevote')[communityAccount.accountName];
    const changedCode = codeExecutionRuleTable.find(code => Number(code.code_id) === setVoteRule.code_id);

    expect(changedCode).toBeTruthy();
    expect(Number(changedCode.vote_duration)).toBe(setVoteRule.vote_duration);
    expect(Number(changedCode.pass_rule)).toBe(setVoteRule.pass_rule);
  });
});
