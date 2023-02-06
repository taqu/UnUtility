#pragma once
#ifndef INC_REGEX_H_
#    define INC_REGEX_H_
#    include <CoreMinimal.h>
#    include <Misc/CString.h>

namespace sregex
{

constexpr uint32 pow2(uint32 x, uint32 s)
{
    return s <= x ? x : pow2(x << 2, s);
}

constexpr uint32 pow2(uint32 s)
{
    return pow2(1, s);
}

template<class T, uint32 E = 128>
class Array
{
public:
    static_assert(std::is_trivially_copyable<T>::value, "T should be trivially copyable.");
    inline static constexpr uint32 Expand = pow2(E);

    Array();
    ~Array();
    bool empty() const;
    uint32 size() const;
    void clear();
    void reserve(uint32 capacity);

    void push_back(const T& x);
    template<class... Args>
    void emplace_back(Args&&... args);

    void pop_back();
    T pop_value();

    const T& back() const;
    const T& operator[](uint32 index) const;
    T& operator[](uint32 index);

private:
    Array(const Array&) = delete;
    Array& operator=(const Array&) = delete;

    union Data
    {
        T static_[Expand];
        T* dynamic_;
    };

    void expand();
    const T* get() const;
    T* get();
    uint32 capacity_;
    uint32 size_;
    Data items_;
};

template<class T, uint32 E>
Array<T, E>::Array()
    : capacity_(Expand)
    , size_(0)
    , items_{}
{
}

template<class T, uint32 E>
Array<T, E>::~Array()
{
    if(Expand < capacity_) {
        FMemory::Free(get());
    }
    capacity_ = Expand;
    size_ = 0;
    items_ = {};
}

template<class T, uint32 E>
bool Array<T, E>::empty() const
{
    return size_ <= 0;
}

template<class T, uint32 E>
uint32 Array<T, E>::size() const
{
    return size_;
}

template<class T, uint32 E>
void Array<T, E>::clear()
{
    size_ = 0;
}

template<class T, uint32 E>
void Array<T, E>::reserve(uint32 capacity)
{
    capacity = pow2(capacity);
}

template<class T, uint32 E>
void Array<T, E>::push_back(const T& x)
{
    if(capacity_ <= size_) {
        expand();
    }
    get()[size_] = x;
    ++size_;
}

template<class T, uint32 E>
template<class... Args>
void Array<T, E>::emplace_back(Args&&... args)
{
    if(capacity_ <= size_) {
        expand();
    }
    T* ptr = get();
    new(&ptr[size_]) T{std::forward<Args>(args)...};
    ++size_;
}

template<class T, uint32 E>
void Array<T, E>::pop_back()
{
    check(0 < size_);
    --size_;
}

template<class T, uint32 E>
T Array<T, E>::pop_value()
{
    check(0 < size_);
    --size_;
    return get()[size_];
}

template<class T, uint32 E>
const T& Array<T, E>::back() const
{
    check(0 < size_);
    return get()[size_ - 1];
}

template<class T, uint32 E>
const T& Array<T, E>::operator[](uint32 index) const
{
    check(index < size_);
    return get()[index];
}

template<class T, uint32 E>
T& Array<T, E>::operator[](uint32 index)
{
    check(index < size_);
    return get()[index];
}

template<class T, uint32 E>
void Array<T, E>::expand()
{
    uint32 capacity = capacity_;
    do {
        capacity += Expand;
    } while(capacity <= size_);
    T* items = static_cast<T*>(FMemory::Malloc(sizeof(T) * capacity));
    T* prev = get();
    FMemory::Memcpy(items, prev, sizeof(T) * capacity_);
    if(Expand < capacity_) {
        FMemory::Free(prev);
    }
    capacity_ = capacity;
    items_.dynamic_ = items;
}

template<class T, uint32 E>
const T* Array<T, E>::get() const
{
    return (capacity_ <= Expand) ? items_.static_ : items_.dynamic_;
}

template<class T, uint32 E>
T* Array<T, E>::get()
{
    return (capacity_ <= Expand) ? items_.static_ : items_.dynamic_;
}

template<class T>
struct Symbols
{
};

template<>
struct Symbols<uint8_t>
{
    inline static constexpr uint8_t Concatenate = '\0';
    inline static constexpr uint8_t LeftParen = '(';
    inline static constexpr uint8_t RightParen = ')';
    inline static constexpr uint8_t LeftSquare = '[';
    inline static constexpr uint8_t RightSquare = ']';
    inline static constexpr uint8_t Vertical = '|';
    inline static constexpr uint8_t Dot = '.';
    inline static constexpr uint8_t Asterisk = '*';
    inline static constexpr uint8_t Plus = '+';
    inline static constexpr uint8_t Question = '?';
};

template<>
struct Symbols<char16_t>
{
    inline static constexpr char16_t Concatenate = u'\0';
    inline static constexpr char16_t LeftParen = u'(';
    inline static constexpr char16_t RightParen = u')';
    inline static constexpr char16_t LeftSquare = u'[';
    inline static constexpr char16_t RightSquare = u']';
    inline static constexpr char16_t Vertical = u'|';
    inline static constexpr char16_t Dot = u'.';
    inline static constexpr char16_t Asterisk = u'*';
    inline static constexpr char16_t Plus = u'+';
    inline static constexpr char16_t Question = u'?';
};

template<>
struct Symbols<wchar_t>
{
    inline static constexpr wchar_t Concatenate = L'\0';
    inline static constexpr wchar_t LeftParen = L'(';
    inline static constexpr wchar_t RightParen = L')';
    inline static constexpr wchar_t LeftSquare = L'[';
    inline static constexpr wchar_t RightSquare = L']';
    inline static constexpr wchar_t Vertical = L'|';
    inline static constexpr wchar_t Dot = L'.';
    inline static constexpr wchar_t Asterisk = L'*';
    inline static constexpr wchar_t Plus = L'+';
    inline static constexpr wchar_t Question = L'?';
};

template<class T>
struct Traits
{
};

template<>
struct Traits<UTF8CHAR>
{
    typedef UTF8CHAR type;
    inline static size_t strlen(const UTF8CHAR* s)
    {
        return FCStringUtf8::Strlen(s);
    }
};

template<>
struct Traits<char16_t>
{
    typedef char16_t type;
    inline static size_t strlen(const char16_t* s)
    {
        return TCString<UCS2CHAR>::Strlen(reinterpret_cast<const UCS2CHAR*>(s));
    }
};

template<>
struct Traits<WIDECHAR>
{
    typedef WIDECHAR type;
    inline static size_t strlen(const WIDECHAR* s)
    {
        return FCStringWide::Strlen(s);
    }
};

template<class T>
class SRegex
{
public:
    inline static constexpr int32 ListMask = 0x7FFF'FFFF;

