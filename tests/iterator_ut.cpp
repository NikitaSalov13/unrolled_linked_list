#include <unrolled_list.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(UnrolledListIterator, IncrementIterator) {
    unrolled_list<int, 3> ul{1, 2, 3, 4, 5};
    auto it = ul.begin();

    EXPECT_EQ(*it, 1);
    EXPECT_EQ(*(++it), 2);
    EXPECT_EQ(*(it++), 2);
    EXPECT_EQ(*it, 3);

}


TEST(UnrolledListIterator, DecrementIterator) {
    unrolled_list<int, 3> ul{10, 20, 30, 40, 50};
    auto it = ul.begin() + 4;

    EXPECT_EQ(*it, 50);
    --it;
    EXPECT_EQ(*it, 40);
    --it;
    EXPECT_EQ(*it, 30);
    --it;
    EXPECT_EQ(*it, 20);
    --it;
    EXPECT_EQ(*it, 10);
    EXPECT_EQ(it, ul.begin());
}

TEST(UnrolledListIterator, DereferenceIterator) {
    struct Data {
        int value;
        std::string name;
    };

    unrolled_list<Data, 2> ul{{1, "one"}, {2, "two"}, {3, "three"}};
    auto it = ul.begin();

    EXPECT_EQ(it->value, 1);
    EXPECT_EQ(it->name, "one");
    EXPECT_EQ(it->value, 1);

    ++it;
    EXPECT_EQ(it->value, 2);
    EXPECT_EQ(it->name, "two");

    ++it;
    EXPECT_EQ(it->value, 3);
    EXPECT_EQ(it->name, "three");
}

TEST(UnrolledListIterator, RandomAccessIterator) {
    unrolled_list<int, 2> ul{100, 200, 300, 400, 500};
    auto it = ul.begin();

    auto it_plus = it + 3;
    EXPECT_EQ(*it_plus, 400);

    auto it_minus = it_plus - 2;
    EXPECT_EQ(*it_minus, 200);

    it_plus = it_plus + 1;
    EXPECT_EQ(*it_plus, 500);

    it_minus = it_plus - 4;
    EXPECT_EQ(*it_minus, 100);
}

TEST(UnrolledListIterator, IteratorComparison) {
    unrolled_list<int, 3> ul{7, 14, 21};
    auto it1 = ul.begin();
    auto it2 = ul.begin();

    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 != it2);

    ++it2;
    EXPECT_FALSE(it1 == it2);
    EXPECT_TRUE(it1 != it2);

    auto empty_ul = unrolled_list<int, 3>{};
    EXPECT_EQ(empty_ul.begin(), empty_ul.end());
}
