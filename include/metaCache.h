/**
 * @file metaCache.h
 * @author Rafal Slota
 * @copyright (C) 2013 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in 'LICENSE.txt'
 */

#ifndef VEILCLIENT_META_CACHE_H
#define VEILCLIENT_META_CACHE_H


#include <boost/thread/shared_mutex.hpp>
#include <sys/stat.h>

#include <ctime>
#include <memory>
#include <string>
#include <unordered_map>

namespace veil
{
namespace client
{

class Context;

/**
 * Class responsible for caching file attributes.
 * @see VeilFS::getattr
 */
class MetaCache: public std::enable_shared_from_this<MetaCache>
{
protected:
    std::unordered_map<std::string, std::pair<time_t, struct stat> > m_statMap;  ///< This is the cache map.
                                                                        ///< Value of this std::map is std::pair containing expiration time of attributes and
                                                                        ///< stat struct itself
    boost::shared_mutex m_statMapMutex;                                 ///< Mutex used to synchronize access to MetaCache::m_statMap

public:

    MetaCache(std::shared_ptr<Context> context);
    virtual ~MetaCache();

    virtual void addAttr(const std::string&, struct stat&); ///< Cache given attributes
                                                        ///< Expiration time can be set using configuration file.
    virtual void clearAttrs();                          ///< Clear whole cache

    /**
     * Gets file attributes from cache.
     * @param stat Pointer to stat structure that should be filled with data from cache
     * @return Bool saying if operation succeed and stat struct was filled with data
     */
    virtual bool getAttr(const std::string&, struct stat*);
    virtual void clearAttr(const std::string &path);        ///< Remove cache for given file
    virtual bool updateTimes(const std::string &path, time_t atime = 0, time_t mtime = 0, time_t ctime = 0); ///< Update *time meta attributes for specific file in cache. Returns true if cache was updated or false if given file wasn't found in cache.
    virtual bool updateSize(const std::string &path, size_t size); ///< Update size meta attribute for specific file in cache. Returns true if cache was updated or false if given file wasn't found in cache.

    /**
     * Checks if file with given attributes can be accessed directly considering
     * user's group membership.
     * @param attrs of the file
     */
    virtual bool canUseDefaultPermissions(const struct stat &attrs);

protected:
    const std::shared_ptr<Context> m_context;
};

} // namespace client
} // namespace veil


#endif // VEILCLIENT_META_CACHE_H
