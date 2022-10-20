#!/usr/bin/env bash
# This bootstrap script is used by both the Github CI action and the
# Vagrant VM configuration.

set -xeo pipefail

# Install additional build tools.
pip3 install -r "$WORKSPACE/resources/build/requirements.txt"
# Use explicit predictable conan root path, where packages are 
# installed.
export CONAN_USER_HOME="$HOME/conan"
# Create default conan profile so we can configure it before install.
# Use --force so that if it already exists we don't error out.
conan profile new default --detect --force

# In the case that this script is run from an environment (cibuildwheel)
# that already has python, don't install over them as they'll conflict.
# We also don't need to install the test dependencies in that case, as
# they're unused in the release workflow.
INSTALL_PYTHON=True
REQUIRE_TEST_DEPS=True
if [ "$CIBUILDWHEEL" = "1" ]
then
  INSTALL_PYTHON=False
  REQUIRE_TEST_DEPS=False
fi

# Install openassetio third-party dependencies from public Conan Center
# package repo.
conan install --install-folder "$WORKSPACE/.conan" --build=missing \
    -o testing=$REQUIRE_TEST_DEPS \
    -o install_cpython=$INSTALL_PYTHON \
    "$WORKSPACE/resources/build"
