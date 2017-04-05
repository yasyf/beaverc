#pragma once

#include <algorithm>
#include <experimental/optional>

using namespace std;
using namespace std::experimental;

template<typename T>
optional<size_t> index(vector<T> &v, T &x, bool insert = false) {
  auto pos = find(v.begin(), v.end(), x);
  if (pos != v.end()) {
    return distance(v.begin(), pos);
  } else if (insert) {
    v.push_back(x);
    return v.size() - 1;
  } else {
    return nullopt;
  }
}

template<typename T>
size_t insert(vector<T> &v, T &x) {
  return index(v, x, true).value();
}
