/**
 * @file eventCommunicator.h
 * @author Krzysztof Trzepla
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#ifndef ONECLIENT_EVENTS_EVENT_COMMUNICATOR_H
#define ONECLIENT_EVENTS_EVENT_COMMUNICATOR_H

#include "communication/communicator.h"
#include "communication/streaming/streamManager.h"

#include <memory>

namespace one {
namespace client {

class Context;

namespace events {

class Event;

/**
 * The EventCommunicator class is responsible for sending event messages to the
 * server using communication stream.
 */
class EventCommunicator {
public:
    /**
     * Constructor.
     * @param context A @c Context instance used to acquire communication
     * stream.
     */
    EventCommunicator(std::shared_ptr<Context> ctx);

    virtual ~EventCommunicator();

    /**
     * Sends event to the server using communication stream.
     * @param evt Event to be sent.
     */
    virtual void send(const Event &evt) const;

private:
    std::unique_ptr<communication::StreamManager> m_stmManager;
    std::shared_ptr<communication::StreamManager::Stream> m_stm;
};

} // namespace events
} // namespace client
} // namespace one

#endif // ONECLIENT_EVENTS_EVENT_COMMUNICATOR_H
