/**
 * @file configuration.cc
 * @author Krzysztof Trzepla
 * @copyright (C) 2016 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#include "configuration.h"

#include "messages.pb.h"

#include <sstream>

namespace one {
namespace messages {

Configuration::Configuration(
    std::unique_ptr<ProtocolServerMessage> serverMessage)
{
    auto &configurationMsg = serverMessage->configuration();
    for (const auto &subscription : configurationMsg.subscriptions())
        m_subscriptionContainer.add(subscription);
}

client::events::SubscriptionContainer Configuration::subscriptionContainer()
{
    return m_subscriptionContainer;
}

std::string Configuration::toString() const
{
    std::stringstream stream;
    stream << "type: 'Configuration', subscriptions: "
           << m_subscriptionContainer.toString();
    return stream.str();
}

} // namespace messages
} // namespace one