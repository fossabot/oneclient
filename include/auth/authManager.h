/**
 * @file authManager.h
 * @author Konrad Zemek
 * @copyright (C) 2014-2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#ifndef ONECLIENT_AUTH_MANAGER_H
#define ONECLIENT_AUTH_MANAGER_H

#include "environment.h"
#include "auth/grAdapter.h"
#include "auth/tokenAuthDetails.h"
#include "communication/communicator.h"
#include "messages/handshakeRequest.h"
#include "messages/handshakeResponse.h"

#include <boost/optional.hpp>

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace one {

namespace communication {
namespace cert {
class CertificateData;
}
}

namespace client {

class Context;

namespace auth {

/**
 * The AuthManager class is responsible for setting an authentication scheme
 * for Client - Provider communication.
 */
class AuthManager {
public:
    using ErrorPolicy = communication::ConnectionPool::ErrorPolicy;

    /**
     * Constructor.
     * @param context An application context.
     * @param defaultHostname A default hostname to be used for communication
     * with a Provider. The hostname ist used as a base for a generated hostname
     * in certificate-based authorization.
     * @param port A port to be used for communication with a Provider.
     * @param checkCertificate Determines whether to check Provider's and
     * Global Registry's server certificates for validity.
     */
    AuthManager(std::weak_ptr<Context> context, std::string defaultHostname,
        const unsigned int port, const bool checkCertificate);

    /**
     * Creates a @c one::communication::Communicator object set up with proper
     * authentication settings.
     * @see one::communication::createCommunicator
     * @param dataPoolSize The size of data pool to be created.
     * @param metaPoolSize The size of meta pool to be created.
     * @return A new instance of @c Communicator .
     */
    virtual std::shared_ptr<communication::Communicator> createCommunicator(
        const unsigned int poolSize, std::string sessionId,
        std::function<bool(messages::HandshakeResponse)> onHandshakeResponse,
        ErrorPolicy errorPolicy) = 0;

protected:
    std::weak_ptr<Context> m_context;
    std::string m_hostname;
    const unsigned int m_port;
    const bool m_checkCertificate;

    Environment m_environment;
};

/**
 * The CertificateAuthManager class is responsible for setting up user
 * authentication using X509 certificates.
 */
class CertificateAuthManager : public AuthManager {
public:
    /**
     * @copydoc AuthManager::AuthManager()
     * @param debugGsi Determines whether to enable more detailed (debug) logs.
     */
    CertificateAuthManager(std::weak_ptr<Context> context,
        std::string defaultHostname, const unsigned int port,
        const bool checkCertificate, const bool debugGsi);

    std::shared_ptr<communication::Communicator> createCommunicator(
        const unsigned int poolSize, std::string sessionId,
        std::function<bool(messages::HandshakeResponse)> onHandshakeResponse,
        ErrorPolicy errorPolicy) override;

private:
    std::shared_ptr<communication::cert::CertificateData> m_certificateData;
};

/**
 * The TokenAuthManager class is responsible for setting up user authentication
 * using an OpenID token-based scheme.
 */
class TokenAuthManager : public AuthManager {
public:
    /**
     * @copydoc AuthManager::AuthManager()
     * @param globalRegistryHostname A hostname of Global Registry to be used
     * for token-based authentication.
     * @param globalRegistryPort A port of globalregistry to be used for
     * token-based authentication
     */
    TokenAuthManager(std::weak_ptr<Context> context,
        std::string defaultHostname, const unsigned int port,
        const bool checkCertificate, std::string globalRegistryHostname,
        const unsigned int globalRegistryPort);

    std::shared_ptr<communication::Communicator> createCommunicator(
        const unsigned int poolSize, std::string sessionId,
        std::function<bool(messages::HandshakeResponse)> onHandshakeResponse,
        ErrorPolicy errorPolicy) override;

private:
    void scheduleRefresh(
        std::weak_ptr<communication::Communicator> communicator);
    void refresh(std::weak_ptr<communication::Communicator> communicator);
    std::string hashAndBase64(const std::string &token) const;

    TokenAuthDetails m_authDetails;
    GRAdapter m_grAdapter;
};

} // namespace auth
} // namespace client
} // namespace one

#endif // ONECLIENT_AUTH_MANAGER_H