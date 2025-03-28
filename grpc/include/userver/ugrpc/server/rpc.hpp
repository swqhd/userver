#pragma once

/// @file userver/ugrpc/server/rpc.hpp
/// @brief Classes representing an incoming RPC

#include <google/protobuf/message.h>

#include <userver/utils/assert.hpp>

#include <userver/ugrpc/deadline_timepoint.hpp>
#include <userver/ugrpc/impl/internal_tag_fwd.hpp>
#include <userver/ugrpc/server/call.hpp>
#include <userver/ugrpc/server/exceptions.hpp>
#include <userver/ugrpc/server/impl/async_methods.hpp>
#include <userver/ugrpc/server/stream.hpp>

USERVER_NAMESPACE_BEGIN

namespace ugrpc::server {

/// @brief Controls a single request -> single response RPC
///
/// The RPC is cancelled on destruction unless `Finish` has been called.
template <typename Response>
class UnaryCall final : public CallAnyBase {
public:
    /// @brief Complete the RPC successfully
    ///
    /// `Finish` must not be called multiple times for the same RPC.
    ///
    /// @param response the single Response to send to the client
    /// @throws ugrpc::server::RpcError on an RPC error
    void Finish(Response& response);

    /// @brief Complete the RPC successfully
    ///
    /// `Finish` must not be called multiple times for the same RPC.
    ///
    /// @param response the single Response to send to the client
    /// @throws ugrpc::server::RpcError on an RPC error
    void Finish(Response&& response);

    /// @brief Complete the RPC with an error
    ///
    /// `Finish` must not be called multiple times for the same RPC.
    ///
    /// @param status error details
    /// @throws ugrpc::server::RpcError on an RPC error
    void FinishWithError(const grpc::Status& status) override;

    /// For internal use only
    UnaryCall(impl::CallParams&& call_params, impl::RawResponseWriter<Response>& stream);

    UnaryCall(UnaryCall&&) = delete;
    UnaryCall& operator=(UnaryCall&&) = delete;
    ~UnaryCall();

    bool IsFinished() const override;

private:
    impl::RawResponseWriter<Response>& stream_;
    bool is_finished_{false};
};

/// @brief Controls a request stream -> single response RPC
///
/// This class is not thread-safe except for `GetContext`.
///
/// The RPC is cancelled on destruction unless the stream has been finished.
///
/// If any method throws, further methods must not be called on the same stream,
/// except for `GetContext`.
template <typename Request, typename Response>
class InputStream final : public CallAnyBase, public Reader<Request> {
public:
    /// @brief Await and read the next incoming message
    /// @param request where to put the request on success
    /// @returns `true` on success, `false` on end-of-input
    bool Read(Request& request) override;

    /// @brief Complete the RPC successfully
    ///
    /// `Finish` must not be called multiple times for the same RPC.
    ///
    /// @param response the single Response to send to the client
    /// @throws ugrpc::server::RpcError on an RPC error
    void Finish(Response& response);

    /// @brief Complete the RPC successfully
    ///
    /// `Finish` must not be called multiple times for the same RPC.
    ///
    /// @param response the single Response to send to the client
    /// @throws ugrpc::server::RpcError on an RPC error
    void Finish(Response&& response);

    /// @brief Complete the RPC with an error
    ///
    /// `Finish` must not be called multiple times for the same RPC.
    ///
    /// @param status error details
    /// @throws ugrpc::server::RpcError on an RPC error
    void FinishWithError(const grpc::Status& status) override;

    /// For internal use only
    InputStream(impl::CallParams&& call_params, impl::RawReader<Request, Response>& stream);

    InputStream(InputStream&&) = delete;
    InputStream& operator=(InputStream&&) = delete;
    ~InputStream() override;

    bool IsFinished() const override;

private:
    enum class State { kOpen, kReadsDone, kFinished };

    impl::RawReader<Request, Response>& stream_;
    State state_{State::kOpen};
};

/// @brief Controls a single request -> response stream RPC
///
/// This class is not thread-safe except for `GetContext`.
///
/// The RPC is cancelled on destruction unless the stream has been finished.
///
/// If any method throws, further methods must not be called on the same stream,
/// except for `GetContext`.
template <typename Response>
class OutputStream final : public CallAnyBase, public Writer<Response> {
public:
    /// @brief Write the next outgoing message
    /// @param response the next message to write
    /// @throws ugrpc::server::RpcError on an RPC error
    void Write(Response& response) override;

    /// @brief Write the next outgoing message
    /// @param response the next message to write
    /// @throws ugrpc::server::RpcError on an RPC error
    void Write(Response&& response) override;

    /// @brief Complete the RPC successfully
    ///
    /// `Finish` must not be called multiple times.
    ///
    /// @throws ugrpc::server::RpcError on an RPC error
    void Finish();

