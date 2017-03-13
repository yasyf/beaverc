#pragma once

#include <vector>
#include <map>
#include <functional>

using namespace std;

class OrderedMapException : public exception {
private:
  string _msg;

public:
  OrderedMapException(const string& msg) : _msg(msg) {}

  virtual const char* what() const throw() {
    return _msg.c_str();
  }
};

class DuplicateAddException : public OrderedMapException {
  using OrderedMapException::OrderedMapException;
};

template<typename K, typename V>
class OrderedMap {
private:
  map<K, V> data;
  vector<K> indexes;

  void _add(K key, V value) {
    data[key] = value;
    indexes.push_back(key);
  }

public:
  OrderedMap() : indexes(), data() {};

  void add(K key, V value) {
    if (data.count(key)) {
      throw DuplicateAddException(key);
    }
    _add(key, value);
  }

  void insert(K key, V value) {
    if (data.count(key)) {
      data[key] = value;
    } else {
      _add(key, value);
    }
  }

  V at(K key) {
    return data[key];
  }

  V operator[] (K key) {
    return at(key);
  }

  int count(K key) {
    return data.count(key);
  }

  void iterate(function<void(K, V)> f) {
    for (K& key : indexes) {
      f(key, data[key]);
    }
  }
};
