// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 The Foundry Visionmongers Ltd
#include <type_traits>
#include <variant>

#include <openassetio/export.h>

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include <openassetio/Context.hpp>
#include <openassetio/TraitsData.hpp>
#include <openassetio/hostApi/HostInterface.hpp>
#include <openassetio/hostApi/Manager.hpp>
#include <openassetio/log/LoggerInterface.hpp>
#include <openassetio/managerApi/Host.hpp>
#include <openassetio/managerApi/HostSession.hpp>
#include <openassetio/managerApi/ManagerInterface.hpp>
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
}  // namespace
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio

SCENARIO("Manager constructor is private") {
  STATIC_REQUIRE_FALSE(std::is_constructible_v<openassetio::hostApi::Manager,
                                               openassetio::managerApi::ManagerInterfacePtr>);
}

SCENARIO("Entities are resolved successfully") {
  namespace managerApi = openassetio::managerApi;
  namespace hostApi = openassetio::hostApi;
  using trompeloeil::_;

  GIVEN("a configured Manager instance") {
    std::shared_ptr<managerApi::ManagerInterface> managerInterface =
        std::make_shared<openassetio::MockManagerInterface>();

    auto mockManagerInterfacePtr =
        std::static_pointer_cast<openassetio::MockManagerInterface>(managerInterface);

    // Create a HostSession with our mock HostInterface
    managerApi::HostSessionPtr hostSessionPtr = managerApi::HostSession::make(
        managerApi::Host::make(std::make_shared<openassetio::MockHostInterface>()),
        std::make_shared<openassetio::MockLoggerInterface>());

    // Create the Manager under test.
    hostApi::ManagerPtr manager = hostApi::Manager::make(mockManagerInterfacePtr, hostSessionPtr);

    openassetio::trait::TraitSet traits = {"fakeTrait", "secondFakeTrait"};
    auto context = openassetio::Context::make();

    GIVEN("manager plugin successfully resolves a single entity reference") {
      openassetio::EntityReference ref = openassetio::EntityReference{"testReference"};
      openassetio::EntityReferences refs = {ref};

      openassetio::TraitsDataPtr expected = openassetio::TraitsData::make();
      expected->addTrait("aTestTrait");

      // With success callback side effect
      REQUIRE_CALL(*mockManagerInterfacePtr, resolve(refs, traits, context, hostSessionPtr, _, _))
          .LR_SIDE_EFFECT(_5(0, expected));

      WHEN("resolve is called with default errorPolicyTag") {
        openassetio::TraitsDataPtr actual = manager->resolve(ref, traits, context);
        THEN("returned TraitsData is as expected") { CHECK(expected.get() == actual.get()); }
      }
      WHEN("resolve is called with except errorPolicyTag") {
        openassetio::TraitsDataPtr actual = manager->resolve(
            ref, traits, context, openassetio::BatchElementErrorPolicyTag::kExcept);
        THEN("returned TraitsData is as expected") { CHECK(expected.get() == actual.get()); }
      }
      WHEN("resolve is called with return errorPolicyTag") {
        std::variant<openassetio::TraitsDataPtr, openassetio::BatchElementError> actual =
            manager->resolve(ref, traits, context,
                             openassetio::BatchElementErrorPolicyTag::kReturn);
        THEN("returned variant contains the expected TraitsData") {
          CHECK(std::holds_alternative<openassetio::TraitsDataPtr>(actual));
          auto actualVal = std::get<openassetio::TraitsDataPtr>(actual);
          CHECK(expected.get() == actualVal.get());
        }
      }
    }
    GIVEN("manager plugin successfully resolves multiple entity references") {
      openassetio::EntityReferences refs = {openassetio::EntityReference{"testReference1"},
                                            openassetio::EntityReference{"testReference2"},
                                            openassetio::EntityReference{"testReference3"}};

      openassetio::TraitsDataPtr expected1 = openassetio::TraitsData::make();
      expected1->addTrait("aTestTrait");
      openassetio::TraitsDataPtr expected2 = openassetio::TraitsData::make();
      expected2->addTrait("aTestTrait");
      openassetio::TraitsDataPtr expected3 = openassetio::TraitsData::make();
      expected3->addTrait("aTestTrait");
      std::vector<openassetio::TraitsDataPtr> expectedVec{expected1, expected2, expected3};

      // With success callback side effect
      REQUIRE_CALL(*mockManagerInterfacePtr, resolve(refs, traits, context, hostSessionPtr, _, _))
          .LR_SIDE_EFFECT(_5(2, expectedVec[2]))
          .LR_SIDE_EFFECT(_5(0, expectedVec[0]))
          .LR_SIDE_EFFECT(_5(1, expectedVec[1]));

      WHEN("resolve is called with default errorPolicyTag") {
        std::vector<openassetio::TraitsDataPtr> actualVec =
            manager->resolve(refs, traits, context);
        THEN("returned list of TraitsDatas is as expected") { CHECK(expectedVec == actualVec); }
      }
      WHEN("resolve is called with except errorPolicyTag") {
        std::vector<openassetio::TraitsDataPtr> actualVec = manager->resolve(
            refs, traits, context, openassetio::BatchElementErrorPolicyTag::kExcept);
        THEN("returned list of TraitsDatas is as expected") { CHECK(expectedVec == actualVec); }
      }
      WHEN("resolve is called with return errorPolicyTag") {
        std::vector<std::variant<openassetio::TraitsDataPtr, openassetio::BatchElementError>>
            actualVec = manager->resolve(refs, traits, context,
                                         openassetio::BatchElementErrorPolicyTag::kReturn);
        THEN("returned lists of variants contains the expected TraitsDatas") {
          CHECK(expectedVec.size() == actualVec.size());
          for (size_t i = 0; i < actualVec.size(); ++i) {
            CHECK(std::holds_alternative<openassetio::TraitsDataPtr>(actualVec[i]));
            auto actualVal = std::get<openassetio::TraitsDataPtr>(actualVec[i]);
            CHECK(expectedVec[i] == actualVal);
          }
        }
      }
    }
    GIVEN(
        "manager plugin encounters an entity reference specific error during resolve of a single "
        "entity reference") {
      openassetio::EntityReference ref = openassetio::EntityReference{"testReference"};
      openassetio::EntityReferences refs = {ref};

      openassetio::BatchElementError expected{
          openassetio::BatchElementError::ErrorCode::kMalformedEntityReference, "Error Message"};

      // With success callback side effect
      REQUIRE_CALL(*mockManagerInterfacePtr, resolve(refs, traits, context, hostSessionPtr, _, _))
          .LR_SIDE_EFFECT(_6(0, expected));

      WHEN("resolve is called with default errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(manager->resolve(ref, traits, context),
                               openassetio::MalformedEntityReferenceBatchElementException,
                               Catch::Message("Error Message"));
        }
      }
      WHEN("resolve is called with except errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(manager->resolve(ref, traits, context,
                                                openassetio::BatchElementErrorPolicyTag::kExcept),
                               openassetio::MalformedEntityReferenceBatchElementException,
                               Catch::Message("Error Message"));
        }
      }
      WHEN("resolve is called with return errorPolicyTag") {
        std::variant<openassetio::TraitsDataPtr, openassetio::BatchElementError> actual =
            manager->resolve(ref, traits, context,
                             openassetio::BatchElementErrorPolicyTag::kReturn);
        THEN("returned variant contains the expected BatchElementError") {
          CHECK(std::holds_alternative<openassetio::BatchElementError>(actual));
          const auto& actualVal = std::get<openassetio::BatchElementError>(actual);
          CHECK(expected.code == actualVal.code);
          CHECK(expected.message == actualVal.message);
          // TODO(EM):
          // https://github.com/OpenAssetIO/OpenAssetIO/issues/858
          CHECK_FALSE(expected.message.data() == actualVal.message.data());
        }
      }
    }
    GIVEN(
        "manager plugin encounters an entity reference specific error during resolve of multiple "
        "entity references") {
      openassetio::EntityReferences refs = {openassetio::EntityReference{"testReference1"},
                                            openassetio::EntityReference{"testReference2"},
                                            openassetio::EntityReference{"testReference3"}};

      openassetio::TraitsDataPtr expectedValue2 = openassetio::TraitsData::make();
      expectedValue2->addTrait("aTestTrait");
      openassetio::BatchElementError expectedError0{
          openassetio::BatchElementError::ErrorCode::kMalformedEntityReference,
          "Malformed Mock Error🤖"};
      openassetio::BatchElementError expectedError1{
          openassetio::BatchElementError::ErrorCode::kEntityAccessError,
          "Entity Access Error Message"};

      // With success callback side effect
      REQUIRE_CALL(*mockManagerInterfacePtr, resolve(refs, traits, context, hostSessionPtr, _, _))
          .LR_SIDE_EFFECT(_5(2, expectedValue2))
          .LR_SIDE_EFFECT(_6(0, expectedError0))
          .LR_SIDE_EFFECT(_6(1, expectedError1));

      WHEN("resolve is called with default errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(manager->resolve(refs, traits, context),
                               openassetio::MalformedEntityReferenceBatchElementException,
                               Catch::Message("Malformed Mock Error🤖"));
        }
      }
      WHEN("resolve is called with except errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(manager->resolve(refs, traits, context,
                                                openassetio::BatchElementErrorPolicyTag::kExcept),
                               openassetio::MalformedEntityReferenceBatchElementException,
                               Catch::Message("Malformed Mock Error🤖"));
        }
      }
      WHEN("resolve is called with return errorPolicyTag") {
        std::vector<std::variant<openassetio::TraitsDataPtr, openassetio::BatchElementError>>
            actualVec = manager->resolve(refs, traits, context,
                                         openassetio::BatchElementErrorPolicyTag::kReturn);
        THEN("returned lists of variants contains the expected objects") {
          auto error0 = std::get<openassetio::BatchElementError>(actualVec[0]);
          CHECK(error0.code == expectedError0.code);
          CHECK(error0.message == expectedError0.message);

          auto error1 = std::get<openassetio::BatchElementError>(actualVec[1]);
          CHECK(error1.code == expectedError1.code);
          CHECK(error1.message == expectedError1.message);

          CHECK(std::get<openassetio::TraitsDataPtr>(actualVec[2]) == expectedValue2);
        }
      }
    }
  }
}
