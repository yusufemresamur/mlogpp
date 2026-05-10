#include "src/registry.hpp"
#include "src/sink/sink.hpp"
#include <memory>
#include <mutex>
namespace mlogpp {

namespace {

// Retrieve an existing entry. Caller must hold at least a shared lock.
std::shared_ptr<Logger> RegistryImpl::find(std::string_view const name) const {
  auto it = loggers.find(std::string(name));
  return it != loggers.end() ? it->second : nullptr;
}

// Insert or replace. Caller must hold an exclusive lock.
void RegistryImpl::store(std::shared_ptr<Logger> const& lgr) {
  loggers[std::string(lgr->Name())] = std::move(lgr);
}
}  // namespace

Registry::Registry() : pimpl_(std::make_unique<RegistryImpl>()) {
  // Seed the registry with a default root logger that writes to stdout.
  auto root_lgr =
      std::make_shared<Logger>("root", std::vector{MakeConsoleSink()});
  pimpl_->store(std::move(root_lgr));
}

[[nodiscard]] std::shared_ptr<Logger> Registry::GetImpl(
    std::string_view const name) {
  {
    std::shared_lock lock(pimpl_->mtx);
    if (auto lgr = pimpl_->find(name); lgr != nullptr) {
      return lgr;
    }
  }
  // Not found, create a new one and insert it.
  auto new_lgr = std::make_shared<Logger>(name);
  {
    std::unique_lock lock(pimpl_->mtx);
    pimpl_->store(new_lgr);
  }
  return new_lgr;
};

}  // namespace mlogpp