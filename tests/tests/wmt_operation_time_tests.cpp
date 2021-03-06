
#if defined IS_TEST_NET && defined WORTHEUM_ENABLE_WMT

#include <boost/test/unit_test.hpp>

#include <worth/chain/worth_fwd.hpp>

#include <worth/protocol/exceptions.hpp>
#include <worth/protocol/hardfork.hpp>

#include <worth/chain/block_summary_object.hpp>
#include <worth/chain/database.hpp>
#include <worth/chain/history_object.hpp>
#include <worth/chain/worth_objects.hpp>

#include <worth/chain/util/reward.hpp>

#include <worth/plugins/debug_node/debug_node_plugin.hpp>

#include <fc/crypto/digest.hpp>

#include "../db_fixture/database_fixture.hpp"

#include <cmath>

using namespace worth;
using namespace worth::chain;
using namespace worth::protocol;

BOOST_FIXTURE_TEST_SUITE( wmt_operation_time_tests, wmt_database_fixture )

BOOST_AUTO_TEST_CASE( wmt_liquidity_rewards )
{
   using std::abs;

   try
   {
      db->liquidity_rewards_enabled = false;

      ACTORS( (alice)(bob)(sam)(dave)(wmtcreator) )

      //Create WMT and give some WMT to creators.
      signed_transaction tx;
      asset_symbol_type any_wmt_symbol = create_wmt( "wmtcreator", wmtcreator_private_key, 3);

      generate_block();
      vest( WORTH_INIT_MINER_NAME, "alice", ASSET( "10.000 TESTS" ) );
      vest( WORTH_INIT_MINER_NAME, "bob", ASSET( "10.000 TESTS" ) );
      vest( WORTH_INIT_MINER_NAME, "sam", ASSET( "10.000 TESTS" ) );
      vest( WORTH_INIT_MINER_NAME, "dave", ASSET( "10.000 TESTS" ) );

      tx.operations.clear();
      tx.signatures.clear();

      BOOST_TEST_MESSAGE( "Rewarding Bob with TESTS" );

      auto exchange_rate = price( ASSET( "1.250 TESTS" ), asset( 1000, any_wmt_symbol ) );

      const account_object& alice_account = db->get_account( "alice" );
      FUND( "alice", asset( 25522, any_wmt_symbol ) );
      asset alice_wmt = db->get_balance( alice_account, any_wmt_symbol );

      FUND( "alice", alice_wmt.amount );
      FUND( "bob", alice_wmt.amount );
      FUND( "sam", alice_wmt.amount );
      FUND( "dave", alice_wmt.amount );

      int64_t alice_wmt_volume = 0;
      int64_t alice_worth_volume = 0;
      time_point_sec alice_reward_last_update = fc::time_point_sec::min();
      int64_t bob_wmt_volume = 0;
      int64_t bob_worth_volume = 0;
      time_point_sec bob_reward_last_update = fc::time_point_sec::min();
      int64_t sam_wmt_volume = 0;
      int64_t sam_worth_volume = 0;
      time_point_sec sam_reward_last_update = fc::time_point_sec::min();
      int64_t dave_wmt_volume = 0;
      int64_t dave_worth_volume = 0;
      time_point_sec dave_reward_last_update = fc::time_point_sec::min();

      BOOST_TEST_MESSAGE( "Creating Limit Order for WORTH that will stay on the books for 30 minutes exactly." );

      limit_order_create_operation op;
      op.owner = "alice";
      op.amount_to_sell = asset( alice_wmt.amount.value / 20, any_wmt_symbol ) ;
      op.min_to_receive = op.amount_to_sell * exchange_rate;
      op.orderid = 1;
      op.expiration = db->head_block_time() + fc::seconds( WORTH_MAX_LIMIT_ORDER_EXPIRATION );

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "Waiting 10 minutes" );

      generate_blocks( db->head_block_time() + WORTH_MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10, true );

      BOOST_TEST_MESSAGE( "Creating Limit Order for WMT that will be filled immediately." );

      op.owner = "bob";
      op.min_to_receive = op.amount_to_sell;
      op.amount_to_sell = op.min_to_receive * exchange_rate;
      op.fill_or_kill = false;
      op.orderid = 2;

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      alice_worth_volume += ( asset( alice_wmt.amount / 20, any_wmt_symbol ) * exchange_rate ).amount.value;
      alice_reward_last_update = db->head_block_time();
      bob_worth_volume -= ( asset( alice_wmt.amount / 20, any_wmt_symbol ) * exchange_rate ).amount.value;
      bob_reward_last_update = db->head_block_time();

      auto ops = get_last_operations( 1 );
      const auto& liquidity_idx = db->get_index< liquidity_reward_balance_index >().indices().get< by_owner >();
      const auto& limit_order_idx = db->get_index< limit_order_index >().indices().get< by_account >();

      auto reward = liquidity_idx.find( db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward->wbd_volume == alice_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == alice_worth_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward->wbd_volume == bob_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == bob_worth_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      auto fill_order_op = ops[0].get< fill_order_operation >();

      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 1 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == asset( alice_wmt.amount.value / 20, any_wmt_symbol ).amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 2 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == ( asset( alice_wmt.amount.value / 20, any_wmt_symbol ) * exchange_rate ).amount.value );

      BOOST_CHECK( limit_order_idx.find( boost::make_tuple( "alice", 1 ) ) == limit_order_idx.end() );
      BOOST_CHECK( limit_order_idx.find( boost::make_tuple( "bob", 2 ) ) == limit_order_idx.end() );

      BOOST_TEST_MESSAGE( "Creating Limit Order for WMT that will stay on the books for 60 minutes." );

      op.owner = "sam";
      op.amount_to_sell = asset( ( alice_wmt.amount.value / 20 ), WORTH_SYMBOL );
      op.min_to_receive = asset( ( alice_wmt.amount.value / 20 ), any_wmt_symbol );
      op.orderid = 3;

      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( op );
      sign( tx, sam_private_key );
      db->push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "Waiting 10 minutes" );

      generate_blocks( db->head_block_time() + WORTH_MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10, true );

      BOOST_TEST_MESSAGE( "Creating Limit Order for WMT that will stay on the books for 30 minutes." );

      op.owner = "bob";
      op.orderid = 4;
      op.amount_to_sell = asset( ( alice_wmt.amount.value / 10 ) * 3 - alice_wmt.amount.value / 20, WORTH_SYMBOL );
      op.min_to_receive = asset( ( alice_wmt.amount.value / 10 ) * 3 - alice_wmt.amount.value / 20, any_wmt_symbol );

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "Waiting 30 minutes" );

      generate_blocks( db->head_block_time() + WORTH_MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10, true );

      BOOST_TEST_MESSAGE( "Filling both limit orders." );

      op.owner = "alice";
      op.orderid = 5;
      op.amount_to_sell = asset( ( alice_wmt.amount.value / 10 ) * 3, any_wmt_symbol );
      op.min_to_receive = asset( ( alice_wmt.amount.value / 10 ) * 3, WORTH_SYMBOL );

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      alice_wmt_volume -= ( alice_wmt.amount.value / 10 ) * 3;
      alice_reward_last_update = db->head_block_time();
      sam_wmt_volume += alice_wmt.amount.value / 20;
      sam_reward_last_update = db->head_block_time();
      bob_wmt_volume += ( alice_wmt.amount.value / 10 ) * 3 - ( alice_wmt.amount.value / 20 );
      bob_reward_last_update = db->head_block_time();
      ops = get_last_operations( 4 );

      fill_order_op = ops[1].get< fill_order_operation >();
      BOOST_REQUIRE( fill_order_op.open_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 4 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == asset( ( alice_wmt.amount.value / 10 ) * 3 - alice_wmt.amount.value / 20, WORTH_SYMBOL ).amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 5 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == asset( ( alice_wmt.amount.value / 10 ) * 3 - alice_wmt.amount.value / 20, any_wmt_symbol ).amount.value );

      fill_order_op = ops[3].get< fill_order_operation >();
      BOOST_REQUIRE( fill_order_op.open_owner == "sam" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 3 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == asset( alice_wmt.amount.value / 20, WORTH_SYMBOL ).amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 5 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == asset( alice_wmt.amount.value / 20, any_wmt_symbol ).amount.value );

      reward = liquidity_idx.find( db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward->wbd_volume == alice_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == alice_worth_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward->wbd_volume == bob_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == bob_worth_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward->wbd_volume == sam_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == sam_worth_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      BOOST_TEST_MESSAGE( "Testing a partial fill before minimum time and full fill after minimum time" );

      op.orderid = 6;
      op.amount_to_sell = asset( alice_wmt.amount.value / 20 * 2, any_wmt_symbol );
      op.min_to_receive = asset( alice_wmt.amount.value / 20 * 2, WORTH_SYMBOL );

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      generate_blocks( db->head_block_time() + fc::seconds( WORTH_MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10.to_seconds() / 2 ), true );

      op.owner = "bob";
      op.orderid = 7;
      op.amount_to_sell = asset( alice_wmt.amount.value / 20, WORTH_SYMBOL );
      op.min_to_receive = asset( alice_wmt.amount.value / 20, any_wmt_symbol );

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      generate_blocks( db->head_block_time() + fc::seconds( WORTH_MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10.to_seconds() / 2 ), true );

      ops = get_last_operations( 3 );
      fill_order_op = ops[2].get< fill_order_operation >();

      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 6 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == asset( alice_wmt.amount.value / 20, any_wmt_symbol ).amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 7 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == asset( alice_wmt.amount.value / 20, WORTH_SYMBOL ).amount.value );

      reward = liquidity_idx.find( db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward->wbd_volume == alice_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == alice_worth_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward->wbd_volume == bob_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == bob_worth_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward->wbd_volume == sam_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == sam_worth_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      generate_blocks( db->head_block_time() + WORTH_MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10, true );

      op.owner = "sam";
      op.orderid = 8;

      tx.signatures.clear();
      tx.operations.clear();
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      tx.operations.push_back( op );
      sign( tx, sam_private_key );
      db->push_transaction( tx, 0 );

      alice_worth_volume += alice_wmt.amount.value / 20;
      alice_reward_last_update = db->head_block_time();
      sam_worth_volume -= alice_wmt.amount.value / 20;
      sam_reward_last_update = db->head_block_time();

      ops = get_last_operations( 2 );
      fill_order_op = ops[1].get< fill_order_operation >();

      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 6 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == asset( alice_wmt.amount.value / 20, any_wmt_symbol ).amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "sam" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 8 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == asset( alice_wmt.amount.value / 20, WORTH_SYMBOL ).amount.value );

      reward = liquidity_idx.find( db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward->wbd_volume == alice_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == alice_worth_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward->wbd_volume == bob_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == bob_worth_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward->wbd_volume == sam_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == sam_worth_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      BOOST_TEST_MESSAGE( "Trading to give Alice and Bob positive volumes to receive rewards" );

      transfer_operation transfer;
      transfer.to = "dave";
      transfer.from = "alice";
      transfer.amount = asset( alice_wmt.amount / 2, any_wmt_symbol );

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( transfer );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      op.owner = "alice";
      op.amount_to_sell = asset( 8 * ( alice_wmt.amount.value / 20 ), WORTH_SYMBOL );
      op.min_to_receive = asset( op.amount_to_sell.amount, any_wmt_symbol );
      op.orderid = 9;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      generate_blocks( db->head_block_time() + WORTH_MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10, true );

      op.owner = "dave";
      op.amount_to_sell = asset( 7 * ( alice_wmt.amount.value / 20 ), any_wmt_symbol );;
      op.min_to_receive = asset( op.amount_to_sell.amount, WORTH_SYMBOL );
      op.orderid = 10;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, dave_private_key );
      db->push_transaction( tx, 0 );

      alice_wmt_volume += op.amount_to_sell.amount.value;
      alice_reward_last_update = db->head_block_time();
      dave_wmt_volume -= op.amount_to_sell.amount.value;
      dave_reward_last_update = db->head_block_time();

      ops = get_last_operations( 1 );
      fill_order_op = ops[0].get< fill_order_operation >();

      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 9 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == 7 * ( alice_wmt.amount.value / 20 ) );
      BOOST_REQUIRE( fill_order_op.current_owner == "dave" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 10 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == 7 * ( alice_wmt.amount.value / 20 ) );

      reward = liquidity_idx.find( db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward->wbd_volume == alice_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == alice_worth_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward->wbd_volume == bob_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == bob_worth_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward->wbd_volume == sam_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == sam_worth_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "dave" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "dave" ).id );
      BOOST_REQUIRE( reward->wbd_volume == dave_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == dave_worth_volume );
      BOOST_CHECK( reward->last_update == dave_reward_last_update );*/

      op.owner = "bob";
      op.amount_to_sell.amount = alice_wmt.amount / 20;
      op.min_to_receive.amount = op.amount_to_sell.amount;
      op.orderid = 11;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      alice_wmt_volume += op.amount_to_sell.amount.value;
      alice_reward_last_update = db->head_block_time();
      bob_wmt_volume -= op.amount_to_sell.amount.value;
      bob_reward_last_update = db->head_block_time();

      ops = get_last_operations( 1 );
      fill_order_op = ops[0].get< fill_order_operation >();

      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 9 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == alice_wmt.amount.value / 20 );
      BOOST_REQUIRE( fill_order_op.current_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 11 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == alice_wmt.amount.value / 20 );

      reward = liquidity_idx.find( db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward->wbd_volume == alice_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == alice_worth_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward->wbd_volume == bob_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == bob_worth_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward->wbd_volume == sam_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == sam_worth_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "dave" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "dave" ).id );
      BOOST_REQUIRE( reward->wbd_volume == dave_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == dave_worth_volume );
      BOOST_CHECK( reward->last_update == dave_reward_last_update );*/

      transfer.to = "bob";
      transfer.from = "alice";
      transfer.amount = asset( alice_wmt.amount / 5, any_wmt_symbol );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( transfer );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      op.owner = "bob";
      op.orderid = 12;
      op.amount_to_sell = asset( 3 * ( alice_wmt.amount / 40 ), any_wmt_symbol );
      op.min_to_receive = asset( op.amount_to_sell.amount, WORTH_SYMBOL );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      generate_blocks( db->head_block_time() + WORTH_MIN_LIQUIDITY_REWARD_PERIOD_SEC_HF10, true );

      op.owner = "dave";
      op.orderid = 13;
      op.amount_to_sell = op.min_to_receive;
      op.min_to_receive.symbol = any_wmt_symbol;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, dave_private_key );
      db->push_transaction( tx, 0 );

      bob_worth_volume += op.amount_to_sell.amount.value;
      bob_reward_last_update = db->head_block_time();
      dave_worth_volume -= op.amount_to_sell.amount.value;
      dave_reward_last_update = db->head_block_time();

      ops = get_last_operations( 1 );
      fill_order_op = ops[0].get< fill_order_operation >();

      BOOST_REQUIRE( fill_order_op.open_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 12 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == 3 * ( alice_wmt.amount.value / 40 ) );
      BOOST_REQUIRE( fill_order_op.current_owner == "dave" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 13 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == 3 * ( alice_wmt.amount.value / 40 ) );

      reward = liquidity_idx.find( db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "alice" ).id );
      BOOST_REQUIRE( reward->wbd_volume == alice_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == alice_worth_volume );
      BOOST_CHECK( reward->last_update == alice_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "bob" ).id );
      BOOST_REQUIRE( reward->wbd_volume == bob_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == bob_worth_volume );
      BOOST_CHECK( reward->last_update == bob_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward->wbd_volume == sam_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == sam_worth_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      reward = liquidity_idx.find( db->get_account( "dave" ).id );
      BOOST_REQUIRE( reward == liquidity_idx.end() );
      /*BOOST_REQUIRE( reward->owner == db->get_account( "dave" ).id );
      BOOST_REQUIRE( reward->wbd_volume == dave_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == dave_worth_volume );
      BOOST_CHECK( reward->last_update == dave_reward_last_update );*/

      auto alice_balance = db->get_account( "alice" ).balance;
      auto bob_balance = db->get_account( "bob" ).balance;
      auto sam_balance = db->get_account( "sam" ).balance;
      auto dave_balance = db->get_account( "dave" ).balance;

      BOOST_TEST_MESSAGE( "Generating Blocks to trigger liquidity rewards" );

      db->liquidity_rewards_enabled = true;
      generate_blocks( WORTH_LIQUIDITY_REWARD_BLOCKS - ( db->head_block_num() % WORTH_LIQUIDITY_REWARD_BLOCKS ) - 1 );

      BOOST_REQUIRE( db->head_block_num() % WORTH_LIQUIDITY_REWARD_BLOCKS == WORTH_LIQUIDITY_REWARD_BLOCKS - 1 );
      BOOST_REQUIRE( db->get_account( "alice" ).balance.amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db->get_account( "bob" ).balance.amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( db->get_account( "sam" ).balance.amount.value == sam_balance.amount.value );
      BOOST_REQUIRE( db->get_account( "dave" ).balance.amount.value == dave_balance.amount.value );

      generate_block();

      //alice_balance += WORTH_MIN_LIQUIDITY_REWARD;

      BOOST_REQUIRE( db->get_account( "alice" ).balance.amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db->get_account( "bob" ).balance.amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( db->get_account( "sam" ).balance.amount.value == sam_balance.amount.value );
      BOOST_REQUIRE( db->get_account( "dave" ).balance.amount.value == dave_balance.amount.value );

      ops = get_last_operations( 1 );

      WORTH_REQUIRE_THROW( ops[0].get< liquidity_reward_operation>(), fc::exception );
      //BOOST_REQUIRE( ops[0].get< liquidity_reward_operation>().payout.amount.value == WORTH_MIN_LIQUIDITY_REWARD.amount.value );

      generate_blocks( WORTH_LIQUIDITY_REWARD_BLOCKS );

      //bob_balance += WORTH_MIN_LIQUIDITY_REWARD;

      BOOST_REQUIRE( db->get_account( "alice" ).balance.amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db->get_account( "bob" ).balance.amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( db->get_account( "sam" ).balance.amount.value == sam_balance.amount.value );
      BOOST_REQUIRE( db->get_account( "dave" ).balance.amount.value == dave_balance.amount.value );

      ops = get_last_operations( 1 );

      WORTH_REQUIRE_THROW( ops[0].get< liquidity_reward_operation>(), fc::exception );
      //BOOST_REQUIRE( ops[0].get< liquidity_reward_operation>().payout.amount.value == WORTH_MIN_LIQUIDITY_REWARD.amount.value );

      alice_worth_volume = 0;
      alice_wmt_volume = 0;
      bob_worth_volume = 0;
      bob_wmt_volume = 0;

      BOOST_TEST_MESSAGE( "Testing liquidity timeout" );

      generate_blocks( sam_reward_last_update + WORTH_LIQUIDITY_TIMEOUT_SEC - fc::seconds( WORTH_BLOCK_INTERVAL / 2 ) - WORTH_MIN_LIQUIDITY_REWARD_PERIOD_SEC , true );

      op.owner = "sam";
      op.orderid = 14;
      op.amount_to_sell = ASSET( "1.000 TESTS" );
      op.min_to_receive = ASSET( "1.000 TBD" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, sam_private_key );
      db->push_transaction( tx, 0 );

      generate_blocks( db->head_block_time() + ( WORTH_BLOCK_INTERVAL / 2 ) + WORTH_LIQUIDITY_TIMEOUT_SEC, true );

      reward = liquidity_idx.find( db->get_account( "sam" ).id );
      /*BOOST_REQUIRE( reward == liquidity_idx.end() );
      BOOST_REQUIRE( reward->owner == db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward->wbd_volume == sam_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == sam_worth_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      generate_block();

      op.owner = "alice";
      op.orderid = 15;
      op.amount_to_sell.symbol = any_wmt_symbol;
      op.min_to_receive.symbol = WORTH_SYMBOL;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      sam_wmt_volume = ASSET( "1.000 TBD" ).amount.value;
      sam_worth_volume = 0;
      sam_reward_last_update = db->head_block_time();

      reward = liquidity_idx.find( db->get_account( "sam" ).id );
      /*BOOST_REQUIRE( reward == liquidity_idx.end() );
      BOOST_REQUIRE( reward->owner == db->get_account( "sam" ).id );
      BOOST_REQUIRE( reward->wbd_volume == sam_wmt_volume );
      BOOST_REQUIRE( reward->worth_volume == sam_worth_volume );
      BOOST_CHECK( reward->last_update == sam_reward_last_update );*/

      validate_database();
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_SUITE_END()
#endif
