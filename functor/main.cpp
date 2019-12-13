#include <cstdio>
#include <type_traits>
#include <utility>
#include <array>

#include "../factory/typelist.hpp"


class Visitor;


class Acceptor {
public:
    virtual void accept(Visitor*) = 0;
};


class FirstAcceptor;
class SecondAcceptor;
class ThirdAcceptor;


class Visitor {
public:
    virtual void visit(FirstAcceptor*) = 0;
    virtual void visit(SecondAcceptor*) = 0;
    virtual void visit(ThirdAcceptor*) = 0;
};


class FirstAcceptor : public Acceptor {
public:
    void accept(Visitor* visitor) override
    {
        visitor->visit(this);
    }
};


class SecondAcceptor : public Acceptor {
public:
    void accept(Visitor* visitor) override
    {
        visitor->visit(this);
    }
};


class ThirdAcceptor : public Acceptor {
public:
    void accept(Visitor* visitor) override
    {
        visitor->visit(this);
    }
};


class ConcreteVisitor : public Visitor {
public:
    void visit(FirstAcceptor*) override
    {
        puts("I'm visiting FirstAcceptor");
    }

    void visit(SecondAcceptor*) override
    {
        puts("I'm visiting SecondAcceptor");
    }

    void visit(ThirdAcceptor*) override
    {
        puts("I'm visiting ThirdAcceptor");
    }
};


template <typename Acceptor, typename Visitor>
class Functor {
private:
    Acceptor* acceptor_;
    Visitor* visitor_;

public:
    explicit Functor(Acceptor* acceptor, Visitor* visitor)
        : acceptor_(acceptor), visitor_(visitor)
    {}

    template <typename... Args>
    void operator()(Args&&... args)
    {
        ((void)args, ...);
        acceptor_->accept(visitor_);
    }
};

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


template <typename Base>
std::true_type is_base_of_test_(Base&&);

template <typename>
std::false_type is_base_of_test_(...);

template <typename Base, typename T>
struct is_base_of : decltype(is_base_of_test_<Base>(std::declval<T>())) {};

template <typename T, typename Base>
using is_derived_of = is_base_of<Base, T>;

template <typename Base, typename T>
inline constexpr bool is_base_of_v = is_base_of<Base, T>::value;


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

template <typename T>
std::true_type is_acceptor_(decltype(&T::accept));

template <typename>
std::false_type is_acceptor_(...);

template <typename T>
struct is_acceptor : decltype(is_acceptor_<T>(nullptr)) {};

template <typename T, typename U,
          typename = decltype(std::declval<T>().visit((U*)nullptr))>
std::true_type can_visit_(int);

template <typename, typename>
std::false_type can_visit_(...);

template <typename T, typename TL>
struct is_visitor;

template <typename T, typename... Ts>
struct is_visitor<T, typelist::TypeList<Ts...>> :
    std::integral_constant<bool, (decltype(can_visit_<T, Ts>(0))::value || ...)> {};

template <typename T, typename = decltype(new T[1])>
std::false_type is_abstract_(int);

template <typename>
std::true_type is_abstract_(...);

template <typename T>
struct is_abstract : decltype(is_abstract_<T>(0)) {};


template <typename FunctorsTL, typename Acceptor, typename Visitor, typename... Args>
struct find_functor;



// template <typename F, typename... Fs, typename Acceptor, typename Visitor, typename... Args>
// auto find_functor_(...) -> decltype(find_functor_<Fs..., Acceptor, Visitor, Args...>(0, std::declval<Fs>()...));

template <typename Acceptor, typename Visitor, typename... Args>
struct find_functor<typelist::EmptyList, Acceptor, Visitor, Args...> {};


template <typename F, typename... Fs, typename Acceptor, typename Visitor, typename... Args>
struct find_functor<typelist::TypeList<F, Fs...>, Acceptor, Visitor, Args...> {
    template <typename = std::enable_if_t<std::is_constructible_v<F, Acceptor*, Visitor*>>>
    struct test_(int);

    static
    find_functor<typelist::TypeList<Fs...>, Acceptor, Visitor, Args...>
    test_(...);

    using type = typename decltype(test_(0))::type;
};



template <typename TL>
class VisitFactory {
private:
    using splitted1_ = Split<TL, is_acceptor>;
    using acceptors_ = typename splitted1_::accept;

    template <typename T>
    using is_visitor_local_ = is_visitor<T, acceptors_>;

    using splitted2_ = Split<typename splitted1_::reject, is_visitor_local_>;
    
    using visitors_ = typename splitted2_::accept;
    using functors_ = typename splitted2_::reject;

    using AbstractAcceptor = typename Split<acceptors_, is_abstract>::accept::Head;
    using AbstractVisitor = typename Split<visitors_, is_abstract>::accept::Head;

public:
    template <typename... Args, typename Acceptor, typename Visitor,
              typename = std::enable_if_t<typelist::Contains<acceptors_, Acceptor>>,
              typename = std::enable_if_t<typelist::Contains<visitors_, Visitor>>,
              typename Functor = typename find_functor<functors_, Acceptor, Visitor, Args...>::type>
    decltype(auto) go(Acceptor* acceptor, Visitor* visitor, Args&&... args) const
    {
        return Functor(acceptor, visitor)(std::forward<Args>(args)...);
    }
};

template <typename>
struct ShowType {
    ShowType() = delete;
};

struct A {
    explicit A() = delete;
};

struct B : A {
    B() = delete;
};

// template <typename F, typename Acceptor, typename Visitor, typename = decltype(F((Acceptor*)nullptr, (Visitor*)nullptr))>
// F test_(int);

int main()
{
    ShowType<typename find_functor<typelist::TypeList<Functor<FirstAcceptor, ConcreteVisitor>, Functor<SecondAcceptor, ConcreteVisitor>>, SecondAcceptor, ConcreteVisitor>::type> _;

    VisitFactory<typelist::TypeList<
        Acceptor,
        ThirdAcceptor,
        ConcreteVisitor,
        SecondAcceptor,
        Visitor,
        Functor<FirstAcceptor, ConcreteVisitor>,
        Functor<ThirdAcceptor, ConcreteVisitor>
    >> f;

    // decltype(test_<Functor<FirstAcceptor, ConcreteVisitor>, SecondAcceptor, ConcreteVisitor>(0)) d;

    // Functor<FirstAcceptor, ConcreteVisitor> g((SecondAcceptor*)nullptr, (ConcreteVisitor*)nullptr);

    printf("%d", is_abstract<int>::value);
}
