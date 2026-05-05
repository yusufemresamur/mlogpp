#pragma once
#ifndef MLOGPP_REGISTRY_HPP_
#define MLOGPP_REGISTRY_HPP_
#include "fmt_string_with_location.hpp"
#include "logger.hpp"
#include <memory>
#include <shared_mutex>
#include <string_view>
#include <unordered_map>

namespace mlogpp {

namespace {
struct RegistryImpl {
  mutable std::shared_mutex mtx;
  std::unordered_map<std::string, std::shared_ptr<Logger>> loggers;

  std::shared_ptr<Logger> find(std::string_view const name) const;

  // Insert or replace. Caller must hold an exclusive lock.
  void store(std::shared_ptr<Logger> const lgr);
};
}  // namespace

/**
 * @brief Singleton registry that manages named loggers. Provides thread-safe
 * access to loggers and ensures that loggers with the same name are shared
 * across the application.
 *
 */
class Registry {
 public:
  Registry(Registry const&) = delete;
  Registry& operator=(Registry const&) = delete;

  /**
   * @brief Get the singleton instance of the registry.
   *
   * @return Registry& Reference to the singleton registry instance.
   */
  [[nodiscard]] static Registry& instance() noexcept {
    static Registry registry;
    return registry;
  };

  /**
   * @brief Get or create a named logger. Thread-safe.
   *
   * @param name Name of the logger to retrieve or create.
   * @return std::shared_ptr<Logger> Shared pointer to the requested logger.
   */
  [[nodiscard]] std::shared_ptr<Logger> Get(std::string_view name);

  /**
   * @brief Remove a logger from the registry. Thread-safe.
   *
   * @param name Name of the logger to remove.
   */
  void remove(std::string_view name);

  /**
   * @brief Get the root logger, which is a special logger that can be used for
   * global logging. The root logger is created on demand and shared across the
   * application.
   *
   * @return std::shared_ptr<Logger> Shared pointer to the root logger.
   */
  [[nodiscard]] static std::shared_ptr<Logger> root() {
    return instance().Get("root");
  };

 private:
  Registry();
  /// Pimpl idiom to hide implementation details.
  std::unique_ptr<RegistryImpl> pimpl_;
};

// Global logging functions that use the root logger. These are convenient for
// quick logging without needing to manage logger instances. They forward to the
// root logger's methods.
// TODO (yusufemresamur): Add doxygen comments for these functions.

template <typename... Args>
void Trace(FormatStringWithLocation const msg, Args&&... args) {
  Registry::root()->Trace(msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Debug(FormatStringWithLocation const msg, Args&&... args) {
  Registry::root()->Debug(msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Info(FormatStringWithLocation const msg, Args&&... args) {
  Registry::root()->Info(msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Warn(FormatStringWithLocation const msg, Args&&... args) {
  Registry::root()->Warn(msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Error(FormatStringWithLocation const msg, Args&&... args) {
  Registry::root()->Error(msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Fatal(FormatStringWithLocation const msg, Args&&... args) {
  Registry::root()->Fatal(msg, std::forward<Args>(args)...);
}

}  // namespace mlogpp
#endif  // MLOGPP_REGISTRY_HPP_