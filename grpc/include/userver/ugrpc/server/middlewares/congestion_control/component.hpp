#pragma once

/// @file userver/ugrpc/server/middlewares/congestion_control/component.hpp
/// @brief @copybrief ugrpc::server::middlewares::congestion_control::Component

#include <userver/ugrpc/server/middlewares/base.hpp>

USERVER_NAMESPACE_BEGIN

/// Server congestion_control middleware
namespace ugrpc::server::middlewares::congestion_control {

class Middleware;

// clang-format off

/// @ingroup userver_components userver_base_classes
///
/// @brief Component for gRPC server logging
///
/// The component does **not** have any options for service config.
///
/// ## Static configuration example:
///
/// @snippet grpc/functional_tests/basic_chaos/static_config.yaml Sample grpc server congestion control middleware component config

// clang-format on

class Component final : public MiddlewareFactoryComponentBase {
public:
    /// @ingroup userver_component_names
    /// @brief The default name of
    /// ugrpc::server::middlewares::congestion_control::Component
    static constexpr std::string_view kName = "grpc-server-congestion-control";

    Component(const components::ComponentConfig& config, const components::ComponentContext& context);

    std::shared_ptr<MiddlewareBase> CreateMiddleware(
        const ugrpc::server::ServiceInfo&,
        const yaml_config::YamlConfig& middleware_config
    ) const override;

private:
    std::shared_ptr<Middleware> middleware_;
};

}  // namespace ugrpc::server::middlewares::congestion_control

template <>
inline constexpr bool components::kHasValidate<ugrpc::server::middlewares::congestion_control::Component> = true;

template <>
inline constexpr auto components::kConfigFileMode<ugrpc::server::middlewares::congestion_control::Component> =
    ConfigFileMode::kNotRequired;

USERVER_NAMESPACE_END
