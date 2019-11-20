#pragma once

#include <type_traits>


namespace typelist {

template <typename...> class TypeList {};

template <typename H, typename... T>
struct TypeList<H, T...> {
    using Head = H;
    using Tail = TypeList<T...>;
};

using EmptyList = TypeList<>;


namespace impl {

template <typename, typename>
struct AppendImpl;

template <typename... Types, typename U>
struct AppendImpl<TypeList<Types...>, U> {
    using type = TypeList<Types..., U>;
};


template <typename>
struct ReverseImpl {
    using type = EmptyList;
};

template <typename H, typename... T>
struct ReverseImpl<TypeList<H, T...>> {
    using type = typename AppendImpl<typename ReverseImpl<TypeList<T...>>::type, H>::type;
};


template <typename TL, typename Old, typename New>
struct ReplaceImpl {
    using type = EmptyList;
};

template <typename H, typename... T, typename Old, typename New>
struct ReplaceImpl<TypeList<H, T...>, Old, New> {
    using type = std::conditional_t<
        std::is_same_v<H, Old>, 
        TypeList<New, T...>, 
        typename AppendImpl<typename ReplaceImpl<TypeList<T...>, Old, New>::type, H>::type>;
};


template <size_t, typename T>
struct GetImpl {
    using type = T;
};

template <size_t N, typename H, typename... T>
struct GetImpl<N, TypeList<H, T...>> {
    using type = std::conditional_t<N, typename GetImpl<N - 1, TypeList<T...>>::type, H>;
};


template <typename, typename>
struct ContainsImpl;

template <typename T, typename... Ts>
struct ContainsImpl<T, TypeList<Ts...>> {
    static constexpr inline bool value = (false || ... || std::is_same_v<T, Ts>);
};


template <typename TL, typename U>
struct RemoveImpl {
    using type = EmptyList;
};

template <typename H, typename... T, typename U>
struct RemoveImpl<TypeList<H, T...>, U> {
    using type = std::conditional_t<
        std::is_same_v<H, U>, TypeList<T...>, 
        typename AppendImpl<typename RemoveImpl<TypeList<T...>, U>::type, H>::type>;
};

} // namespace impl

template <typename TL, typename T>
using Append = typename impl::AppendImpl<TL, T>::type;

template <typename TL>
using Reverse = typename impl::ReverseImpl<TL>::type;

template <typename TL, typename Old, typename New>
using Replace = typename impl::ReplaceImpl<TL, Old, New>::type;

template <size_t Index, typename TL>
using Get = typename impl::GetImpl<Index, TL>::type;

template <typename T, typename TL>
static constexpr inline bool Contains = impl::ContainsImpl<T, TL>::value;

template <typename TL, typename T>
using Remove = typename impl::RemoveImpl<TL, T>::type;

} // namespace typelist
