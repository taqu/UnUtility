#pragma once
#include <CoreMinimal.h>

#include "HandleTable.h"

template<class T>
class FResourceTable
{
public:
    FResourceTable();
    ~FResourceTable();

    uint32 Pop();
    void Push(uint32& Handle);
    bool Valid(uint32 Handle) const;
    const T& Get(uint32 Handle) const;
    T& Get(uint32 Handle);
private:
    FResourceTable(const FResourceTable&) = delete;
    FResourceTable& operator=(const FResourceTable&) = delete;
    void Clear();

    FHandleTable Handles_;
    uint32 Capacity_;
    T* Resources_;
};

template<class T>
FResourceTable<T>::FResourceTable()
    : Capacity_(0)
    , Resources_(nullptr)
{
}

template<class T>
FResourceTable<T>::~FResourceTable()
{
    Clear();
    Capacity_= 0;
    Resources_ = nullptr;
}

template<class T>
uint32 FResourceTable<T>::Pop()
{
    uint32 Handle = Handles_.Pop();
    uint32 Capacity = Handles_.Capacity();
    if (Capacity_ < Capacity) {
        T* Resources = static_cast<T*>(FMemory::Malloc(sizeof(T)*Capacity));
        for (uint32 i = 0; i < Capacity_; ++i) {
            new (&Resources[i]) T(MoveTemp(Resources_[i]));
        }
        for (uint32 i = Capacity_; i < Capacity; ++i) {
            new (&Resources[i]) T();
        }
        Clear();
        Capacity_ = Capacity;
        Resources_ = Resources;
    }
    Resources_[FHandleTable::Index(Handle)].OnPop();
    return Handle;
}

template<class T>
void FResourceTable<T>::Push(uint32& Handle)
{
    check(Handles_.Valid(Handle));
    Resources_[FHandleTable::Index(Handle)].OnPush();
    Handles_.Push(Handle);
}

template<class T>
bool FResourceTable<T>::Valid(uint32 Handle) const
{
    return Handles_.Valid(Handle);
}

template<class T>
const T& FResourceTable<T>::Get(uint32 Handle) const
{
    check(Handles_.Valid(Handle));
    return Resources_[FHandleTable::Index(Handle)];
}

template<class T>
T& FResourceTable<T>::Get(uint32 Handle)
{
    check(Handles_.Valid(Handle));
    return Resources_[FHandleTable::Index(Handle)];
}

template<class T>
void FResourceTable<T>::Clear()
{
    for (uint32 i = 0; i < Capacity_; ++i) {
        Resources_[i].~T();
    }
    FMemory::Free(Resources_);
}

