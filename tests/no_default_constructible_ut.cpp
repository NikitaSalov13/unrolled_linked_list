#include <unrolled_list.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

struct NoDefaultConstructible {
    NoDefaultConstructible() = delete;

    NoDefaultConstructible(int) {}
};

/*
    Тест проверяет, что ваш контейнер умеет работать с типами, у которых нет конструктора по-умолчанию
*/
TEST(NoDefaultConstructible, canConstruct) {
    unrolled_list<NoDefaultConstructible> unrolled_list;
    unrolled_list.push_front(NoDefaultConstructible(1));
    unrolled_list.push_back(NoDefaultConstructible(2));

    ASSERT_EQ(unrolled_list.size(), 2);
}
