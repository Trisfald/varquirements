#define VARQUIREMENTS_NO_STATIC_ASSERT

#include <gtest/gtest.h>
#include <varquirements/varquirements.hpp>
#include <tuple>
#include <type_traits>

using namespace varquirements;
using namespace varquirements::detail;

namespace
{
    struct i_exist {};

    struct x_created {};
    struct x_deleted {};

    template <bool B> struct j_set {};
}

namespace varquirements
{
    template <>
    struct apply_property<x_created, x_deleted>
    {
        using type = void;
        static constexpr bool append = false;
    };

    template <>
    struct apply_property<x_deleted, x_created>
    {
        using type = void;
        static constexpr bool append = true;
    };

    template <>
    struct apply_property<j_set<true>, j_set<false>>
    {
        using type = void;
        static constexpr bool append = true;
    };

    template <>
    struct apply_property<j_set<false>, j_set<true>>
    {
        using type = void;
        static constexpr bool append = true;
    };
}

TEST(varquirements, append_one)
{
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<float>, typename update_tuple_all<std::tuple<>, float>::type>::value));
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<int, float>, typename update_tuple_all<std::tuple<int>, float>::type>::value));
}

TEST(varquirements, remove_duplicates)
{
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<int, float>,
    				typename update_tuple_all<std::tuple<int, float>, int>::type>::value));
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<int, float>,
    				typename update_tuple_all<std::tuple<int, float>, float>::type>::value));
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<float, int>,
    				typename update_tuple_all<std::tuple<float, int>, int>::type>::value));
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<float, int>,
    				typename update_tuple_all<std::tuple<float, int>, float>::type>::value));
}

TEST(varquirements, multiple_appends)
{
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<float, double, long>,
    				typename update_tuple_all<std::tuple<>, float, double, long>::type>::value));
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<int, float, long, double>,
    				typename update_tuple_all<std::tuple<int, float>, long, double>::type>::value));
}

TEST(varquirements, delete_property)
{
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<>, typename update_tuple_all<std::tuple<x_created>, x_deleted>::type>::value));
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<>,
    				typename update_tuple_all<std::tuple<x_created>, x_deleted, x_created, x_deleted>::type>::value));
}

TEST(varquirements, setting_property)
{
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<j_set<false>>,
    				typename update_tuple_all<std::tuple<j_set<true>>, j_set<false>>::type>::value));
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<j_set<true>>,
    				typename update_tuple_all<std::tuple<j_set<false>>, j_set<true>>::type>::value));
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<j_set<false>>,
    				typename update_tuple_all<std::tuple<j_set<true>>,
							j_set<false>,
							j_set<true>,
							j_set<false>>::type>::value));
    EXPECT_EQ(1,
    		(std::is_same<std::tuple<j_set<true>>,
    				typename update_tuple_all<std::tuple<j_set<false>>,
							j_set<true>,
							j_set<false>,
							j_set<true>>::type>::value));
}

TEST(varquirements, check_simple)
{
    struct A : varquirements::have<i_exist> {};
    struct B : varquirements::require<i_exist> {};

    EXPECT_EQ(1, (varquirements::check_requirements<A, B>::value));
    EXPECT_EQ(0, (varquirements::check_requirements<B, A>::value));
}

TEST(varquirements, check_create_delete)
{
    struct C : varquirements::have<x_created> {};
    struct D : varquirements::have<x_deleted> {};
    struct F : varquirements::require<x_created> {};

    EXPECT_EQ(1, (varquirements::check_requirements<C, D, C, F>::value));
    EXPECT_EQ(1, (varquirements::check_requirements<D, C, F>::value));
    EXPECT_EQ(0, (varquirements::check_requirements<C, D, F>::value));
}

TEST(varquirements, check_set_unset)
{
    struct G : varquirements::have<j_set<true>> {};
    struct H : varquirements::have<j_set<false>> {};
    struct I : varquirements::require<j_set<true>> {};
    struct L : varquirements::require<j_set<false>> {};

    EXPECT_EQ(1, (varquirements::check_requirements<G, H, G, I>::value));
    EXPECT_EQ(0, (varquirements::check_requirements<G, H, G, L>::value));
    EXPECT_EQ(0, (varquirements::check_requirements<H, H, G, H, I>::value));
    EXPECT_EQ(1, (varquirements::check_requirements<H, H, G, H, L>::value));
}

TEST(varquirements, check_multiple)
{
    struct AA : varquirements::have<j_set<false>, x_created> {};
    struct AB : varquirements::have<j_set<true>> {};
    struct AC : varquirements::require<j_set<true>> {};
    struct AD : varquirements::require<x_created> {};
    struct AE : varquirements::require<x_created, j_set<true>> {};
    struct AF : varquirements::have<j_set<false>> {};

    EXPECT_EQ(1, (varquirements::check_requirements<AA, AB, AC, AD, AE>::value));
    EXPECT_EQ(1, (varquirements::check_requirements<AA, AB, AD, AE>::value));
    EXPECT_EQ(1, (varquirements::check_requirements<AA, AB, AC, AE>::value));
    EXPECT_EQ(0, (varquirements::check_requirements<AA, AB, AF, AC, AD, AE>::value));
}
