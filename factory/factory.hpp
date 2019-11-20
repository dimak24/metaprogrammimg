#pragma once

#include <type_traits>
#include <memory>

#include "typelist.hpp"


namespace factory { 

using namespace typelist;

namespace detail {

template <typename T>
struct TypeHolder {
    using type = T;
};

namespace impl {

template <typename TL, typename T>
struct ContainsDerivedImpl;

template <typename T, typename... Ts>
struct ContainsDerivedImpl<TypeList<Ts...>, T> {
    static inline constexpr bool value = (false || ... || std::is_base_of_v<T, Ts>);
}; 

template <typename Lhs, typename Rhs>
struct IsBaseOfImpl;

template <typename... LhsTs, typename Rhs>
struct IsBaseOfImpl<TypeList<LhsTs...>, Rhs> {
    static inline constexpr bool value = (false || ... || ContainsDerivedImpl<Rhs, LhsTs>::value);
};


template <typename TL, typename U>
struct MostDerivedImpl;

template <typename T, typename U>
struct MostDerivedImpl<TypeList<T>, U> {
    using type = T;
};

template <typename H, typename... T, typename U>
struct MostDerivedImpl<TypeList<H, T...>, U> {
    using Candidate_ = typename MostDerivedImpl<TypeList<T...>, U>::type;
    using type = std::conditional_t<
        IsBaseOfImpl<Candidate_, U>::value && IsBaseOfImpl<H, U>::value,
            std::conditional_t<IsBaseOfImpl<Candidate_, H>::value, H, Candidate_>, 
        std::conditional_t<IsBaseOfImpl<Candidate_, U>::value, Candidate_, H>>;
};


template <typename T, typename... TLs>
struct GetTLImpl;

template <typename T, typename TL>
struct GetTLImpl<T, TL> {
    using type = TL;
};

template <typename T, typename TL, typename... TLs>
struct GetTLImpl<T, TL, TLs...> {
    using type = std::conditional_t<Contains<T, TL>, TL, typename GetTLImpl<T, TLs...>::type>;
};


template <typename Hierarchy>
struct LeastDerivedImpl;

template <typename T>
struct LeastDerivedImpl<TypeList<T>> {
    using type = T;
};

template <typename H, typename... T>
struct LeastDerivedImpl<TypeList<H, T...>> {
    using Candidate_ = typename LeastDerivedImpl<TypeList<T...>>::type;
    using type = std::conditional_t<IsBaseOfImpl<Candidate_, H>::value, Candidate_, H>;
};

} // namespace impl


// aliases TypeList from TL which contains T if exists
template <typename T, typename... TLs>
using GetTL = std::enable_if_t<(false || ... || Contains<T, TLs>), typename impl::GetTLImpl<T, TLs...>::type>;

// the most base classes in Hierarchy
template <typename Hierarchy>
using LeastDerived = typename impl::LeastDerivedImpl<Hierarchy>::type;

// the most derived class in TL being base for U
template <typename TL, typename U>
using MostDerived = typename impl::MostDerivedImpl<TL, U>::type;


template <typename T>
class AbstractFactoryUnit {
public:
    virtual std::unique_ptr<T> do_create(TypeHolder<T>) = 0;
    virtual ~AbstractFactoryUnit()
    {}
};


template <typename BaseTL, typename ConcreteProduct>
struct Skip {
    using type = std::conditional_t<
        std::is_base_of_v<typename BaseTL::Head, ConcreteProduct>, 
        BaseTL, typename Skip<typename BaseTL::Tail, ConcreteProduct>::type>;
};

template <typename ConcreteProduct>
struct Skip<EmptyList, ConcreteProduct> {
    using type = EmptyList;
};

template <typename ConcreteProduct, typename Base>
class FactoryUnit : public Base {
public:
    using BaseProductList = typename Skip<typename Base::ProductList, ConcreteProduct>::type;
    using ProductList = typename BaseProductList::Tail;
    using AbstractProduct = typename BaseProductList::Head;

    std::unique_ptr<AbstractProduct> do_create(TypeHolder<AbstractProduct>) override
    {
        return std::make_unique<ConcreteProduct>();
    }
};

template <typename TL, template <typename AtomicType, typename Base> class Unit, typename Root>
class GenLinearHierarchy;

template <typename H, typename... T, template <typename, typename> class Unit, typename Root>
class GenLinearHierarchy<TypeList<H, T...>, Unit, Root>
    : public Unit<H, GenLinearHierarchy<TypeList<T...>, Unit, Root>> {};

template <typename H, template <typename, typename> class Unit, typename Root>
class GenLinearHierarchy<TypeList<H>, Unit, Root> : public Unit<H, Root> {};


template <typename TL, template <typename> class Unit>
class GenScatterHierarchy;

template <typename... T, template <typename> class Unit>
class GenScatterHierarchy<TypeList<T...>, Unit> : public Unit<T>... {};


template <typename TL, template <typename> class Unit = AbstractFactoryUnit>
class AbstractFactory : public GenScatterHierarchy<TL, Unit> {
public:
    using ProductList = TL;

    template <typename T>
    std::unique_ptr<T> create()
    {
        return dynamic_cast<Unit<T>*>(this)->do_create(TypeHolder<T>{}); 
    }
};


template <typename AbstractFact,
          template <typename, typename> class Creator = FactoryUnit,
          typename TypeList = typename AbstractFact::ProductList>
class ConcreteFactory : public GenLinearHierarchy<Reverse<TypeList>, Creator, AbstractFact> {};


template <typename BaseProducts, typename ConcreteProducts, typename AllProducts>
class GenFactoryHierarchy;

template <typename BaseProducts, typename AllProducts>
class GenFactoryHierarchy<BaseProducts, BaseProducts, AllProducts>
    : public AbstractFactory<BaseProducts> {};

template <typename BaseProducts, typename ConcreteProducts, typename AllProducts>
class GenFactoryHierarchy
    : public ConcreteFactory<
        GenFactoryHierarchy<BaseProducts, MostDerived<Remove<AllProducts, ConcreteProducts>, ConcreteProducts>, 
        Remove<AllProducts, ConcreteProducts>>,
        FactoryUnit, ConcreteProducts> {
public:
    using ProductList = BaseProducts;
};

} // namespace detail

template <typename... TLs>
class GetAbstractFactory {
public:
    template <typename T, typename TL = detail::GetTL<T, TLs...>>
    using GetConcreteFactory =
        detail::GenFactoryHierarchy<detail::LeastDerived<TypeList<TLs...>>, TL, TypeList<TLs...>>;
};

} // namespace factory
