#include <xor_list/xor_list.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <utility>
#include <string>


TEST(ITERATOR, OPERATOR_DECR)
{
    xor_list<std::pair<int, std::string>> list;

    list.emplace_back(40, "12312");
    list.emplace_back(-113, "");
    list.emplace_back(10020, "AASDsdgd");
    list.emplace_back(0, "[uq349");
    list.emplace_back(2930, "10juvny0q3p[032hgfy7oqpugrqg");

    auto iter = list.cend();
    iter--;

    ASSERT_EQ(iter->first, 2930);
    ASSERT_EQ(iter->second, "10juvny0q3p[032hgfy7oqpugrqg");

    auto iter2 = iter--;

    ASSERT_EQ(iter2->first, 2930);
    ASSERT_EQ(iter2->second, "10juvny0q3p[032hgfy7oqpugrqg");

    ASSERT_EQ(iter->first, 0);
    ASSERT_EQ(iter->second, "[uq349");

    iter2 = iter--;

    ASSERT_EQ(iter2->first, 0);
    ASSERT_EQ(iter2->second, "[uq349");

    ASSERT_EQ(iter->first, 10020);
    ASSERT_EQ(iter->second, "AASDsdgd");

    iter2 = iter--;

    ASSERT_EQ(iter2->first, 10020);
    ASSERT_EQ(iter2->second, "AASDsdgd");

    ASSERT_EQ(iter->first, -113);
    ASSERT_EQ(iter->second, "");

    iter2 = iter--;

    ASSERT_EQ(iter2->first, -113);
    ASSERT_EQ(iter2->second, "");

    ASSERT_EQ(iter->first, 40);
    ASSERT_EQ(iter->second, "12312");

    ASSERT_EQ(iter, list.cbegin());
}
