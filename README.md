# oneclient

[![Build Status](https://api.travis-ci.org/onedata/oneclient.svg?branch=develop)](https://travis-ci.org/onedata/oneclient)

*oneclient* is a command line [Onedata](onedata.org) client. It provides a POSIX
interface to user's files in *onedata* system.

# User Guide

## Building

### Dependencies

An up-to-date list of *oneclient* build dependencies for Ubuntu and Fedora is
available in [control](pkg_config/debian/control) and
[oneclient.spec](pkg_config/oneclient.spec) files respectively.

## Compilation

When compiling from GitHub, an environment variable ONEDATA_GIT_URL must be
exported to fetch dependencies from public repositories, i.e.:

```bash
export ONEDATA_GIT_URL=https://github.com/onedata
git clone https://github.com/onedata/oneclient.git
cd oneclient
make release # or debug
```

*oneclient* by default compiles with built-in support for Ceph, S3 and Swift.
These drivers can be disabled during compilation by providing the following
flags:

* WITH_CEPH=OFF - disables Ceph support
* WITH_S3=OFF - disables S3 support
* WITH_SWIFT=OFF - disables Swift support

By default *oneclient* is compiled with BoringSSL as default SSL implementation.
To select OpenSSL instead of BoringSSL, add `WITH_OPENSSL=ON` option.

The compiled binary `oneclient` will be created on path `release/oneclient` (or
`debug/oneclient`).

## Installation

### Linux
Oneclient is supported on several major Linux platforms including Ubuntu and
Fedora. To install *oneclient* using packages simply use the following command:

```bash
curl -sS  http://get.onedata.org/oneclient.sh | bash
```

### macOS
An experimental version of *oneclient* is available for macOS (Sierra or higher),
and can be installed using Homebrew:

```bash
# OSXFuse must be installed separately, at least version 3.5.4
brew cask install osxfuse
brew tap onedata/onedata
brew install oneclient
```

In order to enable Desktop icon for the mounted Onedata volume, it is necessary
to enable this feature in the system settings:

```bash
defaults write com.apple.finder ShowMountedServersOnDesktop 1
```

## Usage

```
Usage: oneclient [options] mountpoint

A Onedata command line client.

General options:
  -h [ --help ]                         Show this help and exit.
  -V [ --version ]                      Show current Oneclient version and
                                        exit.
  -u [ --unmount ]                      Unmount Oneclient and exit.
  -c [ --config ] <path> (=/etc/oneclient.conf)
                                        Specify path to config file.
  -H [ --host ] <host>                  Specify the hostname of the Oneprovider
                                        instance to which the Oneclient should
                                        connect.
  -P [ --port ] <port> (=5555)          Specify the port to which the Oneclient
                                        should connect on the Oneprovider.
  -i [ --insecure ]                     Disable verification of server
                                        certificate, allows to connect to
                                        servers without valid certificate.
  -t [ --token ] <token>                Specify Onedata access token for
                                        authentication and authorization.
  -l [ --log-dir ] <path> (=/tmp/oneclient/0)
                                        Specify custom path for Oneclient logs.

FUSE options:
  -f [ --foreground ]         Foreground operation.
  -d [ --debug ]              Enable debug mode (implies -f).
  -s [ --single-thread ]      Single-threaded operation.
  -o [ --opt ] <mount_option> Pass mount arguments directly to FUSE.
```

### Configuration

Besides commandline configuration options, oneclient reads options from a global
configuration file located at `/usr/local/etc/oneclient.conf`
(`/etc/oneclient.conf` when installed from the package). Refer to the
[example configuration file](config/oneclient.conf) for details on the options.

#### Environment variables

Some options in the config file can be overridden using environment variables,
whose names are capitalized version of the config options. For the up-to-date
list of supported environment variables please refer to *oneclient*
[manpage](man/oneclient.1).

## Running oneclient docker image

Running dockerized *oneclient* is easy:

```
docker run -it --privileged onedata/oneclient:3.0.0-rc11
```

### Persisting the token

The application will ask for a token and run in the foreground. In order for
*oneclient* to remember your token, mount volume `/root/.local/share/oneclient`:

```
docker run -it --privileged -v ~/.oneclient_local:/root/.local/share/oneclient onedata/oneclient:3.0.0-rc11
```

You can also pass your token in `ONECLIENT_ACCESS_TOKEN` environment variable:

```
docker run -it --privileged -e ONECLIENT_ACCESS_TOKEN=$TOKEN onedata/oneclient:3.0.0-rc11
```

If *oneclient* knows the token (either by reading its config file or by reading
the environment variable), it can be run as a daemon container:

```
docker run -d --privileged -e ONECLIENT_ACCESS_TOKEN=$TOKEN onedata/oneclient:3.0.0-rc11
```

### Accessing your data

*oneclient* exposes NFS and SMB services for easy outside access to your mounted
spaces.

```
docker run -d --privileged -e ONECLIENT_ACCESS_TOKEN=$TOKEN onedata/oneclient:3.0.0-rc11

# Display container's IP address
docker inspect --format "{{ .NetworkSettings.IPAddress }}" $(docker ps -ql)
```

Now you can mount using *NFS* or *Samba* with:

```
nfs://<CONTAINER_IP_ADDR>/mnt/oneclient
smb://<CONTAINER_IP_ADDR>/onedata
```
