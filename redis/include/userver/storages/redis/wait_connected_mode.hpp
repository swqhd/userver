#pragma once

/// @file
/// @brief @copybrief storages::redis::RedisWaitConnected

#include <chrono>
#include <string>

#include <userver/formats/parse/to.hpp>
#include <userver/testsuite/redis_control.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::redis {

/// Default timeout for Redis connection wait
inline constexpr auto kRedisWaitConnectedDefaultTimeout = std::chrono::seconds(11);

/// Connection wait mode for the Redis
enum class WaitConnectedMode {
    kNoWait,          ///< Do not wait.
    kMaster,          ///< If we need to write to redis.
    kMasterOrSlave,   ///< Enough for reading data from redis.
    kSlave,           ///< It may be no slaves on unstable. Waiting can always fail.
    kMasterAndSlave,  ///< It may be no slaves on unstable.
};

std::string ToString(WaitConnectedMode mode);

WaitConnectedMode Parse(const std::string& str, formats::parse::To<WaitConnectedMode>);

/// Connection wait options for the Redis; mostly used by storages::redis::Client::WaitConnectedOnce.
struct RedisWaitConnected {
    WaitConnectedMode mode{WaitConnectedMode::kNoWait};
    bool throw_on_fail{false};
    std::chrono::milliseconds timeout{kRedisWaitConnectedDefaultTimeout};

    RedisWaitConnected MergeWith(const testsuite::RedisControl& t) const;
};

}  // namespace storages::redis

USERVER_NAMESPACE_END
