#include <userver/components/logging_configurator.hpp>

#include <logging/dynamic_debug.hpp>
#include <logging/dynamic_debug_config.hpp>
#include <userver/components/component.hpp>
#include <userver/dynamic_config/storage/component.hpp>
#include <userver/dynamic_config/value.hpp>
#include <userver/tracing/tracer.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include <logging/rate_limit.hpp>
#include <logging/split_location.hpp>

#include <dynamic_config/variables/USERVER_NO_LOG_SPANS.hpp>

USERVER_NAMESPACE_BEGIN

namespace components {

namespace {

const dynamic_config::Key<logging::DynamicDebugConfig> kDynamicDebugConfig{
    "USERVER_LOG_DYNAMIC_DEBUG",
    dynamic_config::DefaultAsJsonString{R"(
  {
    "force-disabled": [],
    "force-enabled": []
  }
)"}};

}  // namespace

LoggingConfigurator::LoggingConfigurator(const ComponentConfig& config, const ComponentContext& context) {
    logging::impl::SetLogLimitedEnable(config["limited-logging-enable"].As<bool>());
    logging::impl::SetLogLimitedInterval(config["limited-logging-interval"].As<std::chrono::milliseconds>());

    config_subscription_ = context.FindComponent<components::DynamicConfig>().GetSource().UpdateAndListen(
        this, kName, &LoggingConfigurator::OnConfigUpdate
    );
}

LoggingConfigurator::~LoggingConfigurator() { config_subscription_.Unsubscribe(); }

void LoggingConfigurator::OnConfigUpdate(const dynamic_config::Snapshot& config) {
    (void)this;  // silence clang-tidy
    tracing::Tracer::SetNoLogSpans(tracing::NoLogSpans{config[::dynamic_config::USERVER_NO_LOG_SPANS]});

    try {
        const auto& dd = config[kDynamicDebugConfig];
        auto old_dd = dynamic_debug_.Read();
        if (!(*old_dd == dd)) {
            auto lock = dynamic_debug_.StartWrite();
            *lock = dd;

            /* There is a race between multiple AddDynamicDebugLog(), thus some logs
             * may be logged or not logged by mistake. This is on purpose as logging
             * locking would be too slow and heavy.
             */

            // Flush
            logging::RemoveAllDynamicDebugLog();

            for (const auto& [location, level] : dd.force_disabled) {
                const auto [path, line] = logging::SplitLocation(location);
                logging::EntryState state;
                state.force_disabled_level_plus_one = logging::GetForceDisabledLevelPlusOne(level);
                AddDynamicDebugLog(path, line, state);
            }
            for (const auto& [location, level] : dd.force_enabled) {
                const auto [path, line] = logging::SplitLocation(location);
                logging::EntryState state;
                state.force_enabled_level = level;
                AddDynamicDebugLog(path, line, state);
            }

            lock.Commit();
        }
    } catch (const std::exception& e) {
        LOG_ERROR() << "Failed to set dynamic debug logs from config: " << e;
    }
}

yaml_config::Schema LoggingConfigurator::GetStaticConfigSchema() {
    return yaml_config::MergeSchemas<RawComponentBase>(R"(
type: object
description: Helper component to configure logging
additionalProperties: false
properties:
    limited-logging-enable:
        type: boolean
        description: set to true to make LOG_LIMITED drop repeated logs
    limited-logging-interval:
        type: string
        description: utils::StringToDuration suitable duration string to group repeated logs into one message
)");
}

}  // namespace components

USERVER_NAMESPACE_END
