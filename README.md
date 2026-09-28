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