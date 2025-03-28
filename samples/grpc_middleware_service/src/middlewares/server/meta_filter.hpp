#pragma once

#include <userver/utest/using_namespace_userver.hpp>

#include <userver/ugrpc/server/middlewares/base.hpp>

namespace sample::grpc::auth::server {

class MetaFilter final : public ugrpc::server::MiddlewareBase {
public:
    MetaFilter(std::vector<std::string> headers);

    void Handle(ugrpc::server::MiddlewareCallContext& context) const override;

private:
    const std::vector<std::string> headers_;
};

/// [gRPC middleware sample]
class MetaFilterComponent final : public ugrpc::server::MiddlewareFactoryComponentBase {
public:
    static constexpr std::string_view kName = "grpc-server-meta-filter";

    MetaFilterComponent(const components::ComponentConfig& config, const components::ComponentContext& context);

    static yaml_config::Schema GetStaticConfigSchema();

    // Needed to pass static config options to the middleware.
    yaml_config::Schema GetMiddlewareConfigSchema() const override;

    std::shared_ptr<MiddlewareBase> CreateMiddleware(
        const ugrpc::server::ServiceInfo&,
        const yaml_config::YamlConfig& middleware_config
    ) const override;
};
/// [gRPC middleware sample]

}  // namespace sample::grpc::auth::server
