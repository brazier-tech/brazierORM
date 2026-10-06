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

#include "../../include/Model.hpp"

class TestModel : public brazier::Model<TestModel> {
public:
	TestModel() = default;
	TestModel(std::shared_ptr<Database> db) : brazier::Model<TestModel>(db) {}

	static inline std::vector<std::string> fields = { "id_test", "name", "email" };
	static inline std::vector<std::string> fillable = { "name", "email" };

	static inline std::string primary_key = "id_test";
	static inline std::string table_name = "test_migration";
};

class LargeTestModel : public brazier::Model<LargeTestModel> {
public:
	LargeTestModel() = default;
	LargeTestModel(std::shared_ptr<Database> db) : brazier::Model<LargeTestModel>(db) {}

	static inline std::vector<std::string> fields = { "id_test", "test_id", "large_test_name", "type" };
	static inline std::vector<std::string> fillable = { "test_id", "large_test_name", "type" };

	static inline std::string primary_key = "id_test";
	static inline std::string table_name = "large_test_table";
};