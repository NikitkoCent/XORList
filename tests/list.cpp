#include <xor_list/xor_list.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <utility>


template<typename T>
struct Value
{
    T value;

    template<typename... Args>
    Value(Args&&... args)
        : value(std::forward<Args>(args)...)
    {
    }

    ~Value() { value = T{}; }


    operator T&() { return value; }
    operator const T&() const { return value; }
};

template<typename T>
struct NonCopyableValue : Value<T>
{
    template<typename... Args>
    NonCopyableValue(Args&&... args)
        : Value<T>(std::forward<Args>(args)...)
    {
    }

    NonCopyableValue(const NonCopyableValue &) = delete;
    NonCopyableValue& operator=(const NonCopyableValue&) = delete;

    NonCopyableValue(NonCopyableValue &&) = default;
    NonCopyableValue& operator=(NonCopyableValue&&) = default;
};

template<typename T>
struct NonMovableValue : NonCopyableValue<T>
{
    template<typename... Args>
    NonMovableValue(Args&&... args)
        : NonCopyableValue<T>(std::forward<Args>(args)...)
    {
    }

    NonMovableValue(NonMovableValue &&) = delete;
    NonMovableValue& operator=(NonMovableValue&&) = delete;
};


template<typename T>
struct ThrowsOnConstructValue : Value<T>
{
    template<typename... Args>
    ThrowsOnConstructValue(Args&&... args)
        : Value<T>(std::forward<Args>(args)...)
    {
        throw std::runtime_error("YEEEEAAH ROOOCCKKKKKKKKKKKKKKK");
    }

    ThrowsOnConstructValue(const Value<T> &value)
        : Value<T>(value)
    {}


    ThrowsOnConstructValue(const ThrowsOnConstructValue &) = default;
    ThrowsOnConstructValue(ThrowsOnConstructValue &&) = default;
};


template<typename T>
struct ThrowsOnCopyConstructValueCounted : Value<T>
{
    template<typename... Args>
    ThrowsOnCopyConstructValueCounted(Args&&... args)
        : Value<T>(std::forward<Args>(args)...)
    {
    }

    ThrowsOnCopyConstructValueCounted(const ThrowsOnCopyConstructValueCounted &src)
        : Value<T>(src)
    {
        if (--counter == 0)
        {
            throw std::runtime_error("YEEEEAAH ROOOCCKKKKKKKKKKKKKKK");
        }
    }

    static int counter;
};

template<typename T>
int ThrowsOnCopyConstructValueCounted<T>::counter = -1;


template<typename T>
struct RequiredSwapAllocator : std::allocator<T>
{
public:
    struct propagate_on_container_swap : std::true_type {};

    template<typename R>
    struct rebind
    {
        using other = RequiredSwapAllocator<R>;
    };


    using std::allocator<T>::allocator;
};

template<typename T>
struct RequiredCopyOnAssignmentAllocator : std::allocator<T>
{
public:
    struct propagate_on_container_copy_assignment : std::true_type {};

    template<typename R>
    struct rebind
    {
        using other = RequiredCopyOnAssignmentAllocator<R>;
    };


    using std::allocator<T>::allocator;
};

template<typename T>
struct NonRequiredMoveOnAssignmentAllocator1 : std::allocator<T>
{
public:
    struct propagate_on_container_move_assignment : std::false_type {};

    template<typename R>
    struct rebind
    {
        using other = NonRequiredMoveOnAssignmentAllocator1<R>;
    };


    using std::allocator<T>::allocator;
};

template<typename T>
struct NonRequiredMoveOnAssignmentAllocator2 : std::allocator<T>
{
public:
    struct propagate_on_container_move_assignment : std::false_type {};

    template<typename R>
    struct rebind
    {
        using other = NonRequiredMoveOnAssignmentAllocator2<R>;
    };


    using std::allocator<T>::allocator;


    bool operator==(const NonRequiredMoveOnAssignmentAllocator2 &) const noexcept
    {
        return false;
    }
};



TEST(LIST, CONSTRUCTOR_DEFAULT)
{
    LinkedList<NonMovableValue<int>> list;

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}


TEST(LIST, CONSTRUCTOR_ALLOCATOR)
{
    LinkedList<NonMovableValue<int>> list(std::allocator<NonMovableValue<int>>{});

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}


TEST(LIST, CONSTRUCTOR_INITIALIZER_LIST)
{
    LinkedList<Value<int>> list{-1, 0, 1, 2, 3, 4};

    ASSERT_EQ(list.size(), 6U);
    ASSERT_THAT(list, ::testing::ElementsAre(-1, 0, 1, 2, 3, 4));
}

TEST(LIST, CONSTRUCTOR_EMPTY_INITIALIZER_LIST)
{
    LinkedList<Value<int>> list({}, std::allocator<Value<int>>{});

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}


TEST(LIST, CONSTRUCTOR_DEFAULT_FILL)
{
    LinkedList<Value<int>> list(10U);

    ASSERT_EQ(list.size(), 10U);
    ASSERT_THAT(list, ::testing::ElementsAre(int{}, int{}, int{}, int{}, int{}, int{}, int{}, int{}, int{}, int{}));
}

TEST(LIST, CONSTRUCTOR_EMPTY_DEFAULT_FILL)
{
    LinkedList<Value<int>> list(0);

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}


TEST(LIST, CONSTRUCTOR_VALUE_FILL)
{
    LinkedList<Value<int>> list(10U, -1590);

    ASSERT_EQ(list.size(), 10U);
    ASSERT_THAT(list, ::testing::ElementsAre(-1590, -1590, -1590, -1590, -1590, -1590, -1590, -1590, -1590, -1590));
}

TEST(LIST, CONSTRUCTOR_EMPTY_VALUE_FILL)
{
    LinkedList<Value<int>> list(0U, -1590);

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}


TEST(LIST, COPY_CONSTRUCTOR_EMPTY)
{
    LinkedList<Value<int>> l1, l2(l1);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, COPY_CONSTRUCTOR_NON_EMPTY)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9}, l2(l1);

    ASSERT_EQ(l2.size(), 9U);
    ASSERT_THAT(l2, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));

    l1.~LinkedList();

    ASSERT_EQ(l2.size(), 9U);
    ASSERT_THAT(l2, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, MOVE_CONSTRUCTOR_EMPTY)
{
    LinkedList<Value<int>> l1, l2(std::move(l1));

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, MOVE_CONSTRUCTOR_NON_EMPTY)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9};
    LinkedList<Value<int>> l2(std::move(l1));

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 9U);
    ASSERT_THAT(l2, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}


TEST(LIST, COPY_ASSIGNMENT_SELF)
{
    LinkedList<Value<int>> list{1, 2, 3, 4, 5, 6, 7, 8, 9};

    list = list;

    ASSERT_EQ(list.size(), 9U);
    ASSERT_THAT(list, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, COPY_ASSIGNMENT_EMPTY)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9}, l2;

    l1 = l2;

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());
}

