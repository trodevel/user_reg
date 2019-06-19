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

// $Revision: 11753 $ $Date:: 2019-06-17 #$ $Author: serge $

#include "user_reg.h"                   // self

#include <set>

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
        std::string                 * key,
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

    * key           = utils::gen_uuid();
    auto expiration = utils::get_now_epoch() + config_.expiration_days * 24 * 60 * 60;

    add_to_map( * key, * user_id, expiration );
    update_user( * user_id, * key, expiration );

    dummy_log_info( MODULENAME, "register_new_user: id %u, key %s, expiration %u", * user_id, key->c_str(), expiration );

    return true;
}

bool UserReg::confirm_registration(
        const std::string           & key,
        std::string                 * error_msg )
{
    dummy_log_trace( MODULENAME, "confirm_registration: key %s", key.c_str() );

    MUTEX_SCOPE_LOCK( mutex_ );

    remove_expired();

    auto it = map_key_to_user_status_.find( key );

    if( it == map_key_to_user_status_.end() )
    {
        * error_msg = "invalid or expired key";
        dummy_log_debug( MODULENAME, "confirm_registration: key %s - not found", key.c_str() );
        return false;
    }

    auto user_id = it->second.user_id;

    map_key_to_user_status_.erase( it );

    std::string error_msg_2;
    if( confirm_registration( user_id, & error_msg_2 ) == false )
    {
        * error_msg = "failed to change user's state: " + error_msg_2;
        return false;
    }

    dummy_log_info( MODULENAME, "confirm_registration: user id %u - confirmed", user_id );

    return false;
}

void UserReg::remove_expired()
{
    std::set<std::string> expired_keys;

    auto now = utils::get_now_epoch();

    for( auto & e : map_key_to_user_status_ )
    {
        if( now > e.second.expiration )
        {
            auto b = expired_keys.insert( e.first ).second;

            assert( b );

            std::string error_msg;

            b = user_manager_->delete_user( e.second.user_id, & error_msg );

            if( b == false )
            {
                dummy_log_error( MODULENAME, "remove_expired: cannot delete user id %u: %s", e.second.user_id, error_msg.c_str() );
            }
        }
    }

    for( auto & e : expired_keys )
    {
        map_key_to_user_status_.erase( e );
    }

    dummy_log_debug( MODULENAME, "remove_expired: expired %u key(s)", expired_keys.size() );
}

void UserReg::add_to_map( const std::string & key, user_manager::user_id_t user_id, utils::epoch32_t expiration )
{
    UserStatus us   = { user_id, expiration };

    auto b = map_key_to_user_status_.insert( std::make_pair( key, us ) ).second;

    assert( b );
}

void UserReg::update_user( user_manager::user_id_t user_id, const std::string & key, utils::epoch32_t expiration )
{
    auto & mutex = user_manager_->get_mutex();

    MUTEX_SCOPE_LOCK( mutex );

    auto user   = user_manager_->find__unlocked( user_id );

    user->add_field( user_manager::User::STATUS,                    int( user_manager::status_e::WAITING_CONFIRMATION ) );
    user->add_field( user_manager::User::CONFIRMATION_KEY,          key );
    user->add_field( user_manager::User::CONFIRMATION_EXPIRATION,   int( expiration ) );
}

bool UserReg::confirm_registration( user_manager::user_id_t user_id, std::string * error_msg )
{
    auto & mutex = user_manager_->get_mutex();

    MUTEX_SCOPE_LOCK( mutex );

    auto user   = user_manager_->find__unlocked( user_id );

    if( user == nullptr )
    {
        * error_msg = "user id " + std::to_string( user_id ) + " not found";
        return false;
    }

    if( user->is_open == false )
    {
        * error_msg = "user id " + std::to_string( user_id ) + " is deleted";
        return false;
    }

    auto status_v = user->get_field( user_manager::User::STATUS );

    auto status  = static_cast<user_manager::status_e>( status_v );

    if( status != user_manager::status_e::WAITING_CONFIRMATION )
    {
        * error_msg = "user id " + std::to_string( user_id ) + " doesn't require confirmation";
        return false;
    }

    user->update_field( user_manager::User::STATUS,                     int( user_manager::status_e::ACTIVE ) );
    user->delete_field( user_manager::User::CONFIRMATION_KEY );
    user->delete_field( user_manager::User::CONFIRMATION_EXPIRATION );

    return true;
}

} // namespace user_reg
