#include "src/registry.hpp"
#include "src/sink/sink.hpp"
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace mlogpp {

struct RegistryImpl {
  mutable std::shared_mutex mtx;
  std::unordered_map<std::string, std::shared_ptr<DynamicLogger>> loggers;

  std::shared_ptr<DynamicLogger> find(std::string_view name) const {
    auto it = loggers.find(std::string(name));
    return it != loggers.end() ? it->second : nullptr;
  }

  void store(std::shared_ptr<DynamicLogger> const& lgr) {
    loggers[std::string(lgr->Name())] = std::move(lgr);
  }
};

Registry::~Registry() = default;

void Registry::remove(std::string_view name) {
  std::unique_lock lock(pimpl_->mtx);
  pimpl_->loggers.erase(std::string(name));
}

Registry::Registry() : pimpl_(std::make_unique<RegistryImpl>()) {
  // Seed the registry with a default root logger that writes to stdout.
  auto root_lgr =
      std::make_shared<DynamicLogger>("root", std::vector{MakeConsoleSink()});
  pimpl_->store(std::move(root_lgr));
}

[[nodiscard]] std::shared_ptr<DynamicLogger> Registry::GetImpl(
    std::string_view const name) {
  {
    std::shared_lock lock(pimpl_->mtx);
    if (auto lgr = pimpl_->find(name); lgr != nullptr) {
      return lgr;
    }
  }
  // Not found, create a new one and insert it.
  auto new_lgr = std::make_shared<DynamicLogger>(name);
  {
    std::unique_lock lock(pimpl_->mtx);
    pimpl_->store(new_lgr);
  }
  return new_lgr;
};

}  // namespace mlogpp