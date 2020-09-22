import { loadConfig, Blockchain } from '@klevoya/hydra';
import { initGovernanceContract } from './testHelper';
import { CodeTypeEnum, ExecutionType, FillingType, ProposalStatus } from './type/smart-contract-enum';

const config = loadConfig("hydra.yml");

describe("test verify holder", () => {
  let blockchain = new Blockchain(config);
  let governanceContract;
  let communityCreator;
  const communityAccountName = 'community1.c';
  let communityAccount;
  const positionCandidate1 = blockchain.createAccount('candidate1');
  const positionCandidate2 = blockchain.createAccount('candidate2');
  const positionVoter1 = blockchain.createAccount('posvoter1');
  const positionVoter2 = blockchain.createAccount('posvoter2');
  const positionVoter3 = blockchain.createAccount('posvoter3');

  let newCreatedPositionId;

  beforeAll(async () => {
    const initAccount = await initGovernanceContract(blockchain, communityAccountName);
    governanceContract = initAccount.governanceContract;
    communityCreator = initAccount.communityCreator;
    communityAccount = initAccount.communityAccount;
  });

  beforeEach(async () => {
    // governanceContract.resetTables();
  });

  it("should create new position", async () => {
    // add account to community member
    const codeTable = await governanceContract.getTableRowsScoped('v1.code')[communityAccount.accountName];
    const createPositionCode = codeTable.find(code => code.code_name === 'po.create');

    const inputActionData = {
      community_account: communityAccount.accountName,
      creator: communityCreator.accountName,
      pos_name: 'test position',
      max_holder: 9,
      filled_through: FillingType.ELECTION,
      term: 1,
      next_term_start_at: 949363200,
      voting_period: 2591999,
      right_candidate: {
        is_anyone: false,
        is_any_community_member: false,
        required_badges: [],
        required_positions: [],
        required_tokens: [],
        required_exp: 0,
        accounts: [positionCandidate1.accountName, positionCandidate2.accountName],
      },
      right_voter: {
        is_anyone: false,
        is_any_community_member: false,
        required_badges: [],
        required_positions: [],
        required_tokens: [],
        required_exp: 0,
        accounts: [positionVoter1.accountName, positionVoter2.accountName, positionVoter3.accountName],
      },
    };

    const serializeActionData = '8040f0d94d2d2545003256f9d26cd4450d7465737420706f736974696f6e09000000000000000101000000000000000022963800000000ff8c2700000000000000000000000000000000000002004050d92497a641008050d92497a64100000000000000000000000000030000085765ba31ad0000105765ba31ad0000185765ba31ad';
    await governanceContract.contract.execcode({
      community_account: communityAccount.accountName,
      exec_account: communityCreator.accountName,
      code_id: createPositionCode.code_id,
      code_actions: [{
        code_action: 'createpos',
        packed_params: serializeActionData,
      }],
    }, [{
      actor: communityCreator.accountName,
      permission: 'active',
    }]);

    const positions = governanceContract.getTableRowsScoped(
      `v1.position`
    )[communityAccount.accountName];


    const newCreatedPosition = positions.find(p => p.pos_name === inputActionData.pos_name);

    expect(newCreatedPosition).toBeTruthy();
    expect(newCreatedPosition.pos_name).toBe(inputActionData.pos_name);
    expect(+newCreatedPosition.max_holder).toBe(inputActionData.max_holder);
    expect(+newCreatedPosition.fulfillment_type).toBe(inputActionData.filled_through);

    newCreatedPositionId = newCreatedPosition.pos_id;

    const fillingRuleTable = governanceContract.getTableRowsScoped(
      `v1.filling`
    )[communityAccount.accountName];

    const newPositionFillingRule = fillingRuleTable.find(f => f.pos_id === newCreatedPositionId);

    expect(newPositionFillingRule).toBeTruthy();
    expect(+newPositionFillingRule.term).toBe(inputActionData.term);
    expect(newPositionFillingRule.next_term_start_at).toBe((new Date(inputActionData.next_term_start_at * 1000)).toISOString().substring(0, 23));
    expect(+newPositionFillingRule.voting_period).toBe(inputActionData.voting_period);
    expect(newPositionFillingRule.pos_candidates.accounts).toEqual(inputActionData.right_candidate.accounts);
    expect(newPositionFillingRule.pos_voters.accounts).toEqual(inputActionData.right_voter.accounts);
  });

  it('should init code of position when create position', async () => {
    const positions = governanceContract.getTableRowsScoped(
      `v1.position`
    )[communityAccount.accountName];

    const codeTable = governanceContract.getTableRowsScoped(
      `v1.code`
    )[communityAccount.accountName];

    const newCreatedPosition = positions.find(p => p.pos_id === newCreatedPositionId);

    const configPositionCodeId = newCreatedPosition.refer_codes.find(r => r.key === 'po.config');
    const appointPositionCodeId = newCreatedPosition.refer_codes.find(r => r.key === 'po.appoint');
    const dismissPositionCodeId = newCreatedPosition.refer_codes.find(r => r.key === 'po.dismiss');

    const configPositionCode = codeTable.find(c => c.code_id === configPositionCodeId.value);
    expect(configPositionCode).toBeTruthy();
    expect(configPositionCode.code_name).toBe('po.config');
    expect(configPositionCode.code_actions).toEqual(['configpos']);
    expect(configPositionCode.contract_name).toEqual(governanceContract.accountName);
    expect(configPositionCode.code_exec_type).toEqual(ExecutionType.SOLE_DECISION);
    expect(configPositionCode.amendment_exec_type).toEqual(ExecutionType.SOLE_DECISION);
    expect(configPositionCode.code_type.type).toEqual(CodeTypeEnum.POSITION_CONFIG);
    expect(configPositionCode.code_type.refer_id).toEqual(newCreatedPositionId);

    const appointPositionCode = codeTable.find(c => c.code_id === appointPositionCodeId.value);
    expect(appointPositionCode).toBeTruthy();
    expect(appointPositionCode.code_name).toBe('po.appoint');
    expect(appointPositionCode.code_actions).toEqual(['appointpos']);
    expect(appointPositionCode.contract_name).toEqual(governanceContract.accountName);
    expect(appointPositionCode.code_exec_type).toEqual(ExecutionType.SOLE_DECISION);
    expect(appointPositionCode.amendment_exec_type).toEqual(ExecutionType.SOLE_DECISION);
    expect(appointPositionCode.code_type.type).toEqual(CodeTypeEnum.POSITION_APPOINT);
    expect(appointPositionCode.code_type.refer_id).toEqual(newCreatedPositionId);

    const dismissPositionCode = codeTable.find(c => c.code_id === dismissPositionCodeId.value);
    expect(dismissPositionCode).toBeTruthy();
    expect(dismissPositionCode.code_name).toBe('po.dismiss');
    expect(dismissPositionCode.code_actions).toEqual(['dismisspos']);
    expect(dismissPositionCode.contract_name).toEqual(governanceContract.accountName);
    expect(dismissPositionCode.code_exec_type).toEqual(ExecutionType.SOLE_DECISION);
    expect(dismissPositionCode.amendment_exec_type).toEqual(ExecutionType.SOLE_DECISION);
    expect(dismissPositionCode.code_type.type).toEqual(CodeTypeEnum.POSITION_DISMISS);
    expect(dismissPositionCode.code_type.refer_id).toEqual(newCreatedPositionId);
  });

  it('should nominate for position', async () => {
    const testingNominationPositionId = 999;
    const positionProposalId = '1';
    await governanceContract.loadFixtures(`v1.position`, {
      'community1.c': [{
        pos_id: testingNominationPositionId,
          pos_name: 'test vote',
        max_holder: 1,
        holders: [],
        fulfillment_type: FillingType.ELECTION,
        refer_codes: [
        {
          key: 'po.config',
          value: 16
        },
        {
          key: 'po.appoint',
          value: 17
        },
        {
          key: 'po.dismiss',
          value: 18
        },
      ]
      }]
    });
    await governanceContract.loadFixtures(`v1.filling`, {
      'community1.c': [{
        pos_id: testingNominationPositionId,
        term: 1,
        next_term_start_at: '2000-01-02T00:00:01',
        voting_period: 172804,
        pos_candidates: {
          is_anyone: false,
          is_any_community_member: false,
          required_badges: [],
          required_positions: [],
          required_tokens: [],
          required_exp: 0,
          accounts: [positionCandidate1.accountName, positionCandidate2.accountName],
        },
        pos_voters: {
          is_anyone: false,
          is_any_community_member: false,
          required_badges: [],
          required_positions: [],
          required_tokens: [],
          required_exp: 0,
          accounts: [positionVoter1.accountName, positionVoter2.accountName, positionVoter3.accountName],
        },
      }]
    });

    await governanceContract.loadFixtures(`v1.pproposal`, {
      'community1.c': [{
        pos_id: testingNominationPositionId,
        pos_proposal_id: positionProposalId,
        pos_proposal_status: ProposalStatus.IN_PROGRESS,
        approved_at: '1970-01-01T00:00:00'
      }]
    });

    await governanceContract.contract.nominatepos({
      community_account: communityAccount.accountName,
      pos_id: testingNominationPositionId,
      owner: positionCandidate1.accountName
    },[{
      actor: positionCandidate1.accountName,
      permission: 'active',
    }]);

    await governanceContract.contract.nominatepos({
      community_account: communityAccount.accountName,
      pos_id: testingNominationPositionId,
      owner: positionCandidate2.accountName
    },[{
      actor: positionCandidate2.accountName,
      permission: 'active',
    }]);

    const positionCandidateTable = governanceContract.getTableRowsScoped(
      `v1.candidate`
    )[positionProposalId.padStart(13, '.')];

    const newCandidate1 =  positionCandidateTable.find(c => c.cadidate === positionCandidate1.accountName);

    expect(newCandidate1).toBeTruthy();
    expect(newCandidate1.cadidate).toBe(positionCandidate1.accountName);
    expect(newCandidate1.voters.length).toBe(0);

    const newCandidate2 =  positionCandidateTable.find(c => c.cadidate === positionCandidate2.accountName);

    expect(newCandidate2).toBeTruthy();
    expect(newCandidate2.cadidate).toBe(positionCandidate2.accountName);
    expect(newCandidate2.voters.length).toBe(0);
  });

  it('should vote for position candidate', async () => {
    const testingNominationPositionId = 999;
    const positionProposalId = '1';
    // voter1 vote for candidate1;
    await governanceContract.contract.voteforpos({
      community_account: communityAccount.accountName,
      pos_id: testingNominationPositionId,
      voter: positionVoter1.accountName,
      candidate: positionCandidate1.accountName,
      vote_status: 1,
    },[{
      actor: positionVoter1.accountName,
      permission: 'active',
    }]);

    let positionCandidateTable = governanceContract.getTableRowsScoped(
      `v1.candidate`
    )[positionProposalId.padStart(13, '.')];

    let candidate1 =  positionCandidateTable.find(c => c.cadidate === positionCandidate1.accountName);

    expect(candidate1.cadidate).toBe(positionCandidate1.accountName);
    expect(candidate1.voters).toEqual([positionVoter1.accountName]);

    // voter2 vote for candidate1;
    await governanceContract.contract.voteforpos({
      community_account: communityAccount.accountName,
      pos_id: testingNominationPositionId,
      voter: positionVoter2.accountName,
      candidate: positionCandidate1.accountName,
      vote_status: 1,
    },[{
      actor: positionVoter2.accountName,
      permission: 'active',
    }]);

    positionCandidateTable = governanceContract.getTableRowsScoped(
      `v1.candidate`
    )[positionProposalId.padStart(13, '.')];

    candidate1 =  positionCandidateTable.find(c => c.cadidate === positionCandidate1.accountName);

    expect(candidate1.cadidate).toBe(positionCandidate1.accountName);
    expect(candidate1.voters).toEqual([positionVoter1.accountName, positionVoter2.accountName]);
  });

  it('should reject the position candidate', async () => {
    const testingNominationPositionId = 999;
    const positionProposalId = '1';
    // voter1 vote for candidate1;
    await governanceContract.contract.voteforpos({
      community_account: communityAccount.accountName,
      pos_id: testingNominationPositionId,
      voter: positionVoter1.accountName,
      candidate: positionCandidate1.accountName,
      vote_status: 0,
    },[{
      actor: positionVoter1.accountName,
      permission: 'active',
    }]);

    let positionCandidateTable = governanceContract.getTableRowsScoped(
      `v1.candidate`
    )[positionProposalId.padStart(13, '.')];

    let candidate1 =  positionCandidateTable.find(c => c.cadidate === positionCandidate1.accountName);

    expect(candidate1.cadidate).toBe(positionCandidate1.accountName);
    expect(candidate1.voters).not.toContain([positionVoter1.accountName]);
  });

  it('should throw error if vote for position candidate that is not exist', async () => {
    const testingNominationPositionId = 999;
    // voter1 vote for not exist candidate;
    await expect(governanceContract.contract.voteforpos({
      community_account: communityAccount.accountName,
      pos_id: testingNominationPositionId,
      voter: positionVoter1.accountName,
      candidate: 'notexistcan',
      vote_status: 1,
    },[{
      actor: positionVoter1.accountName,
      permission: 'active',
    }])).rejects.toThrowError('ERR::CANDIDATE_NOT_ESIXT::The candidate does not exist');
  });
});
