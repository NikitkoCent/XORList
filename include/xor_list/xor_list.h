#ifndef XORLIST_XOR_LIST_H
#define XORLIST_XOR_LIST_H

#include <initializer_list> // ::std::initializer_list
#include <memory>           // ::std::allocator, ::std::allocator_traits, ::std::addressof
#include <utility>          // ::std::move, ::std::forward, ::std::pair
#include <functional>       // ::std::less, ::std::equal_to
#include <iterator>         // ::std::bidirectional_iterator_tag, ::std::next, ::std::iterator_traits
#include <type_traits>      // ::std::conditional, ::std::enable_if, ::std::is_base_of
#include <cstdint>          // ::std::uint*_t
#include <cstddef>          // ::std::ptrdiff_t
#include <algorithm>        // ::std::swap
#include <tuple>            // ::std::tie
#include <array>            // ::std::array


template <typename T, class TAllocator = ::std::allocator<T>>
class xor_list
{
private:
    template<typename It, typename V>
    class IteratorBase;

    struct Node;

public:
    class const_iterator;

    class iterator : public IteratorBase<iterator, T>
    {
    public:
        iterator() noexcept = default;
        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;

        ~iterator() noexcept = default;

        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;


        operator const_iterator() const noexcept
        {
            return { this->prev, this->current };
        }

    private:
        friend class xor_list<T, TAllocator>;
        friend class const_iterator;


        iterator(Node *prev, Node *current) noexcept
            : IteratorBase<iterator, T>(prev, current)
        {
        }
    };

    class const_iterator : public IteratorBase<const_iterator, const T>
    {
    public:
        const_iterator() noexcept = default;
        const_iterator(const const_iterator&) noexcept = default;
        const_iterator(const_iterator&&) noexcept = default;

        ~const_iterator() noexcept = default;

        const_iterator& operator=(const const_iterator&) noexcept = default;
        const_iterator& operator=(const_iterator&&) noexcept = default;

    private:
        friend class xor_list<T, TAllocator>;
        friend class iterator;


        const_iterator(Node *prev, Node *current) noexcept
            : IteratorBase<const_iterator, const T>(prev, current)
        {
        }

        explicit operator iterator() const noexcept
        {
            return { this->prev, this->current };
        }
    };

    using value_type = T;
    using allocator_type = TAllocator;
    using size_type = ::std::size_t;
    using difference_type = ::std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = typename ::std::allocator_traits<TAllocator>::pointer;
    using const_pointer = typename ::std::allocator_traits<TAllocator>::const_pointer;


    xor_list()
        : xor_list(TAllocator())
    {}

    explicit xor_list(const TAllocator &alloc)
        : allocator(alloc), beforeHead(&afterTail), afterTail(&beforeHead)
    {}

    xor_list(::std::initializer_list<T> il, const TAllocator &alloc = TAllocator())
        : xor_list(alloc)
    {
        assign(::std::move(il));
    }

    explicit xor_list(size_type n, const TAllocator &alloc = TAllocator())
        : xor_list(alloc)
    {
        resize(n);
    }

    xor_list(size_type n, const_reference val, const TAllocator &alloc = TAllocator())
        : xor_list(alloc)
    {
        resize(n, val);
    }

    xor_list(const xor_list &other)
        : xor_list(::std::allocator_traits<NodeAllocator>::select_on_container_copy_construction(other.allocator))
    {
        insert(cbegin(), other.cbegin(), other.cend());
    }

    xor_list(xor_list &&other)
        : allocator(::std::move(other.allocator)), beforeHead(&afterTail), afterTail(&beforeHead)
    {
        if (!other.empty())
        {
            splice(cbegin(), other);
        }
    }

    virtual ~xor_list()
    {
        clear();
    }

    xor_list& operator=(const xor_list &right)
    {
        if (this != ::std::addressof(right))
        {
            copyAssignmentImpl(right);
        }
        return *this;
    }

    xor_list& operator=(xor_list &&right)
    {
        if (this != ::std::addressof(right))
        {
            moveAssignmentImpl(::std::move(right));
        }
        return *this;
    }

