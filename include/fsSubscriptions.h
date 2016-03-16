/**
 * @file fsSubscriptions.h
 * @author Krzysztof Trzepla
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#ifndef ONECLIENT_FS_SUBSCRIPTIONS_H
#define ONECLIENT_FS_SUBSCRIPTIONS_H

#include <tbb/concurrent_hash_map.h>

#include <chrono>
#include <cstddef>
#include <string>

namespace one {
class Scheduler;
namespace client {
namespace events {
class EventManager;
} // namespace events

constexpr std::chrono::seconds FILE_ATTR_SUBSCRIPTION_DURATION{30};

class FsSubscriptions {
public:
    /**
     * Constructor.
     * @param eventManager @c EventManager instance.
     */
    FsSubscriptions(events::EventManager &eventManager);

    /**
     * Adds subscription for file location updates.
     */
    void addFileLocationSubscription(const std::string &fileUuid);

    /**
     * Removes subscription for file location updates.
     * @param fileUuid UUID of file for which subscription is removed.
     */
    void removeFileLocationSubscription(const std::string &fileUuid);

    /**
     * Adds subscription for permission updates.
     * @param fileUuid UUID of file for which subscription is added.
     */
    virtual void addPermissionChangedSubscription(const std::string &fileUuid);

    /**
     * Removes subscription for permission updates.
     * @param fileUuid UUID of file for which subscription is removed.
     */
    virtual void removePermissionChangedSubscription(
        const std::string &fileUuid);

    /**
     * Adds subscription for file attributes updates.
     * @param fileUuid UUID of file for which subscription is added.
     */
    void addFileAttrSubscription(const std::string &fileUuid);

private:
    void removeFileAttrSubscription(const std::string &fileUuid);
    std::int64_t sendFileAttrSubscription(const std::string &fileUuid);
    std::int64_t sendFileLocationSubscription(const std::string &fileUuid);
    std::int64_t sendPermissionChangedSubscription(const std::string &fileUuid);
    void sendSubscriptionCancellation(std::int64_t id);

    events::EventManager &m_eventManager;
    tbb::concurrent_hash_map<std::string, std::int64_t>
        m_fileAttrSubscriptions;
    tbb::concurrent_hash_map<std::string, std::int64_t>
        m_fileLocationSubscriptions;
    tbb::concurrent_hash_map<std::string, std::int64_t>
        m_permissionChangedSubscriptions;
};

} // namespace client
} // namespace one

#endif // ONECLIENT_FS_SUBSCRIPTIONS_H
