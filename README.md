# varquirements

Varquirements is a library that let you define and enforce arbitrary constraints between the types of a variadic pack. The types are treated in sequence and
each one can define new properties, modify existing properties or require that certain preconditions are met.


### Prerequisites
- A C++14 compiler


### Installation
Add the varquirements directory in your includes. The library is header only!  
Tests use the [google test framework](https://github.com/google/googletest). Examples are in the src directory and don't use any extra dependency.  


### How to use
For example, if we have some sort of pipeline defined by types, we can add simple constraints in this way:

```
struct resource_created {};

struct Create_resource : varquirements::have<resource_created> {};
struct Use_resource : varquirements::require<resource_created> {};

const auto value = varquirements::check_requirements<Create_resource, Use_resource>::value;
```

`varquirements::have` let us add new properties while `varquirements::require` let us state the properties that a given type requires.

There can be multiple requirements and properties:

```
struct resource_created {};
struct initialize {};

struct Create_resource : varquirements::have<resource_created> {};
struct Init : varquirements::require<resource_created>, varquirements::have<initialize> {};
struct Use_resource : varquirements::require<resource_created, initialize> {};

const auto value = varquirements::check_requirements<Create_resource, Init, Use_resource>::value;
```

#### Properties customization

The interaction between various properties can be customized like this:

```
struct resource_created {};
struct resource_deleted {};
struct initialize {};

namespace varquirements
{
    template <>
    struct apply_property<resource_created, resource_deleted>
    {
        using type = void;
        static constexpr bool append = true;
    };
}

struct Create_resource : varquirements::have<resource_created> {};
struct Init : varquirements::require<resource_created>, varquirements::have<initialize> {};
struct Use_resource : varquirements::require<resource_created, initialize> {};
struct Delete_resource : varquirements::require<resource_created>, varquirements::have<resource_deleted> {};
struct Close : varquirements::require<resource_deleted> {};

const auto value = varquirements::check_requirements<Create_resource, Init, Use_resource, Delete_resource, Close>::value;
```

#### Disable static assertions

By default, varquirements generates a static assertion when a requirements is violated. If you want to disable this behaviour you must define `VARQUIREMENTS_NO_STATIC_ASSERT` and then to manually make a check just query `varquirements::check_requirements<>::value`.