    void swap(xor_list &other)
    {
        swapImpl(other);
    }

    void push_back(const_reference data)
    {
        emplace_back(data);
    }

    void push_back(T &&data)
    {
        emplace_back(::std::move(data));
    }

    void push_front(const_reference data)
    {
        emplace_front(data);
    }

    void push_front(T &&data)
    {
        emplace_front(::std::move(data));
    }

    template <typename... Args>
    void emplace_back(Args&&... args)
    {
        (void)emplace(cend(), ::std::forward<Args>(args)...);
    }

    template <typename... Args>
    void emplace_front(Args&&... args)
    {
        (void)emplace(cbegin(), ::std::forward<Args>(args)...);
    }

    void pop_front()
    {
        (void)erase(cbegin());
    }

    void pop_back()
    {
        (void)erase(--cend());
    }

    size_type size() const noexcept
    {
        return length;
    }

    bool empty() const noexcept
    {
        return (size() == 0);
    }

    void clear()
    {
        destroySequence(cbegin(), cend(), size());
    }

    T& back()
    {
        return *(--end());
    }

    const T& back() const
    {
        return *(--cend());
    }

    T& front()
    {
        return *begin();
    }

    const T& front() const
    {
        return *cbegin();
    }

    // Iterators and such
    iterator begin() noexcept
    {
        return { &beforeHead, reinterpret_cast<Node*>(beforeHead.xorPtr) };
    }

    iterator end() noexcept
    {
        return { reinterpret_cast<Node*>(afterTail.xorPtr), &afterTail };
    }

    const_iterator begin() const noexcept
    {
        return cbegin();
    }

    const_iterator end() const noexcept
    {
        return cend();
    }

    const_iterator cbegin() const noexcept
    {
        return { &beforeHead, reinterpret_cast<Node*>(beforeHead.xorPtr) };
    }

    const_iterator cend() const noexcept
    {
        return { reinterpret_cast<Node*>(afterTail.xorPtr), &afterTail };
    }

    void sort()
    {
        sort(::std::less<T>{});
    }

    template<typename Compare>
    void sort(Compare isLess)
    {
        using Range = ::std::pair<const_iterator, const_iterator>;
        struct NullableRange
        {
            Range range;
            bool isNull = true;
        };


        const auto thisSize = size();

        if (thisSize < 2)
        {
            return;
        }

        ::std::array<NullableRange, 32> sortedRanges;

        while (!empty())
        {
            Range newRange = cutSequenceFromThis(cbegin(), ++cbegin(), 1).cutted;

            ::std::uint32_t i = 0;
            for ( ; i < sortedRanges.size(); ++i)
            {
                if (sortedRanges[i].isNull)
                {
                    sortedRanges[i].range = newRange;
                    sortedRanges[i].isNull = false;
                    break;
                }
                else
                {
                    newRange = mergeSequences(sortedRanges[i].range.first, sortedRanges[i].range.second,
                                              newRange.first, newRange.second, isLess);
                    sortedRanges[i].isNull = true;
                }
            }

            if (i == sortedRanges.size())
            {
                sortedRanges.back().range = newRange;
                sortedRanges.back().isNull = false;
            }
        }

        NullableRange result;
        for (::std::uint32_t i = 0; i < sortedRanges.size(); ++i)
        {
            if (!sortedRanges[i].isNull)
            {
                if (result.isNull)
                {
                    result = sortedRanges[i];
                }
                else
                {
                    result.range = mergeSequences(sortedRanges[i].range.first, sortedRanges[i].range.second,
                                                  result.range.first, result.range.second, isLess);
                }
            }
        }

        (void)insertSequenceToThisBefore(cend(), result.range.first, result.range.second, thisSize);
    }

    // WARNING! Iterators equal to position will become invalid
    // strong exception-safe guarantee
    iterator insert(const_iterator position, const_reference val)
    {
        return emplace(position, val);
    }

