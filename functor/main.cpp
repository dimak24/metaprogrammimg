#include <cstdio>
#include <type_traits>
#include <utility>

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
    Acceptor acceptor_;
    Visitor visitor_;

public:
    explicit Functor(Acceptor acceptor, Visitor visitor)
        : acceptor_(acceptor), visitor_(visitor)
    {}

    template <typename... Args>
    void operator()(Args&&... args)
    {
        ((void)args, ...);
        acceptor_.accept(&visitor_);
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

template <typename Base, typename T>
inline constexpr bool is_base_of_v = is_base_of<Base, T>::value;


template <typename TL, typename Base>
struct Split {
    using accept = typelist::EmptyList;
    using reject = typelist::EmptyList;
};

template <typename H, typename... T, typename Base>
class Split<typelist::TypeList<H, T...>, Base> {
private:
    using _accept_prev = typename Split<typelist::TypeList<T...>, Base>::accept;
    using _reject_prev = typename Split<typelist::TypeList<T...>, Base>::reject;

public:
    using accept = conditional_t<
        is_base_of_v<Base, H>,
        typelist::Append<_accept_prev, H>,
        _accept_prev
    >;

    using reject = conditional_t<
        is_base_of_v<Base, H>,
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

template <typename T, typename... Acceptors, typename = std::enable_if_t<  &T::visit  >>
std::true_type can_visit_()

// template <typename>
// std::false_type is_visitor_(...);

template <typename T>
struct is_visitor : decltype(is_visitor_<T>(nullptr)) {};


template <typename TL>
class VisitFactory;

template<typename... T>
class VisitFactory<typelist::TypeList<T...>> {
private:

public:
    template <typename... Args>
    decltype(auto) go(Acceptor* acceptor, Visitor* visitor, Args&&... args) const
    {
        // if (d_c && d_c)
        Functor<Acceptor, Visitor> f(*acceptor, *visitor);
        f(std::forward<Args>(args)...);
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

int main()
{
    bool b = is_visitor<ConcreteVisitor>::value;
    printf("%d\n", b);
    // printf("%zu %zu\n", sizeof(_One), sizeof(_Two));
    // ShowType<typename Split<typelist::TypeList<Acceptor, ThirdAcceptor, Visitor, ConcreteVisitor, FirstAcceptor>, Acceptor>::reject> _;
}
