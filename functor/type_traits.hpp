struct false_type {
    static inline constexpr bool value = false;
};

struct true_type {
    static inline constexpr bool value = true;
};

template <typename T>
using add_rvalue_reference_t = T&&;

template <typename T>
add_rvalue_reference_t<T> declval();


namespace detail {

template <typename Base>
true_type is_base_of_test_(Base&&);

template <typename>
false_type is_base_of_test_(...);


template <typename T, typename = decltype(new T[1])>
false_type is_abstract_(int);

template <typename>
true_type is_abstract_(...);

} // namespace detail


template <typename Base, typename T>
struct is_base_of : decltype(detail::is_base_of_test_<Base>(declval<T>())) {};


template <bool, typename = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> {
    using type = T;
};

template <bool B, typename T = void>
using enable_if_t = typename enable_if<B, T>::type;


template <typename T>
struct is_abstract : decltype(detail::is_abstract_<T>(0)) {};


template <bool Condition, typename OnTrue, typename OnFalse>
struct conditional;

template <typename OnTrue, typename OnFalse>
struct conditional<true, OnTrue, OnFalse> {
    using type = OnTrue;
};

template <typename OnTrue, typename OnFalse>
struct conditional<false, OnTrue, OnFalse> {
    using type = OnFalse;
};

template <bool Condition, typename OnTrue, typename OnFalse>
using conditional_t = typename conditional<Condition, OnTrue, OnFalse>::type;


template <typename TL, template <typename> class Condition>
struct Split {
    using accept = typelist::EmptyList;
    using reject = typelist::EmptyList;
};

template <typename H, typename... T, template <typename> class Condition>
class Split<typelist::TypeList<H, T...>, Condition> {
private:
    using _accept_prev = typename Split<typelist::TypeList<T...>, Condition>::accept;
    using _reject_prev = typename Split<typelist::TypeList<T...>, Condition>::reject;

public:
    using accept = conditional_t<
        Condition<H>::value,
        typelist::Append<_accept_prev, H>,
        _accept_prev
    >;

    using reject = conditional_t<
        Condition<H>::value,
        _reject_prev,
        typelist::Append<_reject_prev, H>
    >;
};
