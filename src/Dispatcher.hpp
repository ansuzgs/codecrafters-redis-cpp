#pragma once

#include "RESP.hpp"
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <sys/socket.h>

static std::string readLine(int fd) {
  std::string s;
  char c;
  while (recv(fd, &c, 1, 0) == 1) {
    if (c == '\r') {
      recv(fd, &c, 1, 0);
      break;
    }
    s += c;
  }
  return s;
}

static BulkString readBulk(int fd, std::string const &hdr) {
  int len = std::stoi(hdr.substr(1));
  if (len < 0) {
    return {""};
  }
  std::string b(len, '\0');
  int r = 0;
  while (r < len) {
    int n = recv(fd, b.data() + r, len - r, 0);
    if (n <= 0) {
      throw std::runtime_error("EOF");
    }
    r += n;
  }
  char crlf[2];
  recv(fd, crlf, 2, 0);
  return {std::move(b)};
}

static Value parseValue(int fd) {
  auto hdr = readLine(fd);
  if (hdr.empty())
    throw std::runtime_error("EOF");
  switch (hdr[0]) {
  case '+':
    return SimpleString{hdr.substr(1)};
  case '-':
    return Error{hdr.substr(1)};
  case '$': {
    auto bs = readBulk(fd, hdr);
    return bs;
  }
  case '*': {
    int n = std::stoi(hdr.substr(1));
    Array arr;
    arr.elems.reserve(n);
    for (int i = 0; i < n; ++i) {
      arr.elems.push_back(parseValue(fd));
    }
    return arr;
  }
  default:
    throw std::runtime_error("Invalid RESP header");
  }
}

using Handler = std::function<Value(std::vector<Value> const &)>;

class Dispatcher {
  std::unordered_map<std::string, Handler> m_;
  static std::string up(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return s;
  }

public:
  void register_cmd(std::string name, Handler h) {
    m_[up(name)] = std::move(h);
  }

  Value dispatch(int fd) {
    Value v = parseValue(fd);
    auto *arr = std::get_if<Array>(&v.v);
    if (!arr || arr->elems.empty()) {
      return Error{"ERR invalid command"};
    }

    // El primer elemento debe ser un BulkString
    std::string cmd;
    if (auto *bs = std::get_if<BulkString>(&arr->elems[0].v)) {
      cmd = bs->s;
    } else {
      return Error{"Err command not string"};
    }

    auto it = m_.find(up(cmd));
    if (it == m_.end()) {
      return Error{"ERR unknown command"};
    }

    // Los argumentos son el resto del array
    std::vector<Value> args(arr->elems.begin() + 1, arr->elems.end());

    return it->second(args);
  }
};
