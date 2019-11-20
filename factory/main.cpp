#include <cassert>

#include "factory.hpp"
#include "typelist.hpp"


#define ADD_PROPS_1(base, prop) struct prop##base : base { prop##base() { /* puts(#prop#base); */ }}
#define ADD_PROPS_2(base, prop1, ...) ADD_PROPS_1(base, prop1); ADD_PROPS_1(base, __VA_ARGS__)
#define ADD_PROPS_3(base, prop1, ...) ADD_PROPS_1(base, prop1); ADD_PROPS_2(base, __VA_ARGS__)
#define ADD_PROPS_4(base, prop1, ...) ADD_PROPS_1(base, prop1); ADD_PROPS_3(base, __VA_ARGS__)

#define ADD_PROPS(base, n, ...) ADD_PROPS_##n(base, __VA_ARGS__)

struct _ { virtual ~_() = default; };

#define products  Chair, Table, Sofa
#define materials Wooden, Steel
#define producers Japanese, Spanish, Australian

ADD_PROPS(_, 3, products);

ADD_PROPS(Chair_, 2, materials);
ADD_PROPS(Table_, 2, materials);
ADD_PROPS(Sofa_,  2, materials);

ADD_PROPS(WoodenChair_, 3, producers);
ADD_PROPS(SteelChair_,  3, producers);
ADD_PROPS(WoodenTable_, 3, producers);
ADD_PROPS(SteelTable_,  3, producers);
ADD_PROPS(WoodenSofa_,  3, producers);
ADD_PROPS(SteelSofa_,   3, producers);


using typelist::TypeList;
using factory::GetAbstractFactory;

template <typename T>
static inline
bool instanceof(auto* ptr)
{
    return dynamic_cast<T*>(ptr);
}

template <typename>
struct ShowType {
    ShowType() = delete;
};

int main()
{
    using factory_t = GetAbstractFactory<
        TypeList<Chair_, Table_, Sofa_>,

        TypeList< SteelChair_,  SteelTable_,  SteelSofa_>,
        TypeList< WoodenChair_, WoodenTable_, WoodenSofa_>,

        TypeList<   SpanishSteelChair_,        SpanishSteelTable_,     SpanishSteelSofa_>,
        TypeList<   AustralianSteelChair_,     AustralianSteelTable_,  AustralianSteelSofa_>,
        TypeList</* JapaneseWoodenChair_, */   JapaneseWoodenTable_,   JapaneseWoodenSofa_>,
        TypeList</* SpanishWoodenChair_, */    SpanishWoodenTable_,    SpanishWoodenSofa_>,
        TypeList<   JapaneseSteelChair_,    /* JapaneseSteelTable_, */ JapaneseSteelSofa_>,
        TypeList</* AustralianWoodenChair_, */ AustralianWoodenTable_, AustralianWoodenSofa_>
    >;


    assert(instanceof<JapaneseSteelSofa_>(
        factory_t::GetConcreteFactory<JapaneseSteelChair_>{}.create<Sofa_>().get()));
    assert(instanceof<SpanishWoodenTable_>(
        factory_t::GetConcreteFactory<SpanishWoodenSofa_>{}.create<Table_>().get()));

    {
        // JapaneseSteeltable_ is missing => should return SteelTable_
        auto product = factory_t::GetConcreteFactory<JapaneseSteelSofa_>{}.create<Table_>();

        assert(instanceof<SteelTable_>(product.get()) &&
              !instanceof<JapaneseSteelTable_>(product.get()));
    }
}
