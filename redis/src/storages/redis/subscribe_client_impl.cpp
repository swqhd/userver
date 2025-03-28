#include "subscribe_client_impl.hpp"

#include <storages/redis/impl/subscribe_sentinel.hpp>
#include <storages/redis/subscription_token_impl.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::redis {

SubscribeClient::~SubscribeClient() = default;

SubscribeClientImpl::SubscribeClientImpl(std::shared_ptr<impl::SubscribeSentinel> subscribe_sentinel)
    : redis_client_(std::move(subscribe_sentinel)) {}

SubscriptionToken SubscribeClientImpl::Subscribe(
    std::string channel,
    SubscriptionToken::OnMessageCb on_message_cb,
    const CommandControl& command_control
) {
    return {std::make_unique<SubscriptionTokenImpl>(
        *redis_client_, std::move(channel), std::move(on_message_cb), command_control
    )};
}

SubscriptionToken SubscribeClientImpl::Psubscribe(
    std::string pattern,
    SubscriptionToken::OnPmessageCb on_pmessage_cb,
    const CommandControl& command_control
) {
    return {std::make_unique<PsubscriptionTokenImpl>(
        *redis_client_, std::move(pattern), std::move(on_pmessage_cb), command_control
    )};
}

SubscriptionToken SubscribeClientImpl::Ssubscribe(
    std::string channel,
    SubscriptionToken::OnMessageCb on_message_cb,
    const CommandControl& command_control
) {
    return {std::make_unique<SsubscriptionTokenImpl>(
        *redis_client_, std::move(channel), std::move(on_message_cb), command_control
    )};
}

void SubscribeClientImpl::WaitConnectedOnce(RedisWaitConnected wait_connected) {
    redis_client_->WaitConnectedOnce(wait_connected);
}

size_t SubscribeClientImpl::ShardsCount() const { return redis_client_->ShardsCount(); }

bool SubscribeClientImpl::IsInClusterMode() const { return redis_client_->IsInClusterMode(); }

impl::SubscribeSentinel& SubscribeClientImpl::GetNative() const { return *redis_client_; }

}  // namespace storages::redis

USERVER_NAMESPACE_END
