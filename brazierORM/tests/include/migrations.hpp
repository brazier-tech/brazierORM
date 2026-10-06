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