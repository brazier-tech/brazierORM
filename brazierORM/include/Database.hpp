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

#pragma once

#include <string>
#include <stdexcept>
#include <memory>
#include <iostream>
#include <libpq-fe.h>
#include <map>
#include <vector>
#include "Logger.hpp"
#include "SQLString.hpp"

namespace brazier {

    class Database {
    public:
		Database() = delete;
        Database(std::string db_host, std::string db_port, std::string db_user, std::string db_password, std::string db_name);
        ~Database();
        void execute(const std::string& sql);
        void transaction(const std::string name);
        void rollback(const std::string name);
        void commit();

        bool isInTransaction() const { return in_transaction_; }

        std::string query(const std::string& sql);

        std::map<std::string, std::string> queryMap(const std::string& sql_template, const std::vector<std::string>& params);
        std::vector<std::map<std::string, std::string>> queryToVector(const std::string& sql_template, const std::vector<std::string>& params = {});

        PGconn* getConnection() const { return conn_; }

    private:
        PGconn* conn_;
        bool in_transaction_ = false;
    };
}

template<typename T>
class Array {

public:
    T ret(T value) { return value; }

};

template <typename T>
T ret(T value) {
    return value;
}
