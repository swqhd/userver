#include <gtest/gtest.h>

#include <chrono>
#include <string_view>

#include <userver/engine/async.hpp>
#include <userver/engine/deadline.hpp>
#include <userver/engine/future_status.hpp>
#include <userver/engine/single_consumer_event.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/engine/task/task_with_result.hpp>
#include <userver/server/request/task_inherited_data.hpp>
#include <userver/utils/algo.hpp>

#include <ugrpc/client/impl/client_configs.hpp>
#include <ugrpc/server/impl/server_configs.hpp>
#include <userver/ugrpc/client/client_qos.hpp>
#include <userver/ugrpc/client/exceptions.hpp>
#include <userver/ugrpc/client/impl/completion_queue_pool.hpp>
#include <userver/ugrpc/client/middlewares/deadline_propagation/middleware.hpp>
#include <userver/ugrpc/server/middlewares/deadline_propagation/middleware.hpp>

#include <tests/deadline_helpers.hpp>
#include <tests/timed_out_service.hpp>
#include <tests/unit_test_client.usrv.pb.hpp>
#include <tests/unit_test_client_qos.hpp>
#include <tests/unit_test_service.usrv.pb.hpp>
#include <userver/ugrpc/tests/service_fixtures.hpp>

USERVER_NAMESPACE_BEGIN

namespace {

template <typename Call, typename Response>
void CheckSuccessRead(Call& call, Response& response, std::string_view result) {
    bool res = false;
    UEXPECT_NO_THROW(res = call.Read(response));
    EXPECT_EQ(result, response.name());
    EXPECT_TRUE(res);
}

template <typename Call, typename Request>
void CheckSuccessWrite(Call& call, Request& request, const char* message) {
    bool res = false;
    request.set_name(message);
    // NOLINTNEXTLINE(clang-analyzer-optin.cplusplus.UninitializedObject)
    UEXPECT_NO_THROW(res = call.Write(request));
    EXPECT_TRUE(res);
}

class GrpcDeadlinePropagation : public ugrpc::tests::ServiceFixture<tests::TimedOutUnitTestService> {
public:
    using ClientType = sample::ugrpc::UnitTestServiceClient;

    GrpcDeadlinePropagation()
        : client_deadline_(engine::Deadline::FromDuration(tests::kShortTimeout)),
          long_deadline_(engine::Deadline::FromDuration(tests::kLongTimeout)),
          client_(MakeClient<ClientType>()) {
        tests::InitTaskInheritedDeadline(client_deadline_);
    }

    ClientType& Client() { return client_; }

    void WaitClientDeadline() { tests::WaitUntilRpcDeadlineClient(client_deadline_); }

    ~GrpcDeadlinePropagation() override {
        EXPECT_TRUE(client_deadline_.IsReached());
        EXPECT_FALSE(long_deadline_.IsReached());
    }

private:
    engine::Deadline client_deadline_;
    engine::Deadline long_deadline_;
    ClientType client_;
};

}  // namespace

UTEST_F(GrpcDeadlinePropagation, TestClientUnaryCall) {
    sample::ugrpc::GreetingRequest request;
    request.set_name("userver");

    auto context = std::make_unique<grpc::ClientContext>();

    sample::ugrpc::GreetingResponse in;
    UEXPECT_THROW(in = Client().SayHello(request, std::move(context)), ugrpc::client::DeadlineExceededError);
}

UTEST_F(GrpcDeadlinePropagation, TestClientUnaryCallAsync) {
    sample::ugrpc::GreetingRequest request;
    request.set_name("userver");

    auto context = std::make_unique<grpc::ClientContext>();
    auto future = Client().AsyncSayHello(request, std::move(context));

    sample::ugrpc::GreetingResponse in;
    UEXPECT_THROW(in = future.Get(), ugrpc::client::DeadlineExceededError);
}

UTEST_F(GrpcDeadlinePropagation, TestClientUnaryCallAsyncWaitUntil) {
    sample::ugrpc::GreetingRequest request;
    request.set_name("userver");

    auto context = std::make_unique<grpc::ClientContext>();
    context->set_deadline(engine::Deadline::FromDuration(tests::kShortTimeout));
    auto deadline = engine::Deadline::FromDuration(tests::kShortTimeout / 100);

    auto future = Client().AsyncSayHello(request, std::move(context));

    EXPECT_EQ(future.WaitUntil(deadline), engine::FutureStatus::kTimeout);

    sample::ugrpc::GreetingResponse in;
    UEXPECT_THROW(in = future.Get(), ugrpc::client::DeadlineExceededError);
}

UTEST_F(GrpcDeadlinePropagation, TestClientReadManyRead) {
    sample::ugrpc::StreamGreetingRequest request;
    request.set_name("userver");

    auto context = std::make_unique<grpc::ClientContext>();
    auto call = Client().ReadMany(request, std::move(context));

    sample::ugrpc::StreamGreetingResponse response;
    bool res = false;

    CheckSuccessRead(call, response, "One userver");
    CheckSuccessRead(call, response, "Two userver");
    CheckSuccessRead(call, response, "Three userver");

    UEXPECT_THROW(res = call.Read(response), ugrpc::client::DeadlineExceededError);
}

