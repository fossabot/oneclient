/**
 * @file storageHelperFactory.h
 * @author Rafal Slota
 * @copyright (C) 2013 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in 'LICENSE.txt'
 */

#ifndef STORAGE_HELPER_FACTORY_H
#define STORAGE_HELPER_FACTORY_H

#include "helpers/IStorageHelper.h"

#include "simpleConnectionPool.h"

#include <boost/any.hpp>
#include <boost/atomic.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread_time.hpp>

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#define PROTOCOL_VERSION 1


namespace veil {
namespace helpers {

namespace config {

    extern unsigned int clusterPort;
    extern std::string  proxyCert;
    extern std::string  clusterHostname;
    extern boost::atomic<bool> checkCertificate;

namespace buffers {

    extern size_t writeBufferGlobalSizeLimit;
    extern size_t readBufferGlobalSizeLimit;

    extern size_t writeBufferPerFileSizeLimit;
    extern size_t readBufferPerFileSizeLimit;

    extern size_t preferedBlockSize;

} // namespace buffers


} // namespace config

namespace utils {

    std::string tolower(std::string input);

    template<typename T>
    T fromString(const std::string &in) {
        T out;
        std::istringstream iss(in);
        iss >> out;
        return out;
    }

    template<typename T>
    T mtime()
    {
        boost::posix_time::ptime time_t_epoch(boost::gregorian::date(1970,1,1));
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
        boost::posix_time::time_duration diff = now - time_t_epoch;

        return diff.total_milliseconds();
    }

} // namespace utils

/**
 * Factory providing objects of requested storage helpers.
 */
class StorageHelperFactory {
public:
    StorageHelperFactory(boost::shared_ptr<SimpleConnectionPool> connectionPool);
    virtual ~StorageHelperFactory();

    /**
     * Produces storage helper object.
     * @param sh Name of storage helper that has to be returned.
     * @param args Arguments vector passed as argument to storge helper's constructor.
     * @return Pointer to storage helper object along with its ownership.
     */
    virtual boost::shared_ptr<IStorageHelper> getStorageHelper(const std::string &sh, const IStorageHelper::ArgsMap &args);

private:
    boost::shared_ptr<SimpleConnectionPool> m_connectionPool;
};

} // namespace helpers
} // namespace veil

#endif // STORAGE_HELPER_FACTORY_H
