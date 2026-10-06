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
#include "../../include/Database.hpp"
#include "../../include/Model.hpp"
#include "../config/config_example.hpp"

using namespace brazier;

class TestModel : public Model<TestModel> {
public:
	static inline std::string table_name = "test_table";
	static inline std::string primary_key = "id_test";

	static inline std::vector<std::string> fillable = { "test", "description" };
	static inline std::vector<std::string> fields = { "id_test", "test", "description" };


	TestModel() = default;
	TestModel(const std::shared_ptr<Database>& db) : Model<TestModel>(db) {}
};

static inline std::shared_ptr<Database> db_ptr = std::make_shared<Database>(db_host, db_port, db_user, db_password, db_name);

TEST(DatabaseTest, ConnectionTest) {
	try {
		EXPECT_NE(db_ptr->getConnection(), nullptr);
	}
	catch (const std::exception& e) {
		FAIL() << e.what();
	}
}

TEST(DatabaseTest, ExecuteQueryTest) {
	try {
		db_ptr->execute("CREATE TABLE IF NOT EXISTS test_table (id_test SERIAL PRIMARY KEY, test VARCHAR(255), description TEXT);");
	}
	catch (const std::exception& e) {
		FAIL() << e.what();
	}
}

TEST(DatabaseTest, ModelSaveTest) {
	try {
		TestModel model(db_ptr);
		model.setAttribute("test", "Sample Test");
		model.setAttribute("description", "This is a sample description.");
		model.setAttribute("description", "This is a sample description.");
		EXPECT_TRUE(model.save());
	}
	catch (const std::exception& e) {
		FAIL() << e.what();
	}
}

TEST(DatabaseTest, ModelFindTest) {
	try {
		auto model = TestModel::find(1, db_ptr);
		EXPECT_NE(model, nullptr);
		EXPECT_EQ(model->getAttribute("test"), "Sample Test");
	}
	catch (const std::exception& e) {
		FAIL() << e.what();
	}
}

TEST(DatabaseTest, ModelUpdateTest) {
	try {
		TestModel::update(1, { {"test", "Updated Test"}, {"description", "Updated description."} }, db_ptr);
		auto model = TestModel::find(1, db_ptr);
		EXPECT_EQ(model->getAttribute("test"), "Updated Test");
		EXPECT_EQ(model->getAttribute("description"), "Updated description.");
	}
	catch (const std::exception& e) {
		FAIL() << e.what();
	}
}

TEST(DatabaseTest, ModelDeleteTest) {
	try {
		auto model = TestModel::find(1, db_ptr);
		ASSERT_NE(model, nullptr);
		model->delete_();
		auto deletedModel = TestModel::find(1, db_ptr);
		EXPECT_EQ(deletedModel, nullptr);
	}
	catch (const std::exception& e) {
		FAIL() << e.what();
	}
}

TEST(DatabaseTest, CleanupTest) {
	try {
		db_ptr->execute("DROP TABLE IF EXISTS test_table;");
	}
	catch (const std::exception& e) {
		FAIL() << e.what();
	}
}