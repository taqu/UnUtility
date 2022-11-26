#pragma once

template<class T0>
class TArrayView
{
public:
    TArrayView();
    TArrayView(T0& X0);

private:
    T0* Array_;
};

template<class T0>
TArrayView<T0>::TArrayView()
    :Array_(nullptr)
{
}

template<class T0>
TArrayView<T0>::TArrayView(T0& X0)
    :Array_(&X0)
{
}

