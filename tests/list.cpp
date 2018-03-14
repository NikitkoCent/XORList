#include <xor_list.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>


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

    EXPECT_TRUE(list.empty());
}

TEST(LIST, CONSTRUCTOR_ALLOCATOR)
{
    LinkedList<NonMovableValue<int>> list(std::allocator<NonMovableValue<int>>{});

    EXPECT_TRUE(list.empty());

    list.emplace_back(10);
    list.emplace_back(20);

    EXPECT_EQ(list.size(), 2U);
    EXPECT_EQ(list.front(), 10);
    EXPECT_EQ(list.back(), 20);
}

TEST(LIST, CONSTRUCTOR_INITIALIZER_LIST)
{
    LinkedList<Value<int>> l1{-1, 0, 1, 2, 3, 4};

    EXPECT_EQ(l1.size(), 6U);

    int i = -1;
    for (int e : l1)
    {
        EXPECT_EQ(e, i++);
    }
    EXPECT_EQ(i, 5);
}

TEST(LIST, CONSTRUCTOR_EMPTY_INITIALIZER_LIST)
{
    LinkedList<Value<int>> l1({}, std::allocator<Value<int>>{});

    EXPECT_TRUE(l1.empty());

    for (int e : l1)
    {
        e = 0;
        FAIL();
    }
}

TEST(LIST, CONSTRUCTOR_DEFAULT_FILL)
{
    LinkedList<Value<int>> l1(10U);

    EXPECT_EQ(l1.size(), 10U);

    int i = 0;
    for (int e : l1)
    {
        EXPECT_EQ(e, int{});
        ++i;
    }
    EXPECT_EQ(i, 10);
}

TEST(LIST, CONSTRUCTOR_EMPTY_DEFAULT_FILL)
{
    LinkedList<Value<int>> l1(0);

    EXPECT_TRUE(l1.empty());

    for (int e : l1)
    {
        e = 0;
        FAIL();
    }
}

TEST(LIST, CONSTRUCTOR_VALUE_FILL)
{
    LinkedList<Value<int>> l1(10U, -1590);

    EXPECT_EQ(l1.size(), 10U);

    int i = 0;
    for (int e : l1)
    {
        EXPECT_EQ(e, -1590);
        ++i;
    }
    EXPECT_EQ(i, 10);
}

TEST(LIST, CONSTRUCTOR_EMPTY_VALUE_FILL)
{
    LinkedList<Value<int>> l1(0U, -1590);

    EXPECT_TRUE(l1.empty());

    for (int e : l1)
    {
        e = 0;
        FAIL();
    }
}

TEST(LIST, COPY_CONSTRUCTOR_EMPTY)
{
    LinkedList<Value<int>> l1, l2(l1);

    EXPECT_TRUE(l1.empty());
    EXPECT_TRUE(l2.empty());

    for (int e : l1)
    {
        FAIL();
    }

    for (int e : l2)
    {
        FAIL();
    }
}

TEST(LIST, COPY_CONSTRUCTOR_NON_EMPTY)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9}, l2(l1);

    EXPECT_EQ(l1.size(), 9U);
    EXPECT_EQ(l2.size(), l1.size());

    int i = 0;
    auto it = l1.cbegin();
    for (int e : l2)
    {
        EXPECT_EQ(e, *it++);
        ++i;
    }

    EXPECT_EQ(i, 9);
}

TEST(LIST, MOVE_CONSTRUCTOR_EMPTY)
{
    LinkedList<Value<int>> l1, l2(std::move(l1));

    EXPECT_TRUE(l1.empty());
    EXPECT_TRUE(l2.empty());

    for (int e : l1)
    {
        FAIL();
    }

    for (int e : l2)
    {
        FAIL();
    }
}

TEST(LIST, MOVE_CONSTRUCTOR_NON_EMPTY)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4, 5, 6, 7, 8, 9}, l2(std::move(l1));

    EXPECT_TRUE(l1.empty());
    for (int e : l1)
    {
        FAIL();
    }

    EXPECT_EQ(l2.size(), 9U);

    int i = 1;
    for (int e : l2)
    {
        EXPECT_EQ(e, i++);
    }
    EXPECT_EQ(i, 9);
}


TEST(LIST, MERGE_BOTH_EMPTY)
{
    LinkedList<NonMovableValue<int>> l1, l2;

    l2.merge(l1);

    EXPECT_TRUE(l1.empty());
    EXPECT_TRUE(l2.empty());
}

TEST(LIST, MERGE_SINGLE_TO_EMPTY)
{
    LinkedList<Value<int>> l1{-10}, l2;

    l2.merge(l1);

    EXPECT_TRUE(l1.empty());
    EXPECT_EQ(l2.size(), 1U);
    EXPECT_EQ(l2.front(), -10);
}

TEST(LIST, MERGE_EMPTY_TO_SINGLE)
{
    LinkedList<Value<int>> l1, l2{-10};

    l2.merge(l1);

    EXPECT_TRUE(l1.empty());
    EXPECT_EQ(l2.size(), 1U);
    EXPECT_EQ(l2.front(), -10);
}

TEST(LIST, MERGE_SINGLES_WITHOUT_SWAP)
{
    LinkedList<Value<int>> l1{10}, l2{-10};

    l2.merge(l1);

    EXPECT_TRUE(l1.empty());
    EXPECT_EQ(l2.size(), 2U);
    EXPECT_EQ(l2.front(), -10);
    EXPECT_EQ(l2.back(), 10);
}

TEST(LIST, MERGE_SINGLES_WITH_SWAP)
{
    LinkedList<Value<int>> l1{-10}, l2{10};

    l2.merge(l1);

    EXPECT_TRUE(l1.empty());
    EXPECT_EQ(l2.size(), 2U);
    EXPECT_EQ(l2.front(), -10);
    EXPECT_EQ(l2.back(), 10);
}

TEST(LIST, MERGE_SAME_SINGLES)
{
    LinkedList<Value<int>> l1{10}, l2{10};

    l2.merge(l1);

    EXPECT_TRUE(l1.empty());
    EXPECT_EQ(l2.size(), 2U);
    EXPECT_EQ(l2.front(), 10);
    EXPECT_EQ(l2.back(), 10);
}

TEST(LIST, MERGE_WITHOUT_SWAP)
{
    LinkedList<Value<int>> l1{5, 6, 7, 8}, l2{1, 2, 3, 4};

    l2.merge(l1);

    EXPECT_TRUE(l1.empty());
    EXPECT_EQ(l2.size(), 8U);

    int i = 0;
    for (int e : l1) {
        EXPECT_EQ(e, ++i);
    }
}

TEST(LIST, MERGE_WITH_SWAP)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4}, l2{5, 6, 7, 8};

    l2.merge(l1);

    EXPECT_TRUE(l1.empty());
    EXPECT_EQ(l2.size(), 8U);

    int i = 0;
    for (int e : l1) {
        EXPECT_EQ(e, ++i);
    }
}

TEST(LIST, MERGE_GENERIC)
{
    LinkedList<Value<int>> l1{1, 2, 3, 4}, l2{0, 2, 4, 8};

    l2.merge(l1);

    EXPECT_TRUE(l1.empty());
    EXPECT_EQ(l2.size(), 8U);

    auto it = l2.cbegin();
    EXPECT_EQ(*it++, 0);
    EXPECT_EQ(*it++, 1);
    EXPECT_EQ(*it++, 2);
    EXPECT_EQ(*it++, 2);
    EXPECT_EQ(*it++, 3);
    EXPECT_EQ(*it++, 4);
    EXPECT_EQ(*it++, 4);
    EXPECT_EQ(*it++, 8);
}
