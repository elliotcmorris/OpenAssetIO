// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 The Foundry Visionmongers Ltd
#pragma once
#include <memory>

#include <openassetio/export.h>

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include <openassetio/Context.hpp>
#include <openassetio/hostApi/HostInterface.hpp>
#include <openassetio/hostApi/Manager.hpp>
#include <openassetio/log/LoggerInterface.hpp>
#include <openassetio/managerApi/Host.hpp>
#include <openassetio/managerApi/HostSession.hpp>
#include <openassetio/managerApi/ManagerInterface.hpp>

namespace openassetio {
inline namespace OPENASSETIO_CORE_ABI_VERSION {
/**
 * Mock implementation of a ManagerInterface.
 *
 * Used as constructor parameter to the Manager under test.
 */
struct MockManagerInterface : trompeloeil::mock_interface<managerApi::ManagerInterface> {
  IMPLEMENT_CONST_MOCK0(identifier);
  IMPLEMENT_CONST_MOCK0(displayName);
  IMPLEMENT_MOCK0(info);
  IMPLEMENT_MOCK2(initialize);
  IMPLEMENT_MOCK4(managementPolicy);
  IMPLEMENT_MOCK2(isEntityReferenceString);
  IMPLEMENT_MOCK5(entityExists);
  IMPLEMENT_MOCK7(resolve);
  IMPLEMENT_MOCK7(preflight);
  IMPLEMENT_MOCK7(register_);  // NOLINT(readability-identifier-naming)
};
/**
 * Mock implementation of a HostInterface.
 *
 * Used as constructor parameter to Host classes required as part of these tests
 */
struct MockHostInterface : trompeloeil::mock_interface<hostApi::HostInterface> {
  IMPLEMENT_CONST_MOCK0(identifier);
  IMPLEMENT_CONST_MOCK0(displayName);
  IMPLEMENT_MOCK0(info);
};
/**
 * Mock implementation of a LoggerInterface
 *
 * Used as constructor parameter to Host classes required as part of these tests
 */
struct MockLoggerInterface : trompeloeil::mock_interface<log::LoggerInterface> {
  IMPLEMENT_MOCK2(log);
};

/**
 * Fixture providing a Manager instance injected with mock dependencies.
 */
struct ManagerFixture {
  const std::shared_ptr<managerApi::ManagerInterface> managerInterface =
      std::make_shared<openassetio::MockManagerInterface>();

  // For convenience, to avoid casting all the time in tests.
  MockManagerInterface& mockManagerInterface =
      static_cast<openassetio::MockManagerInterface&>(*managerInterface);

  // Create a HostSession with our mock HostInterface
  const managerApi::HostSessionPtr hostSession = managerApi::HostSession::make(
      managerApi::Host::make(std::make_shared<openassetio::MockHostInterface>()),
      std::make_shared<openassetio::MockLoggerInterface>());

  // Create the Manager under test.
  const hostApi::ManagerPtr manager = hostApi::Manager::make(managerInterface, hostSession);

  // For convenience, since almost every method takes a Context.
  const openassetio::ContextPtr context{openassetio::Context::make()};
};
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio
