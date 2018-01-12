// Copyright (C) 2018 Andrea Spurio. All rights reserved.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#ifndef VARQUIREMENTS_HPP_
#define VARQUIREMENTS_HPP_

#include <type_traits>
#include <tuple>

namespace varquirements
{
    namespace detail
    {
        /**
         * Empty tuple to indicate no requirements
         */
        using no_reqs = std::tuple<>;

        template <typename... Cs>
        struct require_tag {};

        template <typename... Cs>
        struct have_tag {};
    }

    /**
     * Base class that express a series of requirements
     */
    template <typename... Cs>
    struct require
    {
        using require_tag = detail::require_tag<Cs...>;
    };

    /**
     * Base class that express a series of properties
     */
    template <typename... Cs>
    struct have
    {
        using have_tag = detail::have_tag<Cs...>;
    };

    /**
     * Applies the property U to the property T.
     * Can be specialized on custom properties to add behavior.
     *
     * @param type is the resulting property of applying U to T; use void to remove T
     * @param append tells if U will be appended at the end of processing
     */
    template <typename T, typename U>
    struct apply_property
    {
        using type = T;
        static constexpr bool append = true;
    };

    template <typename T>
    struct apply_property<T, T>
    {
        using type = T;
        static constexpr bool append = false;
    };

    template <typename T, typename U>
    using apply_property_t = typename apply_property<T, U>::type;

    namespace detail
    {
        template <typename T>
        struct has_member_require_tag_impl
        {
        private:
            using yes = char[2];
            using no = char[1];

            struct fallback { int require_tag; };
            struct derived : T, fallback {};

            template <typename U>
            static no& test(decltype(U::require_tag)*);
            template <typename U>
            static yes& test(...);
        public:
            static constexpr bool value = sizeof(test<derived>(nullptr)) == sizeof(yes);
        };

        template <typename T>
        struct has_member_have_tag_impl
        {
        private:
            using yes = char[2];
            using no = char[1];

            struct fallback { int have_tag; };
            struct derived : T, fallback {};

            template <typename U>
            static no& test(decltype(U::have_tag)*);
            template <typename U>
            static yes& test(...);
        public:
            static constexpr bool value = sizeof(test<derived>(nullptr)) == sizeof(yes);
        };

        /**
         * Struct used to check if T has member require_tag
         */
        template <typename T>
        struct has_member_require_tag : public std::integral_constant<bool, has_member_require_tag_impl<T>::value> {};

        /**
         * Struct used to check if T has member require_tag
         */
        template <typename T>
        struct has_member_have_tag : public std::integral_constant<bool, has_member_have_tag_impl<T>::value> {};

        /**
         * Checks if tuple contains type
         */
        template <typename Tuple, typename T>
        struct tuple_has_type;

        template <typename T>
        struct tuple_has_type<std::tuple<>, T> : std::false_type {};

        template <typename T, typename U, typename... Ts>
        struct tuple_has_type<std::tuple<U, Ts...>, T> : tuple_has_type<std::tuple<Ts...>, T> {};

        template <typename T, typename... Ts>
        struct tuple_has_type<std::tuple<T, Ts...>, T> : std::true_type {};

        /**
         * Checks if tuple contains all types Ts
         */
        template <typename Tuple, typename... Ts>
        struct tuple_has_types;

        template <typename Tuple, typename T>
        struct tuple_has_types<Tuple, T> : tuple_has_type<Tuple, T> {};

        template <typename Tuple, typename T, typename... Ts>
        struct tuple_has_types<Tuple, T, Ts...>
        {
            static constexpr auto value = tuple_has_type<Tuple, T>::value && tuple_has_types<Tuple, Ts...>::value;
        };

        /**
         * Concatenates a type T to a tuple, only if T is not void
         */
        template <typename Tuple, typename T>
        struct tuple_concat_no_void
        {
            using type = decltype(std::tuple_cat(Tuple{}, std::tuple<T>{}));
        };

        template <typename Tuple>
        struct tuple_concat_no_void<Tuple, void>
        {
            using type = Tuple;
        };

        template <typename Tuple, typename T>
        using tuple_concat_no_void_t = typename tuple_concat_no_void<Tuple, T>::type;

        /**
         * Updates a tuple of properties with a new property T
         */
        template <typename Tuple, typename T, typename Result = std::tuple<>, bool Append = true>
        struct update_tuple;