    // WARNING! Iterators equal to position will become invalid
    // strong exception-safe guarantee
    template<typename InputIterator>
    iterator insert(const_iterator position, InputIterator first, InputIterator last)
    {
        if (first == last)
        {
            return static_cast<iterator>(position);
        }

        const iterator result = insert(position, *first);
        position = result;
        size_type insertedCount = 1;

        for (++position ; ++first != last; ++position, ++insertedCount)
        {
            try
            {
                position = insert(position, *first);
            }
            catch(...)
            {
                destroySequence(result, position, insertedCount);
                throw;
            }
        }

        return result;
    }

    // WARNING! Iterators equal to position will become invalid
    // strong exception-safe guarantee
    template<typename... Args>
    iterator emplace(const_iterator position, Args&&... args)
    {
        //insertNodeToThisBefore noexcept!
        return insertNodeToThisBefore(position, createNode(::std::forward<Args>(args)...)).first;
    }

    // WARNING! All iterators will become invalid
    // Complexity: O(1)
    void reverse() noexcept
    {
        if (empty())
        {
            return;
        }

        auto first = cbegin();
        auto last = --cend();

        first.current->xorPtr = xorPointers(xorPointers(first.current->xorPtr, ::std::addressof(beforeHead)),
                                            ::std::addressof(afterTail));
        last.current->xorPtr = xorPointers(xorPointers(last.current->xorPtr, ::std::addressof(afterTail)),
                                           ::std::addressof(beforeHead));

        beforeHead.xorPtr = reinterpret_cast<PtrInteger>(last.current);
        afterTail.xorPtr = reinterpret_cast<PtrInteger>(first.current);
    }

    // WARNING! Iterators in the range [position, position + 1] will become invalid
    iterator erase(const_iterator position)
    {
        return erase(position, ::std::next(position));
    }

    // WARNING! Iterators in the range [first, last] will become invalid
    iterator erase(const_iterator first, const_iterator last)
    {
        if (first != last)
        {
            destroySequence(first, last, ::std::distance(first, last));
        }

        return { first.prev, last.current };
    }

    void resize(size_type count)
    {
        resizeImpl(count);
    }

    void resize(size_type count, const_reference val)
    {
        resizeImpl(count, val);
    }

    template <typename InputIterator>
    typename ::std::enable_if<::std::is_base_of<::std::input_iterator_tag,
                                                typename ::std::iterator_traits<InputIterator>::iterator_category>::value>::type
    assign(InputIterator first, InputIterator last)
    {
        for (auto iter = begin(); iter != end(); ++iter, ++first)
        {
            if (first == last)
            {
                (void)erase(iter, end());
                return;
            }

            *iter = *first;
        }

        for ( ; first != last; ++first)
        {
            emplace_back(*first);
        }
    }

    void assign(size_type count, const_reference val)
    {
        auto iter = begin();
        for ( ; (iter != end()) && (count > 0); ++iter, --count)
        {
           *iter = val;
        }

        if (iter != end())
        {
            erase(iter, end());
        }
        else
        {
            while (count > 0)
            {
                emplace_back(val);
                --count;
            }
        }
    }

    void assign(::std::initializer_list<T> il)
    {
        assign(il.begin(), il.end());
    }


    void splice(const_iterator position, xor_list &x)
    {
        if ((this == ::std::addressof(x)) || (x.empty()))
        {
            return;
        }

        const auto distance = x.size();
        const auto range = x.cutSequenceFromThis(x.cbegin(), x.cend(), distance).cutted;

        (void)insertSequenceToThisBefore(position, range.first, range.second, distance);
    }

    void splice(const_iterator position, xor_list &x, const_iterator i)
    {
        if ((this == ::std::addressof(x)) && ((position == i) || (position.prev == i.current)))
        {
            return;
        }

        const auto range = x.cutSequenceFromThis(i, ::std::next(i), 1).cutted;
        (void)insertNodeToThisBefore(position, static_cast<NodeWithValue*>(range.first.current));
    }

