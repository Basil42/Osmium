//
// Created by nicolas.gerard on 2024-12-04.
//

#ifndef PROPERTIES_H
#define PROPERTIES_H
// C# style property (with a few caveats9 found at : https://stackoverflow.com/a/5924594/10971292
template<typename C, typename T, T (C::*getter)() const, void (C::*setter)(const T&)>
struct Property
{
    C *instance;

    Property(C *instance)
        : instance(instance)
    {
    }

    operator T () const
    {
        return (instance->*getter)();
    }

    Property& operator=(const T& value)
    {
        (instance->*setter)(value);
        return *this;
    }

    template<typename C2, typename T2,
             T2 (C2::*getter2)(), void (C2::*setter2)(const T2&)>
    Property& operator=(const Property<C2, T2, getter2, setter2>& other)
    {
        return *this = (other.instance->*getter2)();
    }

    Property& operator=(const Property& other)
    {
        return *this = (other.instance->*getter)();
    }
};
#endif //PROPERTIES_H