TEST(LIST, COPY_ASSIGNMENT_NON_EMPTY_LESS)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9}, l2{10, 20, 30, 40, 50, 60};

    l1 = l2;
    l2.clear();

    ASSERT_EQ(l1.size(), 6U);
    ASSERT_THAT(l1, ::testing::ElementsAre(10, 20, 30, 40, 50, 60));
}

TEST(LIST, COPY_ASSIGNMENT_NON_EMPTY_GREATER)
{
    LinkedList<Value<int>> l1{10, 20, 30, 40, 50, 60}, l2{1, 2, 3, 4, 5, 6, 7, 8, 9};

    l1 = l2;
    l2.clear();

    ASSERT_EQ(l1.size(), 9U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, COPY_ASSIGNMENT_SELF_SPECIAL_ALLOCATOR)
{
    LinkedList<Value<int>, RequiredCopyOnAssignmentAllocator<int>> list{1, 2, 3, 4, 5, 6, 7, 8, 9};

    list = list;

    ASSERT_EQ(list.size(), 9U);
    ASSERT_THAT(list, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, COPY_ASSIGNMENT_EMPTY_SPECIAL_ALLOCATOR)
{
    LinkedList<Value<int>, RequiredCopyOnAssignmentAllocator<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9}, l2;

    l1 = l2;

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());
}

TEST(LIST, COPY_ASSIGNMENT_NON_EMPTY_LESS_SPECIAL_ALLOCATOR)
{
    LinkedList<Value<int>, RequiredCopyOnAssignmentAllocator<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9}, l2{10, 20, 30, 40, 50, 60};

    l1 = l2;
    l2.clear();

    ASSERT_EQ(l1.size(), 6U);
    ASSERT_THAT(l1, ::testing::ElementsAre(10, 20, 30, 40, 50, 60));
}

TEST(LIST, COPY_ASSIGNMENT_NON_EMPTY_GREATER_SPECIAL_ALLOCATOR)
{
    LinkedList<Value<int>, RequiredCopyOnAssignmentAllocator<int>> l1{10, 20, 30, 40, 50, 60}, l2{1, 2, 3, 4, 5, 6, 7, 8, 9};

    l1 = l2;
    l2.clear();

    ASSERT_EQ(l1.size(), 9U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}


TEST(LIST, MOVE_ASSIGNMENT_SELF)
{
    LinkedList<Value<int>> list{1, 2, 3, 4, 5, 6, 7, 8, 9};

    list = std::move(list);

    ASSERT_EQ(list.size(), 9U);
    ASSERT_THAT(list, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, MOVE_ASSIGNMENT_EMPTY)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9}, l2;

    l1 = std::move(l2);

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());
}

TEST(LIST, MOVE_ASSIGNMENT_NON_EMPTY_LESS)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9}, l2{10, 20, 30, 40, 50, 60};

    l1 = std::move(l2);

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());

    ASSERT_EQ(l1.size(), 6U);
    ASSERT_THAT(l1, ::testing::ElementsAre(10, 20, 30, 40, 50, 60));
}

TEST(LIST, MOVE_ASSIGNMENT_NON_EMPTY_GREATER)
{
    LinkedList<Value<int>> l1{10, 20, 30, 40, 50, 60}, l2{1, 2, 3, 4, 5, 6, 7, 8, 9};

    l1 = std::move(l2);

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());

    ASSERT_EQ(l1.size(), 9U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, MOVE_ASSIGNMENT_SELF_SPECIAL_ALLOCATOR1)
{
    LinkedList<Value<int>, NonRequiredMoveOnAssignmentAllocator1<int>> list{1, 2, 3, 4, 5, 6, 7, 8, 9};

    list = std::move(list);

    ASSERT_EQ(list.size(), 9U);
    ASSERT_THAT(list, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, MOVE_ASSIGNMENT_EMPTY_SPECIAL_ALLOCATOR1)
{
    LinkedList<Value<int>, NonRequiredMoveOnAssignmentAllocator1<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9}, l2;

    l1 = std::move(l2);

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());
}

TEST(LIST, MOVE_ASSIGNMENT_NON_EMPTY_LESS_SPECIAL_ALLOCATOR1)
{
    LinkedList<Value<int>, NonRequiredMoveOnAssignmentAllocator1<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9}, l2{10, 20, 30, 40, 50, 60};

    l1 = std::move(l2);

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());

    ASSERT_EQ(l1.size(), 6U);
    ASSERT_THAT(l1, ::testing::ElementsAre(10, 20, 30, 40, 50, 60));
}

TEST(LIST, MOVE_ASSIGNMENT_NON_EMPTY_GREATER_SPECIAL_ALLOCATOR1)
{
    LinkedList<Value<int>, NonRequiredMoveOnAssignmentAllocator1<int>> l1{10, 20, 30, 40, 50, 60}, l2{1, 2, 3, 4, 5, 6, 7, 8, 9};

    l1 = std::move(l2);

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());

    ASSERT_EQ(l1.size(), 9U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, MOVE_ASSIGNMENT_SELF_SPECIAL_ALLOCATOR2)
{
    LinkedList<Value<int>, NonRequiredMoveOnAssignmentAllocator2<int>> list{1, 2, 3, 4, 5, 6, 7, 8, 9};

    list = std::move(list);

    ASSERT_EQ(list.size(), 9U);
    ASSERT_THAT(list, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, MOVE_ASSIGNMENT_EMPTY_SPECIAL_ALLOCATOR2)
{
    LinkedList<Value<int>, NonRequiredMoveOnAssignmentAllocator2<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9}, l2;

    l1 = std::move(l2);

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());
}

TEST(LIST, MOVE_ASSIGNMENT_NON_EMPTY_LESS_SPECIAL_ALLOCATOR2)
{
    LinkedList<Value<int>, NonRequiredMoveOnAssignmentAllocator2<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9}, l2{10, 20, 30, 40, 50, 60};

    l1 = std::move(l2);

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());

    ASSERT_EQ(l1.size(), 6U);
    ASSERT_THAT(l1, ::testing::ElementsAre(10, 20, 30, 40, 50, 60));
}

TEST(LIST, MOVE_ASSIGNMENT_NON_EMPTY_GREATER_SPECIAL_ALLOCATOR2)
{
    LinkedList<Value<int>, NonRequiredMoveOnAssignmentAllocator2<int>> l1{10, 20, 30, 40, 50, 60}, l2{1, 2, 3, 4, 5, 6, 7, 8, 9};

    l1 = std::move(l2);

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());

    ASSERT_EQ(l1.size(), 9U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}


TEST(LIST, MERGE_BOTH_EMPTY)
{
    LinkedList<NonMovableValue<int>> l1, l2;

    l2.merge(l1);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, MERGE_SINGLE_TO_EMPTY)
{
    LinkedList<Value<int>> l1{-10}, l2;

    l2.merge(l1);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 1U);
    ASSERT_THAT(l2, ::testing::ElementsAre(-10));
}

TEST(LIST, MERGE_EMPTY_TO_SINGLE)
{
    LinkedList<Value<int>> l1, l2{-10};

    l2.merge(l1);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 1U);
    ASSERT_THAT(l2, ::testing::ElementsAre(-10));
}

TEST(LIST, MERGE_SINGLES_WITHOUT_SWAP)
{
    LinkedList<Value<int>> l1{10}, l2{-10};

    l2.merge(l1);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 2U);
    ASSERT_THAT(l2, ::testing::ElementsAre(-10, 10));
}

TEST(LIST, MERGE_SINGLES_WITH_SWAP)
{
    LinkedList<Value<int>> l1{-10}, l2{10};

    l2.merge(l1);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 2U);
    ASSERT_THAT(l2, ::testing::ElementsAre(-10, 10));
}

TEST(LIST, MERGE_SAME_SINGLES)
{
    LinkedList<Value<int>> l1{10}, l2{10};

    l2.merge(l1);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 2U);
    ASSERT_THAT(l2, ::testing::ElementsAre(10, 10));
}

TEST(LIST, MERGE_WITHOUT_SWAP)
{
    LinkedList<Value<int>> l1{5, 6, 7, 8}, l2{1, 2, 3, 4};

    l2.merge(l1);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 8U);
    ASSERT_THAT(l2, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8));
}

TEST(LIST, MERGE_WITH_SWAP)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4}, l2{5, 6, 7, 8};

    l2.merge(l1);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 8U);
    ASSERT_THAT(l2, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8));
}

TEST(LIST, MERGE_GENERIC1)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4}, l2{0, 2, 4, 8};

    l2.merge(l1);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 8U);
    ASSERT_THAT(l2, ::testing::ElementsAre(0, 1, 2, 2, 3, 4, 4, 8));
}

TEST(LIST, MERGE_GENERIC2)
{
    LinkedList<Value<int>> l1{0, 2, 4, 8}, l2{1, 2, 3, 4};

    l2.merge(l1);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 8U);
    ASSERT_THAT(l2, ::testing::ElementsAre(0, 1, 2, 2, 3, 4, 4, 8));
}


TEST(LIST, SORT_EMPTY)
{
    LinkedList<Value<int>> list;

    list.sort();

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, SORT_SINGLE)
{
    LinkedList<Value<int>> list{10};

    list.sort();

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(10));
}

TEST(LIST, SORT_NO_TWOS)
{
    LinkedList<Value<int>> list{-10, 10};

    list.sort();

    ASSERT_EQ(list.size(), 2U);
    ASSERT_THAT(list, ::testing::ElementsAre(-10, 10));
}

TEST(LIST, SORT_TWOS)
{
    LinkedList<Value<int>> list{10, -10};

    list.sort();

    ASSERT_EQ(list.size(), 2U);
    ASSERT_THAT(list, ::testing::ElementsAre(-10, 10));
}

TEST(LIST, SORT_EQUAL_TWOS)
{
    LinkedList<Value<int>> list{10, 10};

    list.sort();

    ASSERT_EQ(list.size(), 2U);
    ASSERT_THAT(list, ::testing::ElementsAre(10, 10));
}

TEST(LIST, SORT_SORTED)
{
    LinkedList<Value<int>> list{1, 2, 3, 4, 5, 6, 7, 8, 9};

    list.sort();

    ASSERT_EQ(list.size(), 9U);
    ASSERT_THAT(list, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, SORT_SORTED_REVERSE)
{
    LinkedList<Value<int>> list{9, 8, 7, 6, 5, 4, 3, 2, 1};

    list.sort();

    ASSERT_EQ(list.size(), 9U);
    ASSERT_THAT(list, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, SORT_GENERIC1)
{
    LinkedList<Value<int>> list{1, -2, 5, 65, 3, 42, 67, 35, 7, -10};

    list.sort();

    ASSERT_EQ(list.size(), 10U);
    ASSERT_THAT(list, ::testing::ElementsAre(-10, -2, 1, 3, 5, 7, 35, 42, 65, 67));
}

TEST(LIST, SORT_GENERIC2)
{
    LinkedList<Value<int>> list{-10, 7, 35, 67, 42, 3, 65, 5, -2, 1};

    list.sort();

    ASSERT_EQ(list.size(), 10U);
    ASSERT_THAT(list, ::testing::ElementsAre(-10, -2, 1, 3, 5, 7, 35, 42, 65, 67));
}

TEST(LIST, SORT_GENERIC3)
{
    LinkedList<Value<int>> list{1, -2, 65, 3, 67, 35, -10};

    list.sort();

    ASSERT_EQ(list.size(), 7U);
    ASSERT_THAT(list, ::testing::ElementsAre(-10, -2, 1, 3, 35, 65, 67));
}

TEST(LIST, SORT_GENERIC4)
{
    LinkedList<Value<int>> list{-10, 35, 67, 3, 65, -2, 1};

    list.sort();

    ASSERT_EQ(list.size(), 7U);
    ASSERT_THAT(list, ::testing::ElementsAre(-10, -2, 1, 3, 35, 65, 67));
}


TEST(LIST, SWAP_EMPTY)
{
    LinkedList<Value<int>> l1, l2;

    l1.swap(l2);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SWAP_EMPTY_SINGLE)
{
    LinkedList<Value<int>> l1, l2{-123};

    l1.swap(l2);

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(-123));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SWAP_SINGLE_EMPTY)
{
    LinkedList<Value<int>> l1{-123}, l2;

    l1.swap(l2);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 1U);
    ASSERT_THAT(l2, ::testing::ElementsAre(-123));
}

TEST(LIST, SWAP_EMPTY_GENERIC)
{
    LinkedList<Value<int>> l1, l2{1, -2, 5, 65, 3, 42, 67, 35, 7, -10};

    l1.swap(l2);

    ASSERT_EQ(l1.size(), 10U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, -2, 5, 65, 3, 42, 67, 35, 7, -10));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SWAP_GENERIC_EMPTY)
{
    LinkedList<Value<int>> l1{1, -2, 5, 65, 3, 42, 67, 35, 7, -10}, l2;

    l1.swap(l2);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 10U);
    ASSERT_THAT(l2, ::testing::ElementsAre(1, -2, 5, 65, 3, 42, 67, 35, 7, -10));
}

TEST(LIST, SWAP_GENERICS1)
{
    LinkedList<Value<int>> l1{1, -2, 5, 65, 3, 42, 67, 35, 7, -10}, l2{298034, 78, 5490, 548};

    l1.swap(l2);

    ASSERT_EQ(l1.size(), 4U);
    ASSERT_THAT(l1, ::testing::ElementsAre(298034, 78, 5490, 548));

    ASSERT_EQ(l2.size(), 10U);
    ASSERT_THAT(l2, ::testing::ElementsAre(1, -2, 5, 65, 3, 42, 67, 35, 7, -10));
}

