#include "HandleTable.h"

#include <glslang/glslang/src/glslang_lib/SPIRV/spirv.hpp>

FHandleTable::FHandleTable()
    :Capacity_(0)
	,Top_(Invalid)
	,Handles_(nullptr)
{
    Resize(Expand);
}

FHandleTable::~FHandleTable()
{
    Capacity_ = 0;
    Top_ = 0;
    FMemory::Free(Handles_);
    Handles_ = nullptr;
}

uint32 FHandleTable::Capacity() const
{
    return Capacity_;
}

void FHandleTable::Reserve(uint32 Capacity, bool Clear)
{
    if (Clear) {
        Capacity_ = 0;
        Top_ = Invalid;
        FMemory::Free(Handles_);
        Handles_ = nullptr;
    }
    if(Capacity<=Capacity_) {
        return;
    }
    Capacity = (Capacity<=0)? Expand : (Capacity + (Expand-1)) & ~(Expand-1);
    Resize(Capacity);
}

uint32 FHandleTable::Pop()
{
    if(Invalid == Top_) {
        Resize(Capacity_ + Expand);
    }
    check(Invalid != Top_);
    uint32 Index = Top_&HandleMask;
    uint32 Count = Handles_[Index]>>CountShift;
    Count = (Count+1) & CountMask;
    Top_ = (Invalid != (Handles_[Index] & Invalid))? Handles_[Index]&HandleMask : Invalid;
    Handles_[Index] = Index | (Count<<CountShift) | UsedFlag;
    return Handles_[Index];
}

void FHandleTable::Push(uint32& Handle)
{
    check(Valid(Handle));
    uint32 Index = Handle & HandleMask;
    uint32 Count = Handle & (CountMask << CountShift);
    if (Invalid == Top_) {
        Handles_[Index] = Invalid | Count;
    } else {
        Handles_[Index] = Top_ | Count;
    }
    Top_ = Index;
    Handle = 0;
}

bool FHandleTable::Valid(uint32 Handle) const
{
    uint32 Index = Handle&HandleMask;
    check(Index<Capacity_);
    uint32 Count = Handle>>CountShift;
    return Count == (Handles_[Index]>>CountShift);
}

void FHandleTable::Resize(uint32 Capacity)
{
    check(Capacity<=MaxHandles);
    check(Capacity_<Capacity);
    uint32* Handles = static_cast<uint32*>(FMemory::Malloc(sizeof(uint32)*Capacity));
    FMemory::Memcpy(Handles_, Handles, sizeof(uint32)*Capacity_);
	for(uint32 i=Capacity_+1; i<Capacity; ++i){
	    Handles[i-1] = i;
	}
    Handles[Capacity-1] = Invalid;
    if(Invalid != Top_){
        Handles[Capacity-1] = Top_;
    }
    Top_ = Capacity_;
    Capacity_ = Capacity;
    FMemory::Free(Handles_);
    Handles_ = Handles;
}
