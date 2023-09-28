/* 
 * Copyright (C) 2023 Gleb Bezborodov - All Rights Reserved
 */

#include <map>
#include <vector>
#include <iostream>
#include "join-iterator.h"

const int* extract_value(const std::map<int, float>::const_iterator& iterator, detail::join_iterator_tag) {
    return &iterator->first;
}

int main() {
    std::vector<int> i;
    i.emplace_back(1);
    i.emplace_back(12);
    i.emplace_back(13);
    i.emplace_back(14);
    std::map<int, float> m;
    m.emplace(15, 34);
    m.emplace(230, 34);
    m.emplace(61, 34);
    transform_join_container_t join_c(type_holder_t<int>{}, i, m);
    for (auto&& v : join_c) {
        std::cout << v << std::endl;
    }
    return 0;
} 