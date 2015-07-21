/**
 * @file writeEvent.cc
 * @author Krzysztof Trzepla
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#include "events/types/writeEvent.h"

#include "events/eventStream.h"
#include "messages.pb.h"

#include <sstream>

namespace one {
namespace client {
namespace events {

WriteEvent::WriteEvent()
    : Event{0}
{
}

WriteEvent::WriteEvent(std::string fileId_, off_t offset_, std::size_t size_,
    off_t fileSize_, std::size_t counter_)
    : Event{counter_}
    , m_fileId{std::move(fileId_)}
    , m_size{size_}
    , m_fileSize{fileSize_}
    , m_blocks{boost::icl::discrete_interval<off_t>::right_open(
          offset_, offset_ + size_)}
{
}

const std::string &WriteEvent::fileId() const { return m_fileId; }

size_t WriteEvent::size() const { return m_size; }

off_t WriteEvent::fileSize() const { return m_fileSize; }

const boost::icl::interval_set<off_t> &WriteEvent::blocks() const
{
    return m_blocks;
}

bool operator==(const WriteEvent &lhs, const WriteEvent &rhs)
{
    return lhs.fileId() == rhs.fileId() && lhs.size() == rhs.size() &&
        lhs.fileSize() == rhs.fileSize() && lhs.blocks() == rhs.blocks();
}

WriteEvent &WriteEvent::operator+=(const WriteEvent &event)
{
    m_counter += event.m_counter;
    m_size += event.m_size;
    m_fileSize = event.m_fileSize;
    m_blocks += event.m_blocks;
    m_blocks &= boost::icl::discrete_interval<off_t>::right_open(0, m_fileSize);
    return *this;
}

std::string WriteEvent::toString() const
{
    std::stringstream stream;
    stream << "type: 'WriteEvent', counter: " << m_counter << ", file ID: '"
           << m_fileId << "', file size: " << m_fileSize << ", size: " << m_size
           << ", blocks: " << m_blocks;
    return stream.str();
}

std::unique_ptr<one::messages::ProtocolClientMessage>
WriteEvent::serialize() const
{
    auto clientMsg = std::make_unique<one::messages::ProtocolClientMessage>();
    auto eventMsg = clientMsg->mutable_event();
    auto writeEventMsg = eventMsg->mutable_write_event();
    writeEventMsg->set_counter(m_counter);
    writeEventMsg->set_file_id(m_fileId);
    writeEventMsg->set_file_size(m_fileSize);
    writeEventMsg->set_size(m_size);
    for (const auto &block : m_blocks) {
        auto blockMsg = writeEventMsg->add_blocks();
        blockMsg->set_offset(block.lower());
        blockMsg->set_size(block.upper() - block.lower());
    }

    return clientMsg;
}

} // namespace events
} // namespace client
} // namespace one
