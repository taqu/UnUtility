#pragma once
#include <CoreMinimal.h>

//--- FRandom32
//-------------------------
class UNUTILITY_API FRandom32
{
public:
    FRandom32();
    explicit FRandom32(uint64 Seed);
    ~FRandom32();

    /**
    @brief Initialize with a seed.
    @param Seed
    */
    void Seed(uint64 InSeed);

    /**
    @brief Generate a unsigned number in [0 0xFFFFFFFF]
    */
    uint32 Rand();

    /**
    @brief Generate a float in [0, 1)
    */
    float FRand();
private:
    static constexpr uint64 Increment = 0xDA3E39CB94B95BDBULL;
    static constexpr uint64 Multiplier = 0x5851F42D4C957F2DULL;
    uint64 State_;
};

class UNUTILITY_API FRandom64
{
public:
    FRandom64();
    FRandom64(uint64 Seed0, uint64 Seed1, uint64 Seed2, uint64 Seed3);
    ~FRandom64();

    /**
    @brief Initialize with a seed.
    */
    void Seed(uint64 Seed0, uint64 Seed1, uint64 Seed2, uint64 Seed3);

    /**
    @brief Generate a unsigned number in [0 0xFFFFFFFF'FFFFFFFF]
    */
    uint64 Rand();

    /**
    @brief Generate a double in [0, 1)
    */
    double DRand();

private:
    struct UInt128
    {
        uint64 High_;
        uint64 Low_;
    };
    inline static UInt128 Add(const UInt128& X0, const UInt128& X1);
    inline static UInt128 Mul(uint64 X0, uint64 X1);
    inline static UInt128 Mul(const UInt128& X0, const UInt128& X1);
    inline void Next();

    static constexpr UInt128 Multiplier = {2549297995355413924ULL, 4865540595714422341ULL};
    UInt128 Increment_;
    UInt128 State_;
};

/*
 * @brief return [0, s)
 * [Fast Random Integer Generation in an Interval](https://arxiv.org/abs/1805.10941)
 */
template<class T>
uint32 RangeROpen(T& Random, uint32 s)
{
    uint32 x = Random.Rand();
    uint64 m = static_cast<uint64>(x) * static_cast<uint64>(s);
    uint32 l = static_cast<uint32>(m);
    if(l < s) {
        uint32 t = static_cast<uint32>(-static_cast<int32>(s) % s);
        while(l < t) {
            x = Random.Rand();
            m = static_cast<uint64>(x) * static_cast<uint64>(s);
            l = static_cast<uint32>(m);
        }
    }
    return static_cast<uint32>(m >> 32);
}

/**
 * @brief return [0, v)
 * [Fast Random Integer Generation in an Interval](https://arxiv.org/abs/1805.10941)
 */
template<class T>
uint64 RangeROpen(T& Random, uint64 s)
{
    uint64 t = (-s) % s;
    uint64 x;
    do {
        x = Random.Rand();
    } while(x < t);
    return x % s;
}

template<class T, class U>
void Shuffle(T& random, uint32 num, U* v)
{
    for(uint32 i = num; 1 < i; --i) {
        uint32 offset = RangeROpen(random, i);
        U t = MoveTemp(v[i-1]);
        v[i - 1] = MoveTemp(v[offset]);
        v[offset] = MoveTemp(t);
    }
}

class UNUTILITY_API FRandomAliasSelect
{
public:
    FRandomAliasSelect();
    ~FRandomAliasSelect();
    uint32 Size() const;
    void Build(uint32 Size, float* Weights);
    template<class T>
    uint32 Select(T& Random) const;
private:
    FRandomAliasSelect(const FRandomAliasSelect&) = delete;
    FRandomAliasSelect& operator=(const FRandomAliasSelect&) = delete;
    uint32 Capacity_;
    uint32 Size_;
    float* Weights_;
    uint32* Aliases_;
};

template<class T>
uint32 FRandomAliasSelect::Select(T& Random) const
{
    check(0<Size_);
    uint32 Index = RangeROpen(Random, Size_);
    float W = Random.FRand();
    return W<Weights_[Index]? Index : Aliases_[Index];
}