TEST(LIST, SWAP_GENERICS2)
{
    LinkedList<Value<int>> l1{1, -2, 5, 65, 3, 42, 67, 35, 7, -10}, l2{298034, 78, 5490, 548};

    l2.swap(l1);

    ASSERT_EQ(l1.size(), 4U);
    ASSERT_THAT(l1, ::testing::ElementsAre(298034, 78, 5490, 548));

    ASSERT_EQ(l2.size(), 10U);
    ASSERT_THAT(l2, ::testing::ElementsAre(1, -2, 5, 65, 3, 42, 67, 35, 7, -10));
}

TEST(LIST, SWAP_EMPTY_SPECIAL_ALLOCATOR)
{
    LinkedList<Value<int>, RequiredSwapAllocator<int>> l1, l2;

    l1.swap(l2);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SWAP_EMPTY_SINGLE_SPECIAL_ALLOCATOR)
{
    LinkedList<Value<int>, RequiredSwapAllocator<int>> l1, l2{-123};

    l1.swap(l2);

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(-123));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SWAP_SINGLE_EMPTY_SPECIAL_ALLOCATOR)
{
    LinkedList<Value<int>, RequiredSwapAllocator<int>> l1{-123}, l2;

    l1.swap(l2);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 1U);
    ASSERT_THAT(l2, ::testing::ElementsAre(-123));
}

TEST(LIST, SWAP_EMPTY_GENERIC_SPECIAL_ALLOCATOR)
{
    LinkedList<Value<int>, RequiredSwapAllocator<int>> l1, l2{1, -2, 5, 65, 3, 42, 67, 35, 7, -10};

    l1.swap(l2);

    ASSERT_EQ(l1.size(), 10U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, -2, 5, 65, 3, 42, 67, 35, 7, -10));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SWAP_GENERIC_EMPTY_SPECIAL_ALLOCATOR)
{
    LinkedList<Value<int>, RequiredSwapAllocator<int>> l1{1, -2, 5, 65, 3, 42, 67, 35, 7, -10}, l2;

    l1.swap(l2);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_EQ(l2.size(), 10U);
    ASSERT_THAT(l2, ::testing::ElementsAre(1, -2, 5, 65, 3, 42, 67, 35, 7, -10));
}

TEST(LIST, SWAP_GENERICS1_SPECIAL_ALLOCATOR)
{
    LinkedList<Value<int>, RequiredSwapAllocator<int>> l1{1, -2, 5, 65, 3, 42, 67, 35, 7, -10}, l2{298034, 78, 5490, 548};

    l1.swap(l2);

    ASSERT_EQ(l1.size(), 4U);
    ASSERT_THAT(l1, ::testing::ElementsAre(298034, 78, 5490, 548));

    ASSERT_EQ(l2.size(), 10U);
    ASSERT_THAT(l2, ::testing::ElementsAre(1, -2, 5, 65, 3, 42, 67, 35, 7, -10));
}

TEST(LIST, SWAP_GENERICS2_SPECIAL_ALLOCATOR)
{
    LinkedList<Value<int>, RequiredSwapAllocator<int>> l1{1, -2, 5, 65, 3, 42, 67, 35, 7, -10}, l2{298034, 78, 5490, 548};

    l2.swap(l1);

    ASSERT_EQ(l1.size(), 4U);
    ASSERT_THAT(l1, ::testing::ElementsAre(298034, 78, 5490, 548));

    ASSERT_EQ(l2.size(), 10U);
    ASSERT_THAT(l2, ::testing::ElementsAre(1, -2, 5, 65, 3, 42, 67, 35, 7, -10));
}


TEST(LIST, UNIQUE_EMPTY)
{
    LinkedList<Value<int>> list;

    list.unique();

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, UNIQUE_SINGLE)
{
    LinkedList<Value<int>> list{-30};

    list.unique();

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(-30));
}

TEST(LIST, UNIQUE_TWOS_SAME)
{
    LinkedList<Value<int>> list{-30, -30};

    list.unique();

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(-30));
}

TEST(LIST, UNIQUE_TWOS_NOTSAME)
{
    LinkedList<Value<int>> list{-30, 5};

    list.unique();

    ASSERT_EQ(list.size(), 2U);
    ASSERT_THAT(list, ::testing::ElementsAre(-30, 5));
}

TEST(LIST, UNIQUE_GENERIC1)
{
    LinkedList<Value<int>> list{-1, -1, 2, -1, 2, 2, -1, -1, -1};

    list.unique();

    ASSERT_EQ(list.size(), 5U);
    ASSERT_THAT(list, ::testing::ElementsAre(-1, 2, -1, 2, -1));
}

TEST(LIST, UNIQUE_GENERIC2)
{
    LinkedList<Value<int>> list{100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100};

    list.unique();

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(100));
}


TEST(LIST, PUSH1)
{
    LinkedList<Value<int>> list;

    list.push_back(0);
    list.push_front(-1);
    list.push_back(1);
    list.push_back(2);

    ASSERT_EQ(list.size(), 4U);
    ASSERT_THAT(list, ::testing::ElementsAre(-1, 0, 1, 2));
}

TEST(LIST, PUSH2)
{
    LinkedList<Value<int>> list;

    list.push_front(0);
    list.push_back(-1);
    list.push_front(1);
    list.push_front(2);

    ASSERT_EQ(list.size(), 4U);
    ASSERT_THAT(list, ::testing::ElementsAre(2, 1, 0, -1));
}

TEST(LIST, PUSH_CREF_1)
{
    LinkedList<Value<int>> list;
    const Value<int> values[4] = {0, -1, 1, 2};

    list.push_back(values[0]);
    list.push_front(values[1]);
    list.push_back(values[2]);
    list.push_back(values[3]);

    ASSERT_EQ(list.size(), 4U);
    ASSERT_THAT(list, ::testing::ElementsAre(-1, 0, 1, 2));
}

TEST(LIST, PUSH_CREF_2)
{
    LinkedList<Value<int>> list;
    const Value<int> values[4] = {0, -1, 1, 2};

    list.push_front(values[0]);
    list.push_back(values[1]);
    list.push_front(values[2]);
    list.push_front(values[3]);

    ASSERT_EQ(list.size(), 4U);
    ASSERT_THAT(list, ::testing::ElementsAre(2, 1, 0, -1));
}


TEST(LIST, EMPLACE1)
{
    LinkedList<Value<int>> list;

    list.emplace_back(0);
    list.emplace_front(-1);
    list.emplace_back(1);
    list.emplace_back(2);

    ASSERT_EQ(list.size(), 4U);
    ASSERT_THAT(list, ::testing::ElementsAre(-1, 0, 1, 2));
}

TEST(LIST, EMPLACE2)
{
    LinkedList<Value<int>> list;

    list.emplace_front(0);
    list.emplace_back(-1);
    list.emplace_front(1);
    list.emplace_front(2);

    ASSERT_EQ(list.size(), 4U);
    ASSERT_THAT(list, ::testing::ElementsAre(2, 1, 0, -1));
}