    /// @brief Complete the RPC with an error
    ///
    /// `Finish` must not be called multiple times.
    ///
    /// @param status error details
    /// @throws ugrpc::server::RpcError on an RPC error
    void FinishWithError(const grpc::Status& status) override;

    /// @brief Equivalent to `Write + Finish`
    ///
    /// This call saves one round-trip, compared to separate `Write` and `Finish`.
    ///
    /// `Finish` must not be called multiple times.
    ///
    /// @param response the final response message
    /// @throws ugrpc::server::RpcError on an RPC error
    void WriteAndFinish(Response& response);

    /// @brief Equivalent to `Write + Finish`
    ///
    /// This call saves one round-trip, compared to separate `Write` and `Finish`.
    ///
    /// `Finish` must not be called multiple times.
    ///
    /// @param response the final response message
    /// @throws ugrpc::server::RpcError on an RPC error
    void WriteAndFinish(Response&& response);

    /// For internal use only
    OutputStream(impl::CallParams&& call_params, impl::RawWriter<Response>& stream);

    OutputStream(OutputStream&&) = delete;
    OutputStream& operator=(OutputStream&&) = delete;
    ~OutputStream() override;

    bool IsFinished() const override;

private:
    enum class State { kNew, kOpen, kFinished };

    impl::RawWriter<Response>& stream_;
    State state_{State::kNew};
};

/// @brief Controls a request stream -> response stream RPC
///
/// This class allows the following concurrent calls:
///
///   - `GetContext`
///   - `Read`;
///   - one of (`Write`, `Finish`, `FinishWithError`, `WriteAndFinish`).
///
/// The RPC is cancelled on destruction unless the stream has been finished.
///
/// If any method throws, further methods must not be called on the same stream,
/// except for `GetContext`.
template <typename Request, typename Response>
class BidirectionalStream : public CallAnyBase, public ReaderWriter<Request, Response> {
public:
    /// @brief Await and read the next incoming message
    /// @param request where to put the request on success
    /// @returns `true` on success, `false` on end-of-input
    /// @throws ugrpc::server::RpcError on an RPC error
    bool Read(Request& request) override;

    /// @brief Write the next outgoing message
    /// @param response the next message to write
    /// @throws ugrpc::server::RpcError on an RPC error
    void Write(Response& response) override;

    /// @brief Write the next outgoing message
    /// @param response the next message to write
    /// @throws ugrpc::server::RpcError on an RPC error
    void Write(Response&& response) override;

    /// @brief Complete the RPC successfully
    ///
    /// `Finish` must not be called multiple times.
    ///
    /// @throws ugrpc::server::RpcError on an RPC error
    void Finish();

    /// @brief Complete the RPC with an error
    ///
    /// `Finish` must not be called multiple times.
    ///
    /// @param status error details
    /// @throws ugrpc::server::RpcError on an RPC error
    void FinishWithError(const grpc::Status& status) override;

    /// @brief Equivalent to `Write + Finish`
    ///
    /// This call saves one round-trip, compared to separate `Write` and `Finish`.
    ///
    /// `Finish` must not be called multiple times.
    ///
    /// @param response the final response message
    /// @throws ugrpc::server::RpcError on an RPC error
    void WriteAndFinish(Response& response);

    /// @brief Equivalent to `Write + Finish`
    ///
    /// This call saves one round-trip, compared to separate `Write` and `Finish`.
    ///
    /// `Finish` must not be called multiple times.
    ///
    /// @param response the final response message
    /// @throws ugrpc::server::RpcError on an RPC error
    void WriteAndFinish(Response&& response);

    /// For internal use only
    BidirectionalStream(impl::CallParams&& call_params, impl::RawReaderWriter<Request, Response>& stream);

    BidirectionalStream(const BidirectionalStream&) = delete;
    BidirectionalStream(BidirectionalStream&&) = delete;
    ~BidirectionalStream() override;

