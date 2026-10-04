# Brazier ORM

## Collections
A convenience container that extends std::vector<ModelType> with helper methods for common collection operations — persistence, deletion, JSON serialization, and functional-style querying.

It is designed to work with model classes that follow the brazier ORM conventions (specifically, a static `saveMany(...)` method and instance methods `delete_()` / `toJson()`).

```cpp
#include <brazier/DB>
```

Collection inherits all constructors from `std::vector<ModelType>`, plus a few convenience overloads.

```cpp
Collection<User> users;

Collection<User> users = { User(...), User(...) };

std::vector<User> vec = ...;
Collection<User> users(vec);
Collection<User> users(std::move(vec));
```

### Persistence
`bool save()`

Persists the entire collection by delegating to `ModelType::saveMany(*this)`.

Returns true on success, false on failure.

Returns false immediately (without calling `saveMany`) if the collection is empty.

```cpp
Collection<User> users;
users.push_back(User("User 1"));
users.push_back(User("User 2"));

if (!users.save()) {
    fail();
}
```

Note: The exact semantics of saveMany are defined by your model. Collection only forwards the call.

`bool delete_()`

Deletes every model in the collection by calling `item->delete_()` on each element.

Iterates over the whole collection, continuing on error.

If an individual `delete_()` throws, the exception is logged via
`Logger::log(..., "ERROR")` and the loop continues with the next item.

Returns true only if all deletions succeeded; false if the collection
is empty or any item failed.

The trailing underscore in `delete_` avoids clashing with the C++ keyword delete.

```cpp
Collection<User> users = User::where("inactive = true");
bool ok = users.delete_();

if (!ok) {
    fail();
}
```

`json toJson() const`

Returns a nlohmann::json array where each element is the result of `item->toJson()`.

```cpp
Collection<User> users = ...;
json j = users.toJson();
std::cout << j.dump(2) << std::endl;
```

Output:
```json
[
    { "id": 1, "name": "User 1" },
    { "id": 2, "name": "User 2" }
]
```

### Querying

All query methods take a predicate of type
`std::function<bool(ModelType)>` and do not modify the collection.
___
`Collection<ModelType> filter(predicate) const`

Returns a new Collection containing only the elements for which predicate returns true.
___
```cpp
auto admins = users.filter([](auto u) { return u->isAdmin(); });

bool all(predicate) const
```

Returns true if the predicate holds for every element. Returns true for an empty collection (vacuous truth, matching std::all_of).
___
```cpp
bool everyoneActive = users.all([](auto u) { return u->active; });

bool any(predicate) const
```

Returns true if the predicate holds for at least one element. Returns false for an empty collection.

___

```cpp
bool hasAdmin = users.any([](auto u) { return u->isAdmin(); });

ModelType find(predicate) const
```

Returns the first element matching the predicate, or nullptr if none match.

`auto alice = users.find([](auto u) { return u->name == "Alice"; });`

## Migrations subsystem
### Migrations
Migrations are a way to version your database schema. Each migration is a C++ class that inherits from `BaseMigration` and implements the `up()` and `down()` methods.

```cpp
class YourMigration : public BaseMigration<YourMigration>{
    static std::vector<std::string> up() {}

    static std::string down() {}
}
```

You can use the SQLSchemaBuilder class to create a query.

```cpp
static std::vector<std::string> up() {
    SQLSchemaBuilder builder("TABLE_NAME");
    std::vector<std::string> q;

    q.push_back(builder
        .AddColumn("id", "int primary key serial")
        .AddColumn("name", "varchar(255) not null")
        .AddColumn("email", "varchar(255) unique not null")
        .AddColumn("created_at", "timestamp")
        .CreateTable());

    q.push_back(builder.AddIndex("idx_email", "email"));

    return q;
}
```

This code will create two queries within the `up()` function, and they will execute sequentially. In this case, the `vector` is used to allow you to create multiple distinct queries within a single migration.

Accordingly, the function for rolling back the migration must delete the table created in the `up` function.
Example of a `down` function:

```cpp
static std::string down() {
    SQLSchemaBuilder builder("TABLE_NAME");
    return builder.DropTable();
}
```

So, the result is a ready-made migration that reflects the structure of the future table.

#### SQLSchemaBuilder

SQLSchemaBuilder is a helper class that allows you to create SQL queries in a more structured way. It contains numerous methods for working with table structures.
Listed below are its functions, which you can use when writing migrations.

- `AddColumn(columnDefinition)` : `SQLSchemaBuilder&`
- `AddPrimaryKey(column)` : `SQLSchemaBuilder&`
- `AddForeignKey(column, referenceTable, referenceColumn)` : `SQLSchemaBuilder&`
- `AlterColumn(columnDefinition)` : `SQLSchemaBuilder&`
- `DropColumn(column)` : `SQLSchemaBuilder&`
- `AddIndex(indexName, columns)` : `string`
- `AddUniqueConstraint(constraintName, columns)` : `string`
- `CreateTable()` : `string`
- `DropTable()` : `string`

You can use the "One-by-One" function for more convenient data handling.
Ultimately, the CreateTable function will generate a complete query as a string.

### Migration Manager

Now that we have the migration ready, we need to pass it to the migration manager so it can register it within its system.
First, we need to initialize the migration manager instance before the application starts running.

We can place the following line in the `main()` function to ensure the migration manager creates the migrations table if it does not already exist.

```cpp
Database db(connection_params);
MigrationManager::init(db);
```

Okay, having done that, we can now use the manager to handle migrations.
So, to start, we need to create a migration manager object, and then call the `migrateAll` or `migrate` function to perform the migration.

**NOTE:** Make sure to initialize the migration manager before calling any migration functions.

```cpp
MigrationManager manager(db);
manager.migrateAll<
    YourMigrationClass,
    AnotherYourMigrationClass
>();

// Or you can migrate a single migration class

manager.migrate<YourMigrationClass>();
```

The migration functions will first check whether the migration has been performed.

**WARNING:** If you want to migrate your migration without checking whether it has been performed, you can use the `migrateUnsafe` function. This will execute the migration regardless of whether it has been performed or not.

```cpp
/* 
This will execute the migration regardless of whether it has been performed or not.
Try to avoid this type of entry, and pay close attention to what gets included. 
*/
manager.migrateUnsafe<YourMigrationClass>(); 
```

There is also an unsafe method for rolling back a migration - `rollbackUnsafe<YourMigrationClass>()`.
**NOTE:** Migrations executed from unsafe methods are not registered in the system!

If you want to roll back the last migration using the migration manager, you need to use the `rollbackLast()` function.
You can use it multiple times. The function calls the `down()` method of the migration being rolled back and removes the record of it upon a successful rollback.
This means the next call to `rollbackLast` will roll back the second-to-last migration.

```cpp
manager.rollbackLast();
manager.rollbackLast();
manager.rollbackLast();
```

