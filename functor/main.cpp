#include <cstdio>
#include <type_traits>
#include <utility>

#include "../factory/typelist.hpp"


class Visitor;


class Acceptor {
public:
    virtual void accept(Visitor&) = 0;
};


class FirstAcceptor;
class SecondAcceptor;
class ThirdAcceptor;


class Visitor {
public:
    virtual void visit(FirstAcceptor&) = 0;
    virtual void visit(SecondAcceptor&) = 0;
    virtual void visit(ThirdAcceptor&) = 0;
};


class FirstAcceptor : public Acceptor {
public:
    void accept(Visitor& visitor) override
    {
        visitor.visit(*this);
    }
};


class SecondAcceptor : public Acceptor {
public:
    void accept(Visitor& visitor) override
    {
        visitor.visit(*this);
    }
};


class ThirdAcceptor : public Acceptor {
public:
    void accept(Visitor& visitor) override
    {
        visitor.visit(*this);
    }
};


class ConcreteVisitor : public Visitor {
public:
    void visit(FirstAcceptor&) override
    {
        puts("I'm visiting FirstAcceptor");
    }

    void visit(SecondAcceptor&) override
    {
        puts("I'm visiting SecondAcceptor");
    }

    void visit(ThirdAcceptor&) override
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
    Functor(Acceptor acceptor, Visitor visitor)
        : acceptor_(acceptor), visitor_(visitor)
    {}

    template <typename... Args>
    void operator()(Args&&... args)
    {
        ((void)args, ...);
        acceptor_.accept(visitor_);
    }
};


template <typename TL>
class VisitFactory;

template <typename Acceptor, typename Visitor, typename Functor>
class VisitFactory<typelist::TypeList<Acceptor, Visitor, Functor>> {
public:
    template <typename... Args>
    decltype(auto) go(Acceptor* acceptor, Visitor* visitor, Args&&... args) const
    {
        Functor f(*acceptor, *visitor);
        f(std::forward<Args>(args)...);
    }
};


int main()
{
    VisitFactory<typelist::TypeList<SecondAcceptor, ConcreteVisitor, Functor<SecondAcceptor, ConcreteVisitor>>> f;
    ConcreteVisitor v;
    SecondAcceptor a;
    f.go(&a, &v);
}
