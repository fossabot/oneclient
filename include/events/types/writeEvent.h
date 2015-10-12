/**
 * @file writeEvent.h
 * @author Krzysztof Trzepla
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#ifndef ONECLIENT_EVENTS_TYPES_WRITE_EVENT_H
#define ONECLIENT_EVENTS_TYPES_WRITE_EVENT_H

#include "event.h"

#include <boost/icl/interval_set.hpp>

#include <sys/types.h>

#include <string>
#include <memory>

namespace one {

namespace clproto {
class WriteEventSubscription;
}

namespace client {
namespace events {

/**
 * The WriteEvent class represents a write operation in the file system.
 */
class WriteEvent : public Event {
public:
    static const std::string name;

    typedef typename one::clproto::WriteEventSubscription Subscription;

    /**
     * Default constructor.
     * Creates identity element for write events aggregation operation.
     */
    WriteEvent()
        : Event{0}
    {
    }

    /**
     * Constructor.
     * @param fileId ID of file associated with a write operation.
     * @param offset Distance from the beginning of the file to the first
     * byte written.
     * @param size Number of bytes written.
     * @param fileSize Size of file after a write operation.
     */
    WriteEvent(
        std::string fileId, off_t offset, std::size_t size, off_t fileSize);

    /**
     * @return ID of file associated with the write event.
     */
    const std::string &fileId() const;

    /**
     * @return Number of bytes written.
     */
    std::size_t size() const;

    /**
     * @return Size of file associated with the write event.
     */
    off_t fileSize() const;

    /**
     * @return Set of bytes blocks written.
     */
    const boost::icl::interval_set<off_t> &blocks() const;

    /**
     * Aggregates this write event with an other write event.
     * Aggregates @c this event with an other write event.
     * Aggregation is done by:
     * - addition of events' counters
     * - addition of events' sizes
     * - union of sets of write blocks
     * - substitution of file size with file size associated with other event
     * @param event Write event to be aggregated.
     * @return @c *this
     */
    WriteEvent &operator+=(const WriteEvent &event);

    /**
     * Compares this write event with an other write event.
     * @param evt Write event to be compared.
     * @return 'true' if ID of file associated with this event is
     * lexicographically less than the file ID associated with an other event,
     * otherwise 'false'
     */
    bool operator<(const WriteEvent &evt);

    virtual std::string toString() const override;

    virtual std::unique_ptr<one::messages::ProtocolClientMessage>
    serialize() const override;

protected:
    std::string m_fileId;
    std::size_t m_size = 0;
    off_t m_fileSize = 0;
    boost::icl::interval_set<off_t> m_blocks;
};

} // namespace events
} // namespace client
} // namespace one

#endif // ONECLIENT_EVENTS_TYPES_WRITE_EVENT_H
