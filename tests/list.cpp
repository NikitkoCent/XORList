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
