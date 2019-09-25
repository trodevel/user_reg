#include <iostream>

#include "user_reg.h"

#include "utils/dummy_logger.h" // dummy_logger::set_log_level
#include "utils/log_test.h"     // log_test
#include "utils/mutex_helper.h" // THIS_THREAD_SLEEP_SEC

void init( user_manager::UserManager * um, user_reg::UserReg * ur, uint32_t expiration, uint32_t speedup_factor )
{
    user_reg::Config config = { expiration };

    um->init();

    ur->init( config, um );

    ur->set_speedup_factor( speedup_factor );
}

void test_01_reg_ok_1()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 1, 1 );

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    auto b = ur.register_new_user( 1, "john.doe@example.com", "\xff\xff\xff", & user_id, & registration_key, & error_msg );

    log_test( "test_01_reg_ok_1", b, true, "user was added", "cannot add user", error_msg );
}

void test_02_confirm_ok_1()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 1, 1 );

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    ur.register_new_user( 1, "john.doe@example.com", "\xff\xff\xff", & user_id, & registration_key, & error_msg );

    auto b = ur.confirm_registration( registration_key, & error_msg );

    log_test( "test_02_confirm_ok_1", b, true, "registration was confirmed", "registration was not confirmed", error_msg );
}

void test_02_confirm_nok_1()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 5, 24 * 60 * 60 );    // expire in 5 sec

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    ur.register_new_user( 1, "john.doe@example.com", "\xff\xff\xff", & user_id, & registration_key, & error_msg );

    THIS_THREAD_SLEEP_SEC( 6 );

    auto b = ur.confirm_registration( registration_key, & error_msg );

    log_test( "test_02_confirm_nok_1", b, false, "expired registration was not confirmed", "expired registration was unexpectedly confirmed", error_msg );
}

void test_02_confirm_nok_2()
{
    user_manager::UserManager   um;
    user_reg::UserReg ur;

    init( & um, & ur, 1, 1 );    // expire in 5 sec

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    ur.register_new_user( 1, "john.doe@example.com", "\xff\xff\xff", & user_id, & registration_key, & error_msg );

    auto b = ur.confirm_registration( "asdasd", & error_msg );

    log_test( "test_02_confirm_nok_2", b, false, "invalid registration was not confirmed", "invalid registration was unexpectedly confirmed", error_msg );
}

int main()
{
    dummy_logger::set_log_level( log_levels_log4j::Debug );

    test_01_reg_ok_1();
    test_02_confirm_ok_1();
    test_02_confirm_nok_1();
    test_02_confirm_nok_2();

    return EXIT_SUCCESS;
}
