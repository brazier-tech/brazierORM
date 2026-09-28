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
#include "../../include/BaseMigration.hpp"
#include "../../include/MigrationManager.hpp"
#include "../../include/Database.hpp"
#include "../../include/Model.hpp"
#include "../config/config.hpp"

using namespace brazier;

static inline Database db(db_host, db_port, db_user, db_password, db_name);

class CreateTestTable : public BaseMigration<CreateTestTable> {
public:
	static std::vector<std::string> up() {
		SQLSchemaBuilder builder("test_migration");
		std::vector<std::string> queries;

		queries.push_back(builder
			.AddColumn("name varchar(255)")
			.AddColumn("email varchar(255)")
			.CreateTable()
		);

		queries.push_back(builder.AddUniqueConstraint("uq_email", { "email" }));
		queries.push_back(builder.AddIndex("idx_name", { "name" }));
			
		return std::vector<std::string>();
	}

	static std::string down() {
		SQLSchemaBuilder builder("test_migration");
		builder.DropTable();
		return std::string();
	}
};

TEST(MigrationsTest, MigrateTest) {
	try {
		MigrationManager manager(db);
		manager.migrateAll<CreateTestTable>();
	}
	catch (std::exception& e) {
		FAIL() << e.what();
	}
}