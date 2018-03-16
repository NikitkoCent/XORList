#include <xor_list.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <list>


template<typename T>
struct Value
{
    T value;

    template<typename... Args>
    Value(Args&&... args)
        : value(std::forward<Args>(args)...)
    {
    }

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
