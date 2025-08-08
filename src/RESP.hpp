#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

struct SimpleString {
  std::string s;
};
struct Error {
  std::string s;
};
struct Integer {
  int64_t i;
};
struct BulkString {
  std::string s;
};
struct Array {
  std::vector<struct Value> elems;
};

struct Value {
  std::variant<SimpleString, Error, Integer, BulkString, Array> v;

  Value(SimpleString s) : v(std::move(s)) {}
  Value(Error e) : v(std::move(e)) {}
  Value(Integer i) : v(std::move(i)) {}
  Value(BulkString b) : v(std::move(b)) {}
  Value(Array a) : v(std::move(a)) {}
};

std::string serialize(Value const &rv);
