#!/usr/bin/env bash

set -xeo pipefail
yum clean all
yum update
yum install -y centos-release-scl
yum install -y devtoolset-9
source /opt/rh/devtoolset-9/enable
#scl enable devtoolset-9 bash

# Install gcc, linters, build tools used by conan and Python 3.
yum install -y make pkgconfig python3-pip

#Conan defaults to wanting to use the DNF package manager on CentOS, no reason not to let it.
yum install -y dnf
dnf update

# Install system packages required for the Python 3.9 conan package.
# These would be installed as part of the conan package install, but
# we're caching the conan directory via the `actions/cache` Github
# action, so a fresh Github VM is left without these system packages.
yum install -y xorg-x11-xtrans-devel xkeyboard-config-devel
 #yum install -y libfontenc-devel libXaw-devel libXcomposite-devel libXcursor-devel \
  #libXdmcp-devel libXtst-devel libXinerama-devel libxkbfile-devel libXrandr-devel \
  #libXres-devel libXScrnSaver-devel libXvMC-deve \
  #xcb-util-wm-devel xcb-util-image-devel xcb-util-keysyms-devel xcb-util-renderutil-devel \
  #libXv-devel xcb-util-devel libuuid-devel
# Install additional build tools.
pip3 install -r "resources/build/requirements.txt"
# Use explicit predictable conan root path, to be used for both packages
# and conan CMake toolchain config.
#export CONAN_USER_HOME="$WORKSPACE/.conan"
export CONAN_USER_HOME="$HOME/conan"



# Create default conan profile so we can configure it before instlibXcomposite-develall.
# Use --force so that if it already exists we don't error out.
conan profile new default --detect --force
# Use old C++11 ABI as per VFX Reference Platform CY2022. Not strictly
# necessary as this is the default for conan, but we can't be certain
# it'll remain the default in future.
conan profile update settings.compiler.libcxx=libstdc++ default
# Install openassetio third-party dependencies from public Conan Center
# package repo.
# TODO(DF): conan<1.51 (not yet released) has a bug that means we have
# to allow conan recipes to try to install system packages, even if the
# system packages are already available. In particular, this affects
# recent versions of the xorg/system recipe (a dependency of cpython).
# The problem is reported and fixed in https://github.com/conan-io/conan/pull/11712
#conan install --install-folder "$CONAN_USER_HOME" --build=missing \
conan install --install-folder ".conan" --build=missing \
    -c tools.system.package_manager:mode=install \
    -o testing=False \
    -o install_cpython=False \
    "resources/build"


# Ensure we have the expected version of clang-* available
#update-alternatives --install /usr/bin/clang-tidy clang-tidy /usr/bin/clang-tidy-12 10
#update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-12 10
#update-alternatives --set clang-tidy /usr/bin/clang-tidy-12
#update-alternatives --set clang-format /usr/bin/clang-format-12
