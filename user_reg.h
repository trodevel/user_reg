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

#ifndef USER_REG__USER_REG_H
#define USER_REG__USER_REG_H

#include <mutex>            // std::mutex
#include <map>              // std::map

#include "user_manager/user_manager.h"  // UserManager

namespace user_reg
{

struct Config
{
    uint32_t    expiration_days;
};

class UserReg
{

public:

    UserReg();
    ~UserReg();

    bool init(
            const Config                & config,
            user_manager::UserManager   * user_manager );

    bool register_new_user(
            user_manager::group_id_t    group_id,
            const std::string           & email,
            const std::string           & password_hash,
            user_manager::user_id_t     * user_id,
            std::string                 * error_msg );

    bool confirm_registration(
            const std::string           & key,
            std::string                 * error_msg );

private:

    void remove_expired();

private:

private:
    mutable std::mutex          mutex_;

    Config                      config_;
    user_manager::UserManager   * user_manager_;
};

} // namespace user_reg


#endif // USER_REG__USER_REG_H
