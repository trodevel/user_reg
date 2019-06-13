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

// $Revision: 11743 $ $Date:: 2019-06-13 #$ $Author: serge $

#include "user_reg.h"                   // self

#include "utils/mutex_helper.h"         // MUTEX_SCOPE_LOCK
#include "utils/dummy_logger.h"         // dummy_log
#include "utils/utils_assert.h"         // ASSERT
#include "utils/get_now_epoch.h"        // utils::get_now_epoch()
#include "utils/gen_uuid.h"             // utils::gen_uuid


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
        const Config                & config,
        user_manager::UserManager   * user_manager )
{
    assert( user_manager );

    MUTEX_SCOPE_LOCK( mutex_ );

    dummy_log_info( MODULENAME, "init: not ready yet" );

    config_         = config;
    user_manager_   = user_manager;

    return false;
}

bool UserReg::register_new_user(
        user_manager::group_id_t    group_id,
        const std::string           & email,
        const std::string           & password_hash,
        user_manager::user_id_t     * user_id,
        std::string                 * error_msg )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    remove_expired();

    dummy_log_info( MODULENAME, "register_new_user: not ready yet" );

    auto b = user_manager_->create_and_add_user( group_id, email, password_hash, user_id, error_msg );

    if( b == false )
    {
        dummy_log_error( MODULENAME, "register_new_user: cannot add new user: %s", error_msg->c_str() );
        return false;
    }

    auto uuid       = utils::gen_uuid();
    auto expiration = utils::get_now_epoch() + config_.expiration_days * 24 * 60 * 60;

    auto & mutex = user_manager_->get_mutex();

    MUTEX_SCOPE_LOCK( mutex );

    auto user   = user_manager_->find__unlocked( * user_id );

    user->add_field( user_manager::User::STATUS,                    int( user_manager::status_e::WAITING_CONFIRMATION ) );
    user->add_field( user_manager::User::CONFIRMATION_UUID,         uuid );
    user->add_field( user_manager::User::CONFIRMATION_EXPIRATION,   int( expiration ) );

    dummy_log_info( MODULENAME, "register_new_user: id %u, uuid %s, expiration %u", * user_id, uuid.c_str(), expiration );

    return true;
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
