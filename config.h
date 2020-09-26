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

// $Revision: 12173 $ $Date:: 2019-10-15 #$ $Author: serge $

#ifndef USER_REG__CONFIG_H
#define USER_REG__CONFIG_H

#include <cstdint>

namespace user_reg
{

struct Config
{
    uint32_t    expiration_days;
};

} // namespace user_reg

#endif // USER_REG__CONFIG_H
