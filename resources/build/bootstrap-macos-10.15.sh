#!/usr/bin/env bash
# This bootstrap script is used by both the Github CI action and the
# Vagrant VM configuration.

set -xeo pipefail

# Install additional build tools.
pip3 install -r "$WORKSPACE/resources/build/requirements.txt"
# Use explicit predictable conan root path, where packages are cached.
export CONAN_USER_HOME="$HOME/conan"
# Create default conan profile so we can configure it before install.
# Use --force so that if it already exists we don't error out.
conan profile new default --detect --force

# In the case that this script is run from an environment (cibuildwheel)
# that already has python, don't install over them as they'll conflict.
# We also don't need to install the test dependencies in that case, as
# they're unused in the release workflow.
if [ "$CIBUILDWHEEL" = "1" ]
then
  export OPENASSETIO_CONAN_SKIP_CPYTHON="True"
fi

# Install openassetio third-party dependencies from public Conan Center
# package repo.
conan install --install-folder "$WORKSPACE/.conan" --build=missing
    "$WORKSPACE/resources/build"
