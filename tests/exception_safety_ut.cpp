#include <unrolled_list.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <list>

class NodeTag {};

class SomeObj {
public:
    static inline int CopiesCount = 0;
    static inline int DestructorCalled = 0;

    SomeObj() = default;

    SomeObj(SomeObj&&) {}

    SomeObj(const SomeObj&) {
        ++CopiesCount;
        if (CopiesCount == 3) {
            throw std::runtime_error("");
        }
    }

    ~SomeObj() {
        ++DestructorCalled;
    }
};

struct Bad {};
struct Good {
    std::string Name;
};

struct BadOrGood {
    BadOrGood() = default;

    BadOrGood(Bad b) {
        throw std::runtime_error("");
    }

    BadOrGood(Good g)
        : Name(g.Name) {}

    std::string Name;
};

template<typename T>
class TestAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using size_type = size_t;
    using is_always_equal = std::true_type;

    static inline int DeallocationCount = 0;
    static inline int ElementsDeallocated = 0;

    static inline int AllocationCount = 0;
    static inline int ElementsAllocated = 0;

    TestAllocator() = default;

    template<typename U>
    TestAllocator(const TestAllocator<U>& other) {
    }

    void deallocate(T* p, std::size_t n) {
        if constexpr (std::is_same_v<T, SomeObj>) {
            ++TestAllocator<SomeObj>::DeallocationCount;
            TestAllocator<SomeObj>::ElementsDeallocated += n;
        } else {
            ++TestAllocator<NodeTag>::DeallocationCount;
            TestAllocator<NodeTag>::ElementsDeallocated += n;
        }
        delete[] reinterpret_cast<char*>(p);
    }

    pointer allocate(size_type sz) {
        if constexpr (std::is_same_v<T, SomeObj>) {
            ++TestAllocator<SomeObj>::AllocationCount;
            TestAllocator<SomeObj>::ElementsAllocated += sz;
        } else {
            ++TestAllocator<NodeTag>::AllocationCount;
            TestAllocator<NodeTag>::ElementsAllocated += sz;
        }
        return reinterpret_cast<pointer>(new char[sz * sizeof(value_type)]);
    }

};

class ExceptionSafetyTest : public testing::Test {
public:
    void SetUp() override {
        SomeObj::CopiesCount = 0;
        SomeObj::DestructorCalled = 0;

        TestAllocator<SomeObj>::AllocationCount = 0;
        TestAllocator<SomeObj>::ElementsAllocated = 0;

        TestAllocator<NodeTag>::AllocationCount = 0;
        TestAllocator<NodeTag>::ElementsAllocated = 0;

        TestAllocator<SomeObj>::DeallocationCount = 0;
        TestAllocator<SomeObj>::ElementsDeallocated = 0;

        TestAllocator<NodeTag>::DeallocationCount = 0;
        TestAllocator<NodeTag>::ElementsDeallocated = 0;
    }

};





TEST_F(ExceptionSafetyTest, failesAtConstruct) {
    std::list<SomeObj> std_list;
    for (int i = 0; i < 5; ++i) {
        std_list.push_back(SomeObj{});
    }
    SomeObj::DestructorCalled = 0;

    TestAllocator<SomeObj> allocator;
    using unrolled_list_type = unrolled_list<SomeObj, 16, TestAllocator<SomeObj>>;

    ASSERT_ANY_THROW(
        unrolled_list_type ul(std_list.begin(), std_list.end(), allocator)
    );

    ASSERT_EQ(SomeObj::DestructorCalled, 2);

    ASSERT_EQ(TestAllocator<NodeTag>::AllocationCount, TestAllocator<NodeTag>::DeallocationCount);
    ASSERT_EQ(TestAllocator<NodeTag>::ElementsAllocated, TestAllocator<NodeTag>::ElementsDeallocated);
}

/*
    В тесте используются объекты классов BadOrGood, Good, Bad.
    У класса BadOrGood есть конструкторы как от Good, так и от Bad.
    При вызове конструктора от Bad будет выброшено исключение.

    Тест проверяет:
        1. push_back от Bad выбросит исключение
        2. Инвариант класса не нарушится
*/
TEST_F(ExceptionSafetyTest, failesAtPushBack) {
    unrolled_list<BadOrGood> unrolled_list;

    unrolled_list.push_back(Good{.Name = "first"});
    unrolled_list.push_back(Good{.Name = "second"});
    ASSERT_ANY_THROW(unrolled_list.push_back(Bad{}));

    ASSERT_EQ(unrolled_list.size(), 2);
    ASSERT_EQ(unrolled_list.begin()->Name, std::string("first"));
    ASSERT_EQ((++unrolled_list.begin())->Name, std::string("second"));
}

/*
    В тесте складывается 5 инстансов класса SomeObj в std::list.
    Далее вызывается конструктор unrolled_list от двух итераторов
    Хранимый тип имеет особенность: он выбрасывает исключение, когда конструктор будет вызван в третий раз

    Тест проверяет:
        1. Что конструктор выбросит исключение
        2. Для двух успешно скопированных объектов будет вызван деструктор
        3. Совпадает количество вызовов allocate и deallocate для некого типа Node (который будете вы использовать) *
        4. Совпадает количество аллоцированных и деаллоцированных элементов некого типа Node (который будете вы использовать) *

    * -- под Node имеется в виду какой-то ваш класс, которым вы описали ноду, имя класса не принципиально
*/

/*
    В тесте используются объекты классов BadOrGood, Good, Bad.
    У класса BadOrGood есть конструкторы как от Good, так и от Bad.
    При вызове конструктора от Bad будет выброшено исключение.

    Тест проверяет:
        1. push_front от Bad выбросит исключение
        2. Инвариант класса не нарушится
*/
TEST_F(ExceptionSafetyTest, failesAtPushFront) {
    unrolled_list<BadOrGood> unrolled_list;

    unrolled_list.push_front(Good{.Name = "first"});
    unrolled_list.push_front(Good{.Name = "second"});
    ASSERT_ANY_THROW(unrolled_list.push_front(Bad{}));

    ASSERT_EQ(unrolled_list.size(), 2);
    ASSERT_EQ(unrolled_list.begin()->Name, std::string("second"));
    ASSERT_EQ((++unrolled_list.begin())->Name, std::string("first"));
}