    using traits = Traits<T>;
    using U = typename traits::type;
    using Buffer = Array<U, 128>;

    struct State
    {
        int32 c_;
        int32 lastlist_;
        uint32 out_[2];
    };

    enum class Type : int32
    {
        Any = 0x7FFD'0000,
        Match = 0x7FFE'0000,
        Split = 0x7FFF'0000,
    };

    struct Fragment
    {
        uint32 start_;
        uint32 out_;
    };

    static uint32 make_link(int32 state, uint32 link)
    {
        return static_cast<uint32>(state & ListMask) | (link << 31U);
    }
    static uint32 make_link(int32 state)
    {
        return static_cast<uint32>(state & ListMask);
    }

    static uint32 get_link(uint32 link)
    {
        return link >> 31U;
    }
    static int32 get_state(uint32 link)
    {
        return static_cast<int32>(link & ListMask);
    }

    SRegex();
    ~SRegex();

    bool compile(const T* begin);
    bool compile(const T* begin, const T* end);
    bool match(const T* begin);
    bool match(const T* begin, const T* end);

private:
    SRegex(const SRegex&) = delete;
    SRegex& operator=(const SRegex&) = delete;

    bool regex2postfix(Buffer& result, const T* begin, const T* end);

    bool post2nfa(const Buffer& postfix);

    int32 create(int32 c, int32 out0, int32 out1);
    uint32 list(int32 state, uint32 next);
    void patch(uint32 list, int32 state);
    uint32 append(uint32 list0, uint32 list1);

    bool match(int32 start, const T* begin, const T* end);
    uint32 start(int32 state, uint32 list);
    void add(uint32 list, int32 state);
    void step(uint32 clist, int32 c, uint32 nlist);
    bool is_match(uint32 list) const;

