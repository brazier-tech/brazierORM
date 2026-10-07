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
#include "../include/models.hpp"

class TestModelCase : public ::testing::Test {
public:
	static inline std::unique_ptr<MigrationManager> manager = nullptr;
	static inline std::shared_ptr<Database> db_ptr = nullptr;

	static void SetUpTestSuite() {
		try {
			db_ptr = std::make_shared<Database>(db_host, db_port, db_user, db_password, db_name);

			MigrationManager::init(*db_ptr);
			manager = std::make_unique<MigrationManager>(*db_ptr);
		}
		catch (std::exception& e) {
			FAIL() << e.what();
		}
	}

	static void TearDownTestSuite() {
		try {
			manager->rollbackAll();
			manager->rollbackUnsafe<CreateMigrationTable>();
		}
		catch (std::exception& e) {
			FAIL() << e.what();
		}
		manager.reset();
		db_ptr.reset();
	}

	void TearDown() override {
		try {
			manager->rollbackAll();
		}
		catch (std::exception& e) {
			FAIL() << e.what();
		}
	}
};

TEST_F(TestModelCase, CreateTestModel) {
	try {
		manager->migrate<CreateTestTable>();

		std::shared_ptr<TestModel> model = TestModel::create({ {"name", "Brazier"}, {"email", "brazier@framework.com"} }, 0, db_ptr);
		bool res = model->save();
		EXPECT_EQ(res, true);
	}
	catch (std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(TestModelCase, CreateLargeTestModel) {
	try {
		manager->migrateAll<CreateTestTable, CreateLargeTestTable>();
		TestModel::create({ {"name", "Brazier"}, {"email", "brazier@framework.com"} }, 0, db_ptr)->save();
		std::shared_ptr<LargeTestModel> largeTestModel = LargeTestModel::create({ {"test_id", "1"}, {"large_test_name", "Large Test 1"}, {"type", "type1"} }, 0, db_ptr);
		bool res = largeTestModel->save();
		EXPECT_EQ(res, true);
	}
	catch (std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(TestModelCase, SaveManyTestModels) {
	try {
		manager->migrate<CreateTestTable>();
		std::shared_ptr<TestModel> model1 = TestModel::create({ {"name", "Brazier1"}, {"email", "brazier1@framework.com"} }, 0, db_ptr);
		bool res = model1->save();
		EXPECT_EQ(res, true);
	}
	catch (std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(TestModelCase, SaveManyLargeTestModels) {
	try {
		manager->migrateAll<CreateTestTable, CreateLargeTestTable>();
		TestModel::create({ {"name", "Brazier"}, {"email", "brazier@framework.com"} }, 0, db_ptr)->save();
		std::shared_ptr<LargeTestModel> largeTestModel = LargeTestModel::create({ {"test_id", "1"}, {"large_test_name", "Large Test 1"}, {"type", "type1"} }, 0, db_ptr);
		bool res = largeTestModel->save();
		EXPECT_EQ(res, true);
	}
	catch (std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(TestModelCase, SaveWithQuestions) {
	try {
		manager->migrate<CreateTestTable>();
		std::shared_ptr<TestModel> testModel = TestModel::create({ {"name", "is it Brazier?"}, {"email", "is it brazier@framework.com?"} }, 0, db_ptr);
		bool res = testModel->save();
		EXPECT_EQ(res, true);
	}
	catch (std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(TestModelCase, SaveWithQuestionAndReload) {
	try {
		manager->migrate<CreateTestTable>();
		std::shared_ptr<TestModel> testModel = TestModel::create({ {"name", "is it Brazier?"}, {"email", "is it brazier@framework.com?"} }, 0, db_ptr);
		ASSERT_TRUE(testModel->save());

		auto id = std::stoi(testModel->getAttribute("id_test"));
		auto loaded = TestModel::find(id, db_ptr);
		ASSERT_NE(loaded, nullptr);
		EXPECT_EQ(loaded->getAttribute("name"), "is it Brazier?");
		EXPECT_EQ(loaded->getAttribute("email"), "is it brazier@framework.com?");
	}
	catch (std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(TestModelCase, SaveWithMultipleQuestions) {
	try {
		manager->migrate<CreateTestTable>();
		std::shared_ptr<TestModel> testModel = TestModel::create({
			{"name", "what? where? when?"},
			{"email", "why?@framework.com"}
			}, 0, db_ptr);

		ASSERT_TRUE(testModel->save());

		auto id = std::stoi(testModel->getAttribute("id_test"));
		auto loaded = TestModel::find(id, db_ptr);
		ASSERT_NE(loaded, nullptr);
		EXPECT_EQ(loaded->getAttribute("name"), "what? where? when?");
		EXPECT_EQ(loaded->getAttribute("email"), "why?@framework.com");
	}
	catch (std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(TestModelCase, UpdateWithQuestions) {
	try {
		manager->migrate<CreateTestTable>();
		std::shared_ptr<TestModel> testModel = TestModel::create({ {"name", "Brazier"}, {"email", "brazier@framework.com"} }, 0, db_ptr);
		ASSERT_TRUE(testModel->save());

		auto id = std::stoi(testModel->getAttribute("id_test"));
		TestModel::update(id, {
			{"name", "updated?"},
			{"email", "updated?@framework.com"}
			}, db_ptr);

		auto loaded = TestModel::find(id, db_ptr);

		ASSERT_NE(loaded, nullptr);
		EXPECT_EQ(loaded->getAttribute("name"), "updated?");
		EXPECT_EQ(loaded->getAttribute("email"), "updated?@framework.com");
	}
	catch (std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(TestModelCase, SaveWithSpecialCharacters) {
	try {
		manager->migrate<CreateTestTable>();
		std::shared_ptr<TestModel> testModel = TestModel::create({
			{"name", "quote ' and ? and \"double\""},
			{"email", "backslash\\ and ; semicolon"}
			}, 0, db_ptr);
		ASSERT_TRUE(testModel->save());

		auto id = std::stoi(testModel->getAttribute("id_test"));
		auto loaded = TestModel::find(id, db_ptr);

		ASSERT_NE(loaded, nullptr);
		EXPECT_EQ(loaded->getAttribute("name"), "quote ' and ? and \"double\"");
		EXPECT_EQ(loaded->getAttribute("email"), "backslash\\ and ; semicolon");
	}
	catch (std::exception& e) {
		FAIL() << e.what();
	}
}