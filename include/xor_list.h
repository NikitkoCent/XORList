#ifndef XORLIST_XOR_LIST_H
#define XORLIST_XOR_LIST_H

#include <initializer_list> // ::std::initializer_list
#include <memory>           // ::std::allocator, ::std::allocator_traits, ::std::addressof
#include <utility>          // ::std::move, ::std::forward
#include <functional>       // ::std::less, ::std::equal_to
#include <iterator>         // ::std::bidirectional_iterator_tag, ::std::next
#include <type_traits>      // ::std::conditional
#include <cstdint>          // ::std::uint*_t
#include <cstddef>          // ::std::ptrdiff_t
#include <algorithm>        // ::std::for_each

template <typename T, class TAllocator = ::std::allocator<T>>
class LinkedList
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
        friend class LinkedList<T, TAllocator>;
        friend class const_iterator;


        iterator(Node *prev, Node *current) noexcept
            : IteratorBase<iterator, T>(prev, current)
        {
        }
    };

    class const_iterator : public IteratorBase<iterator, const T>
    {
    public:
        const_iterator() noexcept = default;
        const_iterator(const const_iterator&) noexcept = default;
        const_iterator(const_iterator&&) noexcept = default;

        ~const_iterator() noexcept = default;

        const_iterator& operator=(const const_iterator&) noexcept = default;
        const_iterator& operator=(const_iterator&&) noexcept = default;

    private:
        friend class LinkedList<T, TAllocator>;
        friend class iterator;


        const_iterator(Node *prev, Node *current) noexcept
            : IteratorBase<iterator, const T>(prev, current)
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


    LinkedList()
        : LinkedList(TAllocator())
    {}

    explicit LinkedList(const TAllocator &alloc);

    LinkedList(::std::initializer_list<T> il, const TAllocator &alloc);

    explicit LinkedList(size_type n, const TAllocator &alloc = TAllocator());

    LinkedList(size_type n, const_reference val, const TAllocator &alloc = TAllocator());

    LinkedList(const LinkedList &other);
    LinkedList(LinkedList &&other);

    virtual ~LinkedList()
    {
        clear();
    }

    LinkedList& operator=(const LinkedList &right)
    {
        if (this != ::std::addressof(right))
        {
            LinkedList(right).swap(*this);
        }
        return *this;
    }

    LinkedList& operator=(LinkedList &&right)
    {
        if (this != ::std::addressof(right))
        {
            LinkedList(::std::move(right)).swap(*this);
        }
        return *this;
    }

    void swap(LinkedList &other);

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
        emplaceBefore(cend(), createNode(::std::forward<Args>(args)...));
    }

    template <typename... Args>
    void emplace_front(Args&&... args)
    {
        emplaceBefore(cbegin(), createNode(::std::forward<Args>(args)...));
    }

    void pop_front();
    void pop_back();

    size_type size() const noexcept
    {
        return length;
    }

    bool empty() const noexcept
    {
        return (size() == 0);
    }

    void clear();

    T& back() noexcept;
    const T& back() const noexcept;

    T& front() noexcept;
    const T& front() const noexcept;

    // Iterators and such
    iterator begin() noexcept
    {
        return { &beforeHead, beforeHead.xorPtr };
    }

    iterator end() noexcept
    {
        return { afterTail.xorPtr, &afterTail };
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
        return { &beforeHead, beforeHead.xorPtr };
    }

    const_iterator cend() const noexcept
    {
        return { afterTail.xorPtr, &afterTail };
    }

    void sort() noexcept
    {
        sort(::std::less<T>{});
    }

    template <class Compare>
    void sort(Compare isLess) noexcept;

    iterator insert(const_iterator position, const_reference val)
    {
        //emplaceBefore noexcept!
        emplaceBefore(position, createNode(val));
        return static_cast<iterator>(--position);
    }

    // strong exception-safe guarantee
    template <class InputIterator>
    iterator insert(const_iterator position, InputIterator first, InputIterator last)
    {
        if (first == last)
        {
            return static_cast<iterator>(position);
        }

        iterator result = insert(position, *first);

        for ( ; ++first != last; )
        {
            try
            {
                (void)insert(position, *first);
            }
            catch(...)
            {
                destroySequence(result, position);
                throw;
            }
        }

        return result;
    }

    void reverse() noexcept;

    iterator erase(const_iterator position)
    {
        return erase(position, ::std::next(position));
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        if (first != last)
        {
            destroySequence(first, last);
        }

        return static_cast<iterator>(last);
    }

    void resize(size_type n);
    void resize(size_type n, const_reference val);

    template <typename InputIterator>
    void assign(InputIterator first, InputIterator last);

    void assign(size_type n, const_reference val);
    void assign(::std::initializer_list<T> il);

    void splice(const_iterator position, LinkedList &x) noexcept;
    void splice(const_iterator position, LinkedList &x, const_iterator i) noexcept;
    void splice(const_iterator position, LinkedList &x, const_iterator first, const_iterator last) noexcept;


    void unique()
    {
        unique(::std::equal_to<T>{});
    }

    template <typename BinaryPredicate>
    void unique(BinaryPredicate isEqual);


    void merge(LinkedList &x) noexcept
    {
        merge(x, ::std::less<T>{});
    }

    template <typename Compare>
    void merge(LinkedList &x, Compare comp) noexcept;


