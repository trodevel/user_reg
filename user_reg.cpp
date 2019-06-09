/*

User Reg.

Copyright (C) 2019 Sergey Kolevatov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

// $Revision: 11727 $ $Date:: 2019-06-10 #$ $Author: serge $

#include "user_reg.h"                   // self

#include "utils/mutex_helper.h"         // MUTEX_SCOPE_LOCK
#include "utils/dummy_logger.h"         // dummy_log
#include "utils/utils_assert.h"         // ASSERT
#include "utils/chrono_epoch.h"         // to_epoch()


#define MODULENAME      "UserReg"

namespace user_reg
{

UserReg::UserReg():
        user_manager_( nullptr )
{
}

UserReg::~UserReg()
{
}

bool UserReg::init(
        user_manager::UserManager * user_manager )
{
    assert( user_manager );

    MUTEX_SCOPE_LOCK( mutex_ );

    dummy_log_info( MODULENAME, "init: not ready yet" );

    user_manager_   = user_manager;

    return false;
}

bool UserReg::register_new_user(
        user_manager::group_id_t    group_id,
        const std::string           & enail,
        const std::string           & password_hash,
        user_manager::user_id_t     * user_id,
        std::string                 * error_msg )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    remove_expired();

    dummy_log_info( MODULENAME, "register_new_user: not ready yet" );

    return false;
}

bool UserReg::confirm_registration(
        const std::string           & key,
        std::string                 * error_msg )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    remove_expired();

    dummy_log_info( MODULENAME, "confirm_registration: not ready yet" );

    return false;
}

void UserReg::remove_expired()
{
}

} // namespace user_reg
