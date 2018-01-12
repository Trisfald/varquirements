#include <varquirements/varquirements.hpp>

struct resource_created {};
struct resource_deleted {};

// show spec part I
namespace varquirements
{
    template <>
    struct apply_property<resource_created, resource_deleted>
    {
        using type = void;
        static constexpr bool append = true;
    };
}

int main()
{
    // show simple
    {
        struct resource_created {};

        struct Create_resource : varquirements::have<resource_created> {};
        struct Use_resource : varquirements::require<resource_created> {};

        const auto value = varquirements::check_requirements<Create_resource, Use_resource>::value;
    }

    // show multiple
    {
        struct resource_created {};
        struct initialize {};

        struct Create_resource : varquirements::have<resource_created> {};
        struct Init : varquirements::require<resource_created>, varquirements::have<initialize> {};
        struct Use_resource : varquirements::require<resource_created, initialize> {};

        const auto value = varquirements::check_requirements<Create_resource, Init, Use_resource>::value;
    }

    // show spec part II
    {
        struct initialize {};

        struct Create_resource : varquirements::have<resource_created> {};
        struct Init : varquirements::require<resource_created>, varquirements::have<initialize> {};
        struct Use_resource : varquirements::require<resource_created, initialize> {};
        struct Delete_resource : varquirements::require<resource_created>, varquirements::have<resource_deleted> {};
        struct Close : varquirements::require<resource_deleted> {};

        const auto value = varquirements::check_requirements<Create_resource, Init, Use_resource, Delete_resource, Close>::value;
    }
}
