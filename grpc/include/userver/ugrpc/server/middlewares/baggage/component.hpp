#pragma once

/// @file userver/ugrpc/server/middlewares/baggage/component.hpp
/// @brief @copybrief ugrpc::server::middlewares::baggage::Component

#include <userver/ugrpc/server/middlewares/baggage/middleware.hpp>

USERVER_NAMESPACE_BEGIN

/// Server baggage middleware
namespace ugrpc::server::middlewares::baggage {

// clang-format off

/// @ingroup userver_base_classes
///
/// @brief Component for gRPC server baggage
///
/// The component does **not** have any options for service config.
///
/// ## Static configuration example:
///
/// @snippet grpc/functional_tests/basic_chaos/static_config.yaml Sample grpc server baggage middleware component config

// clang-format on

using Component = SimpleMiddlewareFactoryComponent<Middleware>;

}  // namespace ugrpc::server::middlewares::baggage

template <>
inline constexpr auto components::kConfigFileMode<ugrpc::server::middlewares::baggage::Component> =
    ConfigFileMode::kNotRequired;

USERVER_NAMESPACE_END