UTEST_F(GrpcDeadlinePropagation, TestClientWriteManyWriteAndCheck) {
    sample::ugrpc::StreamGreetingRequest request;

    auto context = std::make_unique<grpc::ClientContext>();
    auto call = Client().WriteMany(std::move(context));

    sample::ugrpc::StreamGreetingResponse response;

    CheckSuccessWrite(call, request, tests::kRequests[0]);
    CheckSuccessWrite(call, request, tests::kRequests[1]);

    WaitClientDeadline();

    request.set_name(tests::kRequests[2]);
    UEXPECT_THROW(call.WriteAndCheck(request), ugrpc::client::DeadlineExceededError);
}

UTEST_F(GrpcDeadlinePropagation, TestClientWriteManyFinish) {
    sample::ugrpc::StreamGreetingRequest request;
    auto context = std::make_unique<grpc::ClientContext>();
    auto call = Client().WriteMany(std::move(context));

    sample::ugrpc::StreamGreetingResponse response;

    CheckSuccessWrite(call, request, tests::kRequests[0]);
    CheckSuccessWrite(call, request, tests::kRequests[1]);
    CheckSuccessWrite(call, request, tests::kRequests[2]);

    UEXPECT_THROW(response = call.Finish(), ugrpc::client::DeadlineExceededError);
}

// Scenario for Chat tests (Read, ReadAsync, Write, WriteAndCheck):
// Client Write x3, WritesDone
// Server Read x3, Write x2, Time consuming calculations, Write
// Client Read x3

UTEST_F(GrpcDeadlinePropagation, TestClientChatWrite) {
    sample::ugrpc::StreamGreetingRequest request;
    sample::ugrpc::StreamGreetingResponse response;
    auto context = std::make_unique<grpc::ClientContext>();

    auto call = Client().Chat(std::move(context));

    WaitClientDeadline();
    bool res{false};
    // I don't really like that in other methods exception is thrown, but here we
    // have False as a result
    UEXPECT_NO_THROW(res = call.Write(request));
    EXPECT_FALSE(res);
}

UTEST_F(GrpcDeadlinePropagation, TestClientChatRead) {
    std::vector<sample::ugrpc::StreamGreetingRequest> requests(3);
    sample::ugrpc::StreamGreetingResponse response;
    auto context = std::make_unique<grpc::ClientContext>();

    auto call = Client().Chat(std::move(context));

    bool res = false;

    for (size_t i = 0; i < requests.size(); ++i) {
        CheckSuccessWrite(call, requests[i], tests::kRequests[i]);
    }

    ASSERT_TRUE(call.WritesDone());

    CheckSuccessRead(call, response, "One request1");
    CheckSuccessRead(call, response, "Two request2");

    UEXPECT_THROW(res = call.Read(response), ugrpc::client::DeadlineExceededError);
}

UTEST_F(GrpcDeadlinePropagation, TestClientChatReadAsync) {
    std::vector<sample::ugrpc::StreamGreetingRequest> requests(3);
    sample::ugrpc::StreamGreetingResponse response;
    auto context = std::make_unique<grpc::ClientContext>();

    auto call = Client().Chat(std::move(context));

    for (size_t i = 0; i < requests.size(); ++i) {
        CheckSuccessWrite(call, requests[i], tests::kRequests[i]);
    }

    ASSERT_TRUE(call.WritesDone());

    CheckSuccessRead(call, response, "One request1");
    CheckSuccessRead(call, response, "Two request2");

    auto future = call.ReadAsync(response);
    UEXPECT_THROW(future.Get(), ugrpc::client::DeadlineExceededError);
}

namespace {

class UnitTestInheritedDeadline final : public sample::ugrpc::UnitTestServiceBase {
public:
    SayHelloResult SayHello(CallContext& /*context*/, sample::ugrpc::GreetingRequest&& request) override {
        const auto& inherited_data = server::request::kTaskInheritedData.Get();

        EXPECT_TRUE(inherited_data.deadline.IsReachable());
        EXPECT_EQ(inherited_data.path, "sample.ugrpc.UnitTestService");
        EXPECT_EQ(inherited_data.method, "SayHello");

        EXPECT_GT(initial_client_timeout_, engine::Deadline::Duration{}) << "Not initialized";

        const auto inherited_time_left = inherited_data.deadline.TimeLeft();
        EXPECT_GT(initial_client_timeout_, inherited_time_left)
            << "initial_client_timeout_=" << initial_client_timeout_.count()
            << " vs. inherited_time_left=" << inherited_time_left.count();

        sample::ugrpc::GreetingResponse response;
        response.set_name("Hello " + request.name());

        return response;
    }

    void SetClientInitialTimeout(engine::Deadline::Duration value) { initial_client_timeout_ = value; }

private:
    engine::Deadline::Duration initial_client_timeout_{};
};

using GrpcTestInheritedDedline = ugrpc::tests::ServiceFixture<UnitTestInheritedDeadline>;

}  // namespace

