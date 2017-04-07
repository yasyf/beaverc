#pragma once

#include <memory>
#include <algorithm>
#include <experimental/optional>

using namespace std;
using namespace std::experimental;

namespace Private {
  template<typename T>
  optional<size_t> index(vector<T> &v, T &x, typename vector<T>::iterator pos, bool insert) {
    if (pos != v.end()) {
      return distance(v.begin(), pos);
    } else if (insert) {
      v.push_back(x);
      return v.size() - 1;
    } else {
      return nullopt;
    }
  }
}

template<typename T>
optional<size_t> index(vector<T> &v, T &x, bool insert) {
  auto pos = find(v.begin(), v.end(), x);
  return Private::index(v, x, pos, insert);
}

template<typename T>
size_t insert(vector<T> &v, T &x) {
  return index(v, x, true).value();
}

template<typename T>
optional<size_t> index_by_val(vector<shared_ptr<T>> &v, shared_ptr<T> x, bool insert) {
  auto pos = find_if(v.begin(), v.end(), [x](const shared_ptr<T> e) { return *e == *x; });
  return Private::index(v, x, pos, insert);
}

template<typename T>
size_t insert_by_val(vector<shared_ptr<T>> &v, shared_ptr<T> x) {
  return index_by_val(v, x, true).value();
}
