/*
 * Copyright (C) 2023 Gleb Bezborodov - All Rights Reserved
 */

#pragma once

#include <type_traits>
#include <tuple>
#include <stdexcept>

namespace detail {

    struct join_iterator_tag final {};

    template<typename T, size_t I>
    struct indexed_ref_t {
        explicit indexed_ref_t(const T& in_value)
                : value(in_value) {}
        const T& value;
    };

    template<typename, typename... TContainer>
    class join_container_impl;

    template<std::size_t... Is, typename... TContainer>
    class join_container_impl<std::index_sequence<Is...>, TContainer...> : public indexed_ref_t<TContainer, Is>... {
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
                    : iterator_pair_t<TIterator, Is>(in_pair)...{}

            join_iterator_t& operator++() {
                (try_increment(static_cast<iterator_pair_t<TIterator, Is> &>(*this)), ...);
                return *this;
            }

            bool operator!=(const join_iterator_t& Rhs) const {
                return active_container_index != Rhs.active_container_index ||
                       (... || !is_same(static_cast<const iterator_pair_t<TIterator, Is> &>(*this),
                                        static_cast<const iterator_pair_t<TIterator, Is> &>(Rhs))
                       );
            }

            decltype(auto) operator*() const {
                const void* extracted[] = { try_extract(static_cast<const iterator_pair_t<TIterator, Is> &>(*this))... };
                for (auto* R : extracted) {
                    if (R) return *(int*)R;
                }

                throw std::runtime_error{ "" };
            }

            uint32_t active_container_index{ 0 };
        private:
            template<typename TThisIterator, std::size_t ThisIndex>
            void try_increment(iterator_pair_t<TThisIterator, ThisIndex>& iterator) {
                if (active_container_index == ThisIndex) {
                    ++iterator.begin;
                    if (iterator.begin == iterator.end)
                        ++active_container_index;
                }
            }

            template<typename TThisIterator, std::size_t ThisIndex>
            const void* try_extract(const iterator_pair_t<TThisIterator, ThisIndex>& iterator) const {
                if (ThisIndex == active_container_index)
                    return &extract_value(iterator.begin, join_iterator_tag{});

                return nullptr;
            }

            template<typename TThis>
            bool is_same(const TThis& lhs, const TThis& rhs) const {
                return lhs.begin == rhs.begin && lhs.end == rhs.end;
            }
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

template<typename... Ts>
class join_container_t final : public detail::join_container_impl<std::make_index_sequence<sizeof...(Ts)>, Ts...> {
public:
    explicit join_container_t(const Ts&...args)
        : detail::join_container_impl<std::make_index_sequence<sizeof...(Ts)>, Ts...> (args...){}
};

template<typename... Ts>
join_container_t(Ts...)->join_container_t<Ts...>;