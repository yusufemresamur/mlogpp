#pragma once
#ifndef MLOGPP_REGISTRY_HPP_
#define MLOGPP_REGISTRY_HPP_
#include "src/fmt_string_with_location.hpp"
#include "src/logger.hpp"
#include <memory>
#include <string_view>

namespace mlogpp {

struct RegistryImpl;

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
   * @return LoggerPtr Shared pointer to the requested logger.
   */
  [[nodiscard]] static LoggerPtr Get(std::string_view name) {
    return instance().GetImpl(name);
  }

  /**
   * @brief Get or create a named logger as a reference. Thread-safe.
   *
   * @param name Name of the logger to retrieve or create.
   * @return Logger& Reference to the requested logger.
   */
  [[nodiscard]] static DynamicLogger& GetRef(std::string_view name) {
    return *instance().GetImpl(name);
  }

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
   * @return LoggerPtr Shared pointer to the root logger.
   */
  [[nodiscard]] static LoggerPtr Root() {
    return mlogpp::Registry::Get("root");
  };

  /**
   * @brief Get the root logger as a reference.
   *
   * @return Logger& Reference to the root logger.
   */
  [[nodiscard]] static DynamicLogger& RootRef() {
    return *mlogpp::Registry::Get("root");
  };

 private:
  Registry();
  ~Registry();

  /**
   * @brief Internal implementation for getting or creating a logger.
   *
   * @param name Name of the logger to retrieve or create.
   * @return LoggerPtr Shared pointer to the requested logger.
   */
  LoggerPtr GetImpl(std::string_view name);

  /// Pimpl idiom to hide implementation details.
  std::unique_ptr<RegistryImpl> pimpl_;
};

// Global logging functions that use the root logger. These are convenient for
// quick logging without needing to manage logger instances. They forward to the
// root logger's methods.
// TODO (yusufemresamur): Add doxygen comments for these functions.

template <typename... Args>
void Trace(FormatStringWithLocation const msg, Args&&... args) {
  Registry::RootRef().Trace(msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Debug(FormatStringWithLocation const msg, Args&&... args) {
  Registry::RootRef().Debug(msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Info(FormatStringWithLocation const msg, Args&&... args) {
  Registry::RootRef().Info(msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Warn(FormatStringWithLocation const msg, Args&&... args) {
  Registry::RootRef().Warn(msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Error(FormatStringWithLocation const msg, Args&&... args) {
  Registry::RootRef().Error(msg, std::forward<Args>(args)...);
}

template <typename... Args>
void Fatal(FormatStringWithLocation const msg, Args&&... args) {
  Registry::RootRef().Fatal(msg, std::forward<Args>(args)...);
}

}  // namespace mlogpp
#endif  // MLOGPP_REGISTRY_HPP_