TEST(LIST, EMPLACE_EXCEPTION1)
{
    LinkedList<ThrowsOnConstructValue<int>> list;
    const Value<int> value(-1);

    list.emplace_back(value);
    ASSERT_ANY_THROW(list.emplace_front(0));

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(value));
}

TEST(LIST, EMPLACE_EXCEPTION2)
{
    LinkedList<ThrowsOnConstructValue<int>> list;

    ASSERT_ANY_THROW(list.emplace_back(-1));

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}


TEST(LIST, POP_BACK_SINGLE)
{
    LinkedList<Value<int>> list{ 5 };

    list.pop_back();

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, POP_FRONT_SINGLE)
{
    LinkedList<Value<int>> list{5};

    list.pop_front();

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, POPS1)
{
    LinkedList<Value<int>> list{ 5, 10, 15 };

    list.pop_back();
    ASSERT_EQ(list.size(), 2U);
    ASSERT_THAT(list, ::testing::ElementsAre(5, 10));

    list.pop_back();
    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(5));

    list.pop_front();
    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, POPS2)
{
    LinkedList<Value<int>> list{5, 10, 15};

    list.pop_front();
    ASSERT_EQ(list.size(), 2U);
    ASSERT_THAT(list, ::testing::ElementsAre(10, 15));

    list.pop_front();
    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(15));

    list.pop_back();
    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}


TEST(LIST, REVERSE_EMPTY)
{
    LinkedList<Value<int>> list;

    list.reverse();

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, REVERSE_SINGLE)
{
    LinkedList<Value<int>> list{500};

    list.reverse();

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(500));
}

TEST(LIST, REVERSE_GENERIC1)
{
    LinkedList<Value<int>> list{1, 2, 3, 4, 5, 6, 7, 8, 9};

    list.reverse();

    ASSERT_EQ(list.size(), 9U);
    ASSERT_THAT(list, ::testing::ElementsAre(9, 8, 7, 6, 5, 4, 3, 2, 1));
}

TEST(LIST, REVERSE_GENERIC2)
{
    LinkedList<Value<int>> list{1, 2};

    list.reverse();

    ASSERT_EQ(list.size(), 2U);
    ASSERT_THAT(list, ::testing::ElementsAre(2, 1));
}


TEST(LIST, BACK_SINGLE)
{
    LinkedList<Value<int>> list{ 50 };
    const auto &listRef = list;

    ASSERT_EQ(list.back(), 50);
    ASSERT_EQ(listRef.back(), 50);
}

TEST(LIST, BACK_GENERIC)
{
    LinkedList<Value<int>> list{1, 2, 3, 4, 5, 6, 7, 8, 9};
    const auto &listRef = list;

    ASSERT_EQ(list.back(), 9);
    ASSERT_EQ(listRef.back(), 9);
}


TEST(LIST, FRONT_SINGLE)
{
    LinkedList<Value<int>> list{50};
    const auto &listRef = list;

    ASSERT_EQ(list.front(), 50);
    ASSERT_EQ(listRef.front(), 50);
}

TEST(LIST, FRONT_GENERIC)
{
    LinkedList<Value<int>> list{1, 2, 3, 4, 5, 6, 7, 8, 9};
    const auto &listRef = list;

    ASSERT_EQ(list.front(), 1);
    ASSERT_EQ(listRef.front(), 1);
}


TEST(LIST, ASSIGN_COUNT_VAL_EMPTY_0)
{
    LinkedList<Value<int>> list;

    list.assign(0, -135);

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, ASSIGN_COUNT_VAL_EMPTY)
{
    LinkedList<Value<int>> list;

    list.assign(10, -135);

    ASSERT_EQ(list.size(), 10U);
    ASSERT_THAT(list, ::testing::ElementsAre(-135, -135, -135, -135, -135, -135, -135, -135, -135, -135));
}

TEST(LIST, ASSIGN_COUNT_VAL_TRUNCATE_TO_EMPTY_SINGLE)
{
    LinkedList<Value<int>> list{ -45672 };

    list.assign(0, -135);

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, ASSIGN_COUNT_VAL_TRUNCATE_TO_EMPTY_SEVERAL)
{
    LinkedList<Value<int>> list{-45672, 234, -2353, 1, 21};

    list.assign(0, -135);

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, ASSIGN_COUNT_VAL_TRUNCATE_TO_SINGLE_SEVERAL)
{
    LinkedList<Value<int>> list{-45672, 234, -2353, 1, 21};

    list.assign(1, -135);

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(-135));
}

TEST(LIST, ASSIGN_COUNT_VAL_TRUNCATE_SEVERAL)
{
    LinkedList<Value<int>> list{-45672, 234, -2353, 1, 21};

    list.assign(3, 200);

    ASSERT_EQ(list.size(), 3U);
    ASSERT_THAT(list, ::testing::ElementsAre(200, 200, 200));
}

TEST(LIST, ASSIGN_COUNT_VAL_APPEND_SINGLE_FROM_EMPTY)
{
    LinkedList<Value<int>> list;

    list.assign(1, 2018);

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(2018));
}

TEST(LIST, ASSIGN_COUNT_VAL_APPEND_SEVERAL_FROM_EMPTY)
{
    LinkedList<Value<int>> list;

    list.assign(5, 2018);

    ASSERT_EQ(list.size(), 5U);
    ASSERT_THAT(list, ::testing::ElementsAre(2018, 2018, 2018, 2018, 2018));
}

TEST(LIST, ASSIGN_COUNT_VAL_APPEND_SINGLE)
{
    LinkedList<Value<int>> list{-100, 0, 100, 200, 300};

    list.assign(6, 2018);

    ASSERT_EQ(list.size(), 6U);
    ASSERT_THAT(list, ::testing::ElementsAre(2018, 2018, 2018, 2018, 2018, 2018));
}

TEST(LIST, ASSIGN_COUNT_VAL_APPEND_SEVERAL)
{
    LinkedList<Value<int>> list{-100, 0, 100, 200, 300};;

    list.assign(10, 98);

    ASSERT_EQ(list.size(), 10U);
    ASSERT_THAT(list, ::testing::ElementsAre(98, 98, 98, 98, 98, 98, 98, 98, 98, 98));
}

TEST(LIST, ASSIGN_COUNT_VAL_WITHOUT_RESIZE)
{
    LinkedList<Value<int>> list{-100, 0, 100, 200, 300};

    list.assign(5, 98);

    ASSERT_EQ(list.size(), 5U);
    ASSERT_THAT(list, ::testing::ElementsAre(98, 98, 98, 98, 98));
}

