#pragma once

#include <memory>
#include <experimental/optional>

using namespace std;
using namespace std::experimental;

template<typename T>
optional<size_t> index(vector<T> &v, T &x, bool insert = false);

template<typename T>
size_t insert(vector<T> &v, T &x);

template<typename T>
optional<size_t> index_by_val(vector<shared_ptr<T>> &v, shared_ptr<T> x, bool insert = false);

template<typename T>
size_t insert_by_val(vector<shared_ptr<T>> &v, shared_ptr<T> x);

#include "Util.impl.h"
