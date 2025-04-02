#include <iostream>
#include <vector>

#include <unrolled_list.h>

struct smth {
    int data = 5;
};

int main(int argc, char** argv) {
    std::vector<int> vector = {11,12,13,14,15,16,17};
    unrolled_list<int> list{1,2,3,4,5,6};

    list.push_back(4);
    for (auto i : list) {
        std::cout << i << ' ';
    }
    std::cout<<std::endl;
    unrolled_list list2 = list;
    for (auto i : list2) {
        std::cout << i << ' ';
    }
    std::cout<<std::endl;



}
