#pragma once

#include <memory>
#include <optional>
#include <string_view>
#include <variant>

#include <google/rpc/status.pb.h>

#include <grpcpp/client_context.h>
#include <grpcpp/completion_queue.h>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/status.h>

#include <userver/dynamic_config/fwd.hpp>
#include <userver/tracing/in_place_span.hpp>
#include <userver/tracing/span.hpp>
#include <userver/utils/function_ref.hpp>

#include <userver/ugrpc/client/exceptions.hpp>
#include <userver/ugrpc/client/impl/async_method_invocation.hpp>
#include <userver/ugrpc/client/impl/call_kind.hpp>
#include <userver/ugrpc/client/impl/call_params.hpp>
#include <userver/ugrpc/impl/async_method_invocation.hpp>
#include <userver/ugrpc/impl/maybe_owned_string.hpp>
#include <userver/ugrpc/impl/statistics_scope.hpp>

USERVER_NAMESPACE_BEGIN

namespace ugrpc::client::impl {

/// @{
/// @brief Helper type aliases for low-level asynchronous gRPC streams
/// @see <grpcpp/impl/codegen/async_unary_call_impl.h>
/// @see <grpcpp/impl/codegen/async_stream_impl.h>
template <typename Response>
using RawResponseReader = std::unique_ptr<grpc::ClientAsyncResponseReader<Response>>;

template <typename Response>
using RawReader = std::unique_ptr<grpc::ClientAsyncReader<Response>>;

template <typename Request>
using RawWriter = std::unique_ptr<grpc::ClientAsyncWriter<Request>>;

template <typename Request, typename Response>
using RawReaderWriter = std::unique_ptr<grpc::ClientAsyncReaderWriter<Request, Response>>;
/// @}

struct RpcConfigValues final {
    explicit RpcConfigValues(const dynamic_config::Snapshot& config);

    bool enforce_task_deadline;
};

/// @brief Contains parsed additional data for grpc status
/// For example parsed status string
struct ParsedGStatus final {
    /// @brief Processes status and builds ParsedGStatus
    static ParsedGStatus ProcessStatus(const grpc::Status& status);

    std::optional<google::rpc::Status> gstatus;
    std::optional<std::string> gstatus_string;
};

using ugrpc::client::impl::FinishAsyncMethodInvocation;
using ugrpc::impl::AsyncMethodInvocation;

class RpcData final {
public:
    RpcData(CallParams&&, CallKind);

    RpcData(RpcData&&) noexcept = delete;
    RpcData& operator=(RpcData&&) noexcept = delete;
    ~RpcData();

    ClientData::StubHandle& GetStub() noexcept;

    const grpc::ClientContext& GetContext() const noexcept;

    grpc::ClientContext& GetContext() noexcept;

    std::string_view GetCallName() const noexcept;

    std::string_view GetClientName() const noexcept;

    tracing::Span& GetSpan() noexcept;

    grpc::CompletionQueue& GetQueue() const noexcept;

    const RpcConfigValues& GetConfigValues() const noexcept;

    const Middlewares& GetMiddlewares() const noexcept;

    CallKind GetCallKind() const noexcept;

    void ResetSpan() noexcept;

    ugrpc::impl::RpcStatisticsScope& GetStatsScope() noexcept;

    void SetWritesFinished() noexcept;

    bool AreWritesFinished() const noexcept;

    void SetFinished() noexcept;

    bool IsFinished() const noexcept;

    bool IsDeadlinePropagated() const noexcept;

    void SetDeadlinePropagated() noexcept;

    // please read comments for 'invocation_' private member on why
    // we use two different invocation types
    void EmplaceAsyncMethodInvocation();

    // please read comments for 'invocation_' private member on why
    // we use two different invocation types
    void EmplaceFinishAsyncMethodInvocation();

    // please read comments for 'invocation_' private member on why
    // we use two different invocation types
    AsyncMethodInvocation& GetAsyncMethodInvocation() noexcept;

    // please read comments for 'invocation_' private member on why
    // we use two different invocation types
    FinishAsyncMethodInvocation& GetFinishAsyncMethodInvocation() noexcept;

    // These are for asserts and invariants. Do NOT branch actual code
    // based on these two functions. Branching based on these two functions
    // is considered UB, no diagnostics required.
    bool HoldsAsyncMethodInvocationDebug() noexcept;
    bool HoldsFinishAsyncMethodInvocationDebug() noexcept;

    bool IsFinishProcessed() const noexcept;
    void SetFinishProcessed() noexcept;

    bool IsStatusExtracted() const noexcept;
    void SetStatusExtracted() noexcept;

    grpc::Status& GetStatus() noexcept;
    ParsedGStatus& GetParsedGStatus() noexcept;

    class AsyncMethodInvocationGuard {
    public:
        AsyncMethodInvocationGuard(RpcData& data) noexcept;
        AsyncMethodInvocationGuard(const AsyncMethodInvocationGuard&) = delete;
        AsyncMethodInvocationGuard(AsyncMethodInvocationGuard&&) = delete;
        ~AsyncMethodInvocationGuard() noexcept;

        void Disarm() noexcept { disarm_ = true; }

    private:
        RpcData& data_;
        bool disarm_{false};
    };

private:
    ClientData::StubHandle stub_;
    std::unique_ptr<grpc::ClientContext> context_;
    std::string client_name_;
    ugrpc::impl::MaybeOwnedString call_name_;
    bool writes_finished_{false};
    bool is_finished_{false};
    bool is_deadline_propagated_{false};

