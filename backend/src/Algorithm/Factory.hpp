#pragma once

#include <map>
#include <functional>
#include <memory>

#include "ISolver.hpp"

class AlgorithmFactory {
private:
  std::map<
    std::string,
    std::function<std::unique_ptr<ISolver>(const ModelMatrix&, Settings::Config)>,
    std::less<>
  > map;

  AlgorithmFactory() { };

public:
  AlgorithmFactory(AlgorithmFactory const&) = delete;
  void operator=(AlgorithmFactory const&)   = delete;

  static AlgorithmFactory& instance() {
    static AlgorithmFactory instance;
    return instance;
  }

  template<typename T>
  constexpr static bool register_algorithm(std::string name, std::function<std::unique_ptr<ISolver>(const ModelMatrix&, Settings::Config)> creator) {
    instance().map[name] = creator;
    return true;
  }

  static std::unique_ptr<ISolver>create(std::string_view key, const ModelMatrix& matrix, Settings::Config config) {
    auto& map = instance().map;
    std::unique_ptr<ISolver> pointer;
    auto found = map.find(key);
    if (found != map.end()) {
      pointer = found->second(matrix, config);
    }
    return pointer;
  }

};