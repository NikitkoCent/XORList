#ifndef XORLIST_XOR_LIST_H
#define XORLIST_XOR_LIST_H

#include <initializer_list> // ::std::initializer_list
#include <memory>           // ::std::allocator, ::std::allocator_traits, ::std::addressof
#include <utility>          // ::std::move, ::std::forward
#include <functional>       // ::std::less, ::std::equal_to
#include <algorithm>        // ::std::for_each

template <typename T, class TAllocator = ::std::allocator<T>>
class LinkedList
{
public:
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
    iterator begin() noexcept;
    iterator end() noexcept;

    const_iterator begin() const noexcept
    {
        return cbegin();
    }

    const_iterator end() const noexcept
    {
        return cend();
    }

    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;

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

    using NodeAllocator = typename ::std::allocator_traits<TAllocator>::rebind_alloc<Node>;


    NodeAllocator allocator;
    Node *head = nullptr;
    Node *tail = nullptr;
    size_type length = 0;
};


#endif //XORLIST_XOR_LIST_H
