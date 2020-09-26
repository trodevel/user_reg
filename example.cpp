#include <iostream>

#include "user_reg.h"

#include "user_manager/str_helper.h"    // user_manager::StrHelper
#include "utils/dummy_logger.h" // dummy_logger::set_log_level
#include "utils/log_test.h"     // log_test
#include "utils/mutex_helper.h" // THIS_THREAD_SLEEP_SEC
#include "config_reader/config_reader.h"    // config_reader::ConfigReader

void dump_selection( const std::vector<user_manager::User> & vec, const std::string & comment )
{
    std::cout << comment << ":" << "\n";

    for( auto & e : vec )
    {
        std::cout << user_manager::StrHelper::to_string( e ) << "\n";
    }

    std::cout << "\n";
}

void init( user_manager::UserManager * um, user_reg::UserReg * ur, uint32_t expiration, uint32_t speedup_factor )
{
    user_reg::Config config = { expiration };

    um->init();

    ur->init( config, um );

    ur->set_speedup_factor( speedup_factor );
}

bool register_user_1(
        user_reg::UserReg           * ur,
        user_reg::user_id_t         * user_id,
        std::string                 * registration_key,
        std::string                 * error_msg )
{
    return ur->register_new_user( 1, "john.doe@example.com", "\xff\xff\xff", user_id, registration_key, error_msg );
}

bool register_user_2(
        user_reg::UserReg           * ur,
        user_reg::user_id_t         * user_id,
        std::string                 * registration_key,
        std::string                 * error_msg )
{
    return ur->register_new_user( 1, "alice.fischer@example.com", "\xaa\xaa\xaa", user_id, registration_key, error_msg );
}

bool register_user_3(
        user_reg::UserReg           * ur,
        user_reg::user_id_t         * user_id,
        std::string                 * registration_key,
        std::string                 * error_msg )
{
    return ur->register_new_user( 1, "max.mustermann@example.com", "\xe1\xe1\xe1", user_id, registration_key, error_msg );
}

void test_01_reg_ok_1()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 1, 1 );

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    auto b = register_user_1( & ur, & user_id, & registration_key, & error_msg );

    log_test( "test_01_reg_ok_1", b, true, "user was added", "cannot add user", error_msg );
}

void test_01_reg_nok_1()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 1, 1 );

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    register_user_1( & ur, & user_id, & registration_key, & error_msg );
    auto b = register_user_1( & ur, & user_id, & registration_key, & error_msg );

    log_test( "test_01_reg_nok_1", b, false, "duplicated user was not added", "unexpectedly added a duplicated user", error_msg );
}

void test_02_confirm_ok_1()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 1, 1 );

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    register_user_1( & ur, & user_id, & registration_key, & error_msg );

    auto b = ur.confirm_registration( registration_key, & error_msg );

    log_test( "test_02_confirm_ok_1", b, true, "registration was confirmed", "registration was not confirmed", error_msg );
}

void test_02_confirm_nok_1()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 2, 24 * 60 * 60 );    // expire in 2 sec

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    register_user_1( & ur, & user_id, & registration_key, & error_msg );

    THIS_THREAD_SLEEP_SEC( 3 );

    auto b = ur.confirm_registration( registration_key, & error_msg );

    log_test( "test_02_confirm_nok_1", b, false, "expired registration was not confirmed", "expired registration was unexpectedly confirmed", error_msg );
}

void test_02_confirm_nok_2()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 1, 1 );    // expire in 2 sec

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    register_user_1( & ur, & user_id, & registration_key, & error_msg );

    auto b = ur.confirm_registration( "asdasd", & error_msg );

    log_test( "test_02_confirm_nok_2", b, false, "invalid registration was not confirmed", "invalid registration was unexpectedly confirmed", error_msg );
}

