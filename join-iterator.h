/*
 * Copyright (C) 2023 Gleb Bezborodov - All Rights Reserved
 */

#pragma once

#include <type_traits>
#include <utility>
#include <stdexcept>
#include <functional>
#include <array>

namespace detail {

    struct join_iterator_tag final {};

    template<typename T, size_t I>
    struct indexed_ref_t {
        explicit indexed_ref_t(const T& in_value)
                : value(in_value) {}
        const T& value;
    };

    template<typename TValue, typename, typename... TContainer>
    class join_container_impl;

    template<typename TValue, std::size_t... Is, typename... TContainer>
    class join_container_impl<TValue, std::index_sequence<Is...>, TContainer...> : public indexed_ref_t<TContainer, Is>... {
    public:
        template<typename T, size_t I>
        struct iterator_pair_t {
            explicit iterator_pair_t(const std::pair<T, T>& in_pair)
                    : begin(in_pair.first)
                    , end(in_pair.second) { }

            T begin;
            T end;
        };

        template<typename... TIterator>
        class join_iterator_t;

        template<typename... TIterator>
        class join_iterator_t<std::index_sequence<Is...>, TIterator...> : public iterator_pair_t<TIterator, Is>... {
        public:
            explicit join_iterator_t(const std::pair<TIterator, TIterator>&... in_pair)
                    : iterator_pair_t<TIterator, Is>(in_pair)...
            {
                increment = {[this] (bool first_iteration) {
                    auto&& cur_pair = static_cast<iterator_pair_t<TIterator, Is>&>(*this);
                    if (cur_pair.begin == cur_pair.end) return true;
                    if (first_iteration)
                        ++cur_pair.begin;
                    return cur_pair.begin == cur_pair.end;
                }...};

                extract = { [this] {
                    auto&& cur_pair = static_cast<iterator_pair_t<TIterator, Is>&>(*this);
                    if (cur_pair.begin == cur_pair.end) return (TValue*)nullptr;
                    auto&& value = *cur_pair.begin;
                    return (TValue*)&value;
                }... };
            }

            join_iterator_t& operator++() {
                const auto cashed_index = active_container_index;
                while (active_container_index < increment.size() && increment[active_container_index](cashed_index == active_container_index))
                    ++active_container_index;
                return *this;
            }

            bool operator!=(const join_iterator_t& Rhs) const {
                return active_container_index != Rhs.active_container_index ||
                       (... || !is_same(static_cast<const iterator_pair_t<TIterator, Is> &>(*this),
                                        static_cast<const iterator_pair_t<TIterator, Is> &>(Rhs))
                       );
            }

            decltype(auto) operator*() const {
                if (auto* value = extract[active_container_index]())
                    return *value;

                throw std::runtime_error{ "" };
            }

            uint32_t active_container_index{ 0 };
        private:
            template<typename TThis>
            static bool is_same(const TThis& lhs, const TThis& rhs) {
                return lhs.begin == rhs.begin && lhs.end == rhs.end;
            }

            std::array<std::function<bool(bool)>, sizeof...(TIterator)> increment;
            std::array<std::function<TValue*()>, sizeof...(TIterator)> extract;
        };

        template<typename... Ts>
        join_iterator_t(std::pair<Ts, Ts>...) ->
        join_iterator_t<std::make_index_sequence<sizeof...(Ts)>, std::decay_t<Ts>...>;

        explicit join_container_impl(const TContainer&... Containers)
                : indexed_ref_t<TContainer, Is>(Containers)...{}

        auto begin() const {
            return join_iterator_t(std::pair{
                static_cast<const indexed_ref_t<TContainer, Is>&>(*this).value.begin(),
                static_cast<const indexed_ref_t<TContainer, Is>&>(*this).value.end()
            }...);
        }

        auto end() const {
            auto end_it = join_iterator_t(std::pair{
                static_cast<const indexed_ref_t<TContainer, Is>&>(*this).value.end(),
                static_cast<const indexed_ref_t<TContainer, Is>&>(*this).value.end()
            }...);
            end_it.active_container_index = sizeof...(TContainer);
            return end_it;
        }
    };

    template<typename TIterator>
    decltype(auto) extract_value(TIterator&& iterator, detail::join_iterator_tag) {
        return *iterator;
    }

}
template<typename T>
struct type_holder_t{};

template<typename TValue, typename... Ts>
class join_container_t final :
        public detail::join_container_impl<TValue, std::make_index_sequence<sizeof...(Ts)>, Ts...> {
public:
    explicit join_container_t(type_holder_t<TValue>, const Ts&...args)
        : detail::join_container_impl<TValue, std::make_index_sequence<sizeof...(Ts)>, Ts...> (args...){}
};

template<typename TValue, typename... Ts>
join_container_t(type_holder_t<TValue>, Ts...)->join_container_t<TValue, Ts...>;