UTEST_F(GrpcTestInheritedDedline, TestServerDataExist) {
    auto client = MakeClient<sample::ugrpc::UnitTestServiceClient>();
    sample::ugrpc::GreetingRequest out;
    out.set_name("userver");

    auto context = std::make_unique<grpc::ClientContext>();
    engine::Deadline deadline = engine::Deadline::FromDuration(tests::kLongTimeout);

    GetService().SetClientInitialTimeout(tests::kLongTimeout);
    context->set_deadline(deadline);

    // In tests the gpr_timespec <> steady_clock conversions were giving
    // ~0.5ms precision loss once in 10k runs. Thus the 10ms delay.
    engine::SleepFor(std::chrono::milliseconds{10});

    sample::ugrpc::GreetingResponse in;
    UEXPECT_NO_THROW(in = client.SayHello(out, std::move(context)));
    EXPECT_EQ("Hello " + out.name(), in.name());
}

UTEST_F(GrpcTestInheritedDedline, TestDeadlineExpiresBeforeCall) {
    auto client = MakeClient<sample::ugrpc::UnitTestServiceClient>();
    sample::ugrpc::GreetingRequest out;
    out.set_name("userver");

    auto context = std::make_unique<grpc::ClientContext>();
    engine::Deadline deadline = engine::Deadline::FromDuration(tests::kShortTimeout);
    context->set_deadline(deadline);

    // Test that the time between client context
    // construction and client request is measured.
    engine::SleepFor(tests::kLongTimeout);

    sample::ugrpc::GreetingResponse in;
    UEXPECT_THROW(in = client.SayHello(out, std::move(context)), ugrpc::client::DeadlineExceededError);
}

namespace {

class UnitTestClientNotSend final : public sample::ugrpc::UnitTestServiceBase {
public:
    SayHelloResult SayHello(CallContext& /*context*/, sample::ugrpc::GreetingRequest&& /*request*/) override {
        ADD_FAILURE();
        return sample::ugrpc::GreetingResponse{};
    }
};

using GrpcTestClientNotSendData = ugrpc::tests::ServiceFixture<UnitTestClientNotSend>;

}  // namespace

UTEST_F(GrpcTestClientNotSendData, TestClientDoNotStartCallWithoutDeadline) {
    auto client = MakeClient<sample::ugrpc::UnitTestServiceClient>();

    auto task_deadline = engine::Deadline::FromDuration(tests::kShortTimeout);
    tests::InitTaskInheritedDeadline(task_deadline);

    sample::ugrpc::GreetingRequest request;
    request.set_name("userver");

    // Wait for deadline before request
    tests::WaitUntilRpcDeadlineClient(task_deadline);

    // Context deadline not set
    sample::ugrpc::GreetingResponse in;
    UEXPECT_THROW(
        in = client.SayHello(request, tests::MakeClientContext(/*set_deadline=*/false)),
        ugrpc::client::DeadlineExceededError
    );
}

UTEST_F(GrpcTestClientNotSendData, TestClientDoNotStartCallWithDeadline) {
    auto client = MakeClient<sample::ugrpc::UnitTestServiceClient>();

    auto task_deadline = engine::Deadline::FromDuration(tests::kShortTimeout);
    tests::InitTaskInheritedDeadline(task_deadline);

    sample::ugrpc::GreetingRequest request;
    request.set_name("userver");

    // Wait for deadline before request
    tests::WaitUntilRpcDeadlineClient(task_deadline);

    // Set additional client deadline
    sample::ugrpc::GreetingResponse in;
    UEXPECT_THROW(
        in = client.SayHello(request, tests::MakeClientContext(/*set_deadline=*/true)),
        ugrpc::client::DeadlineExceededError
    );
}

namespace {

class UnitTestClientInfinite final : public sample::ugrpc::UnitTestServiceBase {
public:
    SayHelloResult SayHello(CallContext& /*context*/, sample::ugrpc::GreetingRequest&& /*request*/) override {
        engine::InterruptibleSleepFor(utest::kMaxTestWaitTime);
        return sample::ugrpc::GreetingResponse{};
    }
};

using UnitTestClientInfiniteTest = ugrpc::tests::ServiceFixture<UnitTestClientInfinite>;

}  // namespace

UTEST_F(UnitTestClientInfiniteTest, NegativeDeadline) {
    auto client = MakeClient<sample::ugrpc::UnitTestServiceClient>();

    auto context = std::make_unique<grpc::ClientContext>();
    context->set_deadline(engine::Deadline::FromDuration(std::chrono::milliseconds{-10}));

    auto task = engine::AsyncNoSpan([&] { return client.SayHello({}, std::move(context)); });

    // Check that SayHello did not hang.
    const auto wait_status = task.WaitNothrowUntil(engine::Deadline::FromDuration(utest::kMaxTestWaitTime / 2));
    ASSERT_EQ(wait_status, engine::FutureStatus::kReady);

    UEXPECT_THROW(task.Get(), ugrpc::client::DeadlineExceededError);
}

USERVER_NAMESPACE_END