    bool IsFinished() const override;

private:
    impl::RawReaderWriter<Request, Response>& stream_;
    bool are_reads_done_{false};
    bool is_finished_{false};
};

template <typename Response>
UnaryCall<Response>::UnaryCall(impl::CallParams&& call_params, impl::RawResponseWriter<Response>& stream)
    : CallAnyBase(utils::impl::InternalTag{}, std::move(call_params), impl::CallKind::kUnaryCall), stream_(stream) {}

template <typename Response>
UnaryCall<Response>::~UnaryCall() {
    if (!is_finished_) {
        impl::CancelWithError(stream_, GetCallName());
        WriteAccessLog(impl::kUnknownErrorStatus);
    }
}

template <typename Response>
void UnaryCall<Response>::Finish(Response&& response) {
    Finish(response);
}

template <typename Response>
void UnaryCall<Response>::Finish(Response& response) {
    UINVARIANT(!is_finished_, "'Finish' called on a finished call");
    ApplyResponseHook(&response);

    // It is important to set is_finished_ after ApplyResponseHook.
    // Otherwise, there would be no way to call FinishWithError there.
    is_finished_ = true;

    WriteAccessLog(grpc::Status::OK);

    impl::Finish(stream_, response, grpc::Status::OK, GetCallName());
    PostFinish(grpc::Status::OK);
}

template <typename Response>
void UnaryCall<Response>::FinishWithError(const grpc::Status& status) {
    if (IsFinished()) return;
    is_finished_ = true;
    WriteAccessLog(status);
    impl::FinishWithError(stream_, status, GetCallName());
    PostFinish(status);
}

template <typename Response>
bool UnaryCall<Response>::IsFinished() const {
    return is_finished_;
}

template <typename Request, typename Response>
InputStream<Request, Response>::InputStream(impl::CallParams&& call_params, impl::RawReader<Request, Response>& stream)
    : CallAnyBase(utils::impl::InternalTag{}, std::move(call_params), impl::CallKind::kInputStream), stream_(stream) {}

template <typename Request, typename Response>
InputStream<Request, Response>::~InputStream() {
    if (state_ != State::kFinished) {
        impl::CancelWithError(stream_, GetCallName());
        WriteAccessLog(impl::kUnknownErrorStatus);
    }
}

template <typename Request, typename Response>
bool InputStream<Request, Response>::Read(Request& request) {
    UINVARIANT(state_ == State::kOpen, "'Read' called while the stream is half-closed for reads");
    if (impl::Read(stream_, request)) {
        ApplyRequestHook(&request);
        return true;
    } else {
        state_ = State::kReadsDone;
        return false;
    }
}

template <typename Request, typename Response>
void InputStream<Request, Response>::Finish(Response&& response) {
    Finish(response);
}

template <typename Request, typename Response>
void InputStream<Request, Response>::Finish(Response& response) {
    UINVARIANT(state_ != State::kFinished, "'Finish' called on a finished stream");
    ApplyResponseHook(&response);

    // It is important to set the state_ after ApplyResponseHook.
    // Otherwise, there would be no way to call FinishWithError there.
    state_ = State::kFinished;

    WriteAccessLog(grpc::Status::OK);

    impl::Finish(stream_, response, grpc::Status::OK, GetCallName());
    PostFinish(grpc::Status::OK);
}

template <typename Request, typename Response>
void InputStream<Request, Response>::FinishWithError(const grpc::Status& status) {
    UASSERT(!status.ok());
    if (IsFinished()) return;
    state_ = State::kFinished;
    WriteAccessLog(status);
    impl::FinishWithError(stream_, status, GetCallName());
    PostFinish(status);
}

template <typename Request, typename Response>
bool InputStream<Request, Response>::IsFinished() const {
    return state_ == State::kFinished;
}

template <typename Response>
OutputStream<Response>::OutputStream(impl::CallParams&& call_params, impl::RawWriter<Response>& stream)
    : CallAnyBase(utils::impl::InternalTag{}, std::move(call_params), impl::CallKind::kOutputStream), stream_(stream) {}

template <typename Response>
OutputStream<Response>::~OutputStream() {
    if (state_ != State::kFinished) {
        impl::Cancel(stream_, GetCallName());
        WriteAccessLog(impl::kUnknownErrorStatus);
    }
}

template <typename Response>
void OutputStream<Response>::Write(Response&& response) {
    Write(response);
}

template <typename Response>
void OutputStream<Response>::Write(Response& response) {
    UINVARIANT(state_ != State::kFinished, "'Write' called on a finished stream");
    ApplyResponseHook(&response);

    // For some reason, gRPC requires explicit 'SendInitialMetadata' in output
    // streams
    impl::SendInitialMetadataIfNew(stream_, GetCallName(), state_);

    // Don't buffer writes, otherwise in an event subscription scenario, events
    // may never actually be delivered
    grpc::WriteOptions write_options{};

    impl::Write(stream_, response, write_options, GetCallName());
}

template <typename Response>
void OutputStream<Response>::Finish() {
    UINVARIANT(state_ != State::kFinished, "'Finish' called on a finished stream");
    state_ = State::kFinished;

    WriteAccessLog(grpc::Status::OK);

    impl::Finish(stream_, grpc::Status::OK, GetCallName());
    PostFinish(grpc::Status::OK);
}

template <typename Response>
void OutputStream<Response>::FinishWithError(const grpc::Status& status) {
    UASSERT(!status.ok());
    if (IsFinished()) return;
    state_ = State::kFinished;
    WriteAccessLog(status);
    impl::Finish(stream_, status, GetCallName());
    PostFinish(status);
}

template <typename Response>
void OutputStream<Response>::WriteAndFinish(Response&& response) {
    WriteAndFinish(response);
}

template <typename Response>
void OutputStream<Response>::WriteAndFinish(Response& response) {
    UINVARIANT(state_ != State::kFinished, "'WriteAndFinish' called on a finished stream");
    ApplyResponseHook(&response);

    // It is important to set the state_ after ApplyResponseHook.
    // Otherwise, there would be no way to call FinishWithError there.
    state_ = State::kFinished;

    // Don't buffer writes, otherwise in an event subscription scenario, events
    // may never actually be delivered
    grpc::WriteOptions write_options{};

    WriteAccessLog(grpc::Status::OK);

    impl::WriteAndFinish(stream_, response, write_options, grpc::Status::OK, GetCallName());
    PostFinish(grpc::Status::OK);
}

template <typename Response>
bool OutputStream<Response>::IsFinished() const {
    return state_ == State::kFinished;
}

template <typename Request, typename Response>
BidirectionalStream<Request, Response>::BidirectionalStream(
    impl::CallParams&& call_params,
    impl::RawReaderWriter<Request, Response>& stream
)
    : CallAnyBase(utils::impl::InternalTag{}, std::move(call_params), impl::CallKind::kBidirectionalStream),
      stream_(stream) {}

template <typename Request, typename Response>
BidirectionalStream<Request, Response>::~BidirectionalStream() {
    if (!is_finished_) {
        impl::Cancel(stream_, GetCallName());
        WriteAccessLog(impl::kUnknownErrorStatus);
    }
}

template <typename Request, typename Response>
bool BidirectionalStream<Request, Response>::Read(Request& request) {
    UINVARIANT(!are_reads_done_, "'Read' called while the stream is half-closed for reads");
    if (impl::Read(stream_, request)) {
        if constexpr (std::is_base_of_v<google::protobuf::Message, Request>) {
            ApplyRequestHook(&request);
        }
        return true;
    } else {
        are_reads_done_ = true;
        return false;
    }
}

template <typename Request, typename Response>
void BidirectionalStream<Request, Response>::Write(Response&& response) {
    Write(response);
}

template <typename Request, typename Response>
void BidirectionalStream<Request, Response>::Write(Response& response) {
    UINVARIANT(!is_finished_, "'Write' called on a finished stream");
    if constexpr (std::is_base_of_v<google::protobuf::Message, Response>) {
        ApplyResponseHook(&response);
    }

    // Don't buffer writes, optimize for ping-pong-style interaction
    grpc::WriteOptions write_options{};

    try {
        impl::Write(stream_, response, write_options, GetCallName());
    } catch (const RpcInterruptedError&) {
        is_finished_ = true;
        throw;
    }
}

template <typename Request, typename Response>
void BidirectionalStream<Request, Response>::Finish() {
    UINVARIANT(!is_finished_, "'Finish' called on a finished stream");
    is_finished_ = true;

    WriteAccessLog(grpc::Status::OK);

    impl::Finish(stream_, grpc::Status::OK, GetCallName());
    PostFinish(grpc::Status::OK);
}

template <typename Request, typename Response>
void BidirectionalStream<Request, Response>::FinishWithError(const grpc::Status& status) {
    UASSERT(!status.ok());
    if (IsFinished()) return;
    is_finished_ = true;
    WriteAccessLog(status);
    impl::Finish(stream_, status, GetCallName());
    PostFinish(status);
}

template <typename Request, typename Response>
void BidirectionalStream<Request, Response>::WriteAndFinish(Response&& response) {
    WriteAndFinish(response);
}

template <typename Request, typename Response>
void BidirectionalStream<Request, Response>::WriteAndFinish(Response& response) {
    UINVARIANT(!is_finished_, "'WriteAndFinish' called on a finished stream");
    if constexpr (std::is_base_of_v<google::protobuf::Message, Response>) {
        ApplyResponseHook(&response);
    }

    // It is important to set is_finished_ after ApplyResponseHook.
    // Otherwise, there would be no way to call FinishWithError there.
    is_finished_ = true;

    // Don't buffer writes, optimize for ping-pong-style interaction
    grpc::WriteOptions write_options{};

    WriteAccessLog(grpc::Status::OK);

    impl::WriteAndFinish(stream_, response, write_options, grpc::Status::OK, GetCallName());
    PostFinish(grpc::Status::OK);
}

template <typename Request, typename Response>
bool BidirectionalStream<Request, Response>::IsFinished() const {
    return is_finished_;
}

}  // namespace ugrpc::server

USERVER_NAMESPACE_END
