#include <userver/ugrpc/server/middlewares/headers_propagator/component.hpp>

#include <userver/components/component_config.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

#include <ugrpc/server/middlewares/headers_propagator/middleware.hpp>

USERVER_NAMESPACE_BEGIN

namespace ugrpc::server::middlewares::headers_propagator {

Component::Component(const components::ComponentConfig& config, const components::ComponentContext& context)
    : MiddlewareFactoryComponentBase(config, context) {}

std::shared_ptr<MiddlewareBase>
Component::CreateMiddleware(const ugrpc::server::ServiceInfo&, const yaml_config::YamlConfig& middleware_config) const {
    return std::make_shared<Middleware>(middleware_config["headers"].As<std::vector<std::string>>({}));
}

yaml_config::Schema Component::GetMiddlewareConfigSchema() const { return GetStaticConfigSchema(); }

yaml_config::Schema Component::GetStaticConfigSchema() {
    return yaml_config::MergeSchemas<MiddlewareFactoryComponentBase>(R"(
type: object
description: gRPC service metadata fields (headers) propagator middleware component
additionalProperties: false
properties:
    headers:
        type: array
        description: array of metadata fields(headers) to propagate
        items:
            type: string
            description: header
)");
}

}  // namespace ugrpc::server::middlewares::headers_propagator

USERVER_NAMESPACE_END