    void splice(const_iterator position, xor_list &x, const_iterator first, const_iterator last)
    {
        if (first == last)
        {
            return;
        }

        const size_type distance = (this == ::std::addressof(x)) ? size() : ::std::distance(first, last);

        ::std::tie(first, last) = x.cutSequenceFromThis(first, last, distance).cutted;
        (void)insertSequenceToThisBefore(position, first, last, distance);
    }

    // All iterators will become invalid
    void unique()
    {
        unique(::std::equal_to<T>{});
    }

    // All iterators will become invalid
    template <typename BinaryPredicate>
    void unique(BinaryPredicate isEqual)
    {
        if (size() < 2)
        {
            return;
        }

        auto current = cbegin();
        for (auto prev = current++; current != cend(); )
        {
            if (isEqual(*prev, *current))
            {
                current = erase(current);
            }
            else
            {
                prev = current++;
            }
        }
    }

    // All iterators from *this and x will become invalid
    void merge(xor_list &x)
    {
        merge(x, ::std::less<T>{});
    }

    // All iterators from *this and x will become invalid
    template <typename Compare>
    void merge(xor_list &x, Compare isLess)
    {
        if (!x.empty())
        {
            const auto distance = x.size();
            const auto range = x.cutSequenceFromThis(x.cbegin(), x.cend(), distance).cutted;

            (void)mergeSequencesToThis(cbegin(), cend(), range.first, range.second, ::std::move(isLess), distance);
        }
    }

private:
    template<bool c, typename TrueType, typename FalseType>
    using Cond = typename ::std::conditional<c, TrueType, FalseType>::type;

    struct NodeWithValue;


    static_assert((sizeof(Node*) == 1) || (sizeof(Node*) == 2)
                  || (sizeof(Node*) == 4) || (sizeof(Node*) == 8), "Invalid sizeof pointer");

    static_assert(sizeof(NodeWithValue*) == sizeof(Node*), "Invalid sizeof pointer");

    using PtrInteger = Cond<sizeof(Node*) == 1,
                            ::std::uint8_t,
                            Cond<sizeof(Node*) == 2,
                                 ::std::uint16_t,
                                 Cond<sizeof(Node*) == 4,
                                      ::std::uint32_t,
                                      ::std::uint64_t> > >;


    struct Node
    {
        PtrInteger xorPtr;


        explicit Node(Node *xorPtr = nullptr)
            : xorPtr(reinterpret_cast<PtrInteger>(xorPtr))
        {}

        Node(const Node&) noexcept = default;
        Node(Node &&) noexcept = default;

        virtual ~Node() = default;

        Node& operator=(const Node&) noexcept = default;
        Node& operator=(Node &&) noexcept = default;
    };

    struct NodeWithValue final : Node
    {
        T value;


        template<typename... Args>
        NodeWithValue(Args&&... args)
            : Node(), value(::std::forward<Args>(args)...)
        {
        }

        NodeWithValue(const NodeWithValue&) = delete;
        NodeWithValue(NodeWithValue &&) = delete;

        ~NodeWithValue() override = default;

        NodeWithValue& operator=(const NodeWithValue&) = delete;
        NodeWithValue& operator=(NodeWithValue &&) = delete;
    };


    #ifdef _MSC_VER
    using NodeAllocator = typename std::allocator_traits<TAllocator>::template rebind_alloc<NodeWithValue>;
    #else
    using NodeAllocator = typename ::std::allocator_traits<TAllocator>::template rebind_alloc<NodeWithValue>;
    #endif


    template<typename It, typename V>
    class IteratorBase
    {
    public:
        // =========================== Iterator Concept ===============================
        using difference_type = ::std::ptrdiff_t;
        using value_type = V;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = ::std::bidirectional_iterator_tag;


        reference operator*() const
        {
            return static_cast<NodeWithValue*>(current)->value;
        }

        It& operator++()
        {
            Node *next = reinterpret_cast<Node*>(xorPointers(prev, current->xorPtr));

            prev = current;
            current = next;

            return static_cast<It&>(*this);
        }
        // ======================== End Iterator Concept ==============================

