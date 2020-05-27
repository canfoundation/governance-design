import { loadConfig, Blockchain } from '@klevoya/hydra';
import { initGovernanceContract } from './testHelper';
import { ApprovalType } from './type/smart-contract-enum';

const config = loadConfig("hydra.yml");

describe("test create/vote/execute proposal", () => {
  let blockchain = new Blockchain(config);
  let governanceContract;
  let communityCreator;
  const communityAccountName = 'community1.c';
  let communityAccount;
  const proposer = blockchain.createAccount('proposer');
  const approver = blockchain.createAccount('approver');
  const voter1 = blockchain.createAccount('voter11');
  const voter2 = blockchain.createAccount('voter22');
  const voter3 = blockchain.createAccount('voter44');

  const codeAmendId = 0;
  const proposalName = 'testproposal';

  const collectiveDecisionRule = {
    code_id: codeAmendId,
    vote_duration: 0,
    pass_rule: 65,
    approval_type: ApprovalType.BOTH_TYPE,
    right_proposer: {
      is_anyone: false,
      is_any_community_member: false,
      required_badges: [],
      required_positions: [],
      required_tokens: [],
      required_exp: 0,
      accounts: [proposer.accountName],
    },
    right_approver: {
      is_anyone: false,
      is_any_community_member: false,
      required_badges: [],
      required_positions: [],
      required_tokens: [],
      required_exp: 0,
      accounts: [approver.accountName],
    },
    right_voter: {
      is_anyone: false,
      is_any_community_member: false,
      required_badges: [],
      required_positions: [],
      required_tokens: [],
      required_exp: 0,
      accounts: [voter1.accountName, voter2.accountName, voter3.accountName],
    },
  };

  beforeAll(async () => {
    const initAccount = await initGovernanceContract(blockchain, communityAccountName);
    governanceContract = initAccount.governanceContract;
    communityCreator = initAccount.communityCreator;
    communityAccount = initAccount.communityAccount;

    const setExecTypePackData = '8040f0d94d2d254500000000000000000100';
    const setApprovalTypePackData = '8040f0d94d2d254500000000000000000002';
    const setProposerPackData = '8040f0d94d2d2545000000000000000000000000000000000000000000000100000057615ae9ad';
    const setApproverPackData = '8040f0d94d2d25450000000000000000000000000000000000000000000001000000576d7a6b35';
    const setVoterPackData = '8040f0d94d2d254500000000000000000000000000000000000000000000030000002084ab32dd0000004088ab32dd000000608cab32dd';
    const setVoteRulePackData = '8040f0d94d2d254500000000000000000000000000004050400100000000000000';
    await governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: collectiveDecisionRule.code_id,
      code_actions: [{
        code_action: 'setexectype',
        packed_params: setExecTypePackData,
      },{
        code_action: 'setapprotype',
        packed_params: setApprovalTypePackData,
      },{
        code_action: 'setapprover',
        packed_params: setApproverPackData,
      },{
        code_action: 'setproposer',
        packed_params: setProposerPackData,
      },{
        code_action: 'setvoter',
        packed_params: setVoterPackData,
      },{
        code_action: 'setvoterule',
        packed_params: setVoteRulePackData,
      }
      ],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);
  });

  beforeEach(async () => {
    // governanceContract.resetTables();
  });

  it("should propose to create new code", async () => {
    const inputActionData = {
      community_account: communityAccount.accountName,
      code_name: 'codetest12',
      contract_name: governanceContract.accountName,
      code_actions: ['testaction1', 'testaction2']
    };

    const serializeActionData = '8040f0d94d2d254500800819abac1245008042d3ccab36650200c2a42e2393b1ca00c4a42e2393b1ca';
    await governanceContract.contract.proposecode({
      community_account: communityAccount.accountName,
      proposer: proposer.accountName,
      proposal_name: proposalName,
      code_id: codeAmendId,
      code_actions: [{
        code_action: 'createcode',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: proposer.accountName,
      permission: 'active',
    }]);

    const codeProposals = governanceContract.getTableRowsScoped(
      `v1.cproposal`
    )[communityAccount.accountName];

    expect(codeProposals[0].proposal_name).toBe(proposalName);
    expect(codeProposals[0].proposer).toBe(proposer.accountName);
    expect(+codeProposals[0].voted_percent).toBe(0);
    expect(+codeProposals[0].code_id).toBe(codeAmendId);
    expect(codeProposals[0].code_actions[0].code_action).toBe('createcode');
    expect(codeProposals[0].code_actions[0].packed_params).toBe(serializeActionData.toUpperCase());
  });

  it("should throw error if create proposal with name that already exist", async () => {
    const inputActionData = {
      community_account: communityAccount.accountName,
      code_name: 'codetest12',
      contract_name: governanceContract.accountName,
      code_actions: ['testaction1', 'testaction2']
    };

    const newProposalName = proposalName;

    const serializeActionData = '8040f0d94d2d254500800819abac1245008042d3ccab36650200c2a42e2393b1ca00c4a42e2393b1ca';
    await expect(governanceContract.contract.proposecode({
      community_account: communityAccount.accountName,
      proposer: proposer.accountName,
      proposal_name: newProposalName,
      code_id: codeAmendId,
      code_actions: [{
        code_action: 'createcode',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: proposer.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::PROPOSAL_NAME_EXIST::The proposal with the this name has already exist');
  });

  it("should throw error if create proposal with code doesn't exist", async () => {
    const inputActionData = {
      community_account: communityAccount.accountName,
      code_name: 'codetest12',
      contract_name: governanceContract.accountName,
      code_actions: ['testaction1', 'testaction2']
    };

    const newProposalName = 'testproposa1';

    const serializeActionData = '8040f0d94d2d254500800819abac1245008042d3ccab36650200c2a42e2393b1ca00c4a42e2393b1ca';
    await expect(governanceContract.contract.proposecode({
      community_account: communityAccount.accountName,
      proposer: proposer.accountName,
      proposal_name: newProposalName,
      code_id: 9999,
      code_actions: [{
        code_action: 'createcode',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: proposer.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::VERIFY_FAILED::Code doesn\'t exist.');
  });

  it("should throw error if create proposal with action doesn't existed", async () => {
    const inputActionData = {
      community_account: communityAccount.accountName,
      code_name: 'codetest12',
      contract_name: governanceContract.accountName,
      code_actions: ['testaction1', 'testaction2']
    };

    const newProposalName = 'testproposa1';

    const serializeActionData = '8040f0d94d2d254500800819abac1245008042d3ccab36650200c2a42e2393b1ca00c4a42e2393b1ca';
    await expect(governanceContract.contract.proposecode({
      community_account: communityAccount.accountName,
      proposer: proposer.accountName,
      proposal_name: newProposalName,
      code_id: codeAmendId,
      code_actions: [{
        code_action: 'abccode',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: proposer.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::VERIFY_FAILED::Action doesn\'t exist.');
  });

  it("should throw error if create proposal with sole decision code", async () => {
    const inputActionData = {
      community_account: communityAccount.accountName,
      code_name: 'codetest12',
      contract_name: governanceContract.accountName,
      code_actions: ['testaction1', 'testaction2']
    };

    // init in test/init-data-test/v1.code.json, position create code is sole decision code
    const positionCreateCode = 1;
    const newProposalName = 'testproposa1';

    const serializeActionData = '8040f0d94d2d254500800819abac1245008042d3ccab36650200c2a42e2393b1ca00c4a42e2393b1ca';
    await expect(governanceContract.contract.proposecode({
      community_account: communityAccount.accountName,
      proposer: proposer.accountName,
      proposal_name: newProposalName,
      code_id: positionCreateCode,
      code_actions: [{
        code_action: 'createpos',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: proposer.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::INVALID_EXEC_TYPE::Can not create proposal for sole decision code');
  });

  it("should throw error if user does not have right to create proposal", async () => {
    const inputActionData = {
      community_account: communityAccount.accountName,
      code_name: 'codetest12',
      contract_name: governanceContract.accountName,
      code_actions: ['testaction1', 'testaction2']
    };

    const newProposalName = 'testproposa1';

    const serializeActionData = '8040f0d94d2d254500800819abac1245008042d3ccab36650200c2a42e2393b1ca00c4a42e2393b1ca';
    await expect(governanceContract.contract.proposecode({
      community_account: communityAccount.accountName,
      // approver doesn't  have right to proposal
      proposer: approver.accountName,
      proposal_name: newProposalName,
      code_id: codeAmendId,
      code_actions: [{
        code_action: 'createcode',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: approver.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::VERIFY_FAILED::Owner doesn\'t belong to code\'s right holder.');
  });

  it("should vote for code proposal", async () => {
    await governanceContract.contract.voteforcode({
      community_account: communityAccount.accountName,
      proposal_name: proposalName,
      approver: voter1.accountName,
      vote_status: 1,
    }, [{
      actor: voter1.accountName,
      permission: 'active',
    }]);

    let codeProposals = governanceContract.getTableRowsScoped(
      `v1.cproposal`
    )[communityAccount.accountName];

    expect(codeProposals[0].voters_detail.length).toBe(1);
    expect(codeProposals[0].voters_detail[0].key).toBe(voter1.accountName);
    expect(+codeProposals[0].voted_percent).toBe(100);

    await governanceContract.contract.voteforcode({
      community_account: communityAccount.accountName,
      proposal_name: proposalName,
      approver: voter2.accountName,
      vote_status: 0,
    }, [{
      actor: voter2.accountName,
      permission: 'active',
    }]);

    codeProposals = governanceContract.getTableRowsScoped(
      `v1.cproposal`
    )[communityAccount.accountName];

    expect(codeProposals[0].voters_detail.length).toBe(2);
    expect(codeProposals[0].voters_detail[1].key).toBe(voter2.accountName);
    expect(+codeProposals[0].voted_percent).toBe(50);
  });

  it("should throw error if vote for proposal that not exist", async () => {
    const notExistProposalName = 'notexist';
    await expect(governanceContract.contract.voteforcode({
      community_account: communityAccount.accountName,
      proposal_name: notExistProposalName,
      approver: voter1.accountName,
      vote_status: 1,
    }, [{
      actor: voter1.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::PROPOSAL_NOT_EXISTED::The proposal is not exist');
  });

  it("should throw error if user does not have right to vote for code", async () => {
    await expect(governanceContract.contract.voteforcode({
      community_account: communityAccount.accountName,
      proposal_name: proposalName,
      // approver does not have right to vote for proposal
      approver: approver.accountName,
      vote_status: 1,
    }, [{
      actor: approver.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::VERIFY_FAILED::You do not have permission to vote for this action.');
  });

  it("should throw error if user does not have right to vote for code", async () => {
    await expect(governanceContract.contract.voteforcode({
      community_account: communityAccount.accountName,
      proposal_name: proposalName,
      // approver does not have right to vote for proposal
      approver: approver.accountName,
      vote_status: 1,
    }, [{
      actor: approver.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::VERIFY_FAILED::You do not have permission to vote for this action.');
  });

  it("should throw error if execute proposal is not exist", async () => {
    const notExistProposal = 'notexist';

    const codeProposals = governanceContract.getTableRowsScoped(
      `v1.cproposal`
    )[communityAccount.accountName];

    await expect(governanceContract.contract.execproposal({
      community_account: communityAccount.accountName,
      proposal_name: notExistProposal,
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::VOTE_NOT_FINISH::Proposal does not exist');
  });

  it("should not execute proposal if voting for proposal have not been finish yet", async () => {
    const inputActionData = {
      community_account: communityAccount.accountName,
      code_name: 'codetest12',
      contract_name: governanceContract.accountName,
      code_actions: ['testaction1', 'testaction2']
    };

    const executeProposalName = 'initproposal';

    const serializeActionData = '8040f0d94d2d254500800819abac1245008042d3ccab36650200c2a42e2393b1ca00c4a42e2393b1ca';
    await governanceContract.loadFixtures(`v1.cproposal`, {
      // scope: [row1, row2, ...],
      'community1.c': [{
        proposal_name: executeProposalName,
        proposer: proposer.accountName,
        voted_percent: 100,
        code_id: 0,
        code_actions: [{
          code_action: 'createcode',
          packed_params: serializeActionData
        }],
        voters_detail: [{
            key: voter1.accountName,
            value: 1,
          },
          {
            key: voter2.accountName,
            value: 1,
          }],
        proposal_status: 0,
        propose_time: Date().toString(),
        exec_at: Date().toString(),
      }],
    });
    await expect(governanceContract.contract.execproposal({
      community_account: communityAccount.accountName,
      proposal_name: executeProposalName,
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::VOTE_NOT_FINISH::The voting proposal for this code has not been finished yet');
  });

  it("should not execute proposal if voting percent less than pass rule", async () => {
    const inputActionData = {
      community_account: communityAccount.accountName,
      code_name: 'codetest12',
      contract_name: governanceContract.accountName,
      code_actions: ['testaction1', 'testaction2']
    };

    const executeProposalName = 'initproposa1';

    const serializeActionData = '8040f0d94d2d254500800819abac1245008042d3ccab36650200c2a42e2393b1ca00c4a42e2393b1ca';
    await governanceContract.loadFixtures(`v1.cproposal`, {
      // scope: [row1, row2, ...],
      'community1.c': [{
        proposal_name: executeProposalName,
        proposer: proposer.accountName,
        // pass rule is 65
        voted_percent: 64,
        code_id: 0,
        code_actions: [{
          code_action: 'createcode',
          packed_params: serializeActionData
        }],
        voters_detail: [{
          key: voter1.accountName,
          value: 1,
        },
          {
            key: voter2.accountName,
            value: 1,
          }],
        proposal_status: 0,
        // server time always 2000-00-00T00:00:00
        propose_time: '1999-12-31T23:59:59',
        exec_at: '1999-12-31T23:59:59',
      }],
    });
    await expect(governanceContract.contract.execproposal({
      community_account: communityAccount.accountName,
      proposal_name: executeProposalName,
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::CODE_NOT_ACCEPTED::This code has not been accepted by voter');
  });

  it("should execute proposal successfully", async () => {
    const inputActionData = {
      community_account: communityAccount.accountName,
      code_name: 'codetest12',
      contract_name: governanceContract.accountName,
      code_actions: ['testaction1', 'testaction2']
    };

    const executeProposalName = 'initproposa2';

    const serializeActionData = '8040f0d94d2d254500800819abac1245008042d3ccab36650200c2a42e2393b1ca00c4a42e2393b1ca';
    await governanceContract.loadFixtures(`v1.cproposal`, {
      // scope: [row1, row2, ...],
      'community1.c': [{
        proposal_name: executeProposalName,
        proposer: proposer.accountName,
        // pass rule is 65
        voted_percent: 70,
        code_id: 0,
        code_actions: [{
          code_action: 'createcode',
          packed_params: serializeActionData
        }],
        voters_detail: [{
            key: voter1.accountName,
            value: 1,
          },
          {
            key: voter2.accountName,
            value: 1,
          }],
        proposal_status: 0,
        // server time always 2000-00-00T00:00:00
        propose_time: '1999-12-31T23:59:59',
        exec_at: '1999-12-31T23:59:59',
      }],
    });
    await governanceContract.contract.execproposal({
      community_account: communityAccount.accountName,
      proposal_name: executeProposalName,
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);

    const codeTable = governanceContract.getTableRowsScoped(
      `v1.code`
    )[communityAccount.accountName];

    const newCreatedCode = codeTable.find(c => c.code_name === inputActionData.code_name);

    expect(newCreatedCode).toBeTruthy();
    expect(newCreatedCode.code_name).toBe(inputActionData.code_name);
    expect(newCreatedCode.contract_name).toBe(inputActionData.contract_name);
    expect(newCreatedCode.code_actions).toEqual(inputActionData.code_actions);
  });
});
