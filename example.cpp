#include <iostream>

#include "user_reg.h"

void test_01_ok()
{
    user_manager::UserManager   um;

    user_reg::UserReg ur;

    user_reg::Config config = { 1 };

    um.init();

    ur.init( config, & um );

    user_reg::user_id_t user_id;
    std::string         registration_key;
    std::string         error_msg;

    ur.register_new_user( 1, "john.doe@example.com", "\xff\xff\xff", & user_id, & registration_key, & error_msg );
}

int main()
{
    return 0;
}
