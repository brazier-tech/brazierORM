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

#include "../../include/BaseMigration.hpp"

using namespace brazier;

class CreateTestTable : public BaseMigration<CreateTestTable> {
public:
	static std::vector<std::string> up() {
		SQLSchemaBuilder builder("test_migration");
		std::vector<std::string> queries;

		queries.push_back(builder
			.AddColumn("id_test serial primary key")
			.AddColumn("name varchar(255)")
			.AddColumn("email varchar(255)")
			.CreateTable()
		);

		queries.push_back(builder.AddUniqueConstraint("uq_email", { "email" }));
		queries.push_back(builder.AddIndex("idx_name", { "name" }));

		return queries;
	}

	static std::vector<std::string> down() {
		SQLSchemaBuilder builder("test_migration");
		std::vector<std::string> q;

		q.push_back(
			builder.DropTable()
		);

		return q;
	}
};

class CreateLargeTestTable : public BaseMigration<CreateLargeTestTable> {
public:
	static std::vector<std::string> up() {
		SQLSchemaBuilder builder("large_test_table");
		std::vector<std::string> q;

		q.push_back(builder
			.AddColumn("id_test serial primary key")
			.AddColumn("test_id integer not null")
			.AddForeignKey("test_id", "test_migration", "id_test")
			.AddColumn("large_test_name varchar(255) not null")
			.AddColumn("type varchar(255)")
			.CreateTable()
		);

		return q;
	}

	static std::vector<std::string> down() {
		std::vector<std::string> q;
		
		q.push_back(
			SQLSchemaBuilder("large_test_table").DropTable()
		);

		return q;
	}
};

class CreateTestTableDB : public BaseMigration<CreateTestTable> {
public:
	static std::vector<std::string> up() {
		SQLSchemaBuilder builder("test_table");
		std::vector<std::string> queries;

		queries.push_back(builder
			.AddColumn("id_test serial primary key")
			.AddColumn("test varchar(255)")
			.AddColumn("description varchar(255)")
			.CreateTable()
		);

		return queries;
	}

	static std::vector<std::string> down() {
		SQLSchemaBuilder builder("test_table");
		std::vector<std::string> q;

		q.push_back(
			builder.DropTable()
		);

		return q;
	}
};