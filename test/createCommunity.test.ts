import { loadConfig, Blockchain } from "@klevoya/hydra";

const config = loadConfig("hydra.yml");

describe("test create community", () => {
  let blockchain = new Blockchain(config);
  let eosio = blockchain.accounts["eosio"];
  let governanceContract = blockchain.createAccount(`governance`);
  let tokenContract = blockchain.createAccount(`eosio.token`);
  let communityCreator = blockchain.createAccount(`creatortest`);
  let cAccount = blockchain.createAccount(`c`);
  const communityAccountName = "community1.c";

  beforeAll(async () => {
    await Promise.all([
      blockchain.createAccount("eosio.bpay"),
      blockchain.createAccount("eosio.msig"),
      blockchain.createAccount("eosio.names"),
      blockchain.createAccount("eosio.ram"),
      blockchain.createAccount("eosio.ramfee"),
      blockchain.createAccount("eosio.saving"),
      blockchain.createAccount("eosio.stake"),
      blockchain.createAccount("eosio.vpay"),
      blockchain.createAccount("eosio.rex"),
      tokenContract.setContract(blockchain.contractTemplates[`eosio.token`]),
      governanceContract.setContract(blockchain.contractTemplates[`community`]),
      eosio.setContract(blockchain.contractTemplates["eosio.system"]),
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
      })
    ]);

    await tokenContract.contract.create(
      {
        issuer: eosio.accountName,
        maximum_supply: "10000000000.0000 CAT"
      },
      [
        {
          actor: tokenContract.accountName,
          permission: "active"
        }
      ]
    );

    await tokenContract.contract.issue(
      {
        to: eosio.accountName,
        quantity: "10000000000.0000 CAT",
        memo: "test"
      },
      [
        {
          actor: eosio.accountName,
          permission: "active"
        }
      ]
    );

    await eosio.contract.init(
      {
        version: 0,
        core: "4,CAT"
      },
      [
        {
          actor: eosio.accountName,
          permission: "active"
        }
      ]
    );

    await tokenContract.contract.transfer(
      {
        from: eosio.accountName,
        to: governanceContract.accountName,
        quantity: "1000.0000 CAT",
        memo: "deposit_core_symbol"
      },
      [
        {
          actor: eosio.accountName,
          permission: "active"
        }
      ]
    );

    await tokenContract.contract.transfer(
        {
          from: eosio.accountName,
          to: communityCreator.accountName,
          quantity: "1000.0000 CAT",
          memo: "deposit_core_symbol"
        },
        [
          {
            actor: eosio.accountName,
            permission: "active"
          }
        ]
      );

    await governanceContract.contract.setconfig(
        {
          community_creator_name: "c",
          cryptobadge_contract_name: "badge",
          token_contract_name: "eosio.token",
          ram_payer_name: "ram.can",
          core_symbol: "4,CAT",
          init_ram_amount: 10000,
          min_active_contract: "10.0000 CAT",
          init_net: "1.0000 CAT",
          init_cpu: "1.0000 CAT"
        },
        [
          {
            actor: governanceContract.accountName,
            permission: "active"
          }
        ]
    );
  });

  beforeEach(async () => {
    // governanceContract.resetTables();
  });

  it("should create community", async () => {
    const transferToken = {
      from: communityCreator.accountName,
      to: governanceContract.accountName,
      quantity: "10.0000 CAT",
      memo: communityAccountName
    };

    await tokenContract.contract.transfer(transferToken, [
      {
        actor: communityCreator.accountName,
        permission: "active"
      }
    ]);

    const communityTableItem = governanceContract.getTableRowsScoped(
      `v1.community`
    )[governanceContract.accountName][0];

    expect(communityTableItem.community_account).toBe(communityAccountName);
    expect(communityTableItem.creator).toBe(communityCreator.accountName);
  }, 100000);

  it("should update community information", async () => {
    const communityInfor = {
      creator: communityCreator.accountName,
      community_account: communityAccountName,
      community_name: "community test",
      member_badge: [],
      community_url: "http://google.com/community-test",
      description: "community to test",
      create_default_code: true
    };

    await governanceContract.contract.create(communityInfor, [
      {
        actor: communityCreator.accountName,
        permission: "active"
      }
    ]);

    const communityTableItem = governanceContract.getTableRowsScoped(
      `v1.community`
    )[governanceContract.accountName][0];

    expect(communityTableItem.community_name).toBe(
      communityInfor.community_name
    );
    expect(communityTableItem.community_url).toBe(communityInfor.community_url);
  });

  it("should create position and its code when create commumity", async () => {
    const communityPositions = governanceContract.getTableRowsScoped(
      `v1.position`
    )[communityAccountName];

    const communityCodes = governanceContract.getTableRowsScoped(`v1.code`)[
      communityAccountName
    ];

    // expect that admin position is created when create community
    expect(communityPositions.length).toBe(1);
    expect(communityPositions[0].pos_name).toBe("Admin");
    expect(Number(communityPositions[0].max_holder)).toBe(10);
    expect(Number(communityPositions[0].pos_id)).toBe(1);

    // expect that three position code is created for admin positon
    const configAdminPositionCode = communityCodes.find(
      code => code.code_name === "po.config"
    );
    expect(configAdminPositionCode).toBeTruthy();
    expect(configAdminPositionCode.code_type).toEqual({
      type: 1,
      refer_id: "1"
    });

    const appointAdminPositionCode = communityCodes.find(
      code => code.code_name === "po.appoint"
    );
    expect(appointAdminPositionCode).toBeTruthy();
    expect(appointAdminPositionCode.code_type).toEqual({
      type: 2,
      refer_id: "1"
    });

    const dismissAdminPositionCode = communityCodes.find(
      code => code.code_name === "po.dismiss"
    );
    expect(dismissAdminPositionCode).toBeTruthy();
    expect(dismissAdminPositionCode.code_type).toEqual({
      type: 3,
      refer_id: "1"
    });
  });

  it("should create default code for community", async () => {
    let execRule;
    const communityCodes = governanceContract.getTableRowsScoped(`v1.code`)[
      communityAccountName
    ];
    const soleExecRule = governanceContract.getTableRowsScoped(`v1.codeexec`)[
      communityAccountName
    ];

    // expect default code for community is created
    const amendCode = communityCodes.find(
      code => code.code_name === "co.amend"
    );
    expect(amendCode).toBeTruthy();
    expect(amendCode.code_actions).toEqual(["createcode"]);
    execRule = soleExecRule.find(rule => rule.code_id === amendCode.code_id);
    expect(execRule.right_executor.required_positions).toEqual(["1"]);

    const memberCode = communityCodes.find(
      code => code.code_name === "co.members"
    );
    expect(memberCode).toBeTruthy();
    expect(memberCode.code_actions).toEqual(["inputmembers"]);
    execRule = soleExecRule.find(rule => rule.code_id === memberCode.code_id);
    expect(execRule.right_executor.required_positions).toEqual(["1"]);

    const positionCreateCode = communityCodes.find(
      code => code.code_name === "po.create"
    );
    expect(positionCreateCode).toBeTruthy();
    expect(positionCreateCode.code_actions).toEqual(["createpos"]);
    execRule = soleExecRule.find(
      rule => rule.code_id === positionCreateCode.code_id
    );
    expect(execRule.right_executor.required_positions).toEqual(["1"]);

    const accessCode = communityCodes.find(
      code => code.code_name === "co.access"
    );
    expect(accessCode).toBeTruthy();
    expect(accessCode.code_actions).toEqual(["setaccess"]);
    execRule = soleExecRule.find(rule => rule.code_id === accessCode.code_id);
    expect(execRule.right_executor.required_positions).toEqual(["1"]);

    const badgeCreateCode = communityCodes.find(
      code => code.code_name === "ba.create"
    );
    expect(badgeCreateCode).toBeTruthy();
    expect(badgeCreateCode.code_actions).toEqual(["createbadge"]);
    execRule = soleExecRule.find(
      rule => rule.code_id === badgeCreateCode.code_id
    );
    expect(execRule.right_executor.required_positions).toEqual(["1"]);
  });
});