        // ====================== Input/Forward Iterator Concept ======================
        bool operator==(const It &right) const noexcept
        {
            return (current == right.current);
        }

        bool operator!=(const It &right) const noexcept
        {
            return !(*this == right);
        }

        pointer operator->() const
        {
            return ::std::addressof(static_cast<NodeWithValue*>(current)->value);
        }

        It operator++(int)
        {
            It result(static_cast<It&>(*this));
            (void)++(*this);
            return result;
        }
        // ==================== End Input/Forward Iterator Concept ====================

        // ===================== Bidirectional Iterator Concept =======================
        It& operator--()
        {
            Node *newPrev = reinterpret_cast<Node*>(xorPointers(prev->xorPtr, current));

            current = prev;
            prev = newPrev;

            return static_cast<It&>(*this);
        }

        It operator--(int)
        {
            It result(static_cast<It&>(*this));
            (void)--(*this);
            return result;
        }
        // =================== End Bidirectional Iterator Concept =====================
    protected:
        Node *prev;
        Node *current;


        IteratorBase(Node *prev = nullptr, Node *current = nullptr) noexcept
            : prev(prev), current(current)
        {
        }

        IteratorBase(const IteratorBase&) noexcept = default;
        IteratorBase(IteratorBase&&) noexcept = default;

        ~IteratorBase() noexcept = default;

        IteratorBase& operator=(const IteratorBase&) noexcept = default;
        IteratorBase& operator=(IteratorBase&&) noexcept = default;

    private:
        friend class xor_list<T, TAllocator>;
    };


    struct CutResult final
    {
        ::std::pair<iterator, iterator> cutted;
        iterator end;

        CutResult(iterator first, iterator last, iterator end)
            : cutted(first, last), end(end)
        {}
    };


    NodeAllocator allocator;
    mutable Node beforeHead;
    mutable Node afterTail;
    size_type length = 0;


    static PtrInteger xorPointers(Node *const first, Node *const second) noexcept
    {
        return xorPointers(reinterpret_cast<const PtrInteger>(first),
                           reinterpret_cast<const PtrInteger>(second));
    }

    static PtrInteger xorPointers(const PtrInteger first, Node *const second) noexcept
    {
        return xorPointers(second, first);
    }

    static PtrInteger xorPointers(Node *const first, const PtrInteger second) noexcept
    {
        return xorPointers(reinterpret_cast<const PtrInteger>(first), second);
    }

    static PtrInteger xorPointers(const PtrInteger first, const PtrInteger second) noexcept
    {
        return first ^ second;
    }


    template<typename... Args>
    NodeWithValue* createNode(Args&&... args)
    {
        NodeWithValue *const result = ::std::allocator_traits<NodeAllocator>::allocate(allocator, 1);

        try
        {
            ::std::allocator_traits<NodeAllocator>::construct(allocator, result, ::std::forward<Args>(args)...);
        }
        catch (...)
        {
            ::std::allocator_traits<NodeAllocator>::deallocate(allocator, result, 1);
            throw;
        }

        return result;
    }


    ::std::pair<iterator, iterator>
    insertNodeToThisBefore(const_iterator position, NodeWithValue *const node) noexcept
    {
        ++length;
        return insertNodeBefore(position, node);
    }

    /*
     * All iterators equal to <position> will become invalid
     * Returns valid range [inserted, position]
     */
    static ::std::pair<iterator, iterator>
    insertNodeBefore(const_iterator position, NodeWithValue *const node) noexcept
    {
        node->xorPtr = xorPointers(position.prev, position.current);

        if (position.prev != nullptr)
        {
            position.prev->xorPtr = xorPointers(xorPointers(position.prev->xorPtr, position.current), node);
        }
        if (position.current != nullptr)
        {
            position.current->xorPtr = xorPointers(xorPointers(position.current->xorPtr, position.prev), node);
        }

        return { { position.prev, node }, { node, position.current } };
    }


    template<typename I>
    ::std::pair<iterator, iterator>
    insertSequenceToThisBefore(const_iterator position, const_iterator begin,
                               const_iterator end, I distance) noexcept
    {
        length += distance;
        return insertSequenceBefore(position, begin, end);
    }

