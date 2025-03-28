#include <ugrpc/impl/grpc_native_logging.hpp>

#include <stdexcept>

#include <fmt/format.h>
#include <grpc/support/log.h>

#include <userver/engine/mutex.hpp>
#include <userver/logging/log.hpp>
#include <userver/utils/assert.hpp>
#include <userver/utils/underlying_value.hpp>

USERVER_NAMESPACE_BEGIN

namespace ugrpc::impl {

namespace {

logging::Level ToLogLevel(::gpr_log_severity severity) noexcept {
    switch (severity) {
        case ::GPR_LOG_SEVERITY_DEBUG:
            return logging::Level::kDebug;
        case ::GPR_LOG_SEVERITY_INFO:
            return logging::Level::kInfo;
        case ::GPR_LOG_SEVERITY_ERROR:
            [[fallthrough]];
        default:
            return logging::Level::kError;
    }
}

::gpr_log_severity ToGprLogSeverity(logging::Level level) {
    switch (level) {
        case logging::Level::kDebug:
            return ::GPR_LOG_SEVERITY_DEBUG;
        case logging::Level::kInfo:
            return ::GPR_LOG_SEVERITY_INFO;
        case logging::Level::kError:
            return ::GPR_LOG_SEVERITY_ERROR;
        default:
            throw std::logic_error(fmt::format(
                "grpcpp log level {} is not allowed. Allowed options: "
                "debug, info, error.",
                logging::ToString(level)
            ));
    }
}

void LogFunction(::gpr_log_func_args* args) noexcept {
    UASSERT(args);
    const auto lvl = ToLogLevel(args->severity);
    if (!logging::ShouldLog(lvl)) return;

    auto& logger = logging::GetDefaultLogger();
    const auto location = utils::impl::SourceLocation::Custom(args->line, args->file, "");
    logging::LogHelper(logger, lvl, logging::LogClass::kLog, location) << args->message;

    // We used to call LogFlush for kError logging level here,
    // but that might lead to a thread switch (there is a coroutine-aware
    // .Wait somewhere down the call chain), which breaks the grpc-core badly:
    // its ExecCtx/ApplicationCallbackExecCtx are attached to a current thread
    // (thread_local that is), and switching threads violates that, obviously.
}

engine::Mutex native_log_level_mutex;
auto native_log_level = logging::Level::kNone;

}  // namespace

void SetupNativeLogging() { ::gpr_set_log_function(&LogFunction); }

void UpdateNativeLogLevel(logging::Level min_log_level_override) {
    std::lock_guard lock(native_log_level_mutex);

    if (utils::UnderlyingValue(min_log_level_override) < utils::UnderlyingValue(native_log_level)) {
        ::gpr_set_log_verbosity(ToGprLogSeverity(min_log_level_override));
        native_log_level = min_log_level_override;
    }
}

}  // namespace ugrpc::impl

USERVER_NAMESPACE_END
