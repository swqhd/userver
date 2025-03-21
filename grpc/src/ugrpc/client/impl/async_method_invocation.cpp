#include <userver/ugrpc/client/impl/async_method_invocation.hpp>

#include <ugrpc/impl/status.hpp>
#include <userver/ugrpc/client/impl/async_methods.hpp>

USERVER_NAMESPACE_BEGIN

namespace ugrpc::client::impl {

ParsedGStatus ParsedGStatus::ProcessStatus(const grpc::Status& status) {
    if (status.ok()) {
        return {};
    }
    auto gstatus = ugrpc::impl::ToGoogleRpcStatus(status);
    std::optional<std::string> gstatus_string;
    if (gstatus) {
        gstatus_string = ugrpc::impl::GetGStatusLimitedMessage(*gstatus);
    }

    return ParsedGStatus{std::move(gstatus), std::move(gstatus_string)};
}

FinishAsyncMethodInvocation::FinishAsyncMethodInvocation(RpcData& rpc_data) : rpc_data_(rpc_data) {}

FinishAsyncMethodInvocation::~FinishAsyncMethodInvocation() { WaitWhileBusy(); }

void FinishAsyncMethodInvocation::Notify(bool ok) noexcept {
    if (ok) {
        try {
            auto& status = rpc_data_.GetStatus();
            rpc_data_.GetStatsScope().OnExplicitFinish(status.error_code());

            if (status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED && rpc_data_.IsDeadlinePropagated()) {
                rpc_data_.GetStatsScope().OnCancelledByDeadlinePropagation();
            }

            rpc_data_.GetStatsScope().Flush();

            rpc_data_.GetParsedGStatus() = ParsedGStatus::ProcessStatus(status);
        } catch (const std::exception& e) {
            LOG_LIMITED_ERROR() << "Error in FinishAsyncMethodInvocation::Notify: " << e;
        }
    }
    AsyncMethodInvocation::Notify(ok);
}

ugrpc::impl::AsyncMethodInvocation::WaitStatus
Wait(ugrpc::impl::AsyncMethodInvocation& invocation, grpc::ClientContext& context) noexcept {
    return impl::WaitUntil(invocation, context, engine::Deadline{});
}

ugrpc::impl::AsyncMethodInvocation::WaitStatus WaitUntil(
    ugrpc::impl::AsyncMethodInvocation& invocation,
    grpc::ClientContext& context,
    engine::Deadline deadline
) noexcept {
    const auto status = invocation.WaitUntil(deadline);
    if (status == ugrpc::impl::AsyncMethodInvocation::WaitStatus::kCancelled) context.TryCancel();

    return status;
}

}  // namespace ugrpc::client::impl

USERVER_NAMESPACE_END
