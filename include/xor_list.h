#ifndef XORLIST_XOR_LIST_H
#define XORLIST_XOR_LIST_H

#include <initializer_list> // ::std::initializer_list
#include <memory>           // ::std::allocator, ::std::allocator_traits, ::std::addressof
#include <utility>          // ::std::move, ::std::forward
#include <functional>       // ::std::less, ::std::equal_to
#include <iterator>         // ::std::bidirectional_iterator_tag
#include <type_traits>      // ::std::conditional, ::std::is_const
#include <cstdint>          // ::std::uint*_t
#include <cstddef>          // ::std::ptrdiff_t

template <typename T, class TAllocator = ::std::allocator<T>>
class LinkedList
{
private:
    template<typename It, typename V>
    class IteratorBase;

public:
    class iterator : public IteratorBase<iterator, T>
    {
    public:
        iterator() noexcept = default;
        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;

        ~iterator() noexcept = default;

        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;

    private:
        friend class LinkedList;


        iterator(typename IteratorBase<iterator, T>::NodePtr prev,
                 typename IteratorBase<iterator, T>::NodePtr current) noexcept
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
        friend class LinkedList;


        const_iterator(typename IteratorBase<iterator, const T>::NodePtr prev,
                       typename IteratorBase<iterator, const T>::NodePtr current) noexcept
                : IteratorBase<iterator, const T>(prev, current)
        {
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
    void emplace_back(Args&&... data);

    template <typename... Args>
    void emplace_front(Args&&... data);

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
        return { nullptr, head };
    }

    iterator end() noexcept
    {
        return { tail, nullptr };
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
        return { nullptr, head };
    }

    const_iterator cend() const noexcept
    {
        return { tail, nullptr };
    }

    void sort() noexcept
    {
        sort(::std::less<T>{});
    }

    template <class Compare>
    void sort(Compare isLess) noexcept;

    iterator insert(const_iterator position, const_reference val);

    template <class InputIterator>
    iterator insert(const_iterator position, InputIterator first, InputIterator last);

    void reverse() noexcept;

    iterator erase(const_iterator position);
    iterator erase(const_iterator first, const_iterator last);

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
    struct Node
    {
        Node *xorPtr = nullptr;
        T value;


        template<typename... Args>
        Node(Args&&... args)
            : value(::std::forward<Args>(args)...)
        {}

        Node(const Node&) = delete;
        Node(Node &&) = delete;
        
        ~Node() = default;

        Node& operator=(const Node&) = delete;
        Node& operator=(Node &&) = delete;
    };

    using NodeAllocator = typename ::std::allocator_traits<TAllocator>::template rebind_alloc<Node>;

    template<typename It, typename V>
    class IteratorBase
    {
    private:
        template<bool c, typename TrueType, typename FalseType>
        using Cond = typename ::std::conditional<c, TrueType, FalseType>::type;

    public:
        // =========================== Iterator Concept ===============================
        using difference_type = ::std::ptrdiff_t;
        using value_type = V;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = ::std::bidirectional_iterator_tag;


        reference operator*() const
        {
            return current->value;
        }

        It& operator++()
        {
            NodePtr next = static_cast<NodePtr>(static_cast<IntPtr>(prev) ^ static_cast<IntPtr>(current->xorPtr));

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
            return ::std::addressof(current->value);
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
            NodePtr newPrev = static_cast<NodePtr>(static_cast<IntPtr>(prev->xorPtr) ^ static_cast<IntPtr>(current));

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
        using NodePtr = Cond<::std::is_const<value_type>::value,
                             const LinkedList::Node*, LinkedList::Node*>;

        static_assert((sizeof(NodePtr) == 1) || (sizeof(NodePtr) == 2)
                      || (sizeof(NodePtr) == 4) || (sizeof(NodePtr) == 8), "Invalid sizeof pointer");


        IteratorBase(NodePtr prev = nullptr, NodePtr current = nullptr) noexcept
                : prev(prev), current(current)
        {
        }

        IteratorBase(const IteratorBase&) noexcept = default;
        IteratorBase(IteratorBase&&) noexcept = default;

        ~IteratorBase() = default;

        IteratorBase& operator=(const IteratorBase&) noexcept = default;
        IteratorBase& operator=(IteratorBase&&) noexcept = default;

    private:
        friend class LinkedList;


        using IntPtr = Cond<sizeof(NodePtr) == 1,
                            ::std::uint8_t,
                            Cond<sizeof(NodePtr) == 2,
                                 ::std::uint16_t,
                                 Cond<sizeof(NodePtr) == 4,
                                      ::std::uint32_t,
                                      ::std::uint64_t> > >;


        NodePtr prev;
        NodePtr current;
    };


    NodeAllocator allocator;
    Node *head = nullptr;
    Node *tail = nullptr;
    size_type length = 0;
};


#endif //XORLIST_XOR_LIST_H
