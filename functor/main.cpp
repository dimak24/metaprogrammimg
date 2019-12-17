#include <cstdio>
#include <utility>
#include <string>
#include <cassert>

#include "../factory/typelist.hpp"
#include "type_traits.hpp"



class Visitor;


class Acceptor {
public:
    virtual void accept(Visitor*, int, const std::string&) = 0;
    [[noreturn]] void accept(...) { assert(false && "must not be called"); }
};


class FirstAcceptor;
class SecondAcceptor;
class ThirdAcceptor;


class Visitor {
public:
    virtual void visit(FirstAcceptor*, int, const std::string&, int) = 0;
    virtual void visit(SecondAcceptor*, int, const std::string&, int) = 0;
    virtual void visit(ThirdAcceptor*, int, const std::string&, int) = 0;
};


class FirstAcceptor : public Acceptor {
public:
    void accept(Visitor* visitor, int sum, const std::string& name) override
    {
        visitor->visit(this, sum, name, 10);
    }
};


class SecondAcceptor : public Acceptor {
public:
    void accept(Visitor* visitor, int sum, const std::string& name) override
    {
        visitor->visit(this, sum, name, 20);
    }
};


class ThirdAcceptor : public Acceptor {
public:
    void accept(Visitor* visitor, int sum, const std::string& name) override
    {
        visitor->visit(this, sum, name, 30);
    }
};


template <typename T>
struct is_acceptor : is_base_of<Acceptor, T> {};

template <typename T>
struct is_visitor : is_base_of<Visitor, T> {};


template <typename, typename>
class Functor;


template <typename A, typename V>
class Functor {
    static_assert(is_base_of<Acceptor, A>::value && is_base_of<Visitor, V>::value);
private:
    static void call_(const char* shop, int num, const std::string& name)
    {
        printf("In %s %s can by %d bananas!!!\n", shop, name.c_str(), num);
    }

    true_type check_(int, const std::string&) const;
    false_type check_(...) const;

public:
    using Acceptor = A;
    using Visitor = V;

    template <typename... Args>
    bool check(Args&&... args) const
    { 
        return decltype(check_(std::forward<Args>(args)...))::value;
    }

    template <typename... Args>
    void operator()(Args&&... args)
    {
        call_(std::forward<Args>(args)...);
    }
};


template <template <typename, typename> class Functor>
class ConcreteVisitor : public Visitor {
public:
    void visit(FirstAcceptor*, int sum, const std::string& name, int price) override
    {
        Functor<FirstAcceptor, ConcreteVisitor<Functor>> f;
        f("Pyaterochka", sum / price, name);
    }

    void visit(SecondAcceptor*, int sum, const std::string& name, int price) override
    {
        Functor<SecondAcceptor, ConcreteVisitor<Functor>> f;
        f("Miratorg", sum / price, name);
    }

    void visit(ThirdAcceptor*, int sum, const std::string& name, int price) override
    {
        Functor<ThirdAcceptor, ConcreteVisitor<Functor>> f;
        f("Ashan", sum / price, name);
    }
};


template <typename TL>
class VisitFactory {
private:
    using splitted1_ = Split<TL, is_acceptor>;
    using all_acceptors_ = typename splitted1_::accept;

    using splitted2_ = Split<typename splitted1_::reject, is_visitor>;
    
    using all_visitors_ = typename splitted2_::accept;
    using functors_ = typename splitted2_::reject;

    using AbstractAcceptor_ = typename Split<all_acceptors_, is_abstract>::accept::Head;
    using acceptors_ = typename Split<all_acceptors_, is_abstract>::reject;

    using AbstractVisitor_ = typename Split<all_visitors_, is_abstract>::accept::Head;
    using visitors_ = typename Split<all_visitors_, is_abstract>::reject;


    template <typename Functors>
    struct FindFunctor_ {
        template <typename... Args>
        static void go(AbstractAcceptor_*, AbstractVisitor_*, Args&&...)
        {
            puts("No appropriate functor found");
        }
    };

    template <typename H, typename... T>
    struct FindFunctor_<typelist::TypeList<H, T...>> {
        template <typename... Args>
        static void go(AbstractAcceptor_* acceptor, AbstractVisitor_* visitor, Args&&... args)
        {
            if (dynamic_cast<typename H::Acceptor*>(acceptor) &&
                dynamic_cast<typename H::Visitor*>(visitor)) {
                H functor;
                if (!functor.check(std::forward<Args>(args)...)) {
                    puts("Wrong arguments");
                } else acceptor->accept(visitor, std::forward<Args>(args)...);
            } else {
                FindFunctor_<typelist::TypeList<T...>>::go(
                    acceptor, visitor, std::forward<Args>(args)...);
            }
        }
    };

public:
    template <typename... Args>
    void go(AbstractAcceptor_* acceptor, AbstractVisitor_* visitor, Args&&... args) const
    {
        FindFunctor_<functors_>::go(acceptor, visitor, std::forward<Args>(args)...);
    }
};


int main()
{
    VisitFactory<typelist::TypeList<
        Acceptor,
        ThirdAcceptor,
        ConcreteVisitor<Functor>,
        SecondAcceptor,
        Visitor,
        Functor<FirstAcceptor, ConcreteVisitor<Functor>>,
        Functor<ThirdAcceptor, ConcreteVisitor<Functor>>
    >> f;

    FirstAcceptor acc_1;
    SecondAcceptor acc_2;
    ThirdAcceptor acc_3;

    ConcreteVisitor<Functor> v;

    f.go(&acc_1, &v, 100, "Borya"); // In Pyaterochka Borya can by 10 bananas!!!
    f.go(&acc_2, &v, 100, "Borya"); // No appropriate functor found
    f.go(&acc_3, &v, 100, "Borya"); // In Ashan Borya can by 3 bananas!!!
    f.go(&acc_3, &v, 100, 200);     // Wrong arguments
}
