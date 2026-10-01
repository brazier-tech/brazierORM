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

#include "../include/MigrationManager.hpp"

using namespace brazier;

MigrationManager::MigrationManager(Database& db) : db(db) {
    try {
        std::vector<std::map<std::string, std::string>> result = db.queryToVector("SELECT name, down FROM migrations;");

        for (std::map<std::string, std::string>& row : result) {
            auto name_it = row.find("name");
            auto down_it = row.find("down");
            if (name_it != row.end() && down_it != row.end()) {
                executedMigrations.insert(migration{ name_it->second, down_it->second });
            }
            else {
                Logger::log("Column 'name' or 'down' not found in row", "Warning");
            }
        }
    }
    catch (const std::exception& e) {
        Logger::log("Error loading migrations", "ERROR");
        throw e;
    }
}

bool MigrationManager::isMigrationExecuted(const std::string& name) {
    for (auto& i : executedMigrations)
        if (i.name == name) return true;

    return false;
}

void MigrationManager::unmarkMigrationAsExecuted(const std::string& name) {
    db.execute("DELETE FROM migrations WHERE name = '" + name + "';");

    for (auto& i : executedMigrations)
        if (i.name == name) executedMigrations.erase(i);
}

void MigrationManager::executeQueries(const std::vector<std::string>& queries) {
    for (const auto& query : queries) {
        db.execute(query);
    }
}

bool MigrationManager::hasTable() {
    try {
        db.execute("select id from migrations;");
        return true;
    }
    catch (...) {
        return false;
    }
}

void MigrationManager::unmarkMigration(std::string name) {
    this->unmarkMigrationAsExecuted(name);
}

void MigrationManager::init(Database& db) {
    try {
        std::string name = typeid(CreateMigrationTable).name();
        db.transaction("");
        db.transaction("migration_" + name);

        auto queries = CreateMigrationTable::up();
        for (const auto& query : queries) {
            db.execute(query);
        }

        db.commit();
        Logger::log("Migration completed: " + name, "INFO");
    }
    catch (std::exception& e) {
        Logger::log(e.what(), "ERROR");
        throw std::runtime_error(e.what());
    }

}

void MigrationManager::rollbackAll() {

}