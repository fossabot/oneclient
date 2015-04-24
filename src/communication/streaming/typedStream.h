/**
 * @file typedStream.h
 * @author Konrad Zemek
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#ifndef HELPERS_COMMUNICATION_STREAMING_TYPED_STREAM_H
#define HELPERS_COMMUNICATION_STREAMING_TYPED_STREAM_H

#include "communication/declarations.h"

#include <tbb/concurrent_priority_queue.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <vector>

namespace one {
namespace communication {
namespace streaming {

struct StreamLess {
    bool operator()(const ClientMessagePtr &a, const ClientMessagePtr &b) const
    {
        return a->message_stream().sequence_number() <
            b->message_stream().sequence_number();
    }
};

/**
 * @c TypedStream provides outgoing message stream functionalities to an
 * existing communication stack.
 */
template <class Communicator> class TypedStream {
public:
    /**
     * Constructor.
     * @param communicator The communication stack used to send stream messages.
     * It must at least implement @c send(ClientMessagePtr, const int).
     * @param streamId ID number of this stream.
     */
    TypedStream(std::shared_ptr<Communicator> communicator,
        const std::uint64_t streamId, std::function<void()> unregister = [] {});

    /**
     * Destructor.
     * Closes the stream if not yet closed.
     */
    virtual ~TypedStream();

    /**
     * Sends a next message in the stream.
     * @param msg The message to send through the stream.
     * @note The @c int argument is provided for interface compatibility with
     * other communication layers. Stream's @c send always sets the retry
     * number to 0.
     */
    void send(ClientMessagePtr msg, const int);

    /**
     * Resends messages requested by the remote party.
     * @param msg Details of the request.
     */
    void handleMessageRequest(const clproto::MessageRequest &msg);

    /**
     * Removes already processed messages from an internal buffer.
     * @param msg Details of processed messages.
     */
    void handleMessageAcknowledgement(
        const clproto::MessageAcknowledgement &msg);

    /**
     * Closes the stream by sending an end-of-stream message.
     */
    void close();

    /**
     * Resets the stream's counters.
     */
    void reset();

    TypedStream(TypedStream &&) = delete;
    TypedStream(const TypedStream &) = delete;
    TypedStream &operator=(TypedStream &&) = delete;
    TypedStream &operator=(const TypedStream) = delete;

private:
    void saveAndPass(ClientMessagePtr msg);

    std::shared_ptr<Communicator> m_communicator;
    const std::uint64_t m_streamId;
    std::function<void()> m_unregister;
    std::atomic<std::uint64_t> m_sequenceId{0};
    std::shared_timed_mutex m_bufferMutex;
    tbb::concurrent_priority_queue<ClientMessagePtr, StreamLess> m_buffer;
};

template <class Communicator>
TypedStream<Communicator>::TypedStream(
    std::shared_ptr<Communicator> communicator, const uint64_t streamId,
    std::function<void()> unregister)
    : m_communicator{std::move(communicator)}
    , m_streamId{streamId}
    , m_unregister{std::move(unregister)}
{
}

template <class Communicator> TypedStream<Communicator>::~TypedStream()
{
    close();
    m_unregister();
}

template <class Communicator>
void TypedStream<Communicator>::send(ClientMessagePtr msg, const int)
{
    auto msgStream = msg->mutable_message_stream();
    msgStream->set_stream_id(m_streamId);
    msgStream->set_sequence_number(m_sequenceId++);
    saveAndPass(std::move(msg));
}

template <class Communicator> void TypedStream<Communicator>::close()
{
    auto msg = std::make_unique<clproto::ClientMessage>();
    msg->mutable_end_of_stream();
    send(std::move(msg), int{});
}

template <class Communicator> void TypedStream<Communicator>::reset()
{
    std::lock_guard<std::shared_timed_mutex> guard{m_bufferMutex};
    m_buffer.clear();
    m_sequenceId = 0;
}

template <class Communicator>
void TypedStream<Communicator>::saveAndPass(ClientMessagePtr msg)
{
    auto msgCopy = std::make_unique<clproto::ClientMessage>(*msg);

    std::shared_lock<std::shared_timed_mutex> lock{m_bufferMutex};
    m_buffer.emplace(std::move(msgCopy));
    lock.unlock();

    m_communicator->send(std::move(msg), 0);
}

template <class Communicator>
void TypedStream<Communicator>::handleMessageRequest(
    const clproto::MessageRequest &msg)
{
    std::vector<ClientMessagePtr> processed;
    processed.reserve(
        msg.upper_sequence_number() - msg.lower_sequence_number() + 1);

    std::shared_lock<std::shared_timed_mutex> lock{m_bufferMutex};
    for (ClientMessagePtr it; m_buffer.try_pop(it) &&
             it->message_stream().sequence_number() <=
                 msg.lower_sequence_number();)
        processed.emplace_back(std::move(it));

    for (auto &streamMsg : processed) {
        if (streamMsg->message_stream().sequence_number() >=
            msg.lower_sequence_number()) {

            saveAndPass(std::move(streamMsg));
        }
        else {
            m_buffer.emplace(std::move(streamMsg));
        }
    }
    lock.unlock();
}

template <class Communicator>
void TypedStream<Communicator>::handleMessageAcknowledgement(
    const clproto::MessageAcknowledgement &msg)
{
    std::shared_lock<std::shared_timed_mutex> lock{m_bufferMutex};
    for (ClientMessagePtr it; m_buffer.try_pop(it);) {
        if (it->message_stream().sequence_number() > msg.sequence_number()) {
            m_buffer.emplace(std::move(it));
            break;
        }
    }
}

} // namespace streaming
} // namespace one
} // namespace communication

#endif // HELPERS_COMMUNICATION_STREAMING_TYPED_STREAM_H