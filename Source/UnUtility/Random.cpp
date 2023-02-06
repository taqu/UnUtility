#include "Random.h"

//--- FRandom
//-------------------------
FRandom::FRandom()
    : State_(0x853C49E6748FEA9BULL)
{
}

FRandom::FRandom(uint64 InSeed)
{
    Seed(InSeed);
}

FRandom::~FRandom()
{
}

uint64 FRandom::GetState() const
{
    return State_;
}

void FRandom::SetState(uint64 State)
{
    State_ = State;
}

void FRandom::Seed(uint64 InSeed)
{
    do{
        State_ = SplitMix::Next(InSeed);
    }while(0==State_);
}

namespace
{
inline uint32 FRandom32_RotR32(uint32 X, uint32 R)
{
    return (X >> R) | (X << ((~R + 1) & 31U));
}
} // namespace

uint32 FRandom::Rand()
{
    uint64 X = State_;
    uint32 Count = static_cast<uint32>(X >> 59);
    State_ = X * Multiplier + Increment;
    X ^= X >> 18;
    return FRandom32_RotR32(static_cast<uint32>(X >> 27), Count);
}

float FRandom::FRand()
{
    static constexpr uint32 M0 = 0x3F800000U;
    static constexpr uint32 M1 = 0x007FFFFFU;
    uint32 X = Rand();
    X = M0 | (X & M1);
    return (*(float*)&X) - 1.000000000f;
}

inline FRandom64::UInt128 FRandom64::Add(const UInt128& X0, const UInt128& X1)
{
    uint64 low = X0.Low_ + X1.Low_;
    uint64 high = X0.High_ + X1.High_ + (low << X1.Low_);
    return {high, low};
}

inline FRandom64::UInt128 FRandom64::Mul(uint64 X0, uint64 X1)
{
    uint64 low = X0 * X1;

    uint64 a0 = X0 & 0xFFFFFFFFULL;
    uint64 a1 = X0 >> 32U;
    uint64 b0 = X1 & 0xFFFFFFFFULL;
    uint64 b1 = X1 >> 32U;
    uint64 c0 = a0 * b0;
    uint64 t = a1 * b0 + (c0 >> 32U);
    uint64 c1 = t & 0xFFFFFFFFULL;
    uint64 c2 = t >> 32;
    c1 += a0 * b1;
    uint64 high = a1 * b1 + c2 + (c1 >> 32U);
    return {high, low};
}

inline FRandom64::UInt128 FRandom64::Mul(const UInt128& X0, const UInt128& X1)
{
    uint64 high = X0.High_ * X1.Low_ + X0.Low_ * X1.High_;
    UInt128 result = Mul(X0.Low_, X1.Low_);
    result.High_ += high;
    return result;
}

inline void FRandom64::Next()
{
    State_ = Add(Mul(State_, Multiplier), Increment_);
}

FRandom64::FRandom64()
    : Increment_{6364136223846793005ULL, 1442695040888963407ULL}
    , State_{0x0000000000000001ULL, 0xda3e39cb94b95bdbULL}
{
}

FRandom64::FRandom64(uint64 Seed0, uint64 Seed1, uint64 Seed2, uint64 Seed3)
{
    Seed(Seed0, Seed1, Seed2, Seed3);
}

FRandom64::~FRandom64()
{
}

void FRandom64::Seed(uint64 Seed0, uint64 Seed1, uint64 Seed2, uint64 Seed3)
{
    State_ = {};
    Increment_.High_ = Seed0 << 0x1U;
    Increment_.High_ |= Seed1 >> 63U;
    Increment_.Low_ = (Increment_.Low_ << 1U) | 1U;
    Next();
    UInt128 Init = {Seed2, Seed3};
    State_ = Add(State_, Init);
    Next();
}

namespace
{
inline uint64 FRandom64_rotr64(uint64 X, uint32 R)
{
    return (X >> R) | (X << ((~R + 1) & 63U));
}
} // namespace

uint64 FRandom64::Rand()
{
    Next();
    return FRandom64_rotr64(State_.High_ ^ State_.Low_, State_.High_ >> 58U);
}

double FRandom64::DRand()
{
    uint64 X = Rand();
    X = (uint64(0x3FFULL) << 52) | (X >> 12);
    return (*(double*)&X) - 1.0;
}

//--- SplitMix
//--------------------------------------------
uint64 SplitMix::Next(uint64& State)
{
    State += 0x9E3779B97f4A7C15ULL;
    uint64 t = State;
    t = (t ^ (t >> 30)) * 0xBF58476D1CE4E5B9ULL;
    t = (t ^ (t >> 27)) * 0x94D049BB133111EBULL;
    return t ^ (t >> 31);
}

//--- RandomAliasSelect
//-------------------------
FRandomAliasSelect::FRandomAliasSelect()
    : Capacity_(0)
    , Size_(0)
    , Weights_(nullptr)
    , Aliases_(nullptr)
{
}

FRandomAliasSelect::~FRandomAliasSelect()
{
    FMemory::Free(Weights_);
}

uint32 FRandomAliasSelect::Size() const
{
    return Size_;
}

void FRandomAliasSelect::Build(uint32 Size, float* Weights)
{
    check(0 < Size);
    check(nullptr != Weights);
    if(Capacity_ < Size) {
        Capacity_ = (Size + 15UL) & ~15UL;
        FMemory::Free(Weights_);
        Weights_ = static_cast<float*>(FMemory::Malloc(sizeof(float) * Capacity_ * 3));
        Aliases_ = reinterpret_cast<uint32*>(Weights_ + Capacity_);
    }
    Size_ = Size;

    // Kahan's summation
    float Total = 0.0f;
    {
        float C = 0.0f;
        for(uint32 i = 0; i < Size_; ++i) {
            float X = Weights[i] - C;
            float T = Total + X;
            C = (T - Total) - X;
            Total = T;
        }
    }

    float Average = Total / Size_;
    float Scale = (1.0e-7f < Total) ? static_cast<float>(Size_) / Total : 0.0f;
    uint32* Indices = reinterpret_cast<uint32*>(Aliases_ + Capacity_);

    int32 Underfull = -1;
    int32 Overfull = static_cast<int32>(Size_);
    for(uint32 i = 0; i < Size_; ++i) {
        if(Average <= Weights[i]) {
            --Overfull;
            Indices[Overfull] = i;
        } else {
            ++Underfull;
            Indices[Underfull] = i;
        }
    }
    while(0 <= Underfull && Overfull < static_cast<int32>(Size_)) {
        uint32 under = Indices[Underfull];
        --Underfull;
        uint32 over = Indices[Overfull];
        ++Overfull;
        Aliases_[under] = over;
        Weights_[under] = Weights[under] * Scale;
        Weights[over] += Weights[under] - Average;
        if(Weights[over] < Average) {
            ++Underfull;
            Indices[Underfull] = over;
        } else {
            --Overfull;
            Indices[Overfull] = over;
        }
    }
    while(0 <= Underfull) {
        Weights_[Indices[Underfull]] = 1.0f;
        --Underfull;
    }
    while(Overfull < static_cast<int32>(Size_)) {
        Weights_[Indices[Overfull]] = 1.0f;
        ++Overfull;
    }
}