    std::optional<tracing::InPlaceSpan> span_;
    ugrpc::impl::RpcStatisticsScope stats_scope_;
    grpc::CompletionQueue& queue_;
    RpcConfigValues config_values_;
    const Middlewares& mws_;

    CallKind call_kind_{};

    grpc::Status status_;
    ParsedGStatus parsed_g_status_;
    bool finish_processed_{false};
    bool status_extracted_{false};

    // This data is common for all types of grpc calls - unary and streaming
    // However, in unary call the call is finished as soon as grpc core
    // gives us back a response - so for unary call we use
    // FinishAsyncMethodInvocation that will correctly close all our
    // tracing::Span objects and account everything in statistics.
    // In stream response, we use AsyncMethodInvocation for every intermediate
    // Read* call, because we don't need to close span and/or account stats
    // when finishing Read* call.
    //
    // This field must go after other fields to be destroyed first!
    std::variant<std::monostate, AsyncMethodInvocation, FinishAsyncMethodInvocation> invocation_;
};

void CheckOk(RpcData& data, AsyncMethodInvocation::WaitStatus status, std::string_view stage);

template <typename GrpcStream>
void StartCall(GrpcStream& stream, RpcData& data) {
    AsyncMethodInvocation start_call;
    stream.StartCall(start_call.GetTag());
    CheckOk(data, Wait(start_call, data.GetContext()), "StartCall");
}

void PrepareFinish(RpcData& data);

void ProcessFinish(RpcData& data, utils::function_ref<void(RpcData& data, const grpc::Status& status)> post_finish);

void CheckFinishStatus(RpcData& data);

void ProcessFinishResult(
    RpcData& data,
    AsyncMethodInvocation::WaitStatus wait_status,
    utils::function_ref<void(RpcData& data, const grpc::Status& status)> post_finish,
    bool throw_on_error
);

template <typename GrpcStream>
void Finish(
    GrpcStream& stream,
    RpcData& data,
    utils::function_ref<void(RpcData& data, const grpc::Status& status)> post_finish,
    bool throw_on_error
) {
    PrepareFinish(data);

    FinishAsyncMethodInvocation finish(data);
    auto& status = data.GetStatus();
    stream.Finish(&status, finish.GetTag());

    const auto wait_status = Wait(finish, data.GetContext());
    if (wait_status == impl::AsyncMethodInvocation::WaitStatus::kCancelled) {
        data.GetStatsScope().OnCancelled();
        if (throw_on_error) {
            throw RpcCancelledError(data.GetCallName(), "Finish");
        }
        return;
    }

    ProcessFinishResult(data, wait_status, post_finish, throw_on_error);
}

void PrepareRead(RpcData& data);

template <typename GrpcStream, typename Response>
[[nodiscard]] bool Read(GrpcStream& stream, Response& response, RpcData& data) {
    PrepareRead(data);
    AsyncMethodInvocation read;
    stream.Read(&response, read.GetTag());
    const auto wait_status = Wait(read, data.GetContext());
    if (wait_status == impl::AsyncMethodInvocation::WaitStatus::kCancelled) {
        data.GetStatsScope().OnCancelled();
    }
    return wait_status == impl::AsyncMethodInvocation::WaitStatus::kOk;
}

template <typename GrpcStream, typename Response>
void ReadAsync(GrpcStream& stream, Response& response, RpcData& data) noexcept {
    PrepareRead(data);
    data.EmplaceAsyncMethodInvocation();
    auto& read = data.GetAsyncMethodInvocation();
    stream.Read(&response, read.GetTag());
}

void PrepareWrite(RpcData& data);

template <typename GrpcStream, typename Request>
bool Write(GrpcStream& stream, const Request& request, grpc::WriteOptions options, RpcData& data) {
    PrepareWrite(data);
    AsyncMethodInvocation write;
    stream.Write(request, options, write.GetTag());
    const auto result = Wait(write, data.GetContext());
    if (result == impl::AsyncMethodInvocation::WaitStatus::kCancelled) {
        data.GetStatsScope().OnCancelled();
    }
    if (result != impl::AsyncMethodInvocation::WaitStatus::kOk) {
        data.SetWritesFinished();
    }
    return result == impl::AsyncMethodInvocation::WaitStatus::kOk;
}

void PrepareWriteAndCheck(RpcData& data);

template <typename GrpcStream, typename Request>
void WriteAndCheck(GrpcStream& stream, const Request& request, grpc::WriteOptions options, RpcData& data) {
    PrepareWriteAndCheck(data);
    AsyncMethodInvocation write;
    stream.Write(request, options, write.GetTag());
    CheckOk(data, Wait(write, data.GetContext()), "WriteAndCheck");
}

template <typename GrpcStream>
bool WritesDone(GrpcStream& stream, RpcData& data) {
    PrepareWrite(data);
    data.SetWritesFinished();
    AsyncMethodInvocation writes_done;
    stream.WritesDone(writes_done.GetTag());
    const auto wait_status = Wait(writes_done, data.GetContext());
    if (wait_status == impl::AsyncMethodInvocation::WaitStatus::kCancelled) {
        data.GetStatsScope().OnCancelled();
    }
    return wait_status == impl::AsyncMethodInvocation::WaitStatus::kOk;
}

}  // namespace ugrpc::client::impl

USERVER_NAMESPACE_END
