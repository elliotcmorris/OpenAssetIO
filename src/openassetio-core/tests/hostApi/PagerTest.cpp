// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 The Foundry Visionmongers Ltd
#include <type_traits>
#include <variant>

#include <openassetio/export.h>

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include <openassetio/hostApi/HostInterface.hpp>
#include <openassetio/log/LoggerInterface.hpp>
#include <openassetio/managerApi/Host.hpp>
#include <openassetio/managerApi/HostSession.hpp>
#include <openassetio/managerApi/ManagerInterface.hpp>

#include <openassetio/hostApi/EntityReferencePager.hpp>
#include <openassetio/managerApi/EntityReferencePagerInterface.hpp>
namespace openassetio {
inline namespace OPENASSETIO_CORE_ABI_VERSION {
namespace {
/**
 * Mock implementation of a ManagerInterface.
 *
 * Used as constructor parameter to the Manager under test.
 */
struct MockManagerInterface : trompeloeil::mock_interface<managerApi::ManagerInterface> {
  IMPLEMENT_CONST_MOCK0(identifier);
  IMPLEMENT_CONST_MOCK0(displayName);
  IMPLEMENT_CONST_MOCK0(info);
  IMPLEMENT_MOCK2(initialize);
  IMPLEMENT_CONST_MOCK3(managementPolicy);
  IMPLEMENT_CONST_MOCK2(isEntityReferenceString);
  IMPLEMENT_MOCK6(resolve);
  IMPLEMENT_MOCK6(preflight);
  IMPLEMENT_MOCK6(register_);  // NOLINT(readability-identifier-naming)
};
/**
 * Mock implementation of a HostInterface.
 *
 * Used as constructor parameter to Host classes required as part of these tests
 */
struct MockHostInterface : trompeloeil::mock_interface<hostApi::HostInterface> {
  IMPLEMENT_CONST_MOCK0(identifier);
  IMPLEMENT_CONST_MOCK0(displayName);
  IMPLEMENT_CONST_MOCK0(info);
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
};

}  // namespace
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio

namespace {
struct MockEntityReferencePagerInterface
    : trompeloeil::mock_interface<openassetio::managerApi::EntityReferencePagerInterface> {
  IMPLEMENT_MOCK1(hasNext);
  IMPLEMENT_MOCK1(get);
  IMPLEMENT_MOCK1(next);
};

}  // namespace

SCENARIO("Using an EntityReferencePager") {
  using trompeloeil::_;

  GIVEN("a configured EntityReferencePager") {
    const openassetio::trait::TraitSet traits = {"fakeTrait", "secondFakeTrait"};
    const openassetio::ManagerFixture fixture;

    std::shared_ptr<MockEntityReferencePagerInterface> mockEntityReferencePagerInterface =
        std::make_shared<MockEntityReferencePagerInterface>();

    openassetio::hostApi::EntityReferencePager::Ptr pager =
        openassetio::hostApi::EntityReferencePager::make(mockEntityReferencePagerInterface,
                                                         fixture.hostSession);

    AND_GIVEN("pagerInterface hasMore expects to be called and returns false") {
      REQUIRE_CALL(*mockEntityReferencePagerInterface, hasNext(fixture.hostSession)).RETURN(false);
      WHEN("pager hasMore is called") {
        bool hasMoreReturn = pager->hasNext();
        THEN("the value is false") { CHECK(hasMoreReturn == false); }
      }
    }
    AND_GIVEN("pagerInterface hasMore expects to be called and returns true") {
      REQUIRE_CALL(*mockEntityReferencePagerInterface, hasNext(fixture.hostSession)).RETURN(true);
      WHEN("pager hasMore is called") {
        bool hasMoreReturn = pager->hasNext();
        THEN("the value is true") { CHECK(hasMoreReturn == true); }
      }
    }
    AND_GIVEN("pagerInterface next expects to be called") {
      REQUIRE_CALL(*mockEntityReferencePagerInterface, next(fixture.hostSession));
      WHEN("pager next is called") {
        pager->next();
        THEN("pagerInterface next was called") {}
      }
    }
    AND_GIVEN("pagerInterface get expects to be called and returns false") {
      const std::vector<openassetio::EntityReference> testEntRefs = {
          openassetio::EntityReference("One!"), openassetio::EntityReference("Two!")};
      REQUIRE_CALL(*mockEntityReferencePagerInterface, get(fixture.hostSession))
          .RETURN(testEntRefs);
      WHEN("pager get is called") {
        const auto getReturn = pager->get();
        THEN("the value is as expected") { CHECK(getReturn == testEntRefs); }
      }
    }
  }
}
