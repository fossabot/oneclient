/**
 * @file connection.cc
 * @author Konrad Zemek
 * @copyright (C) 2014 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in 'LICENSE.txt'
 */

#include "communication/connection.h"

namespace veil
{
namespace communication
{

Connection::Connection(std::shared_ptr<Mailbox> mailbox)
    : m_mailbox{std::move(mailbox)}
{

}

} // namespace communication
} // namespace veil