TEST(LIST, ASSIGN_RANGE_EMPTY_0)
{
    LinkedList<Value<int>> list;

    list.assign((Value<int>*)nullptr, (Value<int>*)nullptr);

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, ASSIGN_RANGE_EMPTY)
{
    LinkedList<Value<int>> list;

    {
        Value<int> range[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        list.assign(&range[0], ((Value<int>*)range) + 9);
    }

    ASSERT_EQ(list.size(), 9U);
    ASSERT_THAT(list, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, ASSIGN_RANGE_TRUNCATE_TO_EMPTY_SINGLE)
{
    LinkedList<Value<int>> list{-45672};

    list.assign((Value<int>*)nullptr, (Value<int>*)nullptr);

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, ASSIGN_RANGE_TRUNCATE_TO_EMPTY_SEVERAL)
{
    LinkedList<Value<int>> list{-45672, 234, -2353, 1, 21};

    list.assign((Value<int>*)nullptr, (Value<int>*)nullptr);

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, ASSIGN_RANGE_TRUNCATE_TO_SINGLE_SEVERAL)
{
    LinkedList<Value<int>> list{-45672, 234, -2353, 1, 21};

    {
        Value<int> range[1] = {-135};
        list.assign(&range[0], ((Value<int>*)range) + 1);
    }

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(-135));
}

TEST(LIST, ASSIGN_RANGE_TRUNCATE_SEVERAL)
{
    LinkedList<Value<int>> list{-45672, 234, -2353, 1, 21};

    {
        Value<int> range[3] = {-5, 25, 8};
        list.assign(&range[0], ((Value<int>*)range) + 3);
    }

    ASSERT_EQ(list.size(), 3U);
    ASSERT_THAT(list, ::testing::ElementsAre(-5, 25, 8));
}

TEST(LIST, ASSIGN_RANGE_APPEND_SINGLE_FROM_EMPTY)
{
    LinkedList<Value<int>> list;

    {
        Value<int> range[1] = {2018};
        list.assign(&range[0], ((Value<int>*)range) + 1);
    }

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(2018));
}

