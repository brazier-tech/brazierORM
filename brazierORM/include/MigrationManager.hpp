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

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <unordered_set>
#include <stdexcept>
#include <typeinfo>
#include "Database.hpp"
#include "SQLSchemaBuilder.hpp"
#include "BaseMigration.hpp"
#include "Logger.hpp"

namespace brazier {

    class CreateMigrationTable : public BaseMigration<CreateMigrationTable> {
    public:
        static std::vector<std::string> up() {
            SQLSchemaBuilder builder("migrations");
            std::vector<std::string> queries;

            queries.push_back(
                builder.AddColumn("id serial")
                .AddColumn("name varchar(255) unique")
                .AddColumn("down text not null")
                .CreateTable()
            );

            return queries;
        }

        static std::string down() {
            SQLSchemaBuilder builder("migrations");
            return builder.DropTable();
        }
    };

    class MigrationManager {
    private:
        struct migration {
            std::string name;
            std::string down;

            bool operator==(const migration&) const = default;
        };

        Database& db;
        std::vector<migration> executedMigrations;

        bool isMigrationExecuted(const std::string& name);
        void unmarkMigrationAsExecuted(const std::string& name);
        void executeQueries(const std::vector<std::string>& queries);

    public:
        MigrationManager(Database& db);

        void unmarkMigration(std::string name);
        void rollbackAll();
        bool hasTable();

        /*
            @brief This function performs the migration without checking for a record in the `migrations` table.
        */
        template <typename Migration>
        void migrateUnsafe() {
            std::string name = typeid(Migration).name();

            try {
                auto queries = Migration::up();
                for (const auto& query : queries) {
                    db.execute(query);
                }
            }
            catch (const std::exception& e) {
                Logger::log("Migration failed: " + name + " - " + e.what(), "ERROR");
                throw std::runtime_error(e.what());
            }
        }

        template <typename Migration>
        void rollbackUnsafe() {
            std::string name = typeid(Migration).name();

            try {
                std::string query = Migration::down();
                db.execute(query);
            }
            catch (const std::exception& e) {
                Logger::log("Migration failed: " + name + " - " + e.what(), "ERROR");
                throw std::runtime_error(e.what());
            }
        }

        template <typename Migration>
        void migrate() {
            std::string name = typeid(Migration).name();

            if (isMigrationExecuted(name)) {
                Logger::log("Migration already executed: " + name, "INFO");
                return;
            }

            try {
                db.transaction("");
                db.transaction("migration_" + name);

                auto queries = Migration::up();
                for (const auto& query : queries) {
                    db.execute(query);
                }

                markAsExecuted<Migration>();
                db.commit();
                Logger::log("Migration completed: " + name, "INFO");
            }
            catch (const std::exception& e) {
                if (db.isInTransaction()) {
                    try {
                        db.rollback("migration_" + name);
                    }
                    catch (...) {}

                    try {
                        db.rollback("");
                    }
                    catch (...) {}
                }

                Logger::log("Migration failed: " + name + " - " + e.what(), "ERROR");
                throw;
            }
        }

        template <typename Migration>
        void rollback() {
            std::string name = typeid(Migration).name();

            if (!isMigrationExecuted(name)) {
                Logger::log("Migration not executed: " + name, "ERROR");
                return;
            }

            try {
                db.transaction("");

                std::string mainQuery = Migration::down();
                db.execute(mainQuery);

                unmarkMigrationAsExecuted(name);

                db.commit();
                Logger::log("Migration rolled back: " + name, "INFO");
            }
            catch (const std::exception& e) {
                try {
                    db.rollback("");
                }
                catch (const std::exception& se) {
                    Logger::log("Could not rollback: " + std::string(se.what()), "WARNING");
                }
                Logger::log("Rollback failed: " + name + " - " + e.what(), "ERROR");
                throw;
            }
        }

        template <typename... Migrations>
        void migrateAll() {
            try {
                (migrate<Migrations>(), ...);
            }
            catch (const std::exception& e) {
                Logger::log("Migration chain stopped due to error: " + std::string(e.what()), "ERROR");
            }
        }

        template <typename... Migrations>
        void rollbackAll() {
            (rollback<Migrations>(), ...);
        }

        template <typename Migration>
        void markAsExecuted() {
            std::string name = typeid(Migration).name();
            db.execute("INSERT INTO migrations (name, down) VALUES ('" + name + "', '" + Migration::down() + "');");
            executedMigrations.push_back({ name, Migration::down() });
        }

        template <typename Migration>
        void unmarkMigration() {
            std::string name = typeid(Migration).name();
            this->unmarkMigrationAsExecuted(name);
        }

		template <typename Migration>
		bool isExecuted() {
			std::string name = typeid(Migration).name();
			return isMigrationExecuted(name);
		}

        /*
            @brief This function is required for the initial initialization of the migration subsystem. It creates the `migrations` table.
        */
        static void init(Database& db);
    };
}