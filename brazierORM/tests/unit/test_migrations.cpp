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

#include <exception>
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <memory>
#include "../../include/MigrationManager.hpp"
#include "../../include/Database.hpp"
#include "../../include/Model.hpp"
#include "../config/config.hpp"
#include "../include/migrations.hpp"

using namespace brazier;

static inline Database db(db_host, db_port, db_user, db_password, db_name);

class MigrationsTest : public ::testing::Test {
public:
	static inline std::unique_ptr<MigrationManager> manager = nullptr;

	static void SetUpTestSuite() {
		try {
			MigrationManager::init(db);
			manager = std::make_unique<MigrationManager>(db);
		}
		catch (std::exception& e) {
			FAIL() << e.what();
		};
	}

	static void TearDownTestSuite() {
		try {
			manager->rollbackAll();
			db.execute(CreateMigrationTable::down());
		}
		catch (std::exception& e) {
			FAIL() << e.what();
		}
	}
};

TEST_F(MigrationsTest, InitMigrationsSubsystem) {
	try {
		bool status = manager->hasTable();
		EXPECT_EQ(status, true);
	}
	catch (std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(MigrationsTest, MigrateTest) {
	try {
		manager->migrateAll<CreateTestTable>();
	}
	catch (std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(MigrationsTest, DropTableTest) {
	try {
		manager->rollback<CreateTestTable>();
		manager->unmarkMigration<CreateTestTable>();
	}
	catch (std::exception& e) {
		FAIL() << e.what();
	}
}