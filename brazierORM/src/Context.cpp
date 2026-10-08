/*
 * Copyright (c) 2026 Kirill Sergeev, Nikolay Sugonyako, Andrey Agarkov, Gleb Safyannikov
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * This file is part of brazier.
 *
 * brazier is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * brazier is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with brazier; if not, see <https://www.gnu.org/licenses/>.
 */

#include "Context.hpp"
#include "Database.hpp"

#include <stdexcept>

namespace brazier::orm {

    namespace {
        std::shared_ptr<Database> g_db;
    }

    void set_active_db(std::shared_ptr<Database> db) {
        g_db = std::move(db);
    }

    std::shared_ptr<Database> active_db() {
        if (!g_db) {
            throw std::runtime_error(
                "brazier-orm: no active DB. "
                "Did you call brazier::orm::set_active_db(db)?");
        }
        return g_db;
    }

}