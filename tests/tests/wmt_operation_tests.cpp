#include <fc/macros.hpp>

#if defined IS_TEST_NET && defined WORTHEUM_ENABLE_WMT

#include <boost/test/unit_test.hpp>

#include <worth/chain/worth_fwd.hpp>

#include <worth/protocol/exceptions.hpp>
#include <worth/protocol/hardfork.hpp>

#include <worth/chain/database.hpp>
#include <worth/chain/database_exceptions.hpp>
#include <worth/chain/worth_objects.hpp>
#include <worth/chain/wmt_objects.hpp>

#include <worth/chain/util/nai_generator.hpp>
#include <worth/chain/util/wmt_token.hpp>

#include "../db_fixture/database_fixture.hpp"

#include <fc/uint128.hpp>

using namespace worth::chain;
using namespace worth::protocol;
using fc::string;
using fc::uint128_t;
using boost::container::flat_set;

BOOST_FIXTURE_TEST_SUITE( wmt_operation_tests, wmt_database_fixture )

BOOST_AUTO_TEST_CASE( wmt_limit_order_create_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_limit_order_create_authorities" );

      ACTORS( (alice)(bob) )

      limit_order_create_operation op;

      //Create WMT and give some WMT to creator.
      signed_transaction tx;
      asset_symbol_type alice_symbol = create_wmt( "alice", alice_private_key, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      op.owner = "alice";
      op.amount_to_sell = ASSET( "1.000 TESTS" );
      op.min_to_receive = asset( 1000, alice_symbol );
      op.expiration = db->head_block_time() + fc::seconds( WORTH_MAX_TIME_UNTIL_EXPIRATION );

      tx.operations.push_back( op );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );

      BOOST_TEST_MESSAGE( "--- Test failure when no signature." );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test success with account signature" );
      sign( tx, alice_private_key );
      db->push_transaction( tx, database::skip_transaction_dupe_check );

      BOOST_TEST_MESSAGE( "--- Test failure with duplicate signature" );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, database::skip_transaction_dupe_check ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with additional incorrect signature" );
      tx.signatures.clear();
      sign( tx, alice_private_key );
      sign( tx, bob_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, database::skip_transaction_dupe_check ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with incorrect signature" );
      tx.signatures.clear();
      sign( tx, alice_post_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_limit_order_create2_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_limit_order_create2_authorities" );

      ACTORS( (alice)(bob) )

      limit_order_create2_operation op;

      //Create WMT and give some WMT to creator.
      signed_transaction tx;
      asset_symbol_type alice_symbol = create_wmt( "alice", alice_private_key, 0 );

      tx.operations.clear();
      tx.signatures.clear();

      op.owner = "alice";
      op.amount_to_sell = ASSET( "1.000 TESTS" );
      op.exchange_rate = price( ASSET( "1.000 TESTS" ), asset( 1000, alice_symbol ) );
      op.expiration = db->head_block_time() + fc::seconds( WORTH_MAX_LIMIT_ORDER_EXPIRATION );

      tx.operations.push_back( op );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );

      BOOST_TEST_MESSAGE( "--- Test failure when no signature." );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test success with account signature" );
      sign( tx, alice_private_key );
      db->push_transaction( tx, database::skip_transaction_dupe_check );

      BOOST_TEST_MESSAGE( "--- Test failure with duplicate signature" );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, database::skip_transaction_dupe_check ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with additional incorrect signature" );
      tx.signatures.clear();
      sign( tx, alice_private_key );
      sign( tx, bob_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, database::skip_transaction_dupe_check ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with incorrect signature" );
      tx.signatures.clear();
      sign( tx, alice_post_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_limit_order_create_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: limit_order_create_apply" );

      ACTORS( (alice)(bob) )

      //Create WMT and give some WMT to creators.
      signed_transaction tx;
      asset_symbol_type alice_symbol = create_wmt( "alice", alice_private_key, 3);

      const account_object& alice_account = db->get_account( "alice" );
      const account_object& bob_account = db->get_account( "bob" );

      asset alice_0 = asset( 0, alice_symbol );

      FUND( "bob", 1000000 );
      convert( "bob", ASSET("1000.000 TESTS" ) );
      generate_block();

      asset alice_wmt_balance = asset( 1000000, alice_symbol );
      asset bob_wmt_balance = asset( 1000000, alice_symbol );

      asset alice_balance = alice_account.balance;

      asset bob_balance = bob_account.balance;

      FUND( "alice", alice_wmt_balance );
      FUND( "bob", bob_wmt_balance );

      tx.operations.clear();
      tx.signatures.clear();

      const auto& limit_order_idx = db->get_index< limit_order_index >().indices().get< by_account >();

      BOOST_TEST_MESSAGE( "--- Test failure when account does not have required funds" );
      limit_order_create_operation op;

      op.owner = "bob";
      op.orderid = 1;
      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.min_to_receive = asset( 10000, alice_symbol );
      op.fill_or_kill = false;
      op.expiration = db->head_block_time() + fc::seconds( WORTH_MAX_LIMIT_ORDER_EXPIRATION );
      tx.operations.push_back( op );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, bob_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "bob", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure when amount to receive is 0" );

      op.owner = "alice";
      op.min_to_receive = alice_0;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "alice", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure when amount to sell is 0" );

      op.amount_to_sell = alice_0;
      op.min_to_receive = ASSET( "10.000 TESTS" ) ;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "alice", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure when expiration is too long" );
      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.min_to_receive = ASSET( "15.000 TBD" );
      op.expiration = db->head_block_time() + fc::seconds( WORTH_MAX_LIMIT_ORDER_EXPIRATION + 1 );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "--- Test success creating limit order that will not be filled" );

      op.expiration = db->head_block_time() + fc::seconds( WORTH_MAX_LIMIT_ORDER_EXPIRATION );
      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.min_to_receive = asset( 15000, alice_symbol );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      alice_balance -= ASSET( "10.000 TESTS" );

      auto limit_order = limit_order_idx.find( boost::make_tuple( "alice", op.orderid ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == op.owner );
      BOOST_REQUIRE( limit_order->orderid == op.orderid );
      BOOST_REQUIRE( limit_order->for_sale == op.amount_to_sell.amount );
      BOOST_REQUIRE( limit_order->sell_price == price( op.amount_to_sell / op.min_to_receive ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( alice_symbol, WORTH_SYMBOL ) );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure creating limit order with duplicate id" );

      op.amount_to_sell = ASSET( "20.000 TESTS" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::exception );

      limit_order = limit_order_idx.find( boost::make_tuple( "alice", op.orderid ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == op.owner );
      BOOST_REQUIRE( limit_order->orderid == op.orderid );
      BOOST_REQUIRE( limit_order->for_sale == 10000 );
      BOOST_REQUIRE( limit_order->sell_price == price( ASSET( "10.000 TESTS" ), op.min_to_receive ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( alice_symbol, WORTH_SYMBOL ) );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test sucess killing an order that will not be filled" );

      op.orderid = 2;
      op.fill_or_kill = true;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "alice", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      validate_database();

      // BOOST_TEST_MESSAGE( "--- Test having a partial match to limit order" );
      // // Alice has order for 15 WMT at a price of 2:3
      // // Fill 5 WORTH for 7.5 WMT

      op.owner = "bob";
      op.orderid = 1;
      op.amount_to_sell = asset (7500, alice_symbol );
      op.min_to_receive = ASSET( "5.000 TESTS" );
      op.fill_or_kill = false;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      bob_wmt_balance -= asset (7500, alice_symbol );
      alice_wmt_balance += asset (7500, alice_symbol );
      bob_balance += ASSET( "5.000 TESTS" );

      auto recent_ops = get_last_operations( 1 );
      auto fill_order_op = recent_ops[0].get< fill_order_operation >();

      limit_order = limit_order_idx.find( boost::make_tuple( "alice", 1 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "alice" );
      BOOST_REQUIRE( limit_order->orderid == op.orderid );
      BOOST_REQUIRE( limit_order->for_sale == 5000 );
      BOOST_REQUIRE( limit_order->sell_price == price( ASSET( "10.000 TESTS" ), asset( 15000, alice_symbol ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( alice_symbol, WORTH_SYMBOL ) );
      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "bob", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 1 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == ASSET( "5.000 TESTS").amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 1 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == asset(7500, alice_symbol ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test filling an existing order fully, but the new order partially" );

      op.amount_to_sell = asset( 15000, alice_symbol );
      op.min_to_receive = ASSET( "10.000 TESTS" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      bob_wmt_balance -= asset( 15000, alice_symbol );
      alice_wmt_balance += asset( 7500, alice_symbol );
      bob_balance += ASSET( "5.000 TESTS" );

      limit_order = limit_order_idx.find( boost::make_tuple( "bob", 1 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "bob" );
      BOOST_REQUIRE( limit_order->orderid == 1 );
      BOOST_REQUIRE( limit_order->for_sale.value == 7500 );
      BOOST_REQUIRE( limit_order->sell_price == price( asset( 15000, alice_symbol ), ASSET( "10.000 TESTS" ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( alice_symbol, WORTH_SYMBOL ) );
      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "alice", 1 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test filling an existing order and new order fully" );

      op.owner = "alice";
      op.orderid = 3;
      op.amount_to_sell = ASSET( "5.000 TESTS" );
      op.min_to_receive = asset( 7500, alice_symbol );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      alice_balance -= ASSET( "5.000 TESTS" );
      alice_wmt_balance += asset( 7500, alice_symbol );
      bob_balance += ASSET( "5.000 TESTS" );

      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "alice", 3 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "bob", 1 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test filling limit order with better order when partial order is better." );

      op.owner = "alice";
      op.orderid = 4;
      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.min_to_receive = asset( 11000, alice_symbol );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      op.owner = "bob";
      op.orderid = 4;
      op.amount_to_sell = asset( 12000, alice_symbol );
      op.min_to_receive = ASSET( "10.000 TESTS" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      alice_balance -= ASSET( "10.000 TESTS" );
      alice_wmt_balance += asset( 11000, alice_symbol );
      bob_wmt_balance -= asset( 12000, alice_symbol );
      bob_balance += ASSET( "10.000 TESTS" );

      limit_order = limit_order_idx.find( boost::make_tuple( "bob", 4 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order_idx.find(boost::make_tuple( "alice", 4 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "bob" );
      BOOST_REQUIRE( limit_order->orderid == 4 );
      BOOST_REQUIRE( limit_order->for_sale.value == 1000 );
      BOOST_REQUIRE( limit_order->sell_price == price( asset( 12000, alice_symbol ), ASSET( "10.000 TESTS" ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( alice_symbol, WORTH_SYMBOL ) );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      validate_database();

      limit_order_cancel_operation can;
      can.owner = "bob";
      can.orderid = 4;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( can );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "--- Test filling limit order with better order when partial order is worse." );

      //auto gpo = db->get_dynamic_global_properties();
      //auto start_wbd = gpo.current_wbd_supply;

      op.owner = "alice";
      op.orderid = 5;
      op.amount_to_sell = ASSET( "20.000 TESTS" );
      op.min_to_receive = asset( 22000, alice_symbol );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      op.owner = "bob";
      op.orderid = 5;
      op.amount_to_sell = asset( 12000, alice_symbol );
      op.min_to_receive = ASSET( "10.000 TESTS" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      alice_balance -= ASSET( "20.000 TESTS" );
      alice_wmt_balance += asset( 12000, alice_symbol );

      bob_wmt_balance -= asset( 11000, alice_symbol );
      bob_balance += ASSET( "10.909 TESTS" );

      limit_order = limit_order_idx.find( boost::make_tuple( "alice", 5 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order_idx.find(boost::make_tuple( "bob", 5 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "alice" );
      BOOST_REQUIRE( limit_order->orderid == 5 );
      BOOST_REQUIRE( limit_order->for_sale.value == 9091 );
      BOOST_REQUIRE( limit_order->sell_price == price( ASSET( "20.000 TESTS" ), asset( 22000, alice_symbol ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( alice_symbol, WORTH_SYMBOL ) );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_limit_order_cancel_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_limit_order_cancel_authorities" );

      ACTORS( (alice)(bob) )

      //Create WMT and give some WMT to creator.
      signed_transaction tx;
      asset_symbol_type alice_symbol = create_wmt( "alice", alice_private_key, 3 );

      FUND( "alice", asset( 100000, alice_symbol ) );

      tx.operations.clear();
      tx.signatures.clear();

      limit_order_create_operation c;
      c.owner = "alice";
      c.orderid = 1;
      c.amount_to_sell = ASSET( "1.000 TESTS" );
      c.min_to_receive = asset( 1000, alice_symbol );
      c.expiration = db->head_block_time() + fc::seconds( WORTH_MAX_LIMIT_ORDER_EXPIRATION );

      tx.operations.push_back( c );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      limit_order_cancel_operation op;
      op.owner = "alice";
      op.orderid = 1;

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );

      BOOST_TEST_MESSAGE( "--- Test failure when no signature." );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      BOOST_TEST_MESSAGE( "--- Test success with account signature" );
      sign( tx, alice_private_key );
      db->push_transaction( tx, database::skip_transaction_dupe_check );

      BOOST_TEST_MESSAGE( "--- Test failure with duplicate signature" );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, database::skip_transaction_dupe_check ), tx_duplicate_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with additional incorrect signature" );
      tx.signatures.clear();
      sign( tx, alice_private_key );
      sign( tx, bob_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, database::skip_transaction_dupe_check ), tx_irrelevant_sig );

      BOOST_TEST_MESSAGE( "--- Test failure with incorrect signature" );
      tx.signatures.clear();
      sign( tx, alice_post_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, database::skip_transaction_dupe_check ), tx_missing_active_auth );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_limit_order_cancel_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_limit_order_cancel_apply" );

      ACTORS( (alice) )

      //Create WMT and give some WMT to creator.
      signed_transaction tx;
      asset_symbol_type alice_symbol = create_wmt( "alice", alice_private_key, 3 );

      const account_object& alice_account = db->get_account( "alice" );

      tx.operations.clear();
      tx.signatures.clear();

      asset alice_wmt_balance = asset( 1000000, alice_symbol );
      asset alice_balance = alice_account.balance;

      FUND( "alice", alice_wmt_balance );

      const auto& limit_order_idx = db->get_index< limit_order_index >().indices().get< by_account >();

      BOOST_TEST_MESSAGE( "--- Test cancel non-existent order" );

      limit_order_cancel_operation op;

      op.owner = "alice";
      op.orderid = 5;
      tx.operations.push_back( op );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "--- Test cancel order" );

      limit_order_create_operation create;
      create.owner = "alice";
      create.orderid = 5;
      create.amount_to_sell = ASSET( "5.000 TESTS" );
      create.min_to_receive = asset( 7500, alice_symbol );
      create.expiration = db->head_block_time() + fc::seconds( WORTH_MAX_LIMIT_ORDER_EXPIRATION );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( create );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "alice", 5 ) ) != limit_order_idx.end() );

      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "alice", 5 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_limit_order_create2_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: limit_order_create2_apply" );

      ACTORS( (alice)(bob) )

      //Create WMT and give some WMT to creators.
      signed_transaction tx;
      asset_symbol_type alice_symbol = create_wmt( "alice", alice_private_key, 3);

      const account_object& alice_account = db->get_account( "alice" );
      const account_object& bob_account = db->get_account( "bob" );

      asset alice_0 = asset( 0, alice_symbol );

      FUND( "bob", 1000000 );
      convert( "bob", ASSET("1000.000 TESTS" ) );
      generate_block();

      asset alice_wmt_balance = asset( 1000000, alice_symbol );
      asset bob_wmt_balance = asset( 1000000, alice_symbol );

      asset alice_balance = alice_account.balance;

      asset bob_balance = bob_account.balance;

      FUND( "alice", alice_wmt_balance );
      FUND( "bob", bob_wmt_balance );

      tx.operations.clear();
      tx.signatures.clear();

      const auto& limit_order_idx = db->get_index< limit_order_index >().indices().get< by_account >();

      BOOST_TEST_MESSAGE( "--- Test failure when account does not have required funds" );
      limit_order_create2_operation op;

      op.owner = "bob";
      op.orderid = 1;
      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.exchange_rate = price( ASSET( "1.000 TESTS" ), asset( 1000, alice_symbol ) );
      op.fill_or_kill = false;
      op.expiration = db->head_block_time() + fc::seconds( WORTH_MAX_LIMIT_ORDER_EXPIRATION );
      tx.operations.push_back( op );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, bob_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "bob", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure when amount to receive is 0" );

      op.owner = "alice";
      op.exchange_rate.base = ASSET( "0.000 TESTS" );
      op.exchange_rate.quote = alice_0;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "alice", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure when amount to sell is 0" );

      op.amount_to_sell = alice_0;
      op.exchange_rate = price( ASSET( "1.000 TESTS" ), asset( 1000, alice_symbol ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "alice", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure when expiration is too long" );
      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.exchange_rate = price( ASSET( "2.000 TESTS" ), ASSET( "3.000 TBD" ) );
      op.expiration = db->head_block_time() + fc::seconds( WORTH_MAX_LIMIT_ORDER_EXPIRATION + 1 );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::exception );

      BOOST_TEST_MESSAGE( "--- Test success creating limit order that will not be filled" );

      op.expiration = db->head_block_time() + fc::seconds( WORTH_MAX_LIMIT_ORDER_EXPIRATION );
      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.exchange_rate = price( ASSET( "2.000 TESTS" ), asset( 3000, alice_symbol ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      alice_balance -= ASSET( "10.000 TESTS" );

      auto limit_order = limit_order_idx.find( boost::make_tuple( "alice", op.orderid ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == op.owner );
      BOOST_REQUIRE( limit_order->orderid == op.orderid );
      BOOST_REQUIRE( limit_order->for_sale == op.amount_to_sell.amount );
      BOOST_REQUIRE( limit_order->sell_price == op.exchange_rate );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( alice_symbol, WORTH_SYMBOL ) );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test failure creating limit order with duplicate id" );

      op.amount_to_sell = ASSET( "20.000 TESTS" );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::exception );

      limit_order = limit_order_idx.find( boost::make_tuple( "alice", op.orderid ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == op.owner );
      BOOST_REQUIRE( limit_order->orderid == op.orderid );
      BOOST_REQUIRE( limit_order->for_sale == 10000 );
      BOOST_REQUIRE( limit_order->sell_price == op.exchange_rate );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( alice_symbol, WORTH_SYMBOL ) );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test sucess killing an order that will not be filled" );

      op.orderid = 2;
      op.fill_or_kill = true;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::exception );

      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "alice", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      validate_database();

      // BOOST_TEST_MESSAGE( "--- Test having a partial match to limit order" );
      // // Alice has order for 15 WMT at a price of 2:3
      // // Fill 5 WORTH for 7.5 WMT

      op.owner = "bob";
      op.orderid = 1;
      op.amount_to_sell = asset( 7500, alice_symbol );
      op.exchange_rate = price( asset( 3000, alice_symbol ), ASSET( "2.000 TESTS" ) );
      op.fill_or_kill = false;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      bob_wmt_balance -= asset (7500, alice_symbol );
      alice_wmt_balance += asset (7500, alice_symbol );
      bob_balance += ASSET( "5.000 TESTS" );

      auto recent_ops = get_last_operations( 1 );
      auto fill_order_op = recent_ops[0].get< fill_order_operation >();

      limit_order = limit_order_idx.find( boost::make_tuple( "alice", 1 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "alice" );
      BOOST_REQUIRE( limit_order->orderid == op.orderid );
      BOOST_REQUIRE( limit_order->for_sale == 5000 );
      BOOST_REQUIRE( limit_order->sell_price == price( ASSET( "2.000 TESTS" ), asset( 3000, alice_symbol ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( alice_symbol, WORTH_SYMBOL ) );
      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "bob", op.orderid ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      BOOST_REQUIRE( fill_order_op.open_owner == "alice" );
      BOOST_REQUIRE( fill_order_op.open_orderid == 1 );
      BOOST_REQUIRE( fill_order_op.open_pays.amount.value == ASSET( "5.000 TESTS").amount.value );
      BOOST_REQUIRE( fill_order_op.current_owner == "bob" );
      BOOST_REQUIRE( fill_order_op.current_orderid == 1 );
      BOOST_REQUIRE( fill_order_op.current_pays.amount.value == asset(7500, alice_symbol ).amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test filling an existing order fully, but the new order partially" );

      op.amount_to_sell = asset( 15000, alice_symbol );
      op.exchange_rate = price( asset( 3000, alice_symbol ), ASSET( "2.000 TESTS" ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      bob_wmt_balance -= asset( 15000, alice_symbol );
      alice_wmt_balance += asset( 7500, alice_symbol );
      bob_balance += ASSET( "5.000 TESTS" );

      limit_order = limit_order_idx.find( boost::make_tuple( "bob", 1 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "bob" );
      BOOST_REQUIRE( limit_order->orderid == 1 );
      BOOST_REQUIRE( limit_order->for_sale.value == 7500 );
      BOOST_REQUIRE( limit_order->sell_price == price( asset( 3000, alice_symbol ), ASSET( "2.000 TESTS" ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( alice_symbol, WORTH_SYMBOL ) );
      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "alice", 1 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test filling an existing order and new order fully" );

      op.owner = "alice";
      op.orderid = 3;
      op.amount_to_sell = ASSET( "5.000 TESTS" );
      op.exchange_rate = price( ASSET( "2.000 TESTS" ), asset( 3000, alice_symbol ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      alice_balance -= ASSET( "5.000 TESTS" );
      alice_wmt_balance += asset( 7500, alice_symbol );
      bob_balance += ASSET( "5.000 TESTS" );

      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "alice", 3 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( limit_order_idx.find( boost::make_tuple( "bob", 1 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test filling limit order with better order when partial order is better." );

      op.owner = "alice";
      op.orderid = 4;
      op.amount_to_sell = ASSET( "10.000 TESTS" );
      op.exchange_rate = price( ASSET( "1.000 TESTS" ), asset( 1100, alice_symbol ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      op.owner = "bob";
      op.orderid = 4;
      op.amount_to_sell = asset( 12000, alice_symbol );
      op.exchange_rate = price( asset( 1200, alice_symbol ), ASSET( "1.000 TESTS" ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      alice_balance -= ASSET( "10.000 TESTS" );
      alice_wmt_balance += asset( 11000, alice_symbol );
      bob_wmt_balance -= asset( 12000, alice_symbol );
      bob_balance += ASSET( "10.000 TESTS" );

      limit_order = limit_order_idx.find( boost::make_tuple( "bob", 4 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order_idx.find(boost::make_tuple( "alice", 4 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "bob" );
      BOOST_REQUIRE( limit_order->orderid == 4 );
      BOOST_REQUIRE( limit_order->for_sale.value == 1000 );
      BOOST_REQUIRE( limit_order->sell_price == op.exchange_rate );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( alice_symbol, WORTH_SYMBOL ) );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      validate_database();

      limit_order_cancel_operation can;
      can.owner = "bob";
      can.orderid = 4;
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( can );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "--- Test filling limit order with better order when partial order is worse." );

      //auto gpo = db->get_dynamic_global_properties();
      //auto start_wbd = gpo.current_wbd_supply;

      op.owner = "alice";
      op.orderid = 5;
      op.amount_to_sell = ASSET( "20.000 TESTS" );
      op.exchange_rate = price( ASSET( "1.000 TESTS" ), asset( 1100, alice_symbol ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      op.owner = "bob";
      op.orderid = 5;
      op.amount_to_sell = asset( 12000, alice_symbol );
      op.exchange_rate = price( asset( 1200, alice_symbol ), ASSET( "1.000 TESTS" ) );
      tx.operations.clear();
      tx.signatures.clear();
      tx.operations.push_back( op );
      sign( tx, bob_private_key );
      db->push_transaction( tx, 0 );

      alice_balance -= ASSET( "20.000 TESTS" );
      alice_wmt_balance += asset( 12000, alice_symbol );

      bob_wmt_balance -= asset( 11000, alice_symbol );
      bob_balance += ASSET( "10.909 TESTS" );

      limit_order = limit_order_idx.find( boost::make_tuple( "alice", 5 ) );
      BOOST_REQUIRE( limit_order != limit_order_idx.end() );
      BOOST_REQUIRE( limit_order_idx.find(boost::make_tuple( "bob", 5 ) ) == limit_order_idx.end() );
      BOOST_REQUIRE( limit_order->seller == "alice" );
      BOOST_REQUIRE( limit_order->orderid == 5 );
      BOOST_REQUIRE( limit_order->for_sale.value == 9091 );
      BOOST_REQUIRE( limit_order->sell_price == price( ASSET( "1.000 TESTS" ), asset( 1100, alice_symbol ) ) );
      BOOST_REQUIRE( limit_order->get_market() == std::make_pair( alice_symbol, WORTH_SYMBOL ) );
      BOOST_REQUIRE( db->get_balance( alice_account, alice_symbol ).amount.value == alice_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( alice_account, WORTH_SYMBOL ).amount.value == alice_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, alice_symbol ).amount.value == bob_wmt_balance.amount.value );
      BOOST_REQUIRE( db->get_balance( bob_account, WORTH_SYMBOL ).amount.value == bob_balance.amount.value );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( claim_reward_balance2_validate )
{
   try
   {
      claim_reward_balance2_operation op;
      op.account = "alice";

      ACTORS( (alice) )

      generate_block();

      // Create WMT(s) and continue.
      auto wmts = create_wmt_3("alice", alice_private_key);
      const auto& wmt1 = wmts[0];
      const auto& wmt2 = wmts[1];
      const auto& wmt3 = wmts[2];

      BOOST_TEST_MESSAGE( "Testing empty rewards" );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "Testing ineffective rewards" );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      // Manually inserted.
      op.reward_tokens.push_back( ASSET( "0.000 TESTS" ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();
      op.reward_tokens.push_back( ASSET( "0.000 TBD" ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();
      op.reward_tokens.push_back( ASSET( "0.000000 VESTS" ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();
      op.reward_tokens.push_back( asset( 0, wmt1 ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();
      op.reward_tokens.push_back( asset( 0, wmt2 ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();
      op.reward_tokens.push_back( asset( 0, wmt3 ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();

      BOOST_TEST_MESSAGE( "Testing single reward claims" );
      op.reward_tokens.push_back( ASSET( "1.000 TESTS" ) );
      op.validate();
      op.reward_tokens.clear();

      op.reward_tokens.push_back( ASSET( "1.000 TBD" ) );
      op.validate();
      op.reward_tokens.clear();

      op.reward_tokens.push_back( ASSET( "1.000000 VESTS" ) );
      op.validate();
      op.reward_tokens.clear();

      op.reward_tokens.push_back( asset( 1, wmt1 ) );
      op.validate();
      op.reward_tokens.clear();

      op.reward_tokens.push_back( asset( 1, wmt2 ) );
      op.validate();
      op.reward_tokens.clear();

      op.reward_tokens.push_back( asset( 1, wmt3 ) );
      op.validate();
      op.reward_tokens.clear();

      BOOST_TEST_MESSAGE( "Testing multiple rewards" );
      op.reward_tokens.push_back( ASSET( "1.000 TBD" ) );
      op.reward_tokens.push_back( ASSET( "1.000 TESTS" ) );
      op.reward_tokens.push_back( ASSET( "1.000000 VESTS" ) );
      op.reward_tokens.push_back( asset( 1, wmt1 ) );
      op.reward_tokens.push_back( asset( 1, wmt2 ) );
      op.reward_tokens.push_back( asset( 1, wmt3 ) );
      op.validate();
      op.reward_tokens.clear();

      BOOST_TEST_MESSAGE( "Testing invalid rewards" );
      op.reward_tokens.push_back( ASSET( "-1.000 TESTS" ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();
      op.reward_tokens.push_back( ASSET( "-1.000 TBD" ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();
      op.reward_tokens.push_back( ASSET( "-1.000000 VESTS" ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();
      op.reward_tokens.push_back( asset( -1, wmt1 ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();
      op.reward_tokens.push_back( asset( -1, wmt2 ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();
      op.reward_tokens.push_back( asset( -1, wmt3 ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();

      BOOST_TEST_MESSAGE( "Testing duplicated reward tokens." );
      op.reward_tokens.push_back( asset( 1, wmt3 ) );
      op.reward_tokens.push_back( asset( 1, wmt3 ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();

      BOOST_TEST_MESSAGE( "Testing inconsistencies of manually inserted reward tokens." );
      op.reward_tokens.push_back( ASSET( "1.000 TESTS" ) );
      op.reward_tokens.push_back( ASSET( "1.000 TBD" ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.push_back( asset( 1, wmt3 ) );
      op.reward_tokens.push_back( asset( 1, wmt1 ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.reward_tokens.clear();
      op.reward_tokens.push_back( asset( 1, wmt1 ) );
      op.reward_tokens.push_back( asset( -1, wmt3 ) );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( claim_reward_balance2_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: decline_voting_rights_authorities" );

      claim_reward_balance2_operation op;
      op.account = "alice";

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      expected.insert( "alice" );
      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( claim_reward_balance2_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: claim_reward_balance2_apply" );
      BOOST_TEST_MESSAGE( "--- Setting up test state" );

      ACTORS( (alice) )
      generate_block();

      auto wmts = create_wmt_3( "alice", alice_private_key );
      const auto& wmt1 = wmts[0];
      const auto& wmt2 = wmts[1];
      const auto& wmt3 = wmts[2];

      FUND_WMT_REWARDS( "alice", asset( 10*std::pow(10, wmt1.decimals()), wmt1 ) );
      FUND_WMT_REWARDS( "alice", asset( 10*std::pow(10, wmt2.decimals()), wmt2 ) );
      FUND_WMT_REWARDS( "alice", asset( 10*std::pow(10, wmt3.decimals()), wmt3 ) );

      db_plugin->debug_update( []( database& db )
      {
         db.modify( db.get_account( "alice" ), []( account_object& a )
         {
            a.reward_wbd_balance = ASSET( "10.000 TBD" );
            a.reward_worth_balance = ASSET( "10.000 TESTS" );
            a.reward_vesting_balance = ASSET( "10.000000 VESTS" );
            a.reward_vesting_worth = ASSET( "10.000 TESTS" );
         });

         db.modify( db.get_dynamic_global_properties(), []( dynamic_global_property_object& gpo )
         {
            gpo.current_wbd_supply += ASSET( "10.000 TBD" );
            gpo.current_supply += ASSET( "20.000 TESTS" );
            gpo.virtual_supply += ASSET( "20.000 TESTS" );
            gpo.pending_rewarded_vesting_shares += ASSET( "10.000000 VESTS" );
            gpo.pending_rewarded_vesting_worth += ASSET( "10.000 TESTS" );
         });
      });

      generate_block();
      validate_database();

      auto alice_worth = db->get_account( "alice" ).balance;
      auto alice_wbd = db->get_account( "alice" ).wbd_balance;
      auto alice_vests = db->get_account( "alice" ).vesting_shares;
      auto alice_wmt1 = db->get_balance( "alice", wmt1 );
      auto alice_wmt2 = db->get_balance( "alice", wmt2 );
      auto alice_wmt3 = db->get_balance( "alice", wmt3 );

      claim_reward_balance2_operation op;
      op.account = "alice";

      BOOST_TEST_MESSAGE( "--- Attempting to claim more than exists in the reward balance." );
      // Legacy symbols
      op.reward_tokens.push_back( ASSET( "0.000 TBD" ) );
      op.reward_tokens.push_back( ASSET( "20.000 TESTS" ) );
      op.reward_tokens.push_back( ASSET( "0.000000 VESTS" ) );
      FAIL_WITH_OP(op, alice_private_key, fc::assert_exception);
      op.reward_tokens.clear();
      // WMTs
      op.reward_tokens.push_back( asset( 0, wmt1 ) );
      op.reward_tokens.push_back( asset( 0, wmt2 ) );
      op.reward_tokens.push_back( asset( 20*std::pow(10, wmt3.decimals()), wmt3 ) );
      FAIL_WITH_OP(op, alice_private_key, fc::assert_exception);
      op.reward_tokens.clear();

      BOOST_TEST_MESSAGE( "--- Claiming a partial reward balance" );
      // Legacy symbols
      asset partial_vests = ASSET( "5.000000 VESTS" );
      op.reward_tokens.push_back( ASSET( "0.000 TBD" ) );
      op.reward_tokens.push_back( ASSET( "0.000 TESTS" ) );
      op.reward_tokens.push_back( partial_vests );
      PUSH_OP(op, alice_private_key);
      BOOST_REQUIRE( db->get_account( "alice" ).balance == alice_worth + ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db->get_account( "alice" ).reward_worth_balance == ASSET( "10.000 TESTS" ) );
      BOOST_REQUIRE( db->get_account( "alice" ).wbd_balance == alice_wbd + ASSET( "0.000 TBD" ) );
      BOOST_REQUIRE( db->get_account( "alice" ).reward_wbd_balance == ASSET( "10.000 TBD" ) );
      BOOST_REQUIRE( db->get_account( "alice" ).vesting_shares == alice_vests + partial_vests );
      BOOST_REQUIRE( db->get_account( "alice" ).reward_vesting_balance == ASSET( "5.000000 VESTS" ) );
      BOOST_REQUIRE( db->get_account( "alice" ).reward_vesting_worth == ASSET( "5.000 TESTS" ) );
      validate_database();
      alice_vests += partial_vests;
      op.reward_tokens.clear();
      // WMTs
      asset partial_wmt2 = asset( 5*std::pow(10, wmt2.decimals()), wmt2 );
      op.reward_tokens.push_back( asset( 0, wmt1 ) );
      op.reward_tokens.push_back( partial_wmt2 );
      op.reward_tokens.push_back( asset( 0, wmt3 ) );
      PUSH_OP(op, alice_private_key);
      BOOST_REQUIRE( db->get_balance( "alice", wmt1 ) == alice_wmt1 + asset( 0, wmt1 ) );
      BOOST_REQUIRE( db->get_balance( "alice", wmt2 ) == alice_wmt2 + partial_wmt2 );
      BOOST_REQUIRE( db->get_balance( "alice", wmt3 ) == alice_wmt3 + asset( 0, wmt3 ) );
      validate_database();
      alice_wmt2 += partial_wmt2;
      op.reward_tokens.clear();

      BOOST_TEST_MESSAGE( "--- Claiming the full reward balance" );
      // Legacy symbols
      asset full_worth = ASSET( "10.000 TESTS" );
      asset full_wbd = ASSET( "10.000 TBD" );
      op.reward_tokens.push_back( full_wbd );
      op.reward_tokens.push_back( full_worth );
      op.reward_tokens.push_back( partial_vests );
      PUSH_OP(op, alice_private_key);
      BOOST_REQUIRE( db->get_account( "alice" ).balance == alice_worth + full_worth );
      BOOST_REQUIRE( db->get_account( "alice" ).reward_worth_balance == ASSET( "0.000 TESTS" ) );
      BOOST_REQUIRE( db->get_account( "alice" ).wbd_balance == alice_wbd + full_wbd );
      BOOST_REQUIRE( db->get_account( "alice" ).reward_wbd_balance == ASSET( "0.000 TBD" ) );
      BOOST_REQUIRE( db->get_account( "alice" ).vesting_shares == alice_vests + partial_vests );
      BOOST_REQUIRE( db->get_account( "alice" ).reward_vesting_balance == ASSET( "0.000000 VESTS" ) );
      BOOST_REQUIRE( db->get_account( "alice" ).reward_vesting_worth == ASSET( "0.000 TESTS" ) );
      validate_database();
      op.reward_tokens.clear();
      // WMTs
      asset full_wmt1 = asset( 10*std::pow(10, wmt1.decimals()), wmt1 );
      asset full_wmt3 = asset( 10*std::pow(10, wmt3.decimals()), wmt3 );
      op.reward_tokens.push_back( full_wmt1 );
      op.reward_tokens.push_back( partial_wmt2 );
      op.reward_tokens.push_back( full_wmt3 );
      PUSH_OP(op, alice_private_key);
      BOOST_REQUIRE( db->get_balance( "alice", wmt1 ) == alice_wmt1 + full_wmt1 );
      BOOST_REQUIRE( db->get_balance( "alice", wmt2 ) == alice_wmt2 + partial_wmt2 );
      BOOST_REQUIRE( db->get_balance( "alice", wmt3 ) == alice_wmt3 + full_wmt3 );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_transfer_to_vesting_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: transfer_to_vesting_validate with WMT" );

      // Dramatis personae
      ACTORS( (alice) )
      generate_block();
      // Create sample WMTs
      auto wmts = create_wmt_3( "alice", alice_private_key );
      const auto& wmt1 = wmts[0];
      // Fund creator with WMTs
      FUND( "alice", asset( 100, wmt1 ) );

      transfer_to_vesting_operation op;
      op.from = "alice";
      op.amount = asset( 20, wmt1 );
      op.validate();

      // Fail on invalid 'from' account name
      op.from = "@@@@@";
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.from = "alice";

      // Fail on invalid 'to' account name
      op.to = "@@@@@";
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.to = "";

      // Fail on vesting symbol (instead of liquid)
      op.amount = asset( 20, wmt1.get_paired_symbol() );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.amount = asset( 20, wmt1 );

      // Fail on 0 amount
      op.amount = asset( 0, wmt1 );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.amount = asset( 20, wmt1 );

      // Fail on negative amount
      op.amount = asset( -20, wmt1 );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.amount = asset( 20, wmt1 );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

// Here would be wmt_transfer_to_vesting_authorities if it differed from transfer_to_vesting_authorities
/*
BOOST_AUTO_TEST_CASE( wmt_transfer_to_vesting_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_transfer_to_vesting_apply" );

      auto test_single_wmt = [this] (const asset_symbol_type& liquid_wmt, const fc::ecc::private_key& key )
      {
         asset_symbol_type vesting_wmt = liquid_wmt.get_paired_symbol();
         // Fund creator with WMTs
         FUND( "alice", asset( 10000, liquid_wmt ) );

         const auto& wmt_object = db->get< wmt_token_object, by_symbol >( liquid_wmt );

         // Check pre-vesting balances
         FC_ASSERT( db->get_balance( "alice", liquid_wmt ).amount == 10000, "WMT balance adjusting error" );
         FC_ASSERT( db->get_balance( "alice", vesting_wmt ).amount == 0, "WMT balance adjusting error" );

         auto wmt_shares = asset( wmt_object.total_vesting_shares, vesting_wmt );
         auto wmt_vests = asset( wmt_object.total_vesting_fund_wmt, liquid_wmt );
         auto wmt_share_price = wmt_object.get_vesting_share_price();
         auto alice_wmt_shares = db->get_balance( "alice", vesting_wmt );
         auto bob_wmt_shares = db->get_balance( "bob", vesting_wmt );;

         // Self transfer Alice's liquid
         transfer_to_vesting_operation op;
         op.from = "alice";
         op.to = "";
         op.amount = asset( 7500, liquid_wmt );
         PUSH_OP( op, key );

         auto new_vest = op.amount * wmt_share_price;
         wmt_shares += new_vest;
         wmt_vests += op.amount;
         alice_wmt_shares += new_vest;

         BOOST_REQUIRE( db->get_balance( "alice", liquid_wmt ) == asset( 2500, liquid_wmt ) );
         BOOST_REQUIRE( db->get_balance( "alice", vesting_wmt ) == alice_wmt_shares );
         BOOST_REQUIRE( wmt_object.total_vesting_fund_wmt.value == wmt_vests.amount.value );
         BOOST_REQUIRE( wmt_object.total_vesting_shares.value == wmt_shares.amount.value );
         validate_database();
         wmt_share_price = wmt_object.get_vesting_share_price();

         // Transfer Alice's liquid to Bob's vests
         op.to = "bob";
         op.amount = asset( 2000, liquid_wmt );
         PUSH_OP( op, key );

         new_vest = op.amount * wmt_share_price;
         wmt_shares += new_vest;
         wmt_vests += op.amount;
         bob_wmt_shares += new_vest;

         BOOST_REQUIRE( db->get_balance( "alice", liquid_wmt ) == asset( 500, liquid_wmt ) );
         BOOST_REQUIRE( db->get_balance( "alice", vesting_wmt ) == alice_wmt_shares );
         BOOST_REQUIRE( db->get_balance( "bob", liquid_wmt ) == asset( 0, liquid_wmt ) );
         BOOST_REQUIRE( db->get_balance( "bob", vesting_wmt ) == bob_wmt_shares );
         BOOST_REQUIRE( wmt_object.total_vesting_fund_wmt.value == wmt_vests.amount.value );
         BOOST_REQUIRE( wmt_object.total_vesting_shares.value == wmt_shares.amount.value );
         validate_database();
      };

      ACTORS( (alice)(bob) )
      generate_block();

      // Create sample WMTs
      auto wmts = create_wmt_3( "alice", alice_private_key );
      test_single_wmt( wmts[0], alice_private_key);
      test_single_wmt( wmts[1], alice_private_key );
      test_single_wmt( wmts[2], alice_private_key );
   }
   FC_LOG_AND_RETHROW()
}
*/

BOOST_AUTO_TEST_CASE( wmt_create_validate )
{
   try
   {
      ACTORS( (alice) );

      BOOST_TEST_MESSAGE( " -- A valid wmt_create_operation" );
      wmt_create_operation op;
      op.control_account = "alice";
      op.wmt_creation_fee = db->get_dynamic_global_properties().wmt_creation_fee;
      op.symbol = get_new_wmt_symbol( 3, db );
      op.precision = op.symbol.decimals();
      op.validate();

      BOOST_TEST_MESSAGE( " -- Test invalid control account name" );
      op.control_account = "@@@@@";
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.control_account = "alice";

      // Test invalid creation fees.
      BOOST_TEST_MESSAGE( " -- Invalid negative creation fee" );
      op.wmt_creation_fee.amount = -op.wmt_creation_fee.amount;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Valid maximum WMT creation fee (WORTH_MAX_SHARE_SUPPLY)" );
      op.wmt_creation_fee.amount = WORTH_MAX_SHARE_SUPPLY;
      op.validate();

      BOOST_TEST_MESSAGE( " -- Invalid WMT creation fee (MAX_SHARE_SUPPLY + 1)" );
      op.wmt_creation_fee.amount++;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Invalid currency for WMT creation fee (VESTS)" );
      op.wmt_creation_fee = ASSET( "1.000000 VESTS" );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.wmt_creation_fee = db->get_dynamic_global_properties().wmt_creation_fee;

      BOOST_TEST_MESSAGE( " -- Invalid WMT creation fee: differing decimals" );
      op.precision = 0;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.precision = op.symbol.decimals();

      // Test symbol
      BOOST_TEST_MESSAGE( " -- Invalid WMT creation symbol: vesting symbol used instead of liquid one" );
      op.symbol = op.symbol.get_paired_symbol();
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Invalid WMT creation symbol: WORTH cannot be an WMT" );
      op.symbol = WORTH_SYMBOL;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Invalid WMT creation symbol: WBD cannot be an WMT" );
      op.symbol = WBD_SYMBOL;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Invalid WMT creation symbol: VESTS cannot be an WMT" );
      op.symbol = VESTS_SYMBOL;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      // If this fails, it could indicate a test above has failed for the wrong reasons
      op.symbol = get_new_wmt_symbol( 3, db );
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_create_authorities )
{
   try
   {
      WMT_SYMBOL( alice, 3, db );

      wmt_create_operation op;
      op.control_account = "alice";
      op.symbol = alice_symbol;
      op.wmt_creation_fee = db->get_dynamic_global_properties().wmt_creation_fee;

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      expected.insert( "alice" );
      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_create_duplicate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_create_duplicate" );

      ACTORS( (alice) )
      asset_symbol_type alice_symbol = create_wmt( "alice", alice_private_key, 0 );

      // We add the NAI back to the pool to ensure the test does not fail because the NAI is not in the pool
      db->modify( db->get< nai_pool_object >(), [&] ( nai_pool_object& obj )
      {
         obj.nais[ 0 ] = alice_symbol;
      } );

      // Fail on duplicate WMT lookup
      WORTH_REQUIRE_THROW( create_wmt_with_nai( "alice", alice_private_key, alice_symbol.to_nai(), alice_symbol.decimals() ), fc::assert_exception)
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( wmt_create_duplicate_differing_decimals )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_create_duplicate_differing_decimals" );

      ACTORS( (alice) )
      asset_symbol_type alice_symbol = create_wmt( "alice", alice_private_key, 3 /* Decimals */ );

      // We add the NAI back to the pool to ensure the test does not fail because the NAI is not in the pool
      db->modify( db->get< nai_pool_object >(), [&] ( nai_pool_object& obj )
      {
         obj.nais[ 0 ] = asset_symbol_type::from_nai( alice_symbol.to_nai(), 0 );
      } );

      // Fail on duplicate WMT lookup
      WORTH_REQUIRE_THROW( create_wmt_with_nai( "alice", alice_private_key, alice_symbol.to_nai(), 2 /* Decimals */ ), fc::assert_exception)
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( wmt_create_duplicate_different_users )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_create_duplicate_different_users" );

      ACTORS( (alice)(bob) )
      asset_symbol_type alice_symbol = create_wmt( "alice", alice_private_key, 0 );

      // We add the NAI back to the pool to ensure the test does not fail because the NAI is not in the pool
      db->modify( db->get< nai_pool_object >(), [&] ( nai_pool_object& obj )
      {
         obj.nais[ 0 ] = alice_symbol;
      } );

      // Fail on duplicate WMT lookup
      WORTH_REQUIRE_THROW( create_wmt_with_nai( "bob", bob_private_key, alice_symbol.to_nai(), alice_symbol.decimals() ), fc::assert_exception)
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( wmt_create_with_worth_funds )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_create_with_worth_funds" );

      // This test expects 1.000 TBD wmt_creation_fee
      db->modify( db->get_dynamic_global_properties(), [&] ( dynamic_global_property_object& dgpo )
      {
         dgpo.wmt_creation_fee = asset( 1000, WBD_SYMBOL );
      } );

      set_price_feed( price( ASSET( "1.000 TBD" ), ASSET( "1.000 TESTS" ) ) );

      ACTORS( (alice) )

      generate_block();

      FUND( "alice", ASSET( "0.999 TESTS" ) );

      wmt_create_operation op;
      op.control_account = "alice";
      op.wmt_creation_fee = ASSET( "1.000 TESTS" );
      op.symbol = get_new_wmt_symbol( 3, db );
      op.precision = op.symbol.decimals();
      op.validate();

      // Fail insufficient funds
      FAIL_WITH_OP( op, alice_private_key, fc::assert_exception );

      BOOST_REQUIRE( util::wmt::find_token( *db, op.symbol, true ) == nullptr );

      FUND( "alice", ASSET( "0.001 TESTS" ) );

      PUSH_OP( op, alice_private_key );

      BOOST_REQUIRE( util::wmt::find_token( *db, op.symbol, true ) != nullptr );
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( wmt_create_with_wbd_funds )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_create_with_wbd_funds" );

      // This test expects 1.000 TBD wmt_creation_fee
      db->modify( db->get_dynamic_global_properties(), [&] ( dynamic_global_property_object& dgpo )
      {
         dgpo.wmt_creation_fee = asset( 1000, WBD_SYMBOL );
      } );

      set_price_feed( price( ASSET( "1.000 TBD" ), ASSET( "1.000 TESTS" ) ) );

      ACTORS( (alice) )

      generate_block();

      FUND( "alice", ASSET( "0.999 TBD" ) );

      wmt_create_operation op;
      op.control_account = "alice";
      op.wmt_creation_fee = ASSET( "1.000 TBD" );
      op.symbol = get_new_wmt_symbol( 3, db );
      op.precision = op.symbol.decimals();
      op.validate();

      // Fail insufficient funds
      FAIL_WITH_OP( op, alice_private_key, fc::assert_exception );

      BOOST_REQUIRE( util::wmt::find_token( *db, op.symbol, true ) == nullptr );

      FUND( "alice", ASSET( "0.001 TBD" ) );

      PUSH_OP( op, alice_private_key );

      BOOST_REQUIRE( util::wmt::find_token( *db, op.symbol, true ) != nullptr );
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( wmt_create_with_invalid_nai )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_create_with_invalid_nai" );

      ACTORS( (alice) )

      uint32_t seed = 0;
      asset_symbol_type ast;
      uint32_t collisions = 0;
      do
      {
         BOOST_REQUIRE( collisions < WMT_MAX_NAI_GENERATION_TRIES );
         collisions++;

         ast = util::nai_generator::generate( seed++ );
      }
      while ( db->get< nai_pool_object >().contains( ast ) || util::wmt::find_token( *db, ast, true ) != nullptr );

      // Fail on NAI pool not containing this NAI
      WORTH_REQUIRE_THROW( create_wmt_with_nai( "alice", alice_private_key, ast.to_nai(), ast.decimals() ), fc::assert_exception)
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( wmt_creation_fee_test )
{
   try
   {
      ACTORS( (alice) );
      generate_block();

      set_price_feed( price( ASSET( "1.000 TBD" ), ASSET( "2.000 TESTS" ) ) );

      // This ensures that our actual wmt_creation_fee is sane in production (either WORTH or WBD)
      const dynamic_global_property_object& dgpo = db->get_dynamic_global_properties();
      FC_ASSERT( dgpo.wmt_creation_fee.symbol == WORTH_SYMBOL || dgpo.wmt_creation_fee.symbol == WBD_SYMBOL,
                "Unexpected symbol for the WMT creation fee on the dynamic global properties object: ${s}", ("s", dgpo.wmt_creation_fee.symbol) );

      FC_ASSERT( dgpo.wmt_creation_fee.amount > 0, "Expected positive wmt_creation_fee." );

      for ( int i = 0; i < 2; i++ )
      {
         FUND( "alice", ASSET( "2.000 TESTS" ) );
         FUND( "alice", ASSET( "1.000 TBD" ) );

         // These values should be equivilant as per our price feed and all tests here should work either way
         if ( !i ) // First pass
            db->modify( dgpo, [&] ( dynamic_global_property_object& dgpo )
            {
               dgpo.wmt_creation_fee = asset( 2000, WORTH_SYMBOL );
            } );
         else // Second pass
            db->modify( dgpo, [&] ( dynamic_global_property_object& dgpo )
            {
               dgpo.wmt_creation_fee = asset( 1000, WBD_SYMBOL );
            } );

         BOOST_TEST_MESSAGE( " -- Invalid creation fee, 0.001 TESTS short" );
         wmt_create_operation fail_op;
         fail_op.control_account = "alice";
         fail_op.wmt_creation_fee = ASSET( "1.999 TESTS" );
         fail_op.symbol = get_new_wmt_symbol( 3, db );
         fail_op.precision = fail_op.symbol.decimals();
         fail_op.validate();

         // Fail because we are 0.001 TESTS short of the fee
         FAIL_WITH_OP( fail_op, alice_private_key, fc::assert_exception );

         BOOST_TEST_MESSAGE( " -- Invalid creation fee, 0.001 TBD short" );
         wmt_create_operation fail_op2;
         fail_op2.control_account = "alice";
         fail_op2.wmt_creation_fee = ASSET( "0.999 TBD" );
         fail_op2.symbol = get_new_wmt_symbol( 3, db );
         fail_op2.precision = fail_op2.symbol.decimals();
         fail_op2.validate();

         // Fail because we are 0.001 TBD short of the fee
         FAIL_WITH_OP( fail_op2, alice_private_key, fc::assert_exception );

         BOOST_TEST_MESSAGE( " -- Valid creation fee, using WORTH" );
         // We should be able to pay with WORTH
         wmt_create_operation op;
         op.control_account = "alice";
         op.wmt_creation_fee = ASSET( "2.000 TESTS" );
         op.symbol = get_new_wmt_symbol( 3, db );
         op.precision = op.symbol.decimals();
         op.validate();

         // Succeed because we have paid the equivilant of 1 TBD or 2 TESTS
         PUSH_OP( op, alice_private_key );

         BOOST_TEST_MESSAGE( " -- Valid creation fee, using WBD" );
         // We should be able to pay with WBD
         wmt_create_operation op2;
         op2.control_account = "alice";
         op2.wmt_creation_fee = ASSET( "1.000 TBD" );
         op2.symbol = get_new_wmt_symbol( 3, db );
         op2.precision = op.symbol.decimals();
         op2.validate();

         // Succeed because we have paid the equivilant of 1 TBD or 2 TESTS
         PUSH_OP( op2, alice_private_key );
      }
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_create_reset )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing wmt_create_operation reset" );

      ACTORS( (alice) )
      generate_block();

      set_price_feed( price( ASSET( "1.000 TBD" ), ASSET( "1.000 TESTS" ) ) );
      fund( "alice", ASSET( "100.000 TESTS" ) );

       WMT_SYMBOL( alice, 3, db )

      db_plugin->debug_update( [=]( database& db )
      {
         db.create< wmt_token_object >( [&]( wmt_token_object& o )
         {
            o.control_account = "alice";
            o.liquid_symbol = alice_symbol;
         });
      });

      generate_block();

      signed_transaction tx;
      wmt_setup_emissions_operation op1;
      op1.control_account = "alice";
      op1.symbol = alice_symbol;
      op1.emissions_unit.token_unit[ "alice" ] = 10;
      op1.schedule_time = db->head_block_time() + fc::days(30);
      op1.interval_seconds = WMT_EMISSION_MIN_INTERVAL_SECONDS;
      op1.interval_count = 1;
      op1.lep_abs_amount = asset( 0, alice_symbol );
      op1.rep_abs_amount = asset( 0, alice_symbol );
      op1.lep_rel_amount_numerator = 1;
      op1.rep_rel_amount_numerator = 0;

      wmt_setup_emissions_operation op2;
      op2.control_account = "alice";
      op2.symbol = alice_symbol;
      op2.emissions_unit.token_unit[ "alice" ] = 10;
      op2.schedule_time = op1.schedule_time + fc::days( 365 );
      op2.interval_seconds = WMT_EMISSION_MIN_INTERVAL_SECONDS;
      op2.interval_count = 10;
      op2.lep_abs_amount = asset( 0, alice_symbol );
      op2.rep_abs_amount = asset( 0, alice_symbol );
      op2.lep_rel_amount_numerator = 1;
      op2.rep_rel_amount_numerator = 0;

      wmt_set_runtime_parameters_operation op3;
      wmt_param_windows_v1 windows;
      windows.cashout_window_seconds = 86400 * 4;
      windows.reverse_auction_window_seconds = 60 * 5;
      wmt_param_vote_regeneration_period_seconds_v1 vote_regen;
      vote_regen.vote_regeneration_period_seconds = 86400 * 6;
      vote_regen.votes_per_regeneration_period = 600;
      wmt_param_rewards_v1 rewards;
      rewards.content_constant = uint128_t( uint64_t( 1000000000000ull ) );
      rewards.percent_curation_rewards = 15 * WORTH_1_PERCENT;
      rewards.author_reward_curve = curve_id::quadratic;
      rewards.curation_reward_curve = curve_id::linear;
      wmt_param_allow_downvotes downvotes;
      downvotes.value = false;
      op3.runtime_parameters.insert( windows );
      op3.runtime_parameters.insert( vote_regen );
      op3.runtime_parameters.insert( rewards );
      op3.runtime_parameters.insert( downvotes );
      op3.control_account = "alice";
      op3.symbol = alice_symbol;

      wmt_set_setup_parameters_operation op4;
      wmt_param_allow_voting voting;
      voting.value = false;
      op4.setup_parameters.insert( voting );
      op4.control_account = "alice";
      op4.symbol = alice_symbol;

      tx.operations.push_back( op1 );
      tx.operations.push_back( op2 );
      tx.operations.push_back( op3 );
      tx.operations.push_back( op4 );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "--- Failure when specifying fee" );
      auto alice_prec_4 = asset_symbol_type::from_nai( alice_symbol.to_nai(), 4 );
      wmt_create_operation op;
      op.control_account = "alice";
      op.symbol = alice_prec_4;
      op.wmt_creation_fee = ASSET( "1.000 TESTS" );
      op.precision = 4;
      tx.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Failure resetting WMT with token emissions" );
      op.wmt_creation_fee = ASSET( "0.000 TBD" );
      tx.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Failure deleting token emissions in wrong order" );
      op1.remove = true;
      op2.remove = true;
      tx.clear();
      tx.operations.push_back( op1 );
      tx.operations.push_back( op2 );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Success deleting token emissions" );
      tx.clear();
      tx.operations.push_back( op2 );
      tx.operations.push_back( op1 );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      BOOST_TEST_MESSAGE( "--- Success resetting WMT" );
      op.wmt_creation_fee = ASSET( "0.000 TBD" );
      tx.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      auto token = db->get< wmt_token_object, by_symbol >( alice_prec_4 );

      BOOST_REQUIRE( token.liquid_symbol == op.symbol );
      BOOST_REQUIRE( token.control_account == "alice" );
      BOOST_REQUIRE( token.allow_voting == true );
      BOOST_REQUIRE( token.cashout_window_seconds == WORTH_CASHOUT_WINDOW_SECONDS );
      BOOST_REQUIRE( token.reverse_auction_window_seconds == WORTH_REVERSE_AUCTION_WINDOW_SECONDS_HF20 );
      BOOST_REQUIRE( token.vote_regeneration_period_seconds == WORTH_VOTING_MANA_REGENERATION_SECONDS );
      BOOST_REQUIRE( token.votes_per_regeneration_period == WMT_DEFAULT_VOTES_PER_REGEN_PERIOD );
      BOOST_REQUIRE( token.content_constant == WORTH_CONTENT_CONSTANT_HF0 );
      BOOST_REQUIRE( token.percent_curation_rewards == WMT_DEFAULT_PERCENT_CURATION_REWARDS );
      BOOST_REQUIRE( token.author_reward_curve == curve_id::linear );
      BOOST_REQUIRE( token.curation_reward_curve == curve_id::square_root );
      BOOST_REQUIRE( token.allow_downvotes == true );

      const auto& emissions_idx = db->get_index< wmt_token_emissions_index, by_id >();
      BOOST_REQUIRE( emissions_idx.begin() == emissions_idx.end() );

      generate_block();

      BOOST_TEST_MESSAGE( "--- Failure resetting a token that has completed setup" );

      db_plugin->debug_update( [=]( database& db )
      {
         db.modify( db.get< wmt_token_object, by_symbol >( alice_prec_4 ), [&]( wmt_token_object& o )
         {
            o.phase = wmt_phase::setup_completed;
         });
      });

      tx.set_expiration( db->head_block_time() + WORTH_BLOCK_INTERVAL * 10 );
      tx.signatures.clear();
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::assert_exception );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_nai_pool_removal )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_nai_pool_removal" );

      ACTORS( (alice) )
      asset_symbol_type alice_symbol = create_wmt( "alice", alice_private_key, 0 );

      // Ensure the NAI does not exist in the pool after being registered
      BOOST_REQUIRE( !db->get< nai_pool_object >().contains( alice_symbol ) );
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( wmt_nai_pool_count )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_nai_pool_count" );
      const auto &npo = db->get< nai_pool_object >();

      // We should begin with a full NAI pool
      BOOST_REQUIRE( npo.num_available_nais == WMT_MAX_NAI_POOL_COUNT );

      ACTORS( (alice) )

      fund( "alice", 10 * 1000 * 1000 );
      this->generate_block();

      set_price_feed( price( ASSET( "1.000 TBD" ), ASSET( "1.000 TESTS" ) ) );
      convert( "alice", ASSET( "10000.000 TESTS" ) );

      // Drain the NAI pool one at a time
      for ( unsigned int i = 1; i <= WMT_MAX_NAI_POOL_COUNT; i++ )
      {
         wmt_create_operation op;
         signed_transaction tx;

         op.symbol = get_new_wmt_symbol( 0, this->db );
         op.precision = op.symbol.decimals();
         op.wmt_creation_fee = db->get_dynamic_global_properties().wmt_creation_fee;
         op.control_account = "alice";

         tx.operations.push_back( op );
         tx.set_expiration( this->db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
         tx.sign( alice_private_key, this->db->get_chain_id(), fc::ecc::bip_0062 );

         this->db->push_transaction( tx, 0 );

         BOOST_REQUIRE( npo.num_available_nais == WMT_MAX_NAI_POOL_COUNT - i );
         BOOST_REQUIRE( npo.nais[ npo.num_available_nais ] == asset_symbol_type() );
      }

      // At this point, there should be no available NAIs
      WORTH_REQUIRE_THROW( get_new_wmt_symbol( 0, this->db ), fc::assert_exception );

      this->generate_block();

      // We should end with a full NAI pool after block generation
      BOOST_REQUIRE( npo.num_available_nais == WMT_MAX_NAI_POOL_COUNT );
   }
   FC_LOG_AND_RETHROW();
}

BOOST_AUTO_TEST_CASE( wmt_setup_emissions_validate )
{
   try
   {
      ACTORS( (alice)(bob) );
      generate_block();

      asset_symbol_type alice_symbol = create_wmt( "alice", alice_private_key, 3 );

      wmt_setup_emissions_operation op;
      op.control_account = "alice";
      op.symbol = alice_symbol;
      op.emissions_unit.token_unit[ "alice" ] = 10;
      op.interval_seconds = WMT_EMISSION_MIN_INTERVAL_SECONDS;
      op.interval_count = 1;

      fc::time_point_sec schedule_time = fc::time_point::now();
      fc::time_point_sec schedule_end_time = schedule_time + fc::seconds( op.interval_seconds * op.interval_count );

      op.schedule_time = schedule_time;
      op.lep_abs_amount = asset( 0, alice_symbol );
      op.rep_abs_amount = asset( 0, alice_symbol );
      op.lep_rel_amount_numerator = 1;
      op.rep_rel_amount_numerator = 0;
      op.lep_time = schedule_time;
      op.rep_time = schedule_end_time;
      op.validate();

      BOOST_TEST_MESSAGE( " -- Invalid token symbol" );
      op.symbol = WORTH_SYMBOL;
      op.lep_abs_amount = asset( 0, WORTH_SYMBOL );
      op.rep_abs_amount = asset( 0, WORTH_SYMBOL );
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );
      op.symbol = alice_symbol;
      op.lep_abs_amount = asset( 0, alice_symbol );
      op.rep_abs_amount = asset( 0, alice_symbol );

      BOOST_TEST_MESSAGE( " -- Mismatching right endpoint token" );
      op.rep_abs_amount = asset( 0, WORTH_SYMBOL );
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );
      op.rep_abs_amount = asset( 0, alice_symbol );

      BOOST_TEST_MESSAGE( " -- Mismatching left endpoint token" );
      op.lep_abs_amount = asset( 0, WORTH_SYMBOL );
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );
      op.lep_abs_amount = asset( 0, alice_symbol );

      BOOST_TEST_MESSAGE( " -- No emissions" );
      op.lep_rel_amount_numerator = 0;
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );
      op.lep_rel_amount_numerator = 1;

      BOOST_TEST_MESSAGE( " -- Invalid control account name" );
      op.control_account = "@@@@";
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );
      op.control_account = "alice";

      BOOST_TEST_MESSAGE(" -- Empty emission unit" );
      op.emissions_unit.token_unit.clear();
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );

      BOOST_TEST_MESSAGE( " -- Invalid emission unit token unit account" );
      op.emissions_unit.token_unit[ "@@@@" ] = 10;
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );
      op.emissions_unit.token_unit.clear();
      op.emissions_unit.token_unit[ "alice" ] = 1;
      op.emissions_unit.token_unit[ "$rewards" ] = 1;
      op.emissions_unit.token_unit[ "$market_maker" ] = 1;
      op.emissions_unit.token_unit[ "$vesting" ] = 1;

      BOOST_TEST_MESSAGE( " -- Invalid schedule time" );
      op.schedule_time = WORTH_GENESIS_TIME;
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );
      op.schedule_time = fc::time_point::now();

      BOOST_TEST_MESSAGE( " -- 0 interval count" );
      op.interval_count = 0;
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );
      op.interval_count = 1;

      BOOST_TEST_MESSAGE( " -- Interval seconds too low" );
      op.interval_seconds = WMT_EMISSION_MIN_INTERVAL_SECONDS - 1;
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );
      op.interval_seconds = WMT_EMISSION_MIN_INTERVAL_SECONDS;

      BOOST_TEST_MESSAGE( " -- Negative asset left endpoint" );
      op.lep_abs_amount = asset( -1, alice_symbol );
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );
      op.lep_abs_amount = asset( 0 , alice_symbol );

      BOOST_TEST_MESSAGE( " -- Negative asset right endpoint" );
      op.rep_abs_amount = asset( -1 , alice_symbol );
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );
      op.rep_abs_amount = asset( 0 , alice_symbol );

      BOOST_TEST_MESSAGE( " -- Left endpoint time cannot be before schedule time" );
      op.lep_time -= fc::seconds( 1 );
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );
      op.lep_time += fc::seconds( 1 );

      BOOST_TEST_MESSAGE( " -- Right endpoint time cannot be after schedule end time" );
      op.rep_time += fc::seconds( 1 );
      WORTH_REQUIRE_THROW( op.validate(), fc::exception );
      op.rep_time -= fc::seconds( 1 );

      BOOST_TEST_MESSAGE( " -- Left endpoint time and right endpoint time can be anything if they're equal" );
      fc::time_point_sec tp = fc::time_point_sec( 0 );
      op.lep_time = tp;
      op.rep_time = tp;

      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_setup_emissions_authorities )
{
   try
   {
      WMT_SYMBOL( alice, 3, db );

      wmt_setup_emissions_operation op;
      op.control_account = "alice";
      fc::time_point now = fc::time_point::now();
      op.schedule_time = now;
      op.emissions_unit.token_unit[ "alice" ] = 10;
      op.lep_abs_amount = op.rep_abs_amount = asset( 1000, alice_symbol );

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      expected.insert( "alice" );
      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_setup_emissions_apply )
{
   try
   {
      ACTORS( (alice)(bob) )

      generate_block();

      asset_symbol_type alice_symbol = create_wmt( "alice", alice_private_key, 3 );

      fc::time_point_sec emissions1_schedule_time = fc::time_point::now();

      wmt_setup_emissions_operation op;
      op.control_account = "alice";
      op.symbol = alice_symbol;
      op.emissions_unit.token_unit[ "alice" ] = 10;
      op.schedule_time = emissions1_schedule_time;
      op.interval_seconds = WMT_EMISSION_MIN_INTERVAL_SECONDS;
      op.interval_count = 1;
      op.lep_abs_amount = asset( 0, alice_symbol );
      op.rep_abs_amount = asset( 0, alice_symbol );
      op.lep_rel_amount_numerator = 1;
      op.rep_rel_amount_numerator = 0;;
      op.validate();

      BOOST_TEST_MESSAGE( " -- Control account does not own WMT" );
      op.control_account = "bob";
      FAIL_WITH_OP( op, bob_private_key, fc::assert_exception );
      op.control_account = "alice";

      BOOST_TEST_MESSAGE( " -- WMT setup phase has already completed" );
      auto wmt_token = util::wmt::find_token( *db, alice_symbol );
      db->modify( *wmt_token, [&] ( wmt_token_object& obj )
      {
         obj.phase = wmt_phase::setup_completed;
      } );
      FAIL_WITH_OP( op, alice_private_key, fc::assert_exception );
      db->modify( *wmt_token, [&] ( wmt_token_object& obj )
      {
         obj.phase = wmt_phase::account_elevated;
      } );
      PUSH_OP( op, alice_private_key );

      BOOST_TEST_MESSAGE( " -- Emissions range is overlapping" );
      wmt_setup_emissions_operation op2;
      op2.control_account = "alice";
      op2.symbol = alice_symbol;
      op2.emissions_unit.token_unit[ "alice" ] = 10;
      op2.schedule_time = emissions1_schedule_time + fc::seconds( WMT_EMISSION_MIN_INTERVAL_SECONDS );
      op2.interval_seconds = WMT_EMISSION_MIN_INTERVAL_SECONDS;
      op2.interval_count = 5;
      op2.lep_abs_amount = asset( 1200, alice_symbol );
      op2.rep_abs_amount = asset( 1000, alice_symbol );
      op2.lep_rel_amount_numerator = 1;
      op2.rep_rel_amount_numerator = 2;;
      op2.validate();

      FAIL_WITH_OP( op2, alice_private_key, fc::assert_exception );

      op2.schedule_time += fc::seconds( 1 );
      PUSH_OP( op2, alice_private_key );

      BOOST_TEST_MESSAGE( " -- Emissions must be created in chronological order" );
      wmt_setup_emissions_operation op3;
      op3.control_account = "alice";
      op3.symbol = alice_symbol;
      op3.emissions_unit.token_unit[ "alice" ] = 10;
      op3.schedule_time = emissions1_schedule_time - fc::seconds( WMT_EMISSION_MIN_INTERVAL_SECONDS + 1 );
      op3.interval_seconds = WMT_EMISSION_MIN_INTERVAL_SECONDS;
      op3.interval_count = WMT_EMIT_INDEFINITELY;
      op3.lep_abs_amount = asset( 0, alice_symbol );
      op3.rep_abs_amount = asset( 1000, alice_symbol );
      op3.lep_rel_amount_numerator = 0;
      op3.rep_rel_amount_numerator = 0;;
      op3.validate();
      FAIL_WITH_OP( op3, alice_private_key, fc::assert_exception );

      op3.schedule_time = op2.schedule_time + fc::seconds( uint64_t( op2.interval_seconds ) * uint64_t( op2.interval_count ) );
      FAIL_WITH_OP( op3, alice_private_key, fc::assert_exception );

      op3.schedule_time += fc::seconds( 1 );
      PUSH_OP( op3, alice_private_key );

      BOOST_TEST_MESSAGE( " -- No more emissions permitted" );
      wmt_setup_emissions_operation op4;
      op4.control_account = "alice";
      op4.symbol = alice_symbol;
      op4.emissions_unit.token_unit[ "alice" ] = 10;
      op4.schedule_time = op3.schedule_time + fc::days( 365 );
      op4.interval_seconds = WMT_EMISSION_MIN_INTERVAL_SECONDS;
      op4.interval_count = 10;
      op4.lep_abs_amount = asset( 0, alice_symbol );
      op4.rep_abs_amount = asset( 0, alice_symbol );
      op4.lep_rel_amount_numerator = 1;
      op4.rep_rel_amount_numerator = 0;;
      op4.validate();

      FAIL_WITH_OP( op4, alice_private_key, fc::assert_exception );

      op4.schedule_time = fc::time_point_sec::maximum();
      FAIL_WITH_OP( op4, alice_private_key, fc::assert_exception );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( set_setup_parameters_validate )
{
   try
   {
      ACTORS( (alice) )
      generate_block();

      asset_symbol_type alice_symbol = create_wmt( "alice", alice_private_key, 3 );

      wmt_set_setup_parameters_operation op;
      op.control_account = "alice";
      op.symbol = alice_symbol;
      op.setup_parameters.emplace( wmt_param_allow_voting { .value = true } );

      op.validate();

      op.symbol = WORTH_SYMBOL;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception ); // Invalid symbol
      op.symbol = VESTS_SYMBOL;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception ); // Invalid symbol
      op.symbol = WBD_SYMBOL;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception ); // Invalid symbol
      op.symbol = alice_symbol;

      op.control_account = "####";
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception ); // Invalid account name
      op.control_account = "alice";

      op.setup_parameters.clear();
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception ); // Empty setup parameters
      op.setup_parameters.emplace( wmt_param_allow_voting { .value = true } );

      op.validate();

      op.setup_parameters.clear();
      op.setup_parameters.emplace( wmt_param_allow_voting { .value = false } );
      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( set_setup_parameters_authorities )
{
   try
   {
      wmt_set_setup_parameters_operation op;
      op.control_account = "alice";

      flat_set<account_name_type> auths;
      flat_set<account_name_type> expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      expected.insert( "alice" );
      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( set_setup_parameters_apply )
{
   try
   {
      ACTORS( (alice)(bob) )

      generate_block();

      FUND( "alice", 5000000 );
      FUND( "bob", 5000000 );

      set_price_feed( price( ASSET( "1.000 TBD" ), ASSET( "1.000 TESTS" ) ) );
      convert( "alice", ASSET( "5000.000 TESTS" ) );
      convert( "bob", ASSET( "5000.000 TESTS" ) );

      auto alice_symbol = create_wmt( "alice", alice_private_key, 3 );
      auto bob_symbol = create_wmt( "bob", bob_private_key, 3 );

      auto alice_token = util::wmt::find_token( *db, alice_symbol );
      auto bob_token = util::wmt::find_token( *db, bob_symbol );

      BOOST_REQUIRE( alice_token->allow_voting == true );

      wmt_set_setup_parameters_operation op;
      op.control_account = "alice";
      op.symbol = alice_symbol;

      BOOST_TEST_MESSAGE( " -- Succeed set voting to false" );
      op.setup_parameters.emplace( wmt_param_allow_voting { .value = false } );
      PUSH_OP( op, alice_private_key );

      BOOST_REQUIRE( alice_token->allow_voting == false );

      BOOST_TEST_MESSAGE( " -- Succeed set voting to true" );
      op.setup_parameters.clear();
      op.setup_parameters.emplace( wmt_param_allow_voting { .value = true } );
      PUSH_OP( op, alice_private_key );

      BOOST_REQUIRE( alice_token->allow_voting == true );

      BOOST_TEST_MESSAGE( "--- Failure with wrong control account" );
      op.symbol = bob_symbol;

      FAIL_WITH_OP( op, alice_private_key, fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Succeed set voting to true" );
      op.control_account = "bob";
      op.setup_parameters.clear();
      op.setup_parameters.emplace( wmt_param_allow_voting { .value = true } );

      PUSH_OP( op, bob_private_key );
      BOOST_REQUIRE( bob_token->allow_voting == true );


      BOOST_TEST_MESSAGE( " -- Failure with mismatching precision" );
      op.symbol.asset_num++;
      FAIL_WITH_OP( op, bob_private_key, fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Failure with non-existent asset symbol" );
      op.symbol = this->get_new_wmt_symbol( 1, db );
      FAIL_WITH_OP( op, bob_private_key, fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Succeed set voting to false" );
      op.symbol = bob_symbol;
      op.setup_parameters.clear();
      op.setup_parameters.emplace( wmt_param_allow_voting { .value = false } );
      PUSH_OP( op, bob_private_key );
      BOOST_REQUIRE( bob_token->allow_voting == false );


      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_set_runtime_parameters_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_set_runtime_parameters_validate" );

      wmt_set_runtime_parameters_operation op;

      auto new_symbol = get_new_wmt_symbol( 3, db );

      op.symbol = new_symbol;
      op.control_account = "alice";
      op.runtime_parameters.insert( wmt_param_allow_downvotes() );

      // If this fails, it could indicate a test above has failed for the wrong reasons
      op.validate();

      BOOST_TEST_MESSAGE( "--- Test invalid control account name" );
      op.control_account = "@@@@@";
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.control_account = "alice";

      // Test symbol
      BOOST_TEST_MESSAGE( "--- Invalid WMT creation symbol: vesting symbol used instead of liquid one" );
      op.symbol = op.symbol.get_paired_symbol();
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Invalid WMT creation symbol: WORTH cannot be an WMT" );
      op.symbol = WORTH_SYMBOL;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Invalid WMT creation symbol: WBD cannot be an WMT" );
      op.symbol = WBD_SYMBOL;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Invalid WMT creation symbol: VESTS cannot be an WMT" );
      op.symbol = VESTS_SYMBOL;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.symbol = new_symbol;

      BOOST_TEST_MESSAGE( "--- Failure when no parameters are set" );
      op.runtime_parameters.clear();
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      /*
       * Inequality to test:
       *
       * 0 <= reverse_auction_window_seconds + WMT_UPVOTE_LOCKOUT < cashout_window_seconds
       * <= WMT_VESTING_WITHDRAW_INTERVAL_SECONDS
       */

      BOOST_TEST_MESSAGE( "--- Failure when cashout_window_second is equal to WMT_UPVOTE_LOCKOUT" );
      wmt_param_windows_v1 windows;
      windows.reverse_auction_window_seconds = 0;
      windows.cashout_window_seconds = WMT_UPVOTE_LOCKOUT;
      op.runtime_parameters.insert( windows );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Success when cashout_window_seconds is above WMT_UPVOTE_LOCKOUT" );
      windows.cashout_window_seconds++;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( windows );
      op.validate();

      BOOST_TEST_MESSAGE( "--- Failure when cashout_window_seconds is equal to reverse_auction_window_seconds + WMT_UPVOTE_LOCKOUT" );
      windows.reverse_auction_window_seconds++;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( windows );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Failure when cashout_window_seconds is greater than WMT_VESTING_WITHDRAW_INTERVAL_SECONDS" );
      windows.cashout_window_seconds = WMT_VESTING_WITHDRAW_INTERVAL_SECONDS + 1;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( windows );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Success when cashout_window_seconds is equal to WMT_VESTING_WITHDRAW_INTERVAL_SECONDS" );
      windows.cashout_window_seconds--;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( windows );
      op.validate();

      BOOST_TEST_MESSAGE( "--- Success when reverse_auction_window_seconds + WMT_UPVOTE_LOCKOUT is one less than cashout_window_seconds" );
      windows.reverse_auction_window_seconds = windows.cashout_window_seconds - WMT_UPVOTE_LOCKOUT - 1;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( windows );
      op.validate();

      BOOST_TEST_MESSAGE( "--- Failure when reverse_auction_window_seconds + WMT_UPVOTE_LOCKOUT is equal to cashout_window_seconds" );
      windows.cashout_window_seconds--;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( windows );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      /*
       * Conditions to test:
       *
       * 0 < vote_regeneration_seconds < WMT_VESTING_WITHDRAW_INTERVAL_SECONDS
       *
       * votes_per_regeneration_period * 86400 / vote_regeneration_period
       * <= WMT_MAX_NOMINAL_VOTES_PER_DAY
       *
       * 0 < votes_per_regeneration_period <= WMT_MAX_VOTES_PER_REGENERATION
       */
      uint32_t practical_regen_seconds_lower_bound = 86400 / WMT_MAX_NOMINAL_VOTES_PER_DAY;

      BOOST_TEST_MESSAGE( "--- Failure when vote_regeneration_period_seconds is 0" );
      wmt_param_vote_regeneration_period_seconds_v1 vote_regen;
      vote_regen.vote_regeneration_period_seconds = 0;
      vote_regen.votes_per_regeneration_period = 1;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Success when vote_regeneration_period_seconds is greater than 0" );
      // Any value less than 86 will violate the nominal votes per day check. 86 is a practical minimum as a consequence.
      vote_regen.vote_regeneration_period_seconds = practical_regen_seconds_lower_bound + 1;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      op.validate();

      BOOST_TEST_MESSAGE( "--- Failure when vote_regeneration_period_seconds is greater WMT_VESTING_WITHDRAW_INTERVAL_SECONDS" );
      vote_regen.vote_regeneration_period_seconds = WMT_VESTING_WITHDRAW_INTERVAL_SECONDS + 1;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Success when vote_regeneration_period_seconds is equal to WMT_VESTING_WITHDRAW_INTERVAL_SECONDS" );
      vote_regen.vote_regeneration_period_seconds--;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      op.validate();

      BOOST_TEST_MESSAGE( "--- Test various \"nominal votes per day\" scenarios" );
      BOOST_TEST_MESSAGE( "--- Mid Point Checks" );
      vote_regen.vote_regeneration_period_seconds = 86400;
      vote_regen.votes_per_regeneration_period = WMT_MAX_NOMINAL_VOTES_PER_DAY;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      op.validate();

      vote_regen.vote_regeneration_period_seconds = 86399;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      vote_regen.vote_regeneration_period_seconds = 86400;
      vote_regen.votes_per_regeneration_period = WMT_MAX_NOMINAL_VOTES_PER_DAY + 1;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      vote_regen.vote_regeneration_period_seconds = 86401;
      vote_regen.votes_per_regeneration_period = WMT_MAX_NOMINAL_VOTES_PER_DAY;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      op.validate();

      vote_regen.vote_regeneration_period_seconds = 86400;
      vote_regen.votes_per_regeneration_period = WMT_MAX_NOMINAL_VOTES_PER_DAY - 1;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      op.validate();

      BOOST_TEST_MESSAGE( "--- Lower Bound Checks" );
      vote_regen.vote_regeneration_period_seconds = practical_regen_seconds_lower_bound;
      vote_regen.votes_per_regeneration_period = 1;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      vote_regen.vote_regeneration_period_seconds = practical_regen_seconds_lower_bound + 1;
      vote_regen.votes_per_regeneration_period = 2;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      vote_regen.vote_regeneration_period_seconds = practical_regen_seconds_lower_bound + 2;
      vote_regen.votes_per_regeneration_period = 1;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      op.validate();

      BOOST_TEST_MESSAGE( "--- Upper Bound Checks" );
      vote_regen.vote_regeneration_period_seconds = WMT_VESTING_WITHDRAW_INTERVAL_SECONDS;
      vote_regen.votes_per_regeneration_period = WMT_MAX_VOTES_PER_REGENERATION;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      op.validate();

      vote_regen.votes_per_regeneration_period = WMT_MAX_VOTES_PER_REGENERATION + 1;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      vote_regen.vote_regeneration_period_seconds = WMT_VESTING_WITHDRAW_INTERVAL_SECONDS - 1;
      vote_regen.votes_per_regeneration_period = WMT_MAX_VOTES_PER_REGENERATION;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      vote_regen.vote_regeneration_period_seconds = WMT_VESTING_WITHDRAW_INTERVAL_SECONDS;
      vote_regen.votes_per_regeneration_period = WMT_MAX_VOTES_PER_REGENERATION - 1;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( vote_regen );
      op.validate();

      /*
       * Conditions to test:
       *
       * percent_curation_rewards <= 10000
       *
       * percent_content_rewards + percent_curation_rewards == 10000
       *
       * author_reward_curve must be quadratic or linear
       *
       * curation_reward_curve must be bounded_curation, linear, or square_root
       */
      BOOST_TEST_MESSAGE( "--- Failure when percent_curation_rewards greater than 10000" );
      wmt_param_rewards_v1 rewards;
      rewards.content_constant = WORTH_CONTENT_CONSTANT_HF0;
      rewards.percent_curation_rewards = WORTH_100_PERCENT + 1;
      rewards.author_reward_curve = curve_id::linear;
      rewards.curation_reward_curve = curve_id::square_root;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( rewards );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Success when percent_curation_rewards is 10000" );
      rewards.percent_curation_rewards = WORTH_100_PERCENT;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( rewards );
      op.validate();

      BOOST_TEST_MESSAGE( "--- Success when author curve is quadratic" );
      rewards.author_reward_curve = curve_id::quadratic;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( rewards );
      op.validate();

      BOOST_TEST_MESSAGE( "--- Failure when author curve is bounded_curation" );
      rewards.author_reward_curve = curve_id::bounded_curation;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( rewards );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Failure when author curve is square_root" );
      rewards.author_reward_curve = curve_id::square_root;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( rewards );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Success when curation curve is bounded_curation" );
      rewards.author_reward_curve = curve_id::linear;
      rewards.curation_reward_curve = curve_id::bounded_curation;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( rewards );
      op.validate();

      BOOST_TEST_MESSAGE( "--- Success when curation curve is linear" );
      rewards.curation_reward_curve = curve_id::linear;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( rewards );
      op.validate();

      BOOST_TEST_MESSAGE( "--- Failure when curation curve is quadratic" );
      rewards.curation_reward_curve = curve_id::quadratic;
      op.runtime_parameters.clear();
      op.runtime_parameters.insert( rewards );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );

      // Literally nothing to test for wmt_param_allow_downvotes because it can only be true or false.
      // Inclusion success was tested in initial positive validation at the beginning of the test.
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_set_runtime_parameters_authorities )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_set_runtime_parameters_authorities" );
      wmt_set_runtime_parameters_operation op;
      op.control_account = "alice";

      flat_set< account_name_type > auths;
      flat_set< account_name_type > expected;

      op.get_required_owner_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      op.get_required_posting_authorities( auths );
      BOOST_REQUIRE( auths == expected );

      expected.insert( "alice" );
      op.get_required_active_authorities( auths );
      BOOST_REQUIRE( auths == expected );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_set_runtime_parameters_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_set_runtime_parameters_evaluate" );

      ACTORS( (alice)(bob) )
      WMT_SYMBOL( alice, 3, db );
      WMT_SYMBOL( bob, 3, db );

      generate_block();

      db_plugin->debug_update( [=](database& db)
      {
         db.create< wmt_token_object >( [&]( wmt_token_object& o )
         {
            o.control_account = "alice";
            o.liquid_symbol = alice_symbol;
         });
      });

      wmt_set_runtime_parameters_operation op;
      signed_transaction tx;

      BOOST_TEST_MESSAGE( "--- Failure with wrong control account" );
      op.control_account = "bob";
      op.symbol = alice_symbol;
      op.runtime_parameters.insert( wmt_param_allow_downvotes() );
      tx.operations.push_back( op );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, bob_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Failure with a non-existent asset symbol" );
      op.control_account = "alice";
      op.symbol = bob_symbol;
      tx.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Failure with wrong precision in asset symbol" );
      op.symbol = alice_symbol;
      op.symbol.asset_num++;
      tx.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, 0 ), fc::assert_exception );

      BOOST_TEST_MESSAGE( "--- Success updating runtime parameters" );
      op.runtime_parameters.clear();
      // These are params different than the default
      wmt_param_windows_v1 windows;
      windows.cashout_window_seconds = 86400 * 4;
      windows.reverse_auction_window_seconds = 60 * 5;
      wmt_param_vote_regeneration_period_seconds_v1 vote_regen;
      vote_regen.vote_regeneration_period_seconds = 86400 * 6;
      vote_regen.votes_per_regeneration_period = 600;
      wmt_param_rewards_v1 rewards;
      rewards.content_constant = uint128_t( uint64_t( 1000000000000ull ) );
      rewards.percent_curation_rewards = 15 * WORTH_1_PERCENT;
      rewards.author_reward_curve = curve_id::quadratic;
      rewards.curation_reward_curve = curve_id::linear;
      wmt_param_allow_downvotes downvotes;
      downvotes.value = false;
      op.runtime_parameters.insert( windows );
      op.runtime_parameters.insert( vote_regen );
      op.runtime_parameters.insert( rewards );
      op.runtime_parameters.insert( downvotes );
      op.symbol = alice_symbol;
      tx.clear();
      tx.operations.push_back( op );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      const auto& token = db->get< wmt_token_object, by_symbol >( alice_symbol );

      BOOST_REQUIRE( token.cashout_window_seconds == windows.cashout_window_seconds );
      BOOST_REQUIRE( token.reverse_auction_window_seconds == windows.reverse_auction_window_seconds );
      BOOST_REQUIRE( token.vote_regeneration_period_seconds == vote_regen.vote_regeneration_period_seconds );
      BOOST_REQUIRE( token.votes_per_regeneration_period == vote_regen.votes_per_regeneration_period );
      BOOST_REQUIRE( token.content_constant == rewards.content_constant );
      BOOST_REQUIRE( token.percent_curation_rewards == rewards.percent_curation_rewards );
      BOOST_REQUIRE( token.author_reward_curve == rewards.author_reward_curve );
      BOOST_REQUIRE( token.curation_reward_curve == rewards.curation_reward_curve );
      BOOST_REQUIRE( token.allow_downvotes == downvotes.value );
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_contribute_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_contribute_validate" );

      auto new_symbol = get_new_wmt_symbol( 3, db );

      wmt_contribute_operation op;
      op.contributor = "alice";
      op.contribution = asset( 1000, WORTH_SYMBOL );
      op.contribution_id = 1;
      op.symbol = new_symbol;
      op.validate();

      BOOST_TEST_MESSAGE( " -- Failure on invalid account name" );
      op.contributor = "@@@@@";
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.contributor = "alice";

      BOOST_TEST_MESSAGE( " -- Failure on negative contribution" );
      op.contribution = asset( -1, WORTH_SYMBOL );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.contribution = asset( 1000, WORTH_SYMBOL );

      BOOST_TEST_MESSAGE( " -- Failure on no contribution" );
      op.contribution = asset( 0, WORTH_SYMBOL );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.contribution = asset( 1000, WORTH_SYMBOL );

      BOOST_TEST_MESSAGE( " -- Failure on VESTS contribution" );
      op.contribution = asset( 1000, VESTS_SYMBOL );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.contribution = asset( 1000, WORTH_SYMBOL );

      BOOST_TEST_MESSAGE( " -- Failure on WBD contribution" );
      op.contribution = asset( 1000, WBD_SYMBOL );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.contribution = asset( 1000, WORTH_SYMBOL );

      BOOST_TEST_MESSAGE( " -- Failure on WMT contribution" );
      op.contribution = asset( 1000, new_symbol );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.contribution = asset( 1000, WORTH_SYMBOL );

      BOOST_TEST_MESSAGE( " -- Failure on contribution to WORTH" );
      op.symbol = WORTH_SYMBOL;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.symbol = new_symbol;

      BOOST_TEST_MESSAGE( " -- Failure on contribution to VESTS" );
      op.symbol = VESTS_SYMBOL;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.symbol = new_symbol;

      BOOST_TEST_MESSAGE( " -- Failure on contribution to WBD" );
      op.symbol = WBD_SYMBOL;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.symbol = new_symbol;

      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_contribute_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_contribute_evaluate" );

      ACTORS( (alice)(bob)(sam) );
      WMT_SYMBOL( alice, 3, db );

      generate_block();

      auto alice_asset_accumulator = asset( 0, WORTH_SYMBOL );
      auto bob_asset_accumulator = asset( 0, WORTH_SYMBOL );
      auto sam_asset_accumulator = asset( 0, WORTH_SYMBOL );

      auto alice_contribution_counter = 0;
      auto bob_contribution_counter = 0;
      auto sam_contribution_counter = 0;

      FUND( "sam", ASSET( "1000.000 TESTS" ) );

      generate_block();

      db_plugin->debug_update( [=] ( database& db )
      {
         db.create< wmt_token_object >( [&]( wmt_token_object& o )
         {
            o.control_account = "alice";
            o.liquid_symbol = alice_symbol;
         } );

         db.create< wmt_ico_object >( [&]( wmt_ico_object& o )
         {
            o.symbol = alice_symbol;
            o.worth_units_soft_cap = WMT_MIN_SOFT_CAP_WORTH_UNITS;
            o.worth_units_hard_cap = 99000;
         } );
      } );

      wmt_contribute_operation bob_op;
      bob_op.contributor = "bob";
      bob_op.contribution = asset( 1000, WORTH_SYMBOL );
      bob_op.contribution_id = bob_contribution_counter;
      bob_op.symbol = alice_symbol;

      wmt_contribute_operation alice_op;
      alice_op.contributor = "alice";
      alice_op.contribution = asset( 2000, WORTH_SYMBOL );
      alice_op.contribution_id = alice_contribution_counter;
      alice_op.symbol = alice_symbol;

      wmt_contribute_operation sam_op;
      sam_op.contributor = "sam";
      sam_op.contribution = asset( 3000, WORTH_SYMBOL );
      sam_op.contribution_id = sam_contribution_counter;
      sam_op.symbol = alice_symbol;

      BOOST_TEST_MESSAGE( " -- Failure on WMT not in contribution phase" );
      FAIL_WITH_OP( bob_op, bob_private_key, fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Failure on WMT not in contribution phase" );
      FAIL_WITH_OP( alice_op, alice_private_key, fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Failure on WMT not in contribution phase" );
      FAIL_WITH_OP( sam_op, sam_private_key, fc::assert_exception );

      db_plugin->debug_update( [=] ( database& db )
      {
         const wmt_token_object *token = util::wmt::find_token( db, alice_symbol );
         db.modify( *token, [&]( wmt_token_object& o )
         {
            o.phase = wmt_phase::contribution_begin_time_completed;
         } );
      } );

      BOOST_TEST_MESSAGE( " -- Failure on insufficient funds" );
      FAIL_WITH_OP( bob_op, bob_private_key, fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Failure on insufficient funds" );
      FAIL_WITH_OP( alice_op, alice_private_key, fc::assert_exception );

      FUND( "alice", ASSET( "1000.000 TESTS" ) );
      FUND( "bob",   ASSET( "1000.000 TESTS" ) );

      generate_block();

      BOOST_TEST_MESSAGE( " -- Succeed on sufficient funds" );
      bob_op.contribution_id = bob_contribution_counter++;
      PUSH_OP( bob_op, bob_private_key );
      bob_asset_accumulator += bob_op.contribution;

      BOOST_TEST_MESSAGE( " -- Failure on duplicate contribution ID" );
      FAIL_WITH_OP( bob_op, bob_private_key, fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Succeed with new contribution ID" );
      bob_op.contribution_id = bob_contribution_counter++;
      PUSH_OP( bob_op, bob_private_key );
      bob_asset_accumulator += bob_op.contribution;

      BOOST_TEST_MESSAGE( " -- Succeed on sufficient funds" );
      alice_op.contribution_id = alice_contribution_counter++;
      PUSH_OP( alice_op, alice_private_key );
      alice_asset_accumulator += alice_op.contribution;

      BOOST_TEST_MESSAGE( " -- Failure on duplicate contribution ID" );
      FAIL_WITH_OP( alice_op, alice_private_key, fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Succeed with new contribution ID" );
      alice_op.contribution_id = alice_contribution_counter++;
      PUSH_OP( alice_op, alice_private_key );
      alice_asset_accumulator += alice_op.contribution;

      BOOST_TEST_MESSAGE( " -- Succeed with sufficient funds" );
      sam_op.contribution_id = sam_contribution_counter++;
      PUSH_OP( sam_op, sam_private_key );
      sam_asset_accumulator += sam_op.contribution;

      validate_database();

      for ( int i = 0; i < 15; i++ )
      {
         BOOST_TEST_MESSAGE( " -- Successful contribution (alice)" );
         alice_op.contribution_id = alice_contribution_counter++;
         PUSH_OP( alice_op, alice_private_key );
         alice_asset_accumulator += alice_op.contribution;

         BOOST_TEST_MESSAGE( " -- Successful contribution (bob)" );
         bob_op.contribution_id = bob_contribution_counter++;
         PUSH_OP( bob_op, bob_private_key );
         bob_asset_accumulator += bob_op.contribution;

         BOOST_TEST_MESSAGE( " -- Successful contribution (sam)" );
         sam_op.contribution_id = sam_contribution_counter++;
         PUSH_OP( sam_op, sam_private_key );
         sam_asset_accumulator += sam_op.contribution;
      }

      BOOST_TEST_MESSAGE( " -- Fail to contribute after hard cap (alice)" );
      alice_op.contribution_id = alice_contribution_counter + 1;
      FAIL_WITH_OP( alice_op, alice_private_key, fc::exception );

      BOOST_TEST_MESSAGE( " -- Fail to contribute after hard cap (bob)" );
      bob_op.contribution_id = bob_contribution_counter + 1;
      FAIL_WITH_OP( bob_op, bob_private_key, fc::exception );

      BOOST_TEST_MESSAGE( " -- Fail to contribute after hard cap (sam)" );
      sam_op.contribution_id = sam_contribution_counter + 1;
      FAIL_WITH_OP( sam_op, sam_private_key, fc::exception );

      validate_database();

      generate_block();

      db_plugin->debug_update( [=] ( database& db )
      {
         const wmt_token_object *token = util::wmt::find_token( db, alice_symbol );
         db.modify( *token, [&]( wmt_token_object& o )
         {
            o.phase = wmt_phase::contribution_end_time_completed;
         } );
      } );

      BOOST_TEST_MESSAGE( " -- Failure WMT contribution phase has ended" );
      alice_op.contribution_id = alice_contribution_counter;
      FAIL_WITH_OP( alice_op, alice_private_key, fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Failure WMT contribution phase has ended" );
      bob_op.contribution_id = bob_contribution_counter;
      FAIL_WITH_OP( bob_op, bob_private_key, fc::assert_exception );

      BOOST_TEST_MESSAGE( " -- Failure WMT contribution phase has ended" );
      sam_op.contribution_id = sam_contribution_counter;
      FAIL_WITH_OP( sam_op, sam_private_key, fc::assert_exception );

      auto alices_contributions = asset( 0, WORTH_SYMBOL );
      auto bobs_contributions = asset( 0, WORTH_SYMBOL );
      auto sams_contributions = asset( 0, WORTH_SYMBOL );

      auto alices_num_contributions = 0;
      auto bobs_num_contributions = 0;
      auto sams_num_contributions = 0;

      const auto& idx = db->get_index< wmt_contribution_index, by_symbol_contributor >();

      auto itr = idx.lower_bound( boost::make_tuple( alice_symbol, account_name_type( "alice" ), 0 ) );
      while( itr != idx.end() && itr->contributor == account_name_type( "alice" ) )
      {
         alices_contributions += itr->contribution;
         alices_num_contributions++;
         ++itr;
      }

      itr = idx.lower_bound( boost::make_tuple( alice_symbol, account_name_type( "bob" ), 0 ) );
      while( itr != idx.end() && itr->contributor == account_name_type( "bob" ) )
      {
         bobs_contributions += itr->contribution;
         bobs_num_contributions++;
         ++itr;
      }

      itr = idx.lower_bound( boost::make_tuple( alice_symbol, account_name_type( "sam" ), 0 ) );
      while( itr != idx.end() && itr->contributor == account_name_type( "sam" ) )
      {
         sams_contributions += itr->contribution;
         sams_num_contributions++;
         ++itr;
      }

      BOOST_TEST_MESSAGE( " -- Checking account contributions" );
      BOOST_REQUIRE( alices_contributions == alice_asset_accumulator );
      BOOST_REQUIRE( bobs_contributions == bob_asset_accumulator );
      BOOST_REQUIRE( sams_contributions == sam_asset_accumulator );

      BOOST_TEST_MESSAGE( " -- Checking contribution counts" );
      BOOST_REQUIRE( alices_num_contributions == alice_contribution_counter );
      BOOST_REQUIRE( bobs_num_contributions == bob_contribution_counter );
      BOOST_REQUIRE( sams_num_contributions == sam_contribution_counter );

      BOOST_TEST_MESSAGE( " -- Checking account balances" );
      BOOST_REQUIRE( db->get_balance( "alice", WORTH_SYMBOL ) == ASSET( "1000.000 TESTS" ) - alice_asset_accumulator );
      BOOST_REQUIRE( db->get_balance( "bob", WORTH_SYMBOL ) == ASSET( "1000.000 TESTS" ) - bob_asset_accumulator );
      BOOST_REQUIRE( db->get_balance( "sam", WORTH_SYMBOL ) == ASSET( "1000.000 TESTS" ) - sam_asset_accumulator );

      BOOST_TEST_MESSAGE( " -- Checking ICO total contributions" );
      const auto* ico_obj = db->find< wmt_ico_object, by_symbol >( alice_symbol );
      BOOST_REQUIRE( ico_obj->contributed == alice_asset_accumulator + bob_asset_accumulator + sam_asset_accumulator );

      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_transfer_validate )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_transfer_validate" );
      ACTORS( (alice) )
      auto symbol = create_wmt( "alice", alice_private_key, 3 );

      transfer_operation op;
      op.from = "alice";
      op.to = "bob";
      op.memo = "Memo";
      op.amount = asset( 100, symbol );
      op.validate();

      BOOST_TEST_MESSAGE( " --- Invalid from account" );
      op.from = "alice-";
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.from = "alice";

      BOOST_TEST_MESSAGE( " --- Invalid to account" );
      op.to = "bob-";
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.to = "bob";

      BOOST_TEST_MESSAGE( " --- Memo too long" );
      std::string memo;
      for ( int i = 0; i < WORTH_MAX_MEMO_SIZE + 1; i++ )
         memo += "x";
      op.memo = memo;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.memo = "Memo";

      BOOST_TEST_MESSAGE( " --- Negative amount" );
      op.amount = -op.amount;
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.amount = -op.amount;

      BOOST_TEST_MESSAGE( " --- Transferring vests" );
      op.amount = asset( 100, symbol.get_paired_symbol() );
      WORTH_REQUIRE_THROW( op.validate(), fc::assert_exception );
      op.amount = asset( 100, symbol );

      op.validate();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_CASE( wmt_transfer_apply )
{
   try
   {
      BOOST_TEST_MESSAGE( "Testing: wmt_transfer_apply" );

      ACTORS( (alice)(bob) )
      generate_block();

      auto symbol = create_wmt( "alice", alice_private_key, 3 );

      fund( "alice", asset( 10000, symbol ) );

      BOOST_REQUIRE( db->get_balance( "alice", symbol ) == asset( 10000, symbol ) );
      BOOST_REQUIRE( db->get_balance( "bob", symbol ) == asset( 0, symbol ) );

      signed_transaction tx;
      transfer_operation op;

      op.from = "alice";
      op.to = "bob";
      op.amount = asset( 5000, symbol );

      BOOST_TEST_MESSAGE( "--- Test normal transaction" );
      tx.operations.push_back( op );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, alice_private_key );
      db->push_transaction( tx, 0 );

      BOOST_REQUIRE( db->get_balance( "alice", symbol ) == asset( 5000, symbol ) );
      BOOST_REQUIRE( db->get_balance( "bob", symbol ) == asset( 5000, symbol ) );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Generating a block" );
      generate_block();

      BOOST_REQUIRE( db->get_balance( "alice", symbol ) == asset( 5000, symbol ) );
      BOOST_REQUIRE( db->get_balance( "bob", symbol ) == asset( 5000, symbol ) );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test emptying an account" );
      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, alice_private_key );
      db->push_transaction( tx, database::skip_transaction_dupe_check );

      BOOST_REQUIRE( db->get_balance( "alice", symbol ) == asset( 0, symbol ) );
      BOOST_REQUIRE( db->get_balance( "bob", symbol ) == asset( 10000, symbol ) );
      validate_database();

      BOOST_TEST_MESSAGE( "--- Test transferring non-existent funds" );
      tx.signatures.clear();
      tx.operations.clear();
      tx.operations.push_back( op );
      tx.set_expiration( db->head_block_time() + WORTH_MAX_TIME_UNTIL_EXPIRATION );
      sign( tx, alice_private_key );
      WORTH_REQUIRE_THROW( db->push_transaction( tx, database::skip_transaction_dupe_check ), fc::exception );

      BOOST_REQUIRE( db->get_balance( "alice", symbol ) == asset( 0, symbol ) );
      BOOST_REQUIRE( db->get_balance( "bob", symbol ) == asset( 10000, symbol ) );
      validate_database();
   }
   FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()
#endif
