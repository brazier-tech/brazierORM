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
#include "../../include/MigrationManager.hpp"
#include "../../include/Model.hpp"
#include "../config/config.hpp"
#include "../include/migrations.hpp"
#include "../include/models.hpp"

using namespace brazier;



class DatabaseTest : public ::testing::Test {
public:
	static inline std::shared_ptr<Database> db_ptr = nullptr;
	static inline std::unique_ptr<MigrationManager> manager = nullptr;

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

    void SetUp() override {
        manager->migrate<CreateTestTableDB>();
    }

    void TearDown() override {
        manager->rollback<CreateTestTableDB>();
    }
};

static inline std::shared_ptr<Database> db_ptr = std::make_shared<Database>(db_host, db_port, db_user, db_password, db_name);

TEST_F(DatabaseTest, ConnectionTest) {
	try {
		EXPECT_NE(db_ptr->getConnection(), nullptr);
	}
	catch (const std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(DatabaseTest, ExecuteQueryTest) {
	try {
		db_ptr->execute("SELECT 1;");
	}
	catch (const std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(DatabaseTest, ModelSaveTest) {
	try {
		TestModelDB model(db_ptr);
		model.setAttribute("test", "Sample Test");
		model.setAttribute("description", "This is a sample description.");
		model.setAttribute("description", "This is a sample description.");
		EXPECT_TRUE(model.save());
	}
	catch (const std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(DatabaseTest, ModelFindTest) {
	try {
		TestModelDB::create({ {"test", "Sample Test"}, {"description", "This is a sample description."} }, 0, db_ptr)->save();
		auto model = TestModelDB::find(1, db_ptr);

		EXPECT_NE(model, nullptr);
		EXPECT_EQ(model->getAttribute("test"), "Sample Test");
	}
	catch (const std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(DatabaseTest, ModelUpdateTest) {
	try {
        TestModelDB::create({ {"test", "Sample Test"}, {"description", "This is a sample description."} }, 0, db_ptr)->save();
		TestModelDB::update(1, { {"test", "Updated Test"}, {"description", "Updated description."} }, db_ptr);
		auto model = TestModelDB::find(1, db_ptr);
		EXPECT_EQ(model->getAttribute("test"), "Updated Test");
		EXPECT_EQ(model->getAttribute("description"), "Updated description.");
	}
	catch (const std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(DatabaseTest, ModelDeleteTest) {
	try {
        TestModelDB::create({ {"test", "Sample Test"}, {"description", "This is a sample description."} }, 0, db_ptr)->save();
		auto model = TestModelDB::find(1, db_ptr);
		ASSERT_NE(model, nullptr);
		model->delete_();
		auto deletedModel = TestModelDB::find(1, db_ptr);
		EXPECT_EQ(deletedModel, nullptr);
	}
	catch (const std::exception& e) {
		FAIL() << e.what();
	}
}

TEST_F(DatabaseTest, ParamWithQuestionMarkInsert) {
    try {
        TestModelDB model(db_ptr);
        model.setAttribute("test", "what?");
        model.setAttribute("description", "is it brazier@framework.com?");
        ASSERT_TRUE(model.save());

        auto id = model.getAttribute("id_test");
        ASSERT_FALSE(id.empty());

        auto loaded = TestModelDB::find(std::stoi(id), db_ptr);
        ASSERT_NE(loaded, nullptr);
        EXPECT_EQ(loaded->getAttribute("test"), "what?");
        EXPECT_EQ(loaded->getAttribute("description"), "is it brazier@framework.com?");
    }
    catch (const std::exception& e) {
        FAIL() << e.what();
    }
}

TEST_F(DatabaseTest, ParamWithQuestionMarkUpdate) {
    try {
        TestModelDB model(db_ptr);
        model.setAttribute("test", "first");
        model.setAttribute("description", "first description");
        ASSERT_TRUE(model.save());

        auto id = std::stoi(model.getAttribute("id_test"));

        TestModelDB::update(id, {
            {"test", "second?"},
            {"description", "ends with question mark?"}
            }, db_ptr);

        auto loaded = TestModelDB::find(id, db_ptr);
        ASSERT_NE(loaded, nullptr);
        EXPECT_EQ(loaded->getAttribute("test"), "second?");
        EXPECT_EQ(loaded->getAttribute("description"), "ends with question mark?");
    }
    catch (const std::exception& e) {
        FAIL() << e.what();
    }
}

TEST_F(DatabaseTest, ParamWithSpecialCharacters) {
    try {
        TestModelDB model(db_ptr);
        model.setAttribute("test", "quote ' and ? and \"");
        model.setAttribute("description", "backslash \\ and semicolon ;");
        ASSERT_TRUE(model.save());

        auto id = std::stoi(model.getAttribute("id_test"));
        auto loaded = TestModelDB::find(id, db_ptr);
        ASSERT_NE(loaded, nullptr);
        EXPECT_EQ(loaded->getAttribute("test"), "quote ' and ? and \"");
        EXPECT_EQ(loaded->getAttribute("description"), "backslash \\ and semicolon ;");
    }
    catch (const std::exception& e) {
        FAIL() << e.what();
    }
}

TEST_F(DatabaseTest, QueryMapParameterCountMismatch) {
    EXPECT_THROW(
        db_ptr->queryMap("SELECT * FROM test_table WHERE id_test = ? AND test = ?", { "only_one" }),
        std::runtime_error
    );
}

TEST_F(DatabaseTest, ExecuteParameterCountMismatch) {
    EXPECT_THROW(
        db_ptr->execute("UPDATE test_table SET test = ? WHERE id_test = ?", { "only_one" }),
        std::runtime_error
    );
}

TEST_F(DatabaseTest, QueryMapBindsParamsCorrectly) {
    try {
        TestModelDB model(db_ptr);
        model.setAttribute("test", "find_me?");
        model.setAttribute("description", "desc");
        ASSERT_TRUE(model.save());

        auto row = db_ptr->queryMap(
            "SELECT test, description FROM test_table WHERE test = ?",
            { "find_me?" });

        ASSERT_FALSE(row.empty());
        EXPECT_EQ(row.at("test"), "find_me?");
        EXPECT_EQ(row.at("description"), "desc");
    }
    catch (const std::exception& e) {
        FAIL() << e.what();
    }
}