    /*
     * All iterators equal to <position>, <begin> will become invalid
     * Iterators equal to end still remains valid (--end == result.second)
     * Returns valid range [begin, position]
     */
    static ::std::pair<iterator, iterator>
    insertSequenceBefore(const_iterator position, const_iterator begin,
                         const_iterator end) noexcept
    {
        begin.current->xorPtr = xorPointers(xorPointers(begin.current->xorPtr, begin.prev), position.prev);
        end.prev->xorPtr = xorPointers(xorPointers(end.prev->xorPtr, end.current), position.current);

        if (position.prev != nullptr)
        {
            position.prev->xorPtr = xorPointers(xorPointers(position.prev->xorPtr, position.current), begin.current);
        }
        if (position.current != nullptr)
        {
            position.current->xorPtr = xorPointers(xorPointers(position.current->xorPtr, position.prev), end.prev);
        }

        return { { position.prev, begin.current }, { end.prev, position.current } };
    }


    template<typename I>
    CutResult
    cutSequenceFromThis(const_iterator first, const_iterator last, I distance) noexcept
    {
        length -= distance;
        return cutSequence(first, last);
    }

    /*
     * Returns iterators to the first cutted and following the last cutted elements
     * Decrement result.first or increment result.second is UB
     * Dereference result.second is UB
     * Increment and dereference result.first are still valid
     * Decrement result.second returns is an iterator to the last cutted element
     * Iterators equal to begin, end will become invalid
     */
    static CutResult
    cutSequence(const_iterator begin, const_iterator end) noexcept
    {
        if (begin.prev != nullptr)
        {
            begin.prev->xorPtr = xorPointers(xorPointers(begin.prev->xorPtr, begin.current), end.current);
        }
        if (end.current != nullptr)
        {
            end.current->xorPtr = xorPointers(xorPointers(end.current->xorPtr, end.prev), begin.prev);
        }

        begin.current->xorPtr = xorPointers(xorPointers(begin.current->xorPtr, begin.prev), nullptr);
        end.prev->xorPtr = xorPointers(xorPointers(end.prev->xorPtr, end.current), nullptr);

        return { { nullptr, begin.current }, { end.prev, nullptr }, { begin.prev, end.current } };
    }


    template<typename I>
    void destroySequence(const_iterator begin, const_iterator end, I distance)
    {
        if (distance == 0)
        {
            return;
        }

        ::std::tie(begin, end) = cutSequenceFromThis(begin, end, distance).cutted; // noexcept!

        for (; begin != end; )
        {
            /*try
            {*/
                ::std::allocator_traits<NodeAllocator>::destroy(allocator, static_cast<NodeWithValue*>((++begin).prev));
            /*}
            catch (...)
            {}*/

            ::std::allocator_traits<NodeAllocator>::deallocate(allocator, static_cast<NodeWithValue*>(begin.prev), 1);
        }
    }


    void swapWithoutAllocators(xor_list &other)
    {
        const auto thisDistance = size();
        const auto otherDistance = other.size();

        if (!empty())
        {
            auto thisCutResult = cutSequenceFromThis(cbegin(), cend(), thisDistance);

            if (!other.empty())
            {
                auto otherCutResult = other.cutSequenceFromThis(other.cbegin(), other.cend(), otherDistance);
                (void)insertSequenceToThisBefore(cbegin(), otherCutResult.cutted.first, otherCutResult.cutted.second,
                                                 otherDistance);
            }

            (void)other.insertSequenceToThisBefore(other.cbegin(), thisCutResult.cutted.first,
                                                   thisCutResult.cutted.second, thisDistance);
        }
        else if (!other.empty())
        {
            auto otherCutResult = other.cutSequenceFromThis(other.cbegin(), other.cend(), otherDistance);
            (void)insertSequenceToThisBefore(cbegin(), otherCutResult.cutted.first, otherCutResult.cutted.second,
                                             otherDistance);
        }
    }

