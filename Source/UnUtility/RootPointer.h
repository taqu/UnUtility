#pragma once

template<class T>
class TRootPointer
{
public:
    TRootPointer();
    TRootPointer(T* Pointer) noexcept;
    ~TRootPointer() noexcept;

    TRootPointer(TRootPointer&) noexcept;
    TRootPointer& operator=(TRootPointer&) noexcept;
    TRootPointer(TRootPointer&& Other) noexcept;
    TRootPointer& operator=(TRootPointer&& Other) noexcept;

    operator bool() const noexcept;

    const T* operator->() const noexcept;
    T* operator->() noexcept;

    const T& operator*() const noexcept;
    T& operator*() noexcept;

    void Reset(T* Pointer = nullptr) noexcept;
    T* Release() noexcept;

    const T* Get() const;
    T* Get();

private:
    void Remove()
    {
        if(nullptr != Pointer_) {
            Pointer_->RemoveFromRoot();
            Pointer_ = nullptr;
        }
    }

    T* Pointer_;
};

template<class T>
TRootPointer<T>::TRootPointer()
    : Pointer_(nullptr)
{
}

template<class T>
TRootPointer<T>::TRootPointer(T* Pointer) noexcept
    : Pointer_(Pointer)
{
    if(nullptr != Pointer_) {
        Pointer_->AddToRoot();
    }
}

template<class T>
TRootPointer<T>::~TRootPointer() noexcept
{
    Remove();
}

template<class T>
TRootPointer<T>::TRootPointer(TRootPointer& Other) noexcept
    : Pointer_(Other.Pointer_)
{
    Other.Pointer_ = nullptr;
}

template<class T>
TRootPointer<T>& TRootPointer<T>::operator=(TRootPointer& Other) noexcept
{
    if(this != &Other) {
        Remove();
        Pointer_ = Other.Pointer_;
        Other.Pointer_ = nullptr;
    }
    return *this;
}

template<class T>
TRootPointer<T>::TRootPointer(TRootPointer&& Other) noexcept
    : Pointer_(Other.Pointer_)
{
    Other.Pointer_ = nullptr;
}

template<class T>
TRootPointer<T>& TRootPointer<T>::operator=(TRootPointer&& Other) noexcept
{
    if(this != &Other) {
        Remove();
        Pointer_ = Other.Pointer_;
        Other.Pointer_ = nullptr;
    }
    return *this;
}

template<class T>
TRootPointer<T>::operator bool() const noexcept
{
    return nullptr != Pointer_;
}

template<class T>
const T* TRootPointer<T>::operator->() const noexcept
{
    return Pointer_;
}

template<class T>
T* TRootPointer<T>::operator->() noexcept
{
    return Pointer_;
}

template<class T>
const T& TRootPointer<T>::operator*() const noexcept
{
    check(nullptr != Pointer_);
    return *Pointer_;
}

template<class T>
T& TRootPointer<T>::operator*() noexcept
{
    check(nullptr != Pointer_);
    return *Pointer_;
}

template<class T>
void TRootPointer<T>::Reset(T* Pointer) noexcept
{
    Remove();
    Pointer_ = Pointer;
    if(nullptr != Pointer_) {
        Pointer_->AddToRoot();
    }
}

template<class T>
T* TRootPointer<T>::Release() noexcept
{
    T* Pointer = Pointer_;
    Pointer_ = nullptr;
    return Pointer;
}

template<class T>
const T* TRootPointer<T>::Get() const
{
    return Pointer_;
}

template<class T>
T* TRootPointer<T>::Get()
{
    return Pointer_;
}