    int32 start_ = ListMask;
    int32 listId_ = 0;
    Array<State, 16> states_;
    Array<uint32, 16> list_[2];
};

template<class T>
SRegex<T>::SRegex()
{
}

template<class T>
SRegex<T>::~SRegex()
{
}

template<class T>
bool SRegex<T>::compile(const T* begin)
{
    check(nullptr != begin);
    return compile(begin, begin + traits::strlen(begin));
}

template<class T>
bool SRegex<T>::compile(const T* begin, const T* end)
{
    check(nullptr != begin);
    check(begin <= end);
    Buffer postfix;
    if(!regex2postfix(postfix, begin, end)) {
        return false;
    }
    return post2nfa(postfix);
}

template<class T>
bool SRegex<T>::match(const T* begin)
{
    check(nullptr != begin);
    return match(begin, begin + traits::strlen(begin));
}

template<class T>
bool SRegex<T>::match(const T* begin, const T* end)
{
    check(nullptr != begin);
    check(begin <= end);
    return match(start_, begin, end);
}

template<class T>
bool SRegex<T>::regex2postfix(Buffer& result, const T* begin, const T* end)
{
    using Symbols = Symbols<U>;
    check(nullptr != begin);
    check(nullptr != end);
    struct Paren
    {
        int32 count_atoms_;
        int32 count_alts_;
    };
    int32 count_atoms = 0;
    int32 count_alts = 0;
    int32 count_squares = 0;
    Array<Paren, 16> parens;
    for(const T* itr = begin; itr < end; ++itr) {
        U c = static_cast<U>(itr[0]);
        switch(c) {
        case Symbols::LeftParen:
            if(1 < count_atoms) {
                --count_atoms;
                result.push_back(Symbols::Concatenate);
            }
            parens.push_back({count_atoms, count_alts});
            count_atoms = 0;
            count_alts = 0;
            break;
        case Symbols::RightParen:
            if(parens.empty() || count_atoms <= 0) {
                return false;
            }
            --count_atoms;
            while(0 < count_atoms) {
                result.push_back(Symbols::Concatenate);
                --count_atoms;
            }
            while(0 < count_alts) {
                --count_alts;
                result.push_back(Symbols::Vertical);
            }
            count_atoms = parens.back().count_atoms_;
            count_alts = parens.back().count_alts_;
            parens.pop_back();
            ++count_atoms;
            break;
        case Symbols::Vertical:
            if(count_atoms <= 0) {
                return false;
            }
            --count_atoms;
            while(0 < count_atoms) {
                result.push_back(Symbols::Concatenate);
                --count_atoms;
            }
            ++count_alts;
            break;
        case Symbols::Asterisk:
        case Symbols::Plus:
        case Symbols::Question:
            if(count_atoms <= 0) {
                return false;
            }
            result.push_back(c);
            break;
        default:
            if(1 < count_atoms) {
                --count_atoms;
                result.push_back(Symbols::Concatenate);
            }
            result.push_back(c);
            ++count_atoms;
            break;
        } // switch(c)
    }     // for(
    if(!parens.empty()) {
        return false;
    }
    --count_atoms;
    while(0 < count_atoms) {
        result.push_back(Symbols::Concatenate);
        --count_atoms;
    }
    while(0 < count_alts) {
        --count_alts;
        result.push_back(Symbols::Vertical);
    }
    return true;
}

template<class T>
bool SRegex<T>::post2nfa(const Buffer& postfix)
{
    using Symbols = Symbols<U>;
    states_.clear();
    create(static_cast<int32>(Type::Match), ListMask, ListMask);
    start_ = ListMask;
    Array<Fragment, 64> stack;
    for(uint32 i = 0; i < postfix.size(); ++i) {
        U c = postfix[i];
        switch(c) {
        case Symbols::Concatenate: {
            Fragment e1 = stack.pop_value();
            Fragment e0 = stack.pop_value();
            patch(e0.out_, e1.start_);
            stack.emplace_back(e0.start_, e1.out_);
        } break;
        case Symbols::Vertical: {
            Fragment e1 = stack.pop_value();
            Fragment e0 = stack.pop_value();
            int32 state = create(static_cast<int32>(Type::Split), get_state(e0.start_), e1.start_);
            stack.emplace_back(make_link(state), append(e0.out_, e1.out_));
        } break;
        case Symbols::Asterisk: {
            Fragment e = stack.pop_value();
            int32 state = create(static_cast<int32>(Type::Split), get_state(e.start_), ListMask);
            patch(e.out_, state);
            stack.emplace_back(make_link(state), list(state, 1));
        } break;
        case Symbols::Plus: {
            Fragment e = stack.pop_value();
            int32 state = create(static_cast<int32>(Type::Split), get_state(e.start_), ListMask);
            patch(e.out_, state);
            stack.emplace_back(e.start_, list(state, 1));
        } break;
        case Symbols::Question: {
            Fragment e = stack.pop_value();
            int32 state = create(static_cast<int32>(Type::Split), get_state(e.start_), ListMask);
            stack.emplace_back(make_link(state), append(e.out_, list(state, 1)));
        } break;
        case Symbols::Dot: {
            int32 state = create(static_cast<int32>(Type::Any), ListMask, ListMask);
            stack.emplace_back(make_link(state), list(state, 0));
        } break;
        default: {
            int32 state = create(c, ListMask, ListMask);
            stack.emplace_back(make_link(state), list(state, 0));
        } break;
        } // switch(c)
    }     // for(
    if(stack.empty()) {
        return false;
    }
    Fragment e = stack.pop_value();
    patch(e.out_, 0);
    start_ = get_state(e.start_);
    return true;
}

template<class T>
int32 SRegex<T>::create(int32 c, int32 out0, int32 out1)
{
    int32 state = static_cast<int32>(states_.size());
    states_.push_back({c, 0, make_link(out0), make_link(out1)});
    return state;
}

template<class T>
uint32 SRegex<T>::list(int32 state, uint32 list)
{
    states_[state].out_[list] = ListMask;
    return make_link(state, list);
}

template<class T>
void SRegex<T>::patch(uint32 list, int32 state)
{
    int32 s = get_state(list);
    uint32 l = get_link(list);
    while(ListMask != s) {
        uint32 next = states_[s].out_[l];
        states_[s].out_[l] = state;
        s = get_state(next);
        l = get_link(next);
    }
}

template<class T>
uint32 SRegex<T>::append(uint32 list0, uint32 list1)
{
    uint32 prev = list0;
    int32 s = get_state(list0);
    uint32 l = get_link(list0);
    while(ListMask != states_[s].out_[l]) {
        list0 = states_[s].out_[l];
        s = get_state(list0);
        l = get_link(list0);
    }
    states_[s].out_[l] = list1;
    return prev;
}

template<class T>
bool SRegex<T>::match(int32 state, const T* begin, const T* end)
{
    check(nullptr != begin);
    check(begin <= end);
    listId_ = 0;
    list_[0].clear();
    list_[1].clear();
    uint32 clist = start(state, 0);
    uint32 nlist = 1;
    for(const T* s = begin; s < end; ++s) {
        int32 c = static_cast<int32>(s[0]);
        step(clist, c, nlist);
        uint32 t = clist;
        clist = nlist;
        nlist = t;
    }
    return is_match(clist);
}

template<class T>
uint32 SRegex<T>::start(int32 state, uint32 list)
{
    list_[list].clear();
    ++listId_;
    add(list, state);
    return list;
}

template<class T>
void SRegex<T>::add(uint32 list, int32 state)
{
    if(ListMask == state || states_[state].lastlist_ == listId_) {
        return;
    }
    State& s = states_[state];
    s.lastlist_ = listId_;
    if(s.c_ == static_cast<int32>(Type::Split)) {
        add(list, get_state(s.out_[0]));
        add(list, get_state(s.out_[1]));
        return;
    }

    uint32 l = static_cast<uint32>(state & ListMask) | (list << 31U);
    list_[list].push_back(l);
}

template<class T>
void SRegex<T>::step(uint32 clist, int32 c, uint32 nlist)
{
    ++listId_;
    list_[nlist].clear();
    for(uint32 i = 0; i < list_[clist].size(); ++i) {
        uint32 s = list_[clist][i] & ListMask;
        const State& state = states_[s];
        if(c == state.c_ || static_cast<int32>(Type::Any) == state.c_) {
            add(nlist, state.out_[0]);
        }
    }
}

template<class T>
bool SRegex<T>::is_match(uint32 list) const
{
    for(uint32 i = 0; i < list_[list].size(); ++i) {
        const State& state = states_[list_[list][i] & ListMask];
        if(static_cast<int32>(Type::Match) == state.c_) {
            return true;
        }
    }
    return false;
}
} // namespace sregex

#endif // INC_REGEX_H_