    template<typename Alloc = NodeAllocator>
    typename ::std::enable_if<::std::allocator_traits<Alloc>::propagate_on_container_swap::value>::type
    swapImpl(xor_list &other)
    {
        ::std::swap(allocator, other.allocator);
        swapWithoutAllocators(other);
    }

    template<typename Alloc = NodeAllocator>
    typename ::std::enable_if<!::std::allocator_traits<Alloc>::propagate_on_container_swap::value>::type
    swapImpl(xor_list &other)
    {
        swapWithoutAllocators(other);
    }


    template<typename Alloc = NodeAllocator>
    typename ::std::enable_if<::std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value>::type
    copyAssignmentImpl(const xor_list &right)
    {
        clear();
        allocator = right.allocator;
        assign(right.cbegin(), right.cend());
    }

    template<typename Alloc = NodeAllocator>
    typename ::std::enable_if<!::std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value>::type
    copyAssignmentImpl(const xor_list &right)
    {
        clear();
        assign(right.cbegin(), right.cend());
    }


    template<typename Alloc = NodeAllocator>
    typename ::std::enable_if<::std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value>::type
    moveAssignmentImpl(xor_list &&right)
    {
        clear();
        allocator = ::std::move(right.allocator);
        splice(cbegin(), right);
    }

    template<typename Alloc = NodeAllocator>
    typename ::std::enable_if<!::std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value>::type
    moveAssignmentImpl(xor_list &&right)
    {
        clear();

        if (allocator == right.allocator)
        {
            splice(cbegin(), right);
        }
        else
        {
            for (T &moved : right)
            {
                emplace_back(::std::move(moved));
            }

            right.clear();
        }
    }


    template<typename... Args>
    void resizeImpl(size_type count, Args&&... args)
    {
        if (count == 0)
        {
            clear();
            return;
        }

        while (size() > count)
        {
            pop_back();
        }

        while (size() < count)
        {
            emplace_back(::std::forward<Args>(args)...);
        }
    }


    template<typename LessCompare, typename I>
    ::std::pair<iterator, iterator>
    mergeSequencesToThis(const_iterator beginTo, const_iterator endTo,
                         const_iterator beginFrom, const_iterator endFrom,
                         LessCompare &&isLess, I distance) noexcept
    {
        length += distance;
        return mergeSequences(beginTo, endTo, beginFrom, endFrom, ::std::forward<LessCompare>(isLess));
    }

    /*
     * All iterators will become invalid
     * Return new range [first, second)
     */
    template<typename LessCompare>
    static ::std::pair<iterator, iterator>
    mergeSequences(const_iterator beginTo, const_iterator endTo,
                   const_iterator beginFrom, const_iterator endFrom,
                   LessCompare &&isLess) noexcept
    {
        const_iterator resultBegin = beginTo;

        while (beginFrom != endFrom)
        {
            if (beginTo == endTo)
            {
                if (resultBegin == beginTo)
                {
                    ::std::tie(resultBegin, endTo) = insertSequenceBefore(beginTo, beginFrom, endFrom);
                }
                else
                {
                    endTo = insertSequenceBefore(beginTo, beginFrom, endFrom).second;
                }

                break;
            }
            else if (::std::forward<LessCompare>(isLess)(*beginFrom, *beginTo))
            {
                auto cutResult = cutSequence(beginFrom, ::std::next(beginFrom));

                if (resultBegin == beginTo)
                {
                    ::std::tie(resultBegin, beginTo) = insertNodeBefore(beginTo,
                                                                        static_cast<NodeWithValue *>(cutResult.cutted.first.current));
                }
                else
                {
                    beginTo = insertNodeBefore(beginTo,
                                               static_cast<NodeWithValue *>(cutResult.cutted.first.current)).second;
                }

                beginFrom = cutResult.end;
            }
            else
            {
                ++beginTo;
            }
        }

        return { static_cast<iterator>(resultBegin), static_cast<iterator>(endTo) };
    }
};

#endif //XORLIST_XOR_LIST_H