void test_03_multi_reg_ok_1()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 1, 1 );

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    auto b = register_user_1( & ur, & user_id, & registration_key, & error_msg );
    b &= register_user_2( & ur, & user_id, & registration_key, & error_msg );
    b &= register_user_3( & ur, & user_id, & registration_key, & error_msg );

    log_test( "test_03_multi_reg_ok_1", b, true, "users were added", "cannot add users", error_msg );
}

void test_04_rereg_ok_1()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 2, 24 * 60 * 60 );    // expire in 2 sec

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    register_user_1( & ur, & user_id, & registration_key, & error_msg );

    THIS_THREAD_SLEEP_SEC( 3 );

    auto b = register_user_1( & ur, & user_id, & registration_key, & error_msg );

    log_test( "test_04_rereg_ok_1", b, true, "user was added", "cannot add user", error_msg );
}

void test_05_show_pending_ok_1()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 1, 1 );

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    auto b = register_user_1( & ur, & user_id, & registration_key, & error_msg );
    b &= register_user_2( & ur, & user_id, & registration_key, & error_msg );
    b &= register_user_3( & ur, & user_id, & registration_key, & error_msg );

    auto res = um.select_users__unlocked( user_manager::User::STATUS, anyvalue::comparison_type_e::EQ, int( user_manager::status_e::WAITING_REGISTRATION_CONFIRMATION ) );

    dump_selection( res, "users" );

    log_test( "test_05_show_pending_ok_1", b, res.size() == 3, "all pending users were found", "not all pending users were found", error_msg );
}

void test_05_show_pending_ok_2()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 1, 1 );

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    auto b = register_user_1( & ur, & user_id, & registration_key, & error_msg );
    b &= register_user_2( & ur, & user_id, & registration_key, & error_msg );
    b &= register_user_3( & ur, & user_id, & registration_key, & error_msg );

    b &= ur.confirm_registration( registration_key, & error_msg );

    auto res = um.select_users__unlocked( user_manager::User::STATUS, anyvalue::comparison_type_e::EQ, int( user_manager::status_e::WAITING_REGISTRATION_CONFIRMATION ) );

    dump_selection( res, "users" );

    log_test( "test_05_show_pending_ok_2", b, res.size() == 2, "all pending users were found", "not all pending users were found", error_msg );
}

void test_05_show_pending_ok_3()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 2, 24 * 60 * 60 );    // expire in 2 sec

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    auto b = register_user_1( & ur, & user_id, & registration_key, & error_msg );

    THIS_THREAD_SLEEP_SEC( 3 );

    b &= register_user_2( & ur, & user_id, & registration_key, & error_msg );

    auto res = um.select_users__unlocked( user_manager::User::STATUS, anyvalue::comparison_type_e::EQ, int( user_manager::status_e::WAITING_REGISTRATION_CONFIRMATION ) );

    dump_selection( res, "users" );

    log_test( "test_05_show_pending_ok_3", b, res.size() == 1, "all pending users were found", "not all pending users were found", error_msg );
}

void test_06_read_config()
{
    bool res = false;
    std::string error_msg;

    try
    {
        std::string config_file( "config_dummy.ini" );

        config_reader::ConfigReader cr;

        user_manager::UserManager   um;
        user_reg::UserReg ur;

        um.init();
        cr.init( config_file );

        user_reg::Config config;

        ur.init( config, & um );

        res = true;
    }
    catch( std::exception & e )
    {
        error_msg   = e.what();
    }

    log_test( "test_06_read_config", res, true, "config read successfully", "cannot read", error_msg );
}

int main()
{
    dummy_logger::set_log_level( log_levels_log4j::Debug );

    test_01_reg_ok_1();
    test_01_reg_nok_1();
    test_02_confirm_ok_1();
    test_02_confirm_nok_1();
    test_02_confirm_nok_2();
    test_03_multi_reg_ok_1();
    test_04_rereg_ok_1();
    test_05_show_pending_ok_1();
    test_05_show_pending_ok_2();
    test_05_show_pending_ok_3();
    test_06_read_config();

    return EXIT_SUCCESS;
}
