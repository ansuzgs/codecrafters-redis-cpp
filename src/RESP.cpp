#include "RESP.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::string serialize(Value const &rv) {
  return std::visit(
      overloaded{
          [&](SimpleString const &x) {
            return std::string{"+"} + x.s + "\r\n";
          },
          [&](Error const &x) { return std::string{"-"} + x.s + "\r\n"; },
          [&](Integer const &x) {
            return std::string{":"} + std::to_string(x.i) + "\r\n";
          },
          [&](BulkString const &x) {
            return std::string{"$"} + std::to_string(x.s.size()) + "\r\n" +
                   x.s + "\r\n";
          },
          [&](Array const &a) {
            std::string out = "*" + std::to_string(a.elems.size()) + "\r\n";
            for (auto const &e : a.elems) {
              out += serialize(e);
            }
            return out;
          }},
      rv.v);
}
