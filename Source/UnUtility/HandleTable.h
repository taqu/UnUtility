#pragma once
#include <CoreMinimal.h>

class UNUTILITY_API FHandleTable
{
public:
#if WITH_AUTOMATION_TESTS
    static constexpr uint32 MaxHandles = 0x7FU + 1;
    static constexpr uint32 Invalid = 0xFFU;
#else
    static constexpr uint32 MaxHandles = 0x7F'FFFFU+1;
    static constexpr uint32 Invalid = 0xFF'FFFFU;
#endif
    static constexpr uint32 HandleMask = MaxHandles - 1;
    static constexpr uint32 CountShift = 24U;
    static constexpr uint32 CountMask = 0x7FU;
    static constexpr uint32 UsedFlag = 0x8000'0000U;
    static constexpr uint32 Expand = 64;

    FHandleTable();
    ~FHandleTable();

    uint32 Capacity() const;
    void Reserve(uint32 Capacity, bool Clear=true);
    uint32 Pop();
    void Push(uint32& Handle);
    bool Valid(uint32 Handle) const;
    static inline uint32 Index(uint32 Handle);
private:
    FHandleTable(const FHandleTable&) = delete;
    FHandleTable& operator=(const FHandleTable&) = delete;

    void Resize(uint32 Capacity);
    uint32 Capacity_;
    uint32 Top_;
    uint32* Handles_;
};

inline uint32 FHandleTable::Index(uint32 Handle)
{
    return Handle&HandleMask;
}

