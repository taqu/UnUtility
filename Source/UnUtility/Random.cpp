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

void FRandom::Seed(uint64 InSeed)
{
    do {
        State_ = Increment * State_ + InSeed;
    } while(0 == State_);
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
    uint64 Count = static_cast<uint64>(X >> 59);
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
    check(0<Size);
    check(nullptr != Weights);
#if 0
    LG3_ASSERT(0<size);
    LG3_ASSERT(LG3_NULL != weights);
    if(capacity_ < size) {
        do{
            capacity_ += 16UL;
        }while(capacity_<size);
        LG3_DELETE_ARRAY(weights_);
        LG3_DELETE_ARRAY(aliases_);
        weights_ = LG3_NEW f32[capacity_];
        aliases_ = LG3_NEW u32[capacity_];
    }
    size_ = size;

    // Kahan's summation
    f32 total = 0.0f;
    {
        f32 c = 0.0f;
        for(u32 i = 0; i < size; ++i) {
            f32 x = weights[i] - c;
            f32 t = total + x;
            c = (t - total) - x;
            total = t;
        }
    }

    f32 average = total/size_;
    f32 scale = (1.0e-7f<total)? static_cast<f32>(size_)/total : 0.0f;
    u32* indices = LG3_NEW u32[capacity_];

    s32 underfull = -1;
    s32 overfull = static_cast<s32>(size_);
    for(u32 i=0; i<size_; ++i){
        if(average<=weights[i]){
            --overfull;
            indices[overfull] = i;
        } else {
            ++underfull;
            indices[underfull] = i;
        }
    }
    while(0<=underfull && overfull<static_cast<s32>(size_)){
        u32 under = indices[underfull]; --underfull;
        u32 over = indices[overfull]; ++overfull;
        aliases_[under] = over;
        weights_[under] = weights[under] * scale;
        weights[over] += weights[under] - average;
        if(weights[over]<average){
            ++underfull;
            indices[underfull] = over;
        } else {
            --overfull;
            indices[overfull] = over;
        }
    }
    while(0<=underfull){
        weights_[indices[underfull]] = 1.0f;
        --underfull;
    }
    while(overfull<static_cast<s32>(size_)){
        weights_[indices[overfull]] = 1.0f;
        ++overfull;
    }
    LG3_DELETE_ARRAY(indices);
#endif
    if(Capacity_ < Size) {
        Capacity_ = (Size+63UL)&~63UL;
        FMemory::Free(Weights_);
        Weights_ = static_cast<float*>(FMemory::Malloc(sizeof(float)*Capacity_*3));
        Aliases_ = reinterpret_cast<uint32*>(Weights_+Capacity_);
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

    float Average = Total/Size_;
    float Scale = (1.0e-7f<Total)? static_cast<float>(Size_)/Total : 0.0f;
    uint32* Indices = reinterpret_cast<uint32*>(Aliases_+Capacity_);

    int32 Underfull = -1;
    int32 Overfull = static_cast<int32>(Size_);
    for(uint32 i=0; i<Size_; ++i){
        if(Average<=Weights_[i]){
            --Overfull;
            Indices[Overfull] = i;
        } else {
            ++Underfull;
            Indices[Underfull] = i;
        }
    }
    while(0<=Underfull && Overfull<static_cast<int32>(Size_)){
        uint32 under = Indices[Underfull]; --Underfull;
        uint32 over = Indices[Overfull]; ++Overfull;
        Aliases_[under] = over;
        Weights_[under] = Weights[under] * Scale;
        Weights[over] += Weights[under] - Average;
        if(Weights[over]<Average){
            ++Underfull;
            Indices[Underfull] = over;
        } else {
            --Overfull;
            Indices[Overfull] = over;
        }
    }
    while(0<=Underfull){
        Weights_[Indices[Underfull]] = 1.0f;
        --Underfull;
    }
    while(Overfull<static_cast<int32>(Size_)){
        Weights_[Indices[Overfull]] = 1.0f;
        ++Overfull;
    }
}