        template <typename Tuple, typename T, typename Result = std::tuple<>, bool Append = true>
        using update_tuple_t = typename update_tuple<Tuple, T, Result, Append>::type;

        template <typename T, typename Result, bool Append, typename U, typename... Ts>
        struct update_tuple<std::tuple<U, Ts...>, T, Result, Append>
                : update_tuple<std::tuple<Ts...>,
                        T,
                        tuple_concat_no_void_t<Result, apply_property_t<U, T>>, Append && apply_property<U, T>::append> {};

        template <typename T, typename Result>
        struct update_tuple<std::tuple<>, T, Result, true>
        {
            using type = decltype(std::tuple_cat(Result{}, std::tuple<T>{}));
        };

        template <typename T, typename Result>
        struct update_tuple<std::tuple<>, T, Result, false>
        {
            using type = Result;
        };

        /**
         * Updates a tuple of properties with all new properties Ts
         */
        template <typename Tuple, typename... Ts>
        struct update_tuple_all;

        template <typename Tuple, typename... Ts>
        using update_tuple_all_t = typename update_tuple_all<Tuple, Ts...>::type;

        template <typename Tuple, typename T>
        struct update_tuple_all<Tuple, T> : update_tuple<Tuple, T> {};

        template <typename Tuple, typename T, typename... Ts>
        struct update_tuple_all<Tuple, T, Ts...>
        {
            using type = update_tuple_all_t<update_tuple_t<Tuple, T>, Ts...>;
        };

        /**
         * Uses require_tag typedef only if the condition evaluates to true
         */
        template <bool B, typename T>
        struct conditionally_use_require_tag
        {
            using type = typename T::require_tag;
        };

        template <typename T>
        struct conditionally_use_require_tag<false, T>
        {
            using type = T;
        };

        template <bool B, typename T>
        using conditionally_use_require_tag_t = typename conditionally_use_require_tag<B, T>::type;

        /**
         * Uses have_tag typedef only if the condition evaluates to true
         */
        template <bool B, typename T>
        struct conditionally_use_have_tag
        {
            using type = typename T::have_tag;
        };

        template <typename T>
        struct conditionally_use_have_tag<false, T>
        {
            using type = T;
        };

        template <bool B, typename T>
        using conditionally_use_have_tag_t = typename conditionally_use_have_tag<B, T>::type;

        template <typename Prop, typename T>
        struct assert_requirements_impl
        {
            static constexpr auto value = true;
        };

        template <typename Prop, typename... Cs>
        struct assert_requirements_impl<Prop, require_tag<Cs...>>
        {
            static constexpr auto value = tuple_has_types<Prop, Cs...>::value;
            #ifndef VARQUIREMENTS_NO_STATIC_ASSERT
                static_assert(tuple_has_types<Prop, Cs...>::value, "broken requirement");
            #endif
        };

        /**
         * Checks if T's requirements are satisfied by the current properties set
         */
        template <typename Prop, typename T>
        struct assert_requirements
        {
            static constexpr auto value = assert_requirements_impl<Prop,
                    conditionally_use_require_tag_t<has_member_require_tag<T>::value, T>>::value;
        };

        template <typename Prop, typename T>
        struct update_properties_impl
        {
            using type = Prop;
        };

        template <typename Prop, typename... Cs>
        struct update_properties_impl<Prop, have_tag<Cs...>>
        {
            using type = update_tuple_all_t<Prop, Cs...>;
        };

        /**
         * Updates properties Prop with the requirements and haves of T
         */
        template <typename Prop, typename T>
        struct update_properties
        {
            using type = typename update_properties_impl<Prop,
                    conditionally_use_have_tag_t<has_member_have_tag<T>::value,
                    T>>::type;
        };

        template <typename Prop, typename T, typename... Ts>
        struct check_requirements_impl
        {
            static constexpr auto value = assert_requirements<Prop, T>::value &&
                    check_requirements_impl<typename update_properties<Prop, T>::type, Ts...>::value;
        };

        template <typename Prop, typename T>
        struct check_requirements_impl<Prop, T>
        {
            static constexpr auto value = assert_requirements<Prop, T>::value;
        };
    }

    /**
     * Checks that all types Ts have their requirements satisfied by their preceding types
     */
    template <typename... Ts>
    struct check_requirements
    {
        static constexpr auto value = detail::check_requirements_impl<detail::no_reqs, Ts...>::value;
    };
}

#endif
