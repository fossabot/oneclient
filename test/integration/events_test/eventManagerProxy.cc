/**
 * @file eventManagerProxy.cc
 * @author Krzysztof Trzepla
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#include "context.h"
#include "communication/communicator.h"
#include "events/eventManager.h"
#include "scheduler.h"

#include "messages.pb.h"

#include <boost/python.hpp>
#include <boost/make_shared.hpp>

#include <map>
#include <atomic>
#include <memory>

using namespace one;
using namespace one::client;
using namespace one::client::events;
using namespace one::communication;
using namespace boost::python;

class EventManagerProxy {
public:
    EventManagerProxy(const unsigned int threadsNumber,
        const unsigned int connectionsNumber, std::string host,
        const unsigned short port)
    {
        auto ctx = std::make_shared<Context>();
        auto sched = std::make_shared<Scheduler>(threadsNumber);
        m_comm = std::make_shared<Communicator>(connectionsNumber,
            std::move(host), port, false, communication::createConnection);
        m_comm->connect();
        ctx->setScheduler(std::move(sched));
        ctx->setCommunicator(m_comm);
        m_evtMan = std::make_unique<EventManager>(std::move(ctx));
    }

    ~EventManagerProxy() { m_comm->stop(); }

    uint64_t emitReadEvent(std::string fileId, off_t offset, size_t size)
    {
        m_evtMan->emitReadEvent(std::move(fileId), offset, size);
        return m_seqNum++;
    }

    uint64_t emitWriteEvent(
        std::string fileId, off_t offset, size_t size, off_t fileSize)
    {
        m_evtMan->emitWriteEvent(std::move(fileId), offset, size, fileSize);
        return m_seqNum++;
    }

    uint64_t emitTruncateEvent(std::string fileId, off_t fileSize)
    {
        m_evtMan->emitTruncateEvent(std::move(fileId), fileSize);
        return m_seqNum++;
    }

private:
    std::atomic<uint64_t> m_seqNum{0};
    std::shared_ptr<Communicator> m_comm;
    std::unique_ptr<EventManager> m_evtMan;
};

one::clproto::ClientMessage createStreamMessage(uint64_t seqNum)
{
    one::clproto::ClientMessage cliMsg{};
    auto stmMsg = cliMsg.mutable_message_stream();
    stmMsg->set_stream_id(0);
    stmMsg->set_sequence_number(seqNum);
    return std::move(cliMsg);
}

std::string createReadEventMsg(
    std::size_t ctr, std::string fileId, list blocks, uint64_t seqNum)
{
    auto cliMsg = createStreamMessage(seqNum);
    auto evtMsg = cliMsg.mutable_event();
    auto readEvtMsg = evtMsg->mutable_read_event();
    readEvtMsg->set_counter(ctr);
    readEvtMsg->set_file_id(std::move(fileId));
    std::size_t size = 0;
    for (int i = 0; i < len(blocks); ++i) {
        off_t blockOffset = extract<off_t>(blocks[i][0]);
        std::size_t blockSize = extract<std::size_t>(blocks[i][1]);
        auto blockMsg = readEvtMsg->add_blocks();
        blockMsg->set_offset(blockOffset);
        blockMsg->set_size(blockSize);
        size += blockSize;
    }
    readEvtMsg->set_size(size);
    return cliMsg.SerializeAsString();
}

std::string createWriteEventMsg(std::size_t ctr, std::string fileId,
    std::size_t fileSize, list blocks, uint64_t seqNum)
{
    auto cliMsg = createStreamMessage(seqNum);
    auto evtMsg = cliMsg.mutable_event();
    auto writeEvtMsg = evtMsg->mutable_write_event();
    writeEvtMsg->set_counter(ctr);
    writeEvtMsg->set_file_id(std::move(fileId));
    writeEvtMsg->set_file_size(fileSize);
    std::size_t size = 0;
    for (int i = 0; i < len(blocks); ++i) {
        off_t blockOffset = extract<off_t>(blocks[i][0]);
        std::size_t blockSize = extract<std::size_t>(blocks[i][1]);
        auto blockMsg = writeEvtMsg->add_blocks();
        blockMsg->set_offset(blockOffset);
        blockMsg->set_size(blockSize);
        size += blockSize;
    }
    writeEvtMsg->set_size(size);
    return cliMsg.SerializeAsString();
}

std::string createTruncateEventMsg(
    std::size_t ctr, std::string fileId, size_t fileSize, uint64_t seqNum)
{
    auto cliMsg = createStreamMessage(seqNum);
    auto evtMsg = cliMsg.mutable_event();
    auto truncateEvtMsg = evtMsg->mutable_write_event();
    truncateEvtMsg->set_counter(ctr);
    truncateEvtMsg->set_file_id(std::move(fileId));
    truncateEvtMsg->set_size(0);
    truncateEvtMsg->set_file_size(fileSize);
    return cliMsg.SerializeAsString();
}

std::string createReadEventSubscriptionMsg(
    uint64_t id, size_t ctrThr, uint64_t timeThr, size_t sizeThr)
{
    auto srvMsg = std::make_unique<one::clproto::ServerMessage>();
    auto evtSubMsg = srvMsg->mutable_event_subscription();
    auto readEvtSubMsg = evtSubMsg->mutable_read_event_subscription();
    readEvtSubMsg->set_id(id);
    if (ctrThr > 0)
        readEvtSubMsg->set_counter_threshold(ctrThr);
    if (timeThr > 0)
        readEvtSubMsg->set_time_threshold(timeThr);
    if (sizeThr > 0)
        readEvtSubMsg->set_size_threshold(sizeThr);
    return srvMsg->SerializeAsString();
}

std::string createWriteEventSubscriptionMsg(
    uint64_t id, size_t ctrThr, uint64_t timeThr, size_t sizeThr)
{
    auto srvMsg = std::make_unique<one::clproto::ServerMessage>();
    auto evtSubMsg = srvMsg->mutable_event_subscription();
    auto writeEvtSubMsg = evtSubMsg->mutable_write_event_subscription();
    writeEvtSubMsg->set_id(id);
    if (ctrThr > 0)
        writeEvtSubMsg->set_counter_threshold(ctrThr);
    if (timeThr > 0)
        writeEvtSubMsg->set_time_threshold(timeThr);
    if (sizeThr > 0)
        writeEvtSubMsg->set_size_threshold(sizeThr);
    return srvMsg->SerializeAsString();
}

std::string createEventSubscriptionCancellationMsg(uint64_t id)
{
    auto srvMsg = std::make_unique<one::clproto::ServerMessage>();
    auto evtSubMsg = srvMsg->mutable_event_subscription();
    auto evtSubCanMsg = evtSubMsg->mutable_event_subscription_cancellation();
    evtSubCanMsg->set_id(id);
    return srvMsg->SerializeAsString();
}

namespace {
boost::shared_ptr<EventManagerProxy> create(const unsigned int threadsNumber,
    const unsigned int connectionsNumber, std::string host,
    const unsigned short port)
{
    return boost::make_shared<EventManagerProxy>(
        threadsNumber, connectionsNumber, std::move(host), port);
}
}

BOOST_PYTHON_MODULE(events)
{
    class_<EventManagerProxy, boost::noncopyable>("EventManager", no_init)
        .def("__init__", make_constructor(create))
        .def("emitReadEvent", &EventManagerProxy::emitReadEvent)
        .def("emitWriteEvent", &EventManagerProxy::emitWriteEvent)
        .def("emitTruncateEvent", &EventManagerProxy::emitTruncateEvent);

    def("createReadEventMsg", &createReadEventMsg);
    def("createWriteEventMsg", &createWriteEventMsg);
    def("createTruncateEventMsg", &createTruncateEventMsg);

    def("createReadEventSubscriptionMsg", &createReadEventSubscriptionMsg);
    def("createWriteEventSubscriptionMsg", &createWriteEventSubscriptionMsg);
    def("createEventSubscriptionCancellationMsg",
        &createEventSubscriptionCancellationMsg);
}
