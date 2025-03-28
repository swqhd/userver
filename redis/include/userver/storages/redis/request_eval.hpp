#pragma once

/// @file
/// @brief @copybrief storages::redis::RequestEval

#include <userver/storages/redis/parse_reply.hpp>
#include <userver/storages/redis/reply.hpp>
#include <userver/storages/redis/request.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::redis {

/// @brief Redis future for a non-scan and non-eval responses.
template <typename ScriptResult, typename ReplyType = ScriptResult>
class [[nodiscard]] RequestEval final {
public:
    explicit RequestEval(RequestEvalCommon&& request) : request_(std::move(request)) {}

    void Wait() { request_.Wait(); }

    void IgnoreResult() const { request_.IgnoreResult(); }

    ReplyType Get(const std::string& request_description = {}) {
        return ParseReply<ScriptResult, ReplyType>(request_.GetRaw(), request_description);
    }

    /// @cond
    /// Internal helper for WaitAny/WaitAll
    engine::impl::ContextAccessor* TryGetContextAccessor() noexcept { return request_.TryGetContextAccessor(); }
    /// @endcond

private:
    RequestEvalCommon request_;
};

}  // namespace storages::redis

USERVER_NAMESPACE_END
