#include <iostream>
#include <userver/clients/http/component.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/server/handlers/tests_control.hpp>
#include <userver/testsuite/testsuite_support.hpp>
#include <userver/ugrpc/server/server_component.hpp>
#include <userver/utils/daemon_run.hpp>

#include <greeter_client.usrv.pb.hpp>
#include <greeter_service.usrv.pb.hpp>

#include "hello.hpp"

namespace samples {

class GreeterServiceComponent final : public api::GreeterServiceBase::Component {
public:
    static constexpr std::string_view kName = "greeter-service";

    GreeterServiceComponent(
        const userver::components::ComponentConfig& config,
        const userver::components::ComponentContext& context
    )
        : api::GreeterServiceBase::Component(config, context), prefix_(config["greeting-prefix"].As<std::string>()) {}

    SayHelloResult SayHello(CallContext& context, api::GreetingRequest&& request) override;

private:
    const std::string prefix_;
};

GreeterServiceComponent::SayHelloResult
GreeterServiceComponent::SayHello(CallContext& /*context*/, api::GreetingRequest&& request) {
    api::GreetingResponse response;
    response.set_greeting(fmt::format("{}, {}!", prefix_, request.name()));

    return response;
}

}  // namespace samples

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
                              .Append<userver::server::handlers::Ping>()
                              .Append<userver::components::TestsuiteSupport>()
                              .Append<userver::components::HttpClient>()
                              .Append<userver::server::handlers::TestsControl>()
                              .Append<userver::ugrpc::server::ServerComponent>("")
                              .Append<samples::GreeterServiceComponent>();

    service_template::AppendHello(component_list);

    auto size = std::distance(component_list.begin(), component_list.end());
    std::cout << size << std::endl;

    return 0;
}
