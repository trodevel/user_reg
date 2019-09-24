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

// $Revision: 12028 $ $Date:: 2019-09-24 #$ $Author: serge $

#include "user_reg.h"                   // self

#include <set>

#include "utils/mutex_helper.h"         // MUTEX_SCOPE_LOCK
#include "utils/dummy_logger.h"         // dummy_log
#include "utils/utils_assert.h"         // ASSERT
#include "utils/get_now_epoch.h"        // utils::get_now_epoch()
#include "utils/gen_uuid.h"             // utils::gen_uuid
#include "utils/epoch_to_string.h"      // utils::epoch_to_string

#define MODULENAME      "UserReg"

namespace user_reg
{

UserReg::UserReg():
        user_manager_( nullptr )
#ifdef DEBUG
, speedup_factor_( 1 )
#endif
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

    config_         = config;
    user_manager_   = user_manager;

    return true;
}

bool UserReg::register_new_user(
        user_manager::group_id_t    group_id,
        const std::string           & email,
        const std::string           & password_hash,
        user_id_t                   * user_id,
        std::string                 * registration_key,
        std::string                 * error_msg )
{
    MUTEX_SCOPE_LOCK( mutex_ );

    remove_expired();

    * registration_key          = utils::gen_uuid();

    auto b = user_manager_->create_and_add_user( group_id, email, password_hash, * registration_key, user_id, error_msg );

    if( b == false )
    {
        dummy_log_error( MODULENAME, "register_new_user: cannot add new user: %s", error_msg->c_str() );
        return false;
    }

    auto expiration = utils::get_now_epoch() + config_.expiration_days * 24 * 60 * 60
#ifdef DEBUG
            / speedup_factor_
#endif
            ;

    update_user( * user_id, expiration );

    dummy_log_info( MODULENAME, "register_new_user: id %u, registration_key %s, expiration %s (%u)", * user_id, registration_key->c_str(), utils::epoch_to_string( expiration ).c_str(), expiration );

    return true;
}

bool UserReg::confirm_registration(
        const std::string           & registration_key,
        std::string                 * error_msg )
{
    dummy_log_trace( MODULENAME, "confirm_registration: registration_key %s", registration_key.c_str() );

    MUTEX_SCOPE_LOCK( mutex_ );

    remove_expired();

    auto user   = user_manager_->find__unlocked( registration_key );

    if( user.is_empty() )
    {
        * error_msg = "invalid or expired registration_key";
        dummy_log_debug( MODULENAME, "confirm_registration: registration_key %s - not found", registration_key.c_str() );
        return false;
    }

    auto user_id = user.get_user_id();

    if( user.is_open() == false )
    {
        * error_msg = "user is already deleted";
        dummy_log_info( MODULENAME, "confirm_registration: user id %u, registration_key %s - user is deleted", user_id, registration_key.c_str() );
        return false;
    }

    auto status_v = user.get_field( user_manager::User::STATUS );

    auto status  = static_cast<user_manager::status_e>( status_v.arg_i );

    if( status != user_manager::status_e::WAITING_REGISTRATION_CONFIRMATION )
    {
        * error_msg = "user id " + std::to_string( user_id ) + " doesn't require confirmation";
        dummy_log_info( MODULENAME, "confirm_registration: user id %u, registration_key %s - user is not waiting for registration confirmation", user_id, registration_key.c_str() );
        return false;
    }

    auto now = utils::get_now_epoch();

    user.update_field( user_manager::User::STATUS, int( user_manager::status_e::ACTIVE ) );
    user.delete_field( user_manager::User::REGISTRATION_EXPIRATION );
    user.add_field( user_manager::User::REGISTRATION_TIME,    int( now ) );

    dummy_log_info( MODULENAME, "confirm_registration: user id %u - confirmed registration", user_id );

    return false;
}


void UserReg::set_speedup_factor( uint32_t factor )
{
#ifdef DEBUG
    speedup_factor_ = factor;
#endif
}

void UserReg::remove_expired()
{
    auto now = utils::get_now_epoch();

    auto res = user_manager_->select_users__unlocked( user_manager::User::REGISTRATION_EXPIRATION, anyvalue::comparison_type_e::LT, int( now ) );

    dummy_log_debug( MODULENAME, "remove_expired: found %u expired registration key(s)", res.size() );

    if( res.empty() )
        return;

    for( auto & u : res )
    {
        auto user_id = u.get_user_id();

        std::string error_msg;

        auto b = user_manager_->delete_user( user_id, & error_msg );

        if( b == false )
        {
            dummy_log_error( MODULENAME, "remove_expired: cannot delete user id %u: %s", user_id, error_msg.c_str() );
        }
    }

    dummy_log_debug( MODULENAME, "remove_expired: expired %u registration key(s)", res.size() );
}

void UserReg::update_user( user_id_t user_id, utils::epoch32_t expiration )
{
    auto & mutex = user_manager_->get_mutex();

    MUTEX_SCOPE_LOCK( mutex );

    auto user   = user_manager_->find__unlocked( user_id );

    user.add_field( user_manager::User::STATUS,                     int( user_manager::status_e::WAITING_REGISTRATION_CONFIRMATION ) );
    user.add_field( user_manager::User::REGISTRATION_EXPIRATION,    int( expiration ) );
}

} // namespace user_reg
