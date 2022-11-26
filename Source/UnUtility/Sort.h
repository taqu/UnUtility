#pragma once
#include <CoreMinimal.h>

//--- Insertionsort
//------------------------------------------------
template<class T, class U>
void Insertionsort(uint32 N, T& V, U Func)
{
    for(uint32 i = 1; i < N; ++i) {
        T X = MoveTemp(V[i]);
        uint32 j;
        for(j = i-1; 0 != j && Func(X, V[j]); --j) {
            V[j + 1] = MoveTemp(V[j]);
        }
        V[j + 1] = MoveTemp(X);
    }
}

//--- Heapsort
//------------------------------------------------
template<class T, class U>
void Heapsort(uint32 N, T& V, U Func)
{
    --V;
    uint32 i, j;
    T X;
    for(uint32 k = N >> 1; 1<=k; --k) {
        i = k;
        X = MoveTemp(V[k]);
        while((j = i << 1) <= N) {
            if(j < N && Func(V[j], V[j + 1])) {
                ++j;
            }

            if(!Func(X, V[j])) {
                break;
            }
            V[i] = MoveTemp(V[j]);
            i = j;
        }
        V[i] = MoveTemp(X);
    }

    while(N > 1) {
        X = MoveTemp(V[N]);
        V[N] = MoveTemp(V[1]);
        --N;
        i = 1;
        while((j = i << 1) <= N) {
            if(j < N && Func(V[j], V[j + 1])) {
                ++j;
            }

            if(!Func(X, V[j])) {
                break;
            }
            V[i] = MoveTemp(V[j]);
            i = j;
        }
        V[i] = MoveTemp(X);
    }
}

//--- Introsort
//------------------------------------------------
template<class T, class U>
void Introsort(uint32 N, T* V, U Func)
{
    static constexpr uint32 SortSwitchN = 47;
    static constexpr uint32 SortMaxDepth = 16;

    uint32 Depth = 0;
    uint32 T = N;
    while(1 < T) {
        ++Depth;
        T >>= 1;
    }
    Depth = SortMaxDepth < Depth ? SortMaxDepth : Depth;
    struct Range
    {
        uint32 Offset_;
        uint32 Num_;
    };

    Range Ranges[SortMaxDepth * 2];
    uint8 Levels[SortMaxDepth * 2];
    uint32 Count = 1;
    Ranges[0] = {0, N};
    Levels[0] = 0;

    while(0 < Count) {
        --Count;
        const Range& Range = Ranges[Count];
        T* Current = V + Range.Offset_;

        if(Range.Num_ < SortSwitchN) {
            Insertionsort(Range.Num_, Current, Func);
            continue;
        }
        if(Depth <= Levels[Count]) {
            Heapsort(Range.Num_, Current, Func);
            continue;
        }

        uint32 i0 = 0;
        uint32 i1 = Range.Num_ - 1;

        T pivot = Current[i0 + (i1 >> 1)];

        for(;;) {
            while(Func(Current[i0], pivot)) {
                ++i0;
            }

            while(Func(pivot, Current[i1])) {
                check(0 < i1);
                --i1;
            }

            if(i1 <= i0) {
                break;
            }
            Swap(Current[i0], Current[i1]);
            ++i0;
            --i1;
        }

        uint32 Offset = Range.Offset_;
        uint8 Level = Levels[Count] + 1;
        ++i1;
        check(i1 <= Range.Num_);
        uint32 Num = Range.Num_ - i1;

        if(1 < i0) {
            check(Count <= (SortMaxDepth * 2));
            Ranges[Count] = {Offset, i0};
            Levels[Count] = Level;
            ++Count;
        }

        if(1 < Num) {
            check(Count <= (SortMaxDepth * 2));
            Ranges[Count] = {Offset+i1, Num};
            Levels[Count] = Level;
            ++Count;
        }
    }
}

template<class T>
inline void Introsort(uint32 N, T* V)
{
    Introsort(N, V, [](const T& X0, const T& X1){return X0<X1;});
}