private:
    template<bool c, typename TrueType, typename FalseType>
    using Cond = typename ::std::conditional<c, TrueType, FalseType>::type;

    struct Node
    {
        Node *xorPtr;


        Node(Node *xorPtr = nullptr)
            : xorPtr(xorPtr)
        {}

        Node(const Node&) = delete;
        Node(Node &&) = delete;
        
        virtual ~Node() = default;

        Node& operator=(const Node&) = delete;
        Node& operator=(Node &&) = delete;
    };

    struct NodeWithValue : Node
    {
        T value;


        template<typename... Args>
        NodeWithValue(Args&&... args)
            : Node(), value(::std::forward<Args>(args)...)
        {
        }

        ~NodeWithValue() override = default;
    };

    static_assert((sizeof(Node*) == 1) || (sizeof(Node*) == 2)
                  || (sizeof(Node*) == 4) || (sizeof(Node*) == 8), "Invalid sizeof pointer");

    static_assert(sizeof(NodeWithValue*) >= sizeof(Node*), "Invalid sizeof pointer");

    using IntPtr = Cond<sizeof(Node*) == 1,
                        ::std::uint8_t,
                        Cond<sizeof(Node*) == 2,
                             ::std::uint16_t,
                             Cond<sizeof(Node*) == 4,
                                  ::std::uint32_t,
                                  ::std::uint64_t> > >;


    using NodeAllocator = typename ::std::allocator_traits<TAllocator>::template rebind_alloc<NodeWithValue>;


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
            Node *next = xorPointers(prev, current->xorPtr);

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
            Node *newPrev = xorPointers(prev->xorPtr, current);

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
        friend class LinkedList<T, TAllocator>;
    };


    NodeAllocator allocator;
    Node beforeHead;
    Node afterTail;
    size_type length = 0;



    static Node* xorPointers(Node *const first, Node *const second) noexcept
    {
        return static_cast<Node*>(static_cast<IntPtr>(first) ^ static_cast<IntPtr>(second));
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

    void emplaceBefore(const const_iterator &position, NodeWithValue *const newNode) noexcept
    {
        newNode->xorPtr = xorPointers(position.prev, position.current);
        position.prev->xorPtr = xorPointers(xorPointers(position.prev->xorPtr, position.current), newNode);
        position.current->xorPtr = xorPointers(xorPointers(position.current->xorPtr, position.prev), newNode);

        ++length;
    }


    static void cutSequence(const const_iterator &first, const const_iterator &last) noexcept
    {
        first.prev->xorPtr = xorPointers(xorPointers(first.prev->xorPtr, first.current), last.current);
        last.current->xorPtr = xorPointers(xorPointers(last.current->xorPtr, last.prev), first.prev);
    }

    void destroySequence(const_iterator first, const_iterator last)
    {
        cutSequence(first, last);   // noexcept!
        length -= ::std::distance(first, last);

        for ( ; first != last; )
        {
            /*try
            {*/
                ::std::allocator_traits<NodeAllocator>::destroy(allocator, (++first).prev);
            /*}
            catch (...)
            {}*/

            ::std::allocator_traits<NodeAllocator>::deallocate(allocator, first.prev, 1);
        }
    }
};

#endif //XORLIST_XOR_LIST_H
