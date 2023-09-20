/* 
 * Copyright (C) 2023 Gleb Bezborodov - All Rights Reserved
 */

#include "join-iterator.h"
#include <map>
#include <vector>

int main() {
    std::vector<int> i;
    std::map<int, float> m;
    join_container_t join_c(i, m);
    for (auto&& v : join_c) {

    }
    return 0;
} 