TEST(LIST, ASSIGN_RANGE_APPEND_SEVERAL_FROM_EMPTY)
{
    LinkedList<Value<int>> list;

    {
        Value<int> range[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        list.assign(&range[0], ((Value<int>*)range) + 9);
    }

    ASSERT_EQ(list.size(), 9U);
    ASSERT_THAT(list, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, ASSIGN_RANGE_APPEND_SINGLE)
{
    LinkedList<Value<int>> list{-100, 0, 100, 200, 300};

    {
        Value<int> range[9] = {1, 2, 3, 4, 5, 6};
        list.assign(&range[0], ((Value<int>*)range) + 6);
    }

    ASSERT_EQ(list.size(), 6U);
    ASSERT_THAT(list, ::testing::ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(LIST, ASSIGN_RANGE_APPEND_SEVERAL)
{
    LinkedList<Value<int>> list{-100, 0, 100, 200, 300};;

    {
        Value<int> range[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        list.assign(&range[0], ((Value<int>*)range) + 9);
    }

    ASSERT_EQ(list.size(), 9U);
    ASSERT_THAT(list, ::testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(LIST, ASSIGN_RANGE_WITHOUT_RESIZE)
{
    LinkedList<Value<int>> list{-100, 0, 100, 200, 300};

    {
        Value<int> range[9] = {1, 2, 3, 4, 5};
        list.assign(&range[0], ((Value<int>*)range) + 5);
    }

    ASSERT_EQ(list.size(), 5U);
    ASSERT_THAT(list, ::testing::ElementsAre(1, 2, 3, 4, 5));
}


TEST(LIST, RESIZE_EMPTY_TO_EMPTY)
{
    LinkedList<Value<int>> list;

    list.resize(0, 50);

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, RESIZE_EMPTY_TO_SINGLE)
{
    LinkedList<Value<int>> list;

    list.resize(1, 50);

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(50));
}

TEST(LIST, RESIZE_EMPTY_TO_SEVERAL)
{
    LinkedList<Value<int>> list;

    list.resize(5, 50);

    ASSERT_EQ(list.size(), 5U);
    ASSERT_THAT(list, ::testing::ElementsAre(50, 50, 50, 50, 50));
}

TEST(LIST, RESIZE_SINGLE_TO_EMPTY)
{
    LinkedList<Value<int>> list{1266};

    list.resize(0, 50);

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, RESIZE_SINGLE_TO_SINGLE)
{
    LinkedList<Value<int>> list{1266};

    list.resize(1, 50);

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(1266));
}

TEST(LIST, RESIZE_SINGLE_TO_SEVERAL)
{
    LinkedList<Value<int>> list{1266};

    list.resize(5, 50);

    ASSERT_EQ(list.size(), 5U);
    ASSERT_THAT(list, ::testing::ElementsAre(1266, 50, 50, 50, 50));
}

TEST(LIST, RESIZE_SEVERAL_TO_EMPTY)
{
    LinkedList<Value<int>> list{123, 56, 102, -12111};

    list.resize(0, 50);

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, RESIZE_SEVERAL_TO_SINGLE)
{
    LinkedList<Value<int>> list{123, 56, 102, -12111};

    list.resize(1, 50);

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(123));
}

TEST(LIST, RESIZE_SEVERAL_TO_SEVERAL_TRUNCATE)
{
    LinkedList<Value<int>> list{123, 56, 102, -12111};

    list.resize(2, 50);

    ASSERT_EQ(list.size(), 2U);
    ASSERT_THAT(list, ::testing::ElementsAre(123, 56));
}

TEST(LIST, RESIZE_SEVERAL_TO_SEVERAL_APPEND)
{
    LinkedList<Value<int>> list{123, 56, 102, -12111};

    list.resize(10, 50);

    ASSERT_EQ(list.size(), 10U);
    ASSERT_THAT(list, ::testing::ElementsAre(123, 56, 102, -12111, 50, 50, 50, 50, 50, 50));
}

TEST(LIST, RESIZE_SEVERAL_TO_SEVERAL_WITHOUT_RESIZE)
{
    LinkedList<Value<int>> list{123, 56, 102, -12111};

    list.resize(4, 50);

    ASSERT_EQ(list.size(), 4U);
    ASSERT_THAT(list, ::testing::ElementsAre(123, 56, 102, -12111));
}


TEST(LIST, SPLICE_POSITION_OTHER_EMPTY_TO_EMPTY)
{
    LinkedList<Value<int>> l1, l2;

    l1.splice(l1.cbegin(), l2);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());

    l1.splice(l1.cend(), l2);

    ASSERT_TRUE(l1.empty());
    ASSERT_THAT(l1, ::testing::ElementsAre());

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_EMPTY_TO_SINGLE)
{
    LinkedList<Value<int>> l1{-5}, l2;

    l1.splice(l1.cbegin(), l2);

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(-5));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());

    l1.splice(l1.cend(), l2);

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(-5));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_EMPTY_TO_GENERIC)
{
    LinkedList<Value<int>> l1{-10, 2, 55, -3, 323}, l2;

    l1.splice(l1.cbegin(), l2);

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(-10, 2, 55, -3, 323));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());

    l1.splice(l1.cend(), l2);

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(-10, 2, 55, -3, 323));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_SINGLE_TO_EMPTY_BEGIN)
{
    LinkedList<Value<int>> l1, l2{4};

    l1.splice(l1.cbegin(), l2);

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(4));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_SINGLE_TO_EMPTY_END)
{
    LinkedList<Value<int>> l1, l2{4};

    l1.splice(l1.cend(), l2);

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(4));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_SINGLE_TO_SINGLE_BEGIN)
{
    LinkedList<Value<int>> l1{1}, l2{4};

    l1.splice(l1.cbegin(), l2);

    ASSERT_EQ(l1.size(), 2U);
    ASSERT_THAT(l1, ::testing::ElementsAre(4, 1));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_SINGLE_TO_SINGLE_END)
{
    LinkedList<Value<int>> l1{1}, l2{4};

    l1.splice(l1.cend(), l2);

    ASSERT_EQ(l1.size(), 2U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 4));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_SINGLE_TO_GENERIC_BEGIN)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4}, l2{100};

    l1.splice(l1.cbegin(), l2);

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(100, 1, 2, 3, 4));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_SINGLE_TO_GENERIC_END)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4}, l2{100};

    l1.splice(l1.cend(), l2);

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 2, 3, 4, 100));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_SINGLE_TO_GENERIC_MIDDLE)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4}, l2{100};

    l1.splice(++++l1.cbegin(), l2);

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 2, 100, 3, 4));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_GENERIC_TO_EMPTY_BEGIN)
{
    LinkedList<Value<int>> l1, l2{1, 4, -5, 124312, 0, 0, 124};

    l1.splice(l1.cbegin(), l2);

    ASSERT_EQ(l1.size(), 7U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 4, -5, 124312, 0, 0, 124));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_GENERIC_TO_EMPTY_END)
{
    LinkedList<Value<int>> l1, l2{1, 4, -5, 124312, 0, 0, 124};

    l1.splice(l1.cend(), l2);

    ASSERT_EQ(l1.size(), 7U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 4, -5, 124312, 0, 0, 124));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_GENERIC_TO_SINGLE_BEGIN)
{
    LinkedList<Value<int>> l1{500}, l2{1, 4, -5, 124312, 0, 0, 124};

    l1.splice(l1.cbegin(), l2);

    ASSERT_EQ(l1.size(), 8U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 4, -5, 124312, 0, 0, 124, 500));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_GENERIC_TO_SINGLE_END)
{
    LinkedList<Value<int>> l1{500}, l2{1, 4, -5, 124312, 0, 0, 124};

    l1.splice(l1.cend(), l2);

    ASSERT_EQ(l1.size(), 8U);
    ASSERT_THAT(l1, ::testing::ElementsAre(500, 1, 4, -5, 124312, 0, 0, 124));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_GENERIC_TO_GENERIC_BEGIN)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4}, l2{899, 4, -5, 124312, 0, 124};

    l1.splice(l1.cbegin(), l2);

    ASSERT_EQ(l1.size(), 10U);
    ASSERT_THAT(l1, ::testing::ElementsAre(899, 4, -5, 124312, 0, 124, 1, 2, 3, 4));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_GENERIC_TO_GENERIC_END)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4}, l2{899, 4, -5, 124312, 0, 124};

    l1.splice(l1.cend(), l2);

    ASSERT_EQ(l1.size(), 10U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 2, 3, 4, 899, 4, -5, 124312, 0, 124));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_GENERIC_TO_GENERIC_MIDDLE)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4}, l2{899, 4, -5, 124312, 0, 124};

    l1.splice(++++l1.cbegin(), l2);

    ASSERT_EQ(l1.size(), 10U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 2, 899, 4, -5, 124312, 0, 124, 3, 4));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_SINGLE_TO_EMPTY_BEGIN)
{
    LinkedList<Value<int>> l1, l2{1};

    l1.splice(l1.cbegin(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_SINGLE_TO_EMPTY_END)
{
    LinkedList<Value<int>> l1, l2{1};

    l1.splice(l1.cend(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_SINGLE_TO_SINGLE_BEGIN)
{
    LinkedList<Value<int>> l1{33}, l2{1};

    l1.splice(l1.cbegin(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 2U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 33));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_SINGLE_TO_SINGLE_END)
{
    LinkedList<Value<int>> l1{33}, l2{1};

    l1.splice(l1.cend(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 2U);
    ASSERT_THAT(l1, ::testing::ElementsAre(33, 1));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_SINGLE_TO_GENERIC_BEGIN)
{
    LinkedList<Value<int>> l1{33, 44, 55, 66}, l2{1};

    l1.splice(l1.cbegin(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 33, 44, 55, 66));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_SINGLE_TO_GENERIC_END)
{
    LinkedList<Value<int>> l1{33, 44, 55, 66}, l2{1};

    l1.splice(l1.cend(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(33, 44, 55, 66, 1));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_SINGLE_TO_GENERIC_MIDDLE)
{
    LinkedList<Value<int>> l1{33, 44, 55, 66}, l2{1};

    l1.splice(++++l1.cbegin(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(33, 44, 1, 55, 66));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_BEGIN_TO_EMPTY_BEGIN)
{
    LinkedList<Value<int>> l1, l2{33, 44, 55, 66};

    l1.splice(l1.cbegin(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(33));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(44, 55, 66));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_LAST_TO_EMPTY_BEGIN)
{
    LinkedList<Value<int>> l1, l2{33, 44, 55, 66};

    l1.splice(l1.cbegin(), l2, --l2.cend());

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(66));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 55));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_MIDDLE_TO_EMPTY_BEGIN)
{
    LinkedList<Value<int>> l1, l2{33, 44, 55, 66};

    l1.splice(l1.cbegin(), l2, ++++l2.cbegin());

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(55));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 66));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_BEGIN_TO_EMPTY_END)
{
    LinkedList<Value<int>> l1, l2{33, 44, 55, 66};

    l1.splice(l1.cend(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(33));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(44, 55, 66));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_LAST_TO_EMPTY_END)
{
    LinkedList<Value<int>> l1, l2{33, 44, 55, 66};

    l1.splice(l1.cend(), l2, --l2.cend());

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(66));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 55));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_MIDDLE_TO_EMPTY_END)
{
    LinkedList<Value<int>> l1, l2{33, 44, 55, 66};

    l1.splice(l1.cend(), l2, ++++l2.cbegin());

    ASSERT_EQ(l1.size(), 1U);
    ASSERT_THAT(l1, ::testing::ElementsAre(55));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 66));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_BEGIN_TO_SINGLE_BEGIN)
{
    LinkedList<Value<int>> l1{1}, l2{33, 44, 55, 66};

    l1.splice(l1.cbegin(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 2U);
    ASSERT_THAT(l1, ::testing::ElementsAre(33, 1));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(44, 55, 66));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_LAST_TO_SINGLE_BEGIN)
{
    LinkedList<Value<int>> l1{1}, l2{33, 44, 55, 66};

    l1.splice(l1.cbegin(), l2, --l2.cend());

    ASSERT_EQ(l1.size(), 2U);
    ASSERT_THAT(l1, ::testing::ElementsAre(66, 1));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 55));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_MIDDLE_TO_SINGLE_BEGIN)
{
    LinkedList<Value<int>> l1{1}, l2{33, 44, 55, 66};

    l1.splice(l1.cbegin(), l2, ++++l2.cbegin());

    ASSERT_EQ(l1.size(), 2U);
    ASSERT_THAT(l1, ::testing::ElementsAre(55, 1));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 66));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_BEGIN_TO_SINGLE_END)
{
    LinkedList<Value<int>> l1{1}, l2{33, 44, 55, 66};

    l1.splice(l1.cend(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 2U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 33));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(44, 55, 66));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_LAST_TO_SINGLE_END)
{
    LinkedList<Value<int>> l1{1}, l2{33, 44, 55, 66};

    l1.splice(l1.cend(), l2, --l2.cend());

    ASSERT_EQ(l1.size(), 2U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 66));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 55));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_MIDDLE_TO_SINGLE_END)
{
    LinkedList<Value<int>> l1{1}, l2{33, 44, 55, 66};

    l1.splice(l1.cend(), l2, ++++l2.cbegin());

    ASSERT_EQ(l1.size(), 2U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 55));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 66));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_BEGIN_TO_GENERIC_BEGIN)
{
    LinkedList<Value<int>> l1{1, -1, 2, -2}, l2{33, 44, 55, 66};

    l1.splice(l1.cbegin(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(33, 1, -1, 2, -2));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(44, 55, 66));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_LAST_TO_GENERIC_BEGIN)
{
    LinkedList<Value<int>> l1{1, -1, 2, -2}, l2{33, 44, 55, 66};

    l1.splice(l1.cbegin(), l2, --l2.cend());

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(66, 1, -1, 2, -2));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 55));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_MIDDLE_TO_GENERIC_BEGIN)
{
    LinkedList<Value<int>> l1{1, -1, 2, -2}, l2{33, 44, 55, 66};

    l1.splice(l1.cbegin(), l2, ++++l2.cbegin());

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(55, 1, -1, 2, -2));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 66));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_BEGIN_TO_GENERIC_END)
{
    LinkedList<Value<int>> l1{1, -1, 2, -2}, l2{33, 44, 55, 66};

    l1.splice(l1.cend(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, -1, 2, -2, 33));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(44, 55, 66));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_LAST_TO_GENERIC_END)
{
    LinkedList<Value<int>> l1{1, -1, 2, -2}, l2{33, 44, 55, 66};

    l1.splice(l1.cend(), l2, --l2.cend());

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, -1, 2, -2, 66));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 55));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_MIDDLE_TO_GENERIC_END)
{
    LinkedList<Value<int>> l1{1, -1, 2, -2}, l2{33, 44, 55, 66};

    l1.splice(l1.cend(), l2, ++++l2.cbegin());

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, -1, 2, -2, 55));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 66));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_BEGIN_TO_GENERIC_MIDDLE)
{
    LinkedList<Value<int>> l1{1, -1, 2, -2}, l2{33, 44, 55, 66};

    l1.splice(++l1.cbegin(), l2, l2.cbegin());

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 33, -1, 2, -2));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(44, 55, 66));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_LAST_TO_GENERIC_MIDDLE)
{
    LinkedList<Value<int>> l1{1, -1, 2, -2}, l2{33, 44, 55, 66};

    l1.splice(++l1.cbegin(), l2, --l2.cend());

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 66, -1, 2, -2));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 55));
}

