from uuid import uuid4
from time import sleep
import logging
import sys
import worth_utils.worth_runner
import worth_utils.worth_tools


LOG_LEVEL = logging.INFO
LOG_FORMAT = "%(asctime)-15s - %(name)s - %(levelname)s - %(message)s"
MAIN_LOG_PATH = "./wps_tester_utils.log"

MODULE_NAME = "WPS-Tester-via-worthpy.WPS-Tester-Utils"
logger = logging.getLogger(MODULE_NAME)
logger.setLevel(LOG_LEVEL)


# create_account "initminer" "pychol" "" true
def create_accounts(node, creator, accounts):
    for account in accounts:
        logger.info("Creating account: {}".format(account['name']))
        node.commit.create_account(account['name'], 
            owner_key=account['public_key'], 
            active_key=account['public_key'], 
            posting_key=account['public_key'],
            memo_key=account['public_key'],
            store_keys = False,
            creator=creator,
            asset='TESTS'
        )
    worth_utils.worth_tools.wait_for_blocks_produced(5, node.url)


# transfer_to_vesting initminer pychol "310.000 TESTS" true
def transfer_to_vesting(node, from_account, accounts, amount, asset):
    for acnt in accounts:
        logger.info("Transfer to vesting from {} to {} amount {} {}".format(
            from_account, acnt['name'], amount, asset)
        )
        
        node.commit.transfer_to_vesting(amount, to = acnt['name'], 
            account = from_account, asset = asset
        )
    worth_utils.worth_tools.wait_for_blocks_produced(5, node.url)


# transfer initminer pychol "399.000 TESTS" "initial transfer" true
# transfer initminer pychol "398.000 TBD" "initial transfer" true
def transfer_assets_to_accounts(node, from_account, accounts, amount, asset):
    for acnt in accounts:
        logger.info("Transfer from {} to {} amount {} {}".format(from_account, 
            acnt['name'], amount, asset)
        )
        node.commit.transfer(acnt['name'], amount, asset, 
            memo = "initial transfer", account = from_account
        )
    worth_utils.worth_tools.wait_for_blocks_produced(5, node.url)


def transfer_assets_to_treasury(node, from_account, treasury_account, amount, asset):
    logger.info("Transfer from {} to {} amount {} {}".format(from_account, 
        treasury_account, amount, asset)
    )
    node.commit.transfer(treasury_account, amount, asset, 
        memo = "initial transfer", account = from_account
    )
    worth_utils.worth_tools.wait_for_blocks_produced(2, node.url)


def get_permlink(account):
    return "worthpy-proposal-title-{}".format(account)


def create_posts(node, accounts):
    logger.info("Creating posts...")
    for acnt in accounts:
        logger.info("New post ==> ({},{},{},{},{})".format(
            "Worthpy proposal title [{}]".format(acnt['name']), 
            "Worthpy proposal body [{}]".format(acnt['name']), 
            acnt['name'], 
            get_permlink(acnt['name']), 
            "proposals"
        ))
        node.commit.post("Worthpy proposal title [{}]".format(acnt['name']), 
            "Worthpy proposal body [{}]".format(acnt['name']), 
            acnt['name'], 
            permlink = get_permlink(acnt['name']), 
            tags = "proposals")
    worth_utils.worth_tools.wait_for_blocks_produced(5, node.url)


def create_proposals(node, proposals):
    logger.info("Creating proposals...")
    for proposal in proposals:
        logger.info("New proposal ==> ({},{},{},{},{},{},{})".format(
            proposal['creator'], 
            proposal['receiver'], 
            proposal['start_date'], 
            proposal['end_date'],
            proposal['daily_pay'],
            "Proposal from account {}".format(proposal['creator']),
            get_permlink(proposal['creator'])
        ))
        node.commit.create_proposal(
            proposal['creator'], 
            proposal['receiver'], 
            proposal['start_date'], 
            proposal['end_date'],
            proposal['daily_pay'],
            "Proposal from account {}".format(proposal['creator']),
            get_permlink(proposal['creator'])
        )
    worth_utils.worth_tools.wait_for_blocks_produced(5, node.url)


def vote_proposals(node, accounts):
    logger.info("Voting proposals...")
    for acnt in accounts:
        proposal_set = [x for x in range(0, len(accounts))]
        logger.info("Account {} voted for proposals: {}".format(acnt["name"], ",".join(str(x) for x in proposal_set)))
        node.commit.update_proposal_votes(acnt["name"], proposal_set, True)
    worth_utils.worth_tools.wait_for_blocks_produced(5, node.url)


def list_proposals(node, start_date, status):
    proposals = node.list_proposals(start_date, "by_start_date", "direction_ascending", 1000, status)
    ret = []
    votes = []
    for proposal in proposals:
        ret.append("{}:{}".format(proposal.get('id', 'Error'), proposal.get('total_votes', 'Error')))
        votes.append(int(proposal.get('total_votes', -1)))
    logger.info("Listing proposals with status {} (id:total_votes): {}".format(status, ",".join(ret)))
    return votes


def print_balance(node, accounts):
    balances = []
    balances_str = []
    for acnt in accounts:
        ret = node.get_account(acnt['name'])
        wbd = ret.get('wbd_balance', 'Error')
        balances_str.append("{}:{}".format(acnt['name'], wbd))
        balances.append(wbd)
    logger.info("Balances ==> {}".format(",".join(balances_str)))
    return balances


def date_to_iso(date):
    return date.replace(microsecond=0).isoformat()


def date_from_iso(date_iso_str):
    import dateutil.parser
    return dateutil.parser.parse(date_iso_str)


def get_start_and_end_date(now, start_days_from_now, end_days_from_start):
    from datetime import timedelta
    start_date = now + timedelta(days = start_days_from_now)
    end_date = start_date + timedelta(days = end_days_from_start)

    assert start_date > now, "Start date must be greater than now"
    assert end_date > start_date, "End date must be greater than start date"

    return date_to_iso(start_date), date_to_iso(end_date)