TEST(LIST, SPLICE_POSITION_OTHER_POSITION_GENERIC_MIDDLE_TO_GENERIC_MIDDLE)
{
    LinkedList<Value<int>> l1{1, -1, 2, -2}, l2{33, 44, 55, 66};

    l1.splice(++l1.cbegin(), l2, ++++l2.cbegin());

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 55, -1, 2, -2));

    ASSERT_EQ(l2.size(), 3U);
    ASSERT_THAT(l2, ::testing::ElementsAre(33, 44, 66));
}

TEST(LIST, SPLICE_POSITION_THIS_SINGLE)
{
    LinkedList<Value<int>> list{87};

    list.splice(list.cend(), list, list.cbegin());

    ASSERT_EQ(list.size(), 1U);
    ASSERT_THAT(list, ::testing::ElementsAre(87));
}

TEST(LIST, SPLICE_POSITION_THIS_ADJACENTS)
{
    LinkedList<Value<int>> list{55, 1323};

    list.splice(list.cbegin(), list, --list.cend());

    ASSERT_EQ(list.size(), 2U);
    ASSERT_THAT(list, ::testing::ElementsAre(1323, 55));
}

TEST(LIST, SPLICE_RANGE_EMPTY)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4, 5}, l2;

    l1.splice(l1.cbegin(), l2, l2.cbegin(), l2.cend());

    ASSERT_EQ(l1.size(), 5U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 2, 3, 4, 5));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_RANGE_GENERIC)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4, 5}, l2{10, 20, 30};

    l1.splice(++++l1.cbegin(), l2, l2.cbegin(), l2.cend());

    ASSERT_EQ(l1.size(), 8U);
    ASSERT_THAT(l1, ::testing::ElementsAre(1, 2, 10, 20, 30, 3, 4, 5));

    ASSERT_TRUE(l2.empty());
    ASSERT_THAT(l2, ::testing::ElementsAre());
}

TEST(LIST, SPLICE_RANGE_THIS_GENERIC)
{
    LinkedList<Value<int>> list{1, 2, 3, 4, 5};

    list.splice(list.cbegin(), list, ++list.cbegin(), list.cend());

    ASSERT_EQ(list.size(), 5U);
    ASSERT_THAT(list, ::testing::ElementsAre(2, 3, 4, 5, 1));
}


TEST(LIST, INSERT_RANGE_EXCEPTION1)
{
    LinkedList<ThrowsOnCopyConstructValueCounted<int>> list;
    ThrowsOnCopyConstructValueCounted<int> range[5] = {5, 4, 1, 2, 5};

    ThrowsOnCopyConstructValueCounted<int>::counter = 1;

    ASSERT_ANY_THROW(list.insert(list.cbegin(), &range[0], &range[5]));

    ASSERT_TRUE(list.empty());
    ASSERT_THAT(list, ::testing::ElementsAre());
}

TEST(LIST, INSERT_RANGE_EXCEPTION2)
{
    LinkedList<ThrowsOnCopyConstructValueCounted<int>> list{100, 200, 300, 400};
    ThrowsOnCopyConstructValueCounted<int> range[5] = {5, 4, 1, 2, 5};

    ThrowsOnCopyConstructValueCounted<int>::counter = 2;

    ASSERT_ANY_THROW(list.insert(++++list.cbegin(), &range[0], &range[5]));

    ASSERT_EQ(list.size(), 4U);
    ASSERT_THAT(list, ::testing::ElementsAre(100, 200, 300, 400));
}
