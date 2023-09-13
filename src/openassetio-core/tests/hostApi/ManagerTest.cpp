// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 The Foundry Visionmongers Ltd
#include <type_traits>
#include <variant>

#include <openassetio/export.h>

#include <fmt/format.h>
#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include <openassetio/Context.hpp>
#include <openassetio/TraitsData.hpp>
#include <openassetio/errors/exceptions.hpp>
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

}  // namespace
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio

SCENARIO("Manager constructor is private") {
  STATIC_REQUIRE_FALSE(std::is_constructible_v<openassetio::hostApi::Manager,
                                               openassetio::managerApi::ManagerInterfacePtr>);
}

SCENARIO("Resolving entities") {
  namespace hostApi = openassetio::hostApi;
  using trompeloeil::_;

  GIVEN("a configured Manager instance") {
    const openassetio::trait::TraitSet traits = {"fakeTrait", "secondFakeTrait"};
    const openassetio::ManagerFixture fixture;
    const auto& manager = fixture.manager;
    auto& mockManagerInterface = fixture.mockManagerInterface;
    const auto& context = fixture.context;
    const auto& hostSession = fixture.hostSession;
    const auto resolveAccess = openassetio::access::ResolveAccess::kRead;

    GIVEN("manager plugin successfully resolves a single entity reference") {
      const openassetio::EntityReference ref = openassetio::EntityReference{"testReference"};
      const openassetio::EntityReferences refs = {ref};

      const openassetio::TraitsDataPtr expected = openassetio::TraitsData::make();
      expected->addTrait("aTestTrait");

      // With success callback side effect
      REQUIRE_CALL(mockManagerInterface,
                   resolve(refs, traits, resolveAccess, context, hostSession, _, _))
          .LR_SIDE_EFFECT(_6(0, expected));

      WHEN("singular resolve is called with default errorPolicyTag") {
        const openassetio::TraitsDataPtr actual =
            manager->resolve(ref, traits, resolveAccess, context);
        THEN("returned TraitsData is as expected") { CHECK(expected.get() == actual.get()); }
      }
      WHEN("singular resolve is called with kException errorPolicyTag") {
        const openassetio::TraitsDataPtr actual =
            manager->resolve(ref, traits, resolveAccess, context,
                             hostApi::Manager::BatchElementErrorPolicyTag::kException);
        THEN("returned TraitsData is as expected") { CHECK(expected.get() == actual.get()); }
      }
      WHEN("singular resolve is called with kVariant errorPolicyTag") {
        std::variant<openassetio::errors::BatchElementError, openassetio::TraitsDataPtr> actual =
            manager->resolve(ref, traits, resolveAccess, context,
                             hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned variant contains the expected TraitsData") {
          CHECK(std::holds_alternative<openassetio::TraitsDataPtr>(actual));
          auto actualVal = std::get<openassetio::TraitsDataPtr>(actual);
          CHECK(expected.get() == actualVal.get());
        }
      }
    }
    GIVEN("manager plugin successfully resolves multiple entity references") {
      const openassetio::EntityReferences refs = {openassetio::EntityReference{"testReference1"},
                                                  openassetio::EntityReference{"testReference2"},
                                                  openassetio::EntityReference{"testReference3"}};

      const openassetio::TraitsDataPtr expected1 = openassetio::TraitsData::make();
      expected1->addTrait("aTestTrait1");
      const openassetio::TraitsDataPtr expected2 = openassetio::TraitsData::make();
      expected2->addTrait("aTestTrait2");
      const openassetio::TraitsDataPtr expected3 = openassetio::TraitsData::make();
      expected3->addTrait("aTestTrait3");
      std::vector<openassetio::TraitsDataPtr> expectedVec{expected1, expected2, expected3};

      // With success callback side effect
      REQUIRE_CALL(mockManagerInterface,
                   resolve(refs, traits, resolveAccess, context, hostSession, _, _))
          .LR_SIDE_EFFECT(_6(0, expectedVec[0]))
          .LR_SIDE_EFFECT(_6(1, expectedVec[1]))
          .LR_SIDE_EFFECT(_6(2, expectedVec[2]));

      WHEN("batch resolve is called with default errorPolicyTag") {
        const std::vector<openassetio::TraitsDataPtr> actualVec =
            manager->resolve(refs, traits, resolveAccess, context);
        THEN("returned list of TraitsDatas is as expected") { CHECK(expectedVec == actualVec); }
      }
      WHEN("batch resolve is called with kException errorPolicyTag") {
        const std::vector<openassetio::TraitsDataPtr> actualVec =
            manager->resolve(refs, traits, resolveAccess, context,
                             hostApi::Manager::BatchElementErrorPolicyTag::kException);
        THEN("returned list of TraitsDatas is as expected") { CHECK(expectedVec == actualVec); }
      }
      WHEN("batch resolve is called with kVariant errorPolicyTag") {
        std::vector<
            std::variant<openassetio::errors::BatchElementError, openassetio::TraitsDataPtr>>
            actualVec = manager->resolve(refs, traits, resolveAccess, context,
                                         hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned lists of variants contains the expected TraitsDatas") {
          CHECK(expectedVec.size() == actualVec.size());
          for (size_t i = 0; i < actualVec.size(); ++i) {
            CHECK(std::holds_alternative<openassetio::TraitsDataPtr>(actualVec[i]));
            auto actualVal = std::get<openassetio::TraitsDataPtr>(actualVec[i]);
            CHECK(expectedVec[i].get() == actualVal.get());
          }
        }
      }
    }
    GIVEN("manager plugin successfully resolves multiple entity references in a non-index order") {
      const openassetio::EntityReferences refs = {openassetio::EntityReference{"testReference1"},
                                                  openassetio::EntityReference{"testReference2"},
                                                  openassetio::EntityReference{"testReference3"}};

      const openassetio::TraitsDataPtr expected1 = openassetio::TraitsData::make();
      expected1->addTrait("aTestTrait1");
      const openassetio::TraitsDataPtr expected2 = openassetio::TraitsData::make();
      expected2->addTrait("aTestTrait2");
      const openassetio::TraitsDataPtr expected3 = openassetio::TraitsData::make();
      expected3->addTrait("aTestTrait3");
      std::vector<openassetio::TraitsDataPtr> expectedVec{expected1, expected2, expected3};

      // With success callback side effect, given out of order.
      REQUIRE_CALL(mockManagerInterface,
                   resolve(refs, traits, resolveAccess, context, hostSession, _, _))
          .LR_SIDE_EFFECT(_6(2, expectedVec[2]))
          .LR_SIDE_EFFECT(_6(0, expectedVec[0]))
          .LR_SIDE_EFFECT(_6(1, expectedVec[1]));

      WHEN("batch resolve is called with default errorPolicyTag") {
        const std::vector<openassetio::TraitsDataPtr> actualVec =
            manager->resolve(refs, traits, resolveAccess, context);
        THEN("returned list of TraitsDatas is ordered in index order") {
          CHECK(expectedVec == actualVec);
        }
      }
      WHEN("batch resolve is called with kException errorPolicyTag") {
        const std::vector<openassetio::TraitsDataPtr> actualVec =
            manager->resolve(refs, traits, resolveAccess, context,
                             hostApi::Manager::BatchElementErrorPolicyTag::kException);
        THEN("returned list of TraitsDatas is ordered in index order") {
          CHECK(expectedVec == actualVec);
        }
      }
      WHEN("batch resolve is called with kVariant errorPolicyTag") {
        std::vector<
            std::variant<openassetio::errors::BatchElementError, openassetio::TraitsDataPtr>>
            actualVec = manager->resolve(refs, traits, resolveAccess, context,
                                         hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned lists of variants is ordered in index order") {
          CHECK(expectedVec.size() == actualVec.size());
          for (size_t i = 0; i < actualVec.size(); ++i) {
            CHECK(std::holds_alternative<openassetio::TraitsDataPtr>(actualVec[i]));
            auto actualVal = std::get<openassetio::TraitsDataPtr>(actualVec[i]);
            CHECK(expectedVec[i].get() == actualVal.get());
          }
        }
      }
    }
    GIVEN(
        "manager plugin will encounter an entity-specific error when next resolving a reference") {
      const openassetio::EntityReference ref = openassetio::EntityReference{"testReference"};
      const openassetio::EntityReferences refs = {ref};

      openassetio::errors::BatchElementError expected{
          openassetio::errors::BatchElementError::ErrorCode::kMalformedEntityReference,
          "Error Message"};

      // With error callback side effect
      REQUIRE_CALL(mockManagerInterface,
                   resolve(refs, traits, resolveAccess, context, hostSession, _, _))
          .LR_SIDE_EFFECT(_7(0, expected));

      WHEN("singular resolve is called with default errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(manager->resolve(ref, traits, resolveAccess, context),
                               openassetio::errors::MalformedEntityReferenceBatchElementException,
                               Catch::Message("Error Message [testReference]"));
        }
      }
      WHEN("singular resolve is called with kException errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(
              manager->resolve(ref, traits, resolveAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kException),
              openassetio::errors::MalformedEntityReferenceBatchElementException,
              Catch::Message("Error Message [testReference]"));
        }
      }
      WHEN("singular resolve is called with kVariant errorPolicyTag") {
        std::variant<openassetio::errors::BatchElementError, openassetio::TraitsDataPtr> actual =
            manager->resolve(ref, traits, resolveAccess, context,
                             hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned variant contains the expected BatchElementError") {
          CHECK(std::holds_alternative<openassetio::errors::BatchElementError>(actual));
          const auto& actualVal = std::get<openassetio::errors::BatchElementError>(actual);
          CHECK(expected == actualVal);
        }
      }
    }
    GIVEN(
        "manager plugin will encounter entity-specific errors when next resolving multiple "
        "references") {
      const openassetio::EntityReferences refs = {openassetio::EntityReference{"testReference1"},
                                                  openassetio::EntityReference{"testReference2"},
                                                  openassetio::EntityReference{"testReference3"}};

      const openassetio::TraitsDataPtr expectedValue2 = openassetio::TraitsData::make();
      expectedValue2->addTrait("aTestTrait");
      const openassetio::errors::BatchElementError expectedError0{
          openassetio::errors::BatchElementError::ErrorCode::kMalformedEntityReference,
          "Malformed Mock Error🤖"};
      const openassetio::errors::BatchElementError expectedError1{
          openassetio::errors::BatchElementError::ErrorCode::kEntityAccessError,
          "Entity Access Error Message"};

      // With mixed callback side effect
      REQUIRE_CALL(mockManagerInterface,
                   resolve(refs, traits, resolveAccess, context, hostSession, _, _))
          .LR_SIDE_EFFECT(_6(2, expectedValue2))
          .LR_SIDE_EFFECT(_7(0, expectedError0))
          .LR_SIDE_EFFECT(_7(1, expectedError1));

      WHEN("batch resolve is called with default errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(manager->resolve(refs, traits, resolveAccess, context),
                               openassetio::errors::MalformedEntityReferenceBatchElementException,
                               Catch::Message("Malformed Mock Error🤖 [testReference1]"));
        }
      }
      WHEN("batch resolve is called with kException errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(
              manager->resolve(refs, traits, resolveAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kException),
              openassetio::errors::MalformedEntityReferenceBatchElementException,
              Catch::Message("Malformed Mock Error🤖 [testReference1]"));
        }
      }
      WHEN("batch resolve is called with kVariant errorPolicyTag") {
        std::vector<
            std::variant<openassetio::errors::BatchElementError, openassetio::TraitsDataPtr>>
            actualVec = manager->resolve(refs, traits, resolveAccess, context,
                                         hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned lists of variants contains the expected objects") {
          auto error0 = std::get<openassetio::errors::BatchElementError>(actualVec[0]);
          CHECK(error0 == expectedError0);

          auto error1 = std::get<openassetio::errors::BatchElementError>(actualVec[1]);
          CHECK(error1 == expectedError1);

          CHECK(std::get<openassetio::TraitsDataPtr>(actualVec[2]) == expectedValue2);
        }
      }
    }
  }
}

namespace errors = openassetio::errors;
using ErrorCode = errors::BatchElementError::ErrorCode;

/// Boolean flags to describe what data an API function should provide
/// as data members to a BatchElementException.
enum HasDataFor { kEntityReference = 1 << 0, kTraitSet = 1 << 1, kTraitsData = 1 << 2 };

/**
 * Parametrization fixture for mapping a BatchElementError code to an
 * exception.
 */
template <class T, ErrorCode C>
struct BatchElementErrorMapping {
  /// Exception type to map BatchElementError code to.
  using ExceptionType = T;
  /// BatchElementError code to map exception to.
  static constexpr ErrorCode kErrorCode = C;
  /// Expected BatchElementError::message to use in tests.
  static constexpr std::string_view kErrorMessage = "You have a 🐛";
  /// Expected entity reference to use in tests, where appropriate..
  static inline const std::string kExpectedEntityReference = "my://entity/reference";
  /// Expected trait set to use in tests, where appropriate..
  static inline const openassetio::trait::TraitSet kExpectedTraitSet = {"trait1", "trait2"};
  /// Expected traits data to use in tests, where appropriate.
  static inline const openassetio::TraitsDataPtr kExpectedTraitsData = [] {
    auto data = openassetio::TraitsData::make({"trait1", "trait2"});
    data->setTraitProperty("trait2", "prop", openassetio::Int{0});
    return data;
  }();

  /// Check exception message matches BatchElementError message.
  static void assertExceptionData(const errors::BatchElementException& exc,
                                  [[maybe_unused]] const int hasDataFor) {
    CHECK(exc.what() == kErrorMessage);
  }
};

/**
 * Parametrization fixture for mapping BatchElementError to an
 * exception, where an entity reference can/should be provided as an
 * exception data member.
 */
template <class T, ErrorCode C>
struct EntityBatchElementErrorMapping : BatchElementErrorMapping<T, C> {
  using Base = BatchElementErrorMapping<T, C>;
  using Base::kErrorMessage;
  using Base::kExpectedEntityReference;

  /// Check exception contains entity reference, if available.
  static void assertExceptionData(const errors::BatchElementEntityReferenceException& exc,
                                  const int hasDataFor) {
    if (hasDataFor & HasDataFor::kEntityReference) {
      CHECK(exc.what() == fmt::format("{} [{}]", kErrorMessage, kExpectedEntityReference));
      CHECK(exc.entityReference == openassetio::EntityReference{kExpectedEntityReference});
    } else {
      Base::assertExceptionData(exc, hasDataFor);
    }
  }
};

/**
 * Parametrization fixture for mapping BatchElementError to an
 * exception, where a trait set can/should be provided as an exception
 * data member.
 */
template <class T, ErrorCode C>
struct InvalidTraitSetBatchElementErrorMapping : EntityBatchElementErrorMapping<T, C> {
  using Base = EntityBatchElementErrorMapping<T, C>;
  using Base::kExpectedTraitSet;

  /// Check exception contains trait set, if available.
  static void assertExceptionData(const errors::InvalidTraitSetBatchElementException& exc,
                                  const int hasDataFor) {
    Base::assertExceptionData(exc, hasDataFor);
    if (hasDataFor & HasDataFor::kTraitSet) {
      CHECK(exc.traitSet.value() == kExpectedTraitSet);
    } else {
      CHECK(!exc.traitSet.has_value());
    }
  }
};

/**
 * Parametrization fixture for mapping BatchElementError to an
 * exception, where a TraitsData can/should be provided as an exception
 * data member.
 */
template <class T, ErrorCode C>
struct InvalidTraitsDataBatchElementErrorMapping : EntityBatchElementErrorMapping<T, C> {
  using Base = EntityBatchElementErrorMapping<T, C>;
  using Base::kExpectedTraitsData;

  /// Check exception contains trait data, if available.
  static void assertExceptionData(const errors::InvalidTraitsDataBatchElementException& exc,
                                  const int hasDataFor) {
    Base::assertExceptionData(exc, hasDataFor);
    if (hasDataFor & HasDataFor::kTraitsData) {
      CHECK(*exc.traitsData.value() == *kExpectedTraitsData);
    } else {
      CHECK(!exc.traitsData.has_value());
    }
  }
};

/// Compile-time list of BatchElementError code<->exception mapping
/// utility classes, for parametrizing test cases.
using BatchElementErrorMappings = std::tuple<
    BatchElementErrorMapping<errors::UnknownBatchElementException, ErrorCode::kUnknown>,
    EntityBatchElementErrorMapping<errors::InvalidEntityReferenceBatchElementException,
                                   ErrorCode::kInvalidEntityReference>,
    EntityBatchElementErrorMapping<errors::MalformedEntityReferenceBatchElementException,
                                   ErrorCode::kMalformedEntityReference>,
    EntityBatchElementErrorMapping<errors::EntityAccessErrorBatchElementException,
                                   ErrorCode::kEntityAccessError>,
    EntityBatchElementErrorMapping<errors::EntityResolutionErrorBatchElementException,
                                   ErrorCode::kEntityResolutionError>,
    InvalidTraitsDataBatchElementErrorMapping<errors::InvalidTraitsDataBatchElementException,
                                              ErrorCode::kInvalidTraitsData>,
    InvalidTraitsDataBatchElementErrorMapping<errors::InvalidPreflightHintBatchElementException,
                                              ErrorCode::kInvalidPreflightHint>,
    InvalidTraitSetBatchElementErrorMapping<errors::InvalidTraitSetBatchElementException,
                                            ErrorCode::kInvalidTraitSet>>;

TEMPLATE_LIST_TEST_CASE("BatchElementError conversion to exceptions when resolving", "",
                        BatchElementErrorMappings) {
  namespace hostApi = openassetio::hostApi;
  using trompeloeil::_;

  // Parametrized test case variables.
  using ExpectedExceptionType = typename TestType::ExceptionType;
  const ErrorCode errorCode = TestType::kErrorCode;
  const openassetio::Str errorMessage{TestType::kErrorMessage};
  const openassetio::EntityReference expectedEntityReference =
      openassetio::EntityReference{TestType::kExpectedEntityReference};
  const openassetio::trait::TraitSet traitSet = TestType::kExpectedTraitSet;
  const openassetio::errors::BatchElementError expectedError{errorCode, errorMessage};

  GIVEN("a configured Manager instance") {
    const openassetio::ManagerFixture fixture;
    const auto& manager = fixture.manager;
    auto& mockManagerInterface = fixture.mockManagerInterface;
    const auto& context = fixture.context;
    const auto& hostSession = fixture.hostSession;
    const auto resolveAccess = openassetio::access::ResolveAccess::kRead;

    AND_GIVEN(
        "manager plugin will encounter an entity-specific error when next resolving a reference") {
      const openassetio::EntityReferences singleRef = {expectedEntityReference};

      // With error callback side effect
      REQUIRE_CALL(mockManagerInterface,
                   resolve(singleRef, traitSet, resolveAccess, context, hostSession, _, _))
          .LR_SIDE_EFFECT(_7(0, expectedError));

      WHEN("resolve is called with kException errorPolicyTag") {
        THEN("an exception is thrown") {
          try {
            manager->resolve(expectedEntityReference, traitSet, resolveAccess, context,
                             hostApi::Manager::BatchElementErrorPolicyTag::kException);
            FAIL_CHECK("Exception not thrown");
          } catch (const ExpectedExceptionType& exc) {
            TestType::assertExceptionData(exc,
                                          HasDataFor::kEntityReference | HasDataFor::kTraitSet);
            CHECK(exc.index == 0);
          }
        }
      }
    }

    AND_GIVEN(
        "manager plugin will encounter entity-specific errors when next resolving multiple "
        "references") {
      const openassetio::EntityReferences twoRefs = {
          openassetio::EntityReference{"testReference1"}, expectedEntityReference};

      // With error callback side effect
      REQUIRE_CALL(mockManagerInterface,
                   resolve(twoRefs, traitSet, resolveAccess, context, hostSession, _, _))
          .LR_SIDE_EFFECT(_7(1, expectedError))
          .LR_SIDE_EFFECT(FAIL_CHECK("Exception should have short-circuited this"));

      WHEN("resolve is called with kException errorPolicyTag") {
        THEN("an exception is thrown") {
          try {
            manager->resolve(twoRefs, traitSet, resolveAccess, context,
                             hostApi::Manager::BatchElementErrorPolicyTag::kException);
            FAIL_CHECK("Exception not thrown");
          } catch (const ExpectedExceptionType& exc) {
            TestType::assertExceptionData(exc,
                                          HasDataFor::kEntityReference | HasDataFor::kTraitSet);
            CHECK(exc.index == 1);
          }
        }
      }
    }
  }
}

SCENARIO("Preflighting entities") {
  namespace hostApi = openassetio::hostApi;
  using trompeloeil::_;

  GIVEN("a configured Manager instance") {
    const openassetio::EntityReference ref = openassetio::EntityReference{"testReference"};
    const openassetio::EntityReferences threeRefs = {
        openassetio::EntityReference{"testReference1"},
        openassetio::EntityReference{"testReference2"},
        openassetio::EntityReference{"testReference3"}};

    const openassetio::TraitsDataPtr traitsData =
        openassetio::TraitsData::make({"fakeTrait", "secondFakeTrait"});
    const openassetio::trait::TraitsDatas threeTraitsDatas{traitsData, traitsData, traitsData};

    const openassetio::ManagerFixture fixture;
    const auto& manager = fixture.manager;
    auto& mockManagerInterface = fixture.mockManagerInterface;
    const auto& context = fixture.context;
    const auto& hostSession = fixture.hostSession;
    const auto publishingAccess = openassetio::access::PublishingAccess::kWrite;

    GIVEN("manager plugin successfully preflights a single entity reference") {
      const openassetio::EntityReference expected{"preflightedRef"};

      // With success callback side effect
      REQUIRE_CALL(mockManagerInterface, preflight(openassetio::EntityReferences{ref},
                                                   openassetio::trait::TraitsDatas{traitsData},
                                                   publishingAccess, context, hostSession, _, _))
          .LR_SIDE_EFFECT(_6(0, expected));

      WHEN("singular preflight is called with default errorPolicyTag") {
        const openassetio::EntityReference actual =
            manager->preflight(ref, traitsData, publishingAccess, context);
        THEN("returned entity reference is as expected") { CHECK(expected == actual); }
      }
      WHEN("singular preflight is called with kException errorPolicyTag") {
        const openassetio::EntityReference actual =
            manager->preflight(ref, traitsData, publishingAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kException);
        THEN("returned entity reference is as expected") { CHECK(expected == actual); }
      }
      WHEN("singular preflight is called with kVariant errorPolicyTag") {
        std::variant<openassetio::errors::BatchElementError, openassetio::EntityReference> actual =
            manager->preflight(ref, traitsData, publishingAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned variant contains the expected entity reference") {
          CHECK(std::holds_alternative<openassetio::EntityReference>(actual));
          auto actualVal = std::get<openassetio::EntityReference>(actual);
          CHECK(expected == actualVal);
        }
      }
    }
    GIVEN("manager plugin successfully preflights multiple entity references") {
      const openassetio::EntityReference expected1{"ref1"};
      const openassetio::EntityReference expected2{"ref2"};
      const openassetio::EntityReference expected3{"ref3"};
      std::vector<openassetio::EntityReference> expectedVec{expected1, expected2, expected3};

      // With success callback side effect
      REQUIRE_CALL(mockManagerInterface, preflight(threeRefs, threeTraitsDatas, publishingAccess,
                                                   context, hostSession, _, _))
          .LR_SIDE_EFFECT(_6(0, expectedVec[0]))
          .LR_SIDE_EFFECT(_6(1, expectedVec[1]))
          .LR_SIDE_EFFECT(_6(2, expectedVec[2]));

      WHEN("batch preflight is called with default errorPolicyTag") {
        const std::vector<openassetio::EntityReference> actualVec =
            manager->preflight(threeRefs, threeTraitsDatas, publishingAccess, context);
        THEN("returned list of entity references is as expected") {
          CHECK(expectedVec == actualVec);
        }
      }
      WHEN("batch preflight is called with kException errorPolicyTag") {
        const std::vector<openassetio::EntityReference> actualVec =
            manager->preflight(threeRefs, threeTraitsDatas, publishingAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kException);
        THEN("returned list of entity references is as expected") {
          CHECK(expectedVec == actualVec);
        }
      }
      WHEN("batch preflight is called with kVariant errorPolicyTag") {
        std::vector<
            std::variant<openassetio::errors::BatchElementError, openassetio::EntityReference>>
            actualVec = manager->preflight(threeRefs, threeTraitsDatas, publishingAccess, context,
                                           hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned lists of variants contains the expected entity references") {
          CHECK(expectedVec.size() == actualVec.size());
          for (size_t i = 0; i < actualVec.size(); ++i) {
            CHECK(std::holds_alternative<openassetio::EntityReference>(actualVec[i]));
            auto actualVal = std::get<openassetio::EntityReference>(actualVec[i]);
            CHECK(expectedVec[i] == actualVal);
          }
        }
      }
    }
    GIVEN(
        "manager plugin successfully preflights multiple entity references in a non-index order") {
      const openassetio::EntityReference expected1{"ref1"};
      const openassetio::EntityReference expected2{"ref2"};
      const openassetio::EntityReference expected3{"ref3"};
      std::vector<openassetio::EntityReference> expectedVec{expected1, expected2, expected3};

      // With success callback side effect, given out of order.
      REQUIRE_CALL(mockManagerInterface, preflight(threeRefs, threeTraitsDatas, publishingAccess,
                                                   context, hostSession, _, _))
          .LR_SIDE_EFFECT(_6(2, expectedVec[2]))
          .LR_SIDE_EFFECT(_6(0, expectedVec[0]))
          .LR_SIDE_EFFECT(_6(1, expectedVec[1]));

      WHEN("batch preflight is called with default errorPolicyTag") {
        const std::vector<openassetio::EntityReference> actualVec =
            manager->preflight(threeRefs, threeTraitsDatas, publishingAccess, context);
        THEN("returned list of entity references is ordered in index order") {
          CHECK(expectedVec == actualVec);
        }
      }
      WHEN("batch preflight is called with kException errorPolicyTag") {
        const std::vector<openassetio::EntityReference> actualVec =
            manager->preflight(threeRefs, threeTraitsDatas, publishingAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kException);
        THEN("returned list of entity references is ordered in index order") {
          CHECK(expectedVec == actualVec);
        }
      }
      WHEN("batch preflight is called with kVariant errorPolicyTag") {
        std::vector<
            std::variant<openassetio::errors::BatchElementError, openassetio::EntityReference>>
            actualVec = manager->preflight(threeRefs, threeTraitsDatas, publishingAccess, context,
                                           hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned lists of variants is ordered in index order") {
          CHECK(expectedVec.size() == actualVec.size());
          for (size_t i = 0; i < actualVec.size(); ++i) {
            CHECK(std::holds_alternative<openassetio::EntityReference>(actualVec[i]));
            auto actualVal = std::get<openassetio::EntityReference>(actualVec[i]);
            CHECK(expectedVec[i] == actualVal);
          }
        }
      }
    }
    GIVEN(
        "manager plugin will encounter an entity-specific error when next preflighting a "
        "reference") {
      const openassetio::errors::BatchElementError expected{
          openassetio::errors::BatchElementError::ErrorCode::kMalformedEntityReference,
          "Error Message"};

      // With error callback side effect
      REQUIRE_CALL(mockManagerInterface, preflight(openassetio::EntityReferences{ref},
                                                   openassetio::trait::TraitsDatas{traitsData},
                                                   publishingAccess, context, hostSession, _, _))
          .LR_SIDE_EFFECT(_7(0, expected));

      WHEN("singular preflight is called with default errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(manager->preflight(ref, traitsData, publishingAccess, context),
                               openassetio::errors::MalformedEntityReferenceBatchElementException,
                               Catch::Message("Error Message [testReference]"));
        }
      }
      WHEN("singular preflight is called with kException errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(
              manager->preflight(ref, traitsData, publishingAccess, context,
                                 hostApi::Manager::BatchElementErrorPolicyTag::kException),
              openassetio::errors::MalformedEntityReferenceBatchElementException,
              Catch::Message("Error Message [testReference]"));
        }
      }
      WHEN("singular preflight is called with kVariant errorPolicyTag") {
        std::variant<openassetio::errors::BatchElementError, openassetio::EntityReference> actual =
            manager->preflight(ref, traitsData, publishingAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned variant contains the expected BatchElementError") {
          CHECK(std::holds_alternative<openassetio::errors::BatchElementError>(actual));
          const auto& actualVal = std::get<openassetio::errors::BatchElementError>(actual);
          CHECK(expected == actualVal);
        }
      }
    }
    GIVEN(
        "manager plugin will encounter entity-specific errors when next preflighting multiple "
        "references") {
      const openassetio::EntityReference expectedValue2{"ref2"};
      const openassetio::errors::BatchElementError expectedError0{
          openassetio::errors::BatchElementError::ErrorCode::kMalformedEntityReference,
          "Malformed Mock Error🤖"};
      const openassetio::errors::BatchElementError expectedError1{
          openassetio::errors::BatchElementError::ErrorCode::kEntityAccessError,
          "Entity Access Error Message"};

      // With mixed callback side effect
      REQUIRE_CALL(mockManagerInterface, preflight(threeRefs, threeTraitsDatas, publishingAccess,
                                                   context, hostSession, _, _))
          .LR_SIDE_EFFECT(_6(2, expectedValue2))
          .LR_SIDE_EFFECT(_7(0, expectedError0))
          .LR_SIDE_EFFECT(_7(1, expectedError1));

      WHEN("batch preflight is called with default errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(
              manager->preflight(threeRefs, threeTraitsDatas, publishingAccess, context),
              openassetio::errors::MalformedEntityReferenceBatchElementException,
              Catch::Message("Malformed Mock Error🤖 [testReference1]"));
        }
      }
      WHEN("batch preflight is called with kException errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(
              manager->preflight(threeRefs, threeTraitsDatas, publishingAccess, context,
                                 hostApi::Manager::BatchElementErrorPolicyTag::kException),
              openassetio::errors::MalformedEntityReferenceBatchElementException,
              Catch::Message("Malformed Mock Error🤖 [testReference1]"));
        }
      }
      WHEN("batch preflight is called with kVariant errorPolicyTag") {
        std::vector<
            std::variant<openassetio::errors::BatchElementError, openassetio::EntityReference>>
            actualVec = manager->preflight(threeRefs, threeTraitsDatas, publishingAccess, context,
                                           hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned lists of variants contains the expected objects") {
          auto error0 = std::get<openassetio::errors::BatchElementError>(actualVec[0]);
          CHECK(error0 == expectedError0);

          auto error1 = std::get<openassetio::errors::BatchElementError>(actualVec[1]);
          CHECK(error1 == expectedError1);

          CHECK(std::get<openassetio::EntityReference>(actualVec[2]) == expectedValue2);
        }
      }
    }
  }
}

TEMPLATE_LIST_TEST_CASE("BatchElementError conversion to exceptions when preflighting", "",
                        BatchElementErrorMappings) {
  namespace hostApi = openassetio::hostApi;
  using trompeloeil::_;

  // Parametrized test case variables.
  using ExpectedExceptionType = typename TestType::ExceptionType;
  const ErrorCode errorCode = TestType::kErrorCode;
  const std::string errorMessage{TestType::kErrorMessage};
  const openassetio::EntityReference expectedEntityReference{TestType::kExpectedEntityReference};
  const openassetio::TraitsDataPtr expectedTraitsData = TestType::kExpectedTraitsData;
  const openassetio::errors::BatchElementError expectedError{errorCode, errorMessage};

  GIVEN("a configured Manager instance") {
    const openassetio::ManagerFixture fixture;
    const auto& manager = fixture.manager;
    auto& mockManagerInterface = fixture.mockManagerInterface;
    const auto& context = fixture.context;
    const auto& hostSession = fixture.hostSession;
    const auto publishingAccess = openassetio::access::PublishingAccess::kWrite;

    AND_GIVEN(
        "manager plugin will encounter an entity-specific error when next preflighting a "
        "reference") {
      const openassetio::EntityReferences singleRef = {expectedEntityReference};
      const openassetio::trait::TraitsDatas singleTraitsDatas = {expectedTraitsData};

      // With error callback side effect
      REQUIRE_CALL(mockManagerInterface, preflight(singleRef, singleTraitsDatas, publishingAccess,
                                                   context, hostSession, _, _))
          .LR_SIDE_EFFECT(_7(0, expectedError));

      WHEN("preflight is called with kException errorPolicyTag") {
        THEN("an exception is thrown") {
          try {
            manager->preflight(expectedEntityReference, expectedTraitsData, publishingAccess,
                               context, hostApi::Manager::BatchElementErrorPolicyTag::kException);
            FAIL_CHECK("Exception not thrown");
          } catch (const ExpectedExceptionType& exc) {
            TestType::assertExceptionData(exc, HasDataFor::kEntityReference |
                                                   HasDataFor::kTraitSet |
                                                   HasDataFor::kTraitsData);
            CHECK(exc.index == 0);
          }
        }
      }
    }

    AND_GIVEN(
        "manager plugin will encounter entity-specific errors when next preflighting multiple "
        "references") {
      const openassetio::EntityReferences threeRefs = {
          openassetio::EntityReference{"testReference1"}, expectedEntityReference,
          openassetio::EntityReference{"testReference3"}};

      const openassetio::TraitsDataPtr unexpectedTraitsData =
          openassetio::TraitsData::make({"fakeTrait", "secondFakeTrait"});
      const openassetio::trait::TraitsDatas threeTraitsDatas{
          unexpectedTraitsData, expectedTraitsData, unexpectedTraitsData};

      // With error callback side effect
      REQUIRE_CALL(mockManagerInterface, preflight(threeRefs, threeTraitsDatas, publishingAccess,
                                                   context, hostSession, _, _))
          .LR_SIDE_EFFECT(_7(1, expectedError))
          .LR_SIDE_EFFECT(FAIL_CHECK("Exception should have short-circuited this"));

      WHEN("preflight is called with kException errorPolicyTag") {
        THEN("an exception is thrown") {
          try {
            manager->preflight(threeRefs, threeTraitsDatas, publishingAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kException);
            FAIL_CHECK("Exception not thrown");
          } catch (const ExpectedExceptionType& exc) {
            TestType::assertExceptionData(exc, HasDataFor::kEntityReference |
                                                   HasDataFor::kTraitSet |
                                                   HasDataFor::kTraitsData);
            CHECK(exc.index == 1);
          }
        }
      }
    }
  }
}

SCENARIO("Registering entities") {
  namespace hostApi = openassetio::hostApi;
  using trompeloeil::_;

  GIVEN("a configured Manager instance") {
    const openassetio::ManagerFixture fixture;
    const auto& manager = fixture.manager;
    auto& mockManagerInterface = fixture.mockManagerInterface;
    const auto& context = fixture.context;
    const auto& hostSession = fixture.hostSession;
    const auto publishingAccess = openassetio::access::PublishingAccess::kWrite;

    // Create TraitSets used in the register method calls
    const openassetio::trait::TraitSet traits = {"fakeTrait", "secondFakeTrait"};
    const openassetio::trait::TraitsDatas threeTraitsDatas = {
        openassetio::TraitsData::make(traits), openassetio::TraitsData::make(traits),
        openassetio::TraitsData::make(traits)};

    auto singleTraitsData = openassetio::TraitsData::make(traits);
    const openassetio::trait::TraitsDatas singleTraitsDatas = {singleTraitsData};

    GIVEN("manager plugin successfully registers a single entity reference") {
      const openassetio::EntityReference ref = openassetio::EntityReference{"testReference"};
      const openassetio::EntityReferences refs = {ref};

      const openassetio::EntityReference expected =
          openassetio::EntityReference{"expectedReference"};

      // With success callback side effect
      REQUIRE_CALL(mockManagerInterface, register_(refs, singleTraitsDatas, publishingAccess,
                                                   context, hostSession, _, _))
          .LR_SIDE_EFFECT(_6(0, expected));

      WHEN("singular register is called with default errorPolicyTag") {
        const openassetio::EntityReference actual =
            manager->register_(ref, singleTraitsData, publishingAccess, context);
        THEN("returned EntityReference is as expected") { CHECK(expected == actual); }
      }
      WHEN("singular register is called with kException errorPolicyTag") {
        const openassetio::EntityReference actual =
            manager->register_(ref, singleTraitsData, publishingAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kException);
        THEN("returned EntityReference is as expected") { CHECK(expected == actual); }
      }
      WHEN("singular register is called with kVariant errorPolicyTag") {
        std::variant<openassetio::errors::BatchElementError, openassetio::EntityReference> actual =
            manager->register_(ref, singleTraitsData, publishingAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned variant contains the expected EntityReference") {
          CHECK(std::holds_alternative<openassetio::EntityReference>(actual));
          auto actualVal = std::get<openassetio::EntityReference>(actual);
          CHECK(expected == actualVal);
        }
      }
    }
    GIVEN("manager plugin successfully registers multiple entity references") {
      const openassetio::EntityReferences refs = {openassetio::EntityReference{"ref1"},
                                                  openassetio::EntityReference{"ref2"},
                                                  openassetio::EntityReference{"ref3"}};

      const openassetio::EntityReference expected1{"expectedRef1"};
      const openassetio::EntityReference expected2{"expectedRef2"};
      const openassetio::EntityReference expected3{"expectedRef3"};
      std::vector<openassetio::EntityReference> expectedVec{expected1, expected2, expected3};

      // With success callback side effect
      REQUIRE_CALL(mockManagerInterface,
                   register_(refs, threeTraitsDatas, publishingAccess, context, hostSession, _, _))
          .LR_SIDE_EFFECT(_6(0, expectedVec[0]))
          .LR_SIDE_EFFECT(_6(1, expectedVec[1]))
          .LR_SIDE_EFFECT(_6(2, expectedVec[2]));

      WHEN("batch register is called with default errorPolicyTag") {
        const std::vector<openassetio::EntityReference> actualVec =
            manager->register_(refs, threeTraitsDatas, publishingAccess, context);
        THEN("returned list of EntityReferences is as expected") {
          CHECK(expectedVec == actualVec);
        }
      }
      WHEN("batch register is called with kException errorPolicyTag") {
        const std::vector<openassetio::EntityReference> actualVec =
            manager->register_(refs, threeTraitsDatas, publishingAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kException);
        THEN("returned list of EntityReferences is as expected") {
          CHECK(expectedVec == actualVec);
        }
      }
      WHEN("batch register is called with kVariant errorPolicyTag") {
        std::vector<
            std::variant<openassetio::errors::BatchElementError, openassetio::EntityReference>>
            actualVec = manager->register_(refs, threeTraitsDatas, publishingAccess, context,
                                           hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned lists of variants contains the expected EntityReferences") {
          CHECK(expectedVec.size() == actualVec.size());
          for (size_t i = 0; i < actualVec.size(); ++i) {
            CHECK(std::holds_alternative<openassetio::EntityReference>(actualVec[i]));
            auto actualVal = std::get<openassetio::EntityReference>(actualVec[i]);
            CHECK(expectedVec[i] == actualVal);
          }
        }
      }
    }
    GIVEN(
        "manager plugin successfully registers multiple entity references in a non-index order") {
      const openassetio::EntityReferences refs = {openassetio::EntityReference{"ref1"},
                                                  openassetio::EntityReference{"ref2"},
                                                  openassetio::EntityReference{"ref3"}};

      const openassetio::EntityReference expected1{"expectedRef1"};
      const openassetio::EntityReference expected2{"expectedRef2"};
      const openassetio::EntityReference expected3{"expectedRef3"};
      std::vector<openassetio::EntityReference> expectedVec{expected1, expected2, expected3};

      // With success callback side effect, given out of order.
      REQUIRE_CALL(mockManagerInterface,
                   register_(refs, threeTraitsDatas, publishingAccess, context, hostSession, _, _))
          .LR_SIDE_EFFECT(_6(2, expectedVec[2]))
          .LR_SIDE_EFFECT(_6(0, expectedVec[0]))
          .LR_SIDE_EFFECT(_6(1, expectedVec[1]));

      WHEN("batch register is called with default errorPolicyTag") {
        const std::vector<openassetio::EntityReference> actualVec =
            manager->register_(refs, threeTraitsDatas, publishingAccess, context);
        THEN("returned list of EntityReferences is ordered in index order") {
          CHECK(expectedVec == actualVec);
        }
      }
      WHEN("batch register is called with kException errorPolicyTag") {
        const std::vector<openassetio::EntityReference> actualVec =
            manager->register_(refs, threeTraitsDatas, publishingAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kException);
        THEN("returned list of EntityReferences is ordered in index order") {
          CHECK(expectedVec == actualVec);
        }
      }
      WHEN("batch register is called with kVariant errorPolicyTag") {
        std::vector<
            std::variant<openassetio::errors::BatchElementError, openassetio::EntityReference>>
            actualVec = manager->register_(refs, threeTraitsDatas, publishingAccess, context,
                                           hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned lists of variants is ordered in index order") {
          CHECK(expectedVec.size() == actualVec.size());
          for (size_t i = 0; i < actualVec.size(); ++i) {
            CHECK(std::holds_alternative<openassetio::EntityReference>(actualVec[i]));
            auto actualVal = std::get<openassetio::EntityReference>(actualVec[i]);
            CHECK(expectedVec[i] == actualVal);
          }
        }
      }
    }
    GIVEN(
        "manager plugin will encounter an entity-specific error when next registering an "
        "EntityReference") {
      const openassetio::EntityReference ref = openassetio::EntityReference{"testReference"};
      const openassetio::EntityReferences refs = {ref};

      const openassetio::errors::BatchElementError expected{
          openassetio::errors::BatchElementError::ErrorCode::kMalformedEntityReference,
          "Error Message"};

      // With error callback side effect
      REQUIRE_CALL(mockManagerInterface, register_(refs, singleTraitsDatas, publishingAccess,
                                                   context, hostSession, _, _))
          .LR_SIDE_EFFECT(_7(0, expected));

      WHEN("singular register is called with default errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(
              manager->register_(ref, singleTraitsData, publishingAccess, context),
              openassetio::errors::MalformedEntityReferenceBatchElementException,
              Catch::Message("Error Message [testReference]"));
        }
      }
      WHEN("singular register is called with kException errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(
              manager->register_(ref, singleTraitsData, publishingAccess, context,
                                 hostApi::Manager::BatchElementErrorPolicyTag::kException),
              openassetio::errors::MalformedEntityReferenceBatchElementException,
              Catch::Message("Error Message [testReference]"));
        }
      }
      WHEN("singular register is called with kVariant errorPolicyTag") {
        std::variant<openassetio::errors::BatchElementError, openassetio::EntityReference> actual =
            manager->register_(ref, singleTraitsData, publishingAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned variant contains the expected BatchElementError") {
          CHECK(std::holds_alternative<openassetio::errors::BatchElementError>(actual));
          const auto& actualVal = std::get<openassetio::errors::BatchElementError>(actual);
          CHECK(expected == actualVal);
        }
      }
    }
    GIVEN(
        "manager plugin will encounter entity-specific errors when next registering multiple "
        "EntityReferences") {
      const openassetio::EntityReferences refs = {openassetio::EntityReference{"ref1"},
                                                  openassetio::EntityReference{"ref2"},
                                                  openassetio::EntityReference{"ref3"}};

      const openassetio::EntityReference expectedValue2{"expectedRef2"};
      const openassetio::errors::BatchElementError expectedError0{
          openassetio::errors::BatchElementError::ErrorCode::kMalformedEntityReference,
          "Malformed Mock Error🤖"};
      const openassetio::errors::BatchElementError expectedError1{
          openassetio::errors::BatchElementError::ErrorCode::kEntityAccessError,
          "Entity Access Error Message"};

      // With mixed callback side effect
      REQUIRE_CALL(mockManagerInterface,
                   register_(refs, threeTraitsDatas, publishingAccess, context, hostSession, _, _))
          .LR_SIDE_EFFECT(_6(2, expectedValue2))
          .LR_SIDE_EFFECT(_7(0, expectedError0))
          .LR_SIDE_EFFECT(_7(1, expectedError1));

      WHEN("batch register is called with default errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(
              manager->register_(refs, threeTraitsDatas, publishingAccess, context),
              openassetio::errors::MalformedEntityReferenceBatchElementException,
              Catch::Message("Malformed Mock Error🤖 [ref1]"));
        }
      }
      WHEN("batch register is called with kException errorPolicyTag") {
        THEN("an exception is thrown") {
          CHECK_THROWS_MATCHES(
              manager->register_(refs, threeTraitsDatas, publishingAccess, context,
                                 hostApi::Manager::BatchElementErrorPolicyTag::kException),
              openassetio::errors::MalformedEntityReferenceBatchElementException,
              Catch::Message("Malformed Mock Error🤖 [ref1]"));
        }
      }
      WHEN("batch register is called with kVariant errorPolicyTag") {
        std::vector<
            std::variant<openassetio::errors::BatchElementError, openassetio::EntityReference>>
            actualVec = manager->register_(refs, threeTraitsDatas, publishingAccess, context,
                                           hostApi::Manager::BatchElementErrorPolicyTag::kVariant);
        THEN("returned lists of variants contains the expected objects") {
          auto error0 = std::get<openassetio::errors::BatchElementError>(actualVec[0]);
          CHECK(error0 == expectedError0);

          auto error1 = std::get<openassetio::errors::BatchElementError>(actualVec[1]);
          CHECK(error1 == expectedError1);

          CHECK(std::get<openassetio::EntityReference>(actualVec[2]) == expectedValue2);
        }
      }
    }
  }
}

TEMPLATE_LIST_TEST_CASE("BatchElementError conversion to exceptions when registering", "",
                        BatchElementErrorMappings) {
  namespace hostApi = openassetio::hostApi;
  using trompeloeil::_;

  // Parametrized test case variables.
  using ExpectedExceptionType = typename TestType::ExceptionType;
  const ErrorCode errorCode = TestType::kErrorCode;
  const std::string errorMessage{TestType::kErrorMessage};
  const openassetio::EntityReference expectedEntityReference{TestType::kExpectedEntityReference};
  const openassetio::TraitsDataPtr expectedTraitsData = TestType::kExpectedTraitsData;

  GIVEN("a configured Manager instance") {
    const openassetio::trait::TraitSet traits = {"fakeTrait", "secondFakeTrait"};
    const openassetio::ManagerFixture fixture;
    const auto& manager = fixture.manager;
    auto& mockManagerInterface = fixture.mockManagerInterface;
    const auto& context = fixture.context;
    const auto& hostSession = fixture.hostSession;
    const auto publishingAccess = openassetio::access::PublishingAccess::kWrite;

    const openassetio::errors::BatchElementError expectedError{errorCode, errorMessage};

    AND_GIVEN(
        "manager plugin will encounter an entity-specific error when next registering a "
        "reference") {
      const openassetio::EntityReferences singleRef = {expectedEntityReference};
      const openassetio::trait::TraitsDatas singleTraitsDatas = {expectedTraitsData};

      // With error callback side effect
      REQUIRE_CALL(mockManagerInterface, register_(singleRef, singleTraitsDatas, publishingAccess,
                                                   context, hostSession, _, _))
          .LR_SIDE_EFFECT(_7(0, expectedError));

      WHEN("register is called with kException errorPolicyTag") {
        THEN("an exception is thrown") {
          try {
            manager->register_(expectedEntityReference, expectedTraitsData, publishingAccess,
                               context, hostApi::Manager::BatchElementErrorPolicyTag::kException);
            FAIL_CHECK("Exception not thrown");
          } catch (const ExpectedExceptionType& exc) {
            TestType::assertExceptionData(exc, HasDataFor::kEntityReference |
                                                   HasDataFor::kTraitSet |
                                                   HasDataFor::kTraitsData);
            CHECK(exc.index == 0);
          }
        }
      }
    }

    AND_GIVEN(
        "manager plugin will encounter entity-specific errors when next registering multiple "
        "references") {
      const openassetio::EntityReferences threeRefs = {openassetio::EntityReference{"ref1"},
                                                       expectedEntityReference,
                                                       openassetio::EntityReference{"ref3"}};
      const openassetio::TraitsDataPtr unexpectedTraitsData =
          openassetio::TraitsData::make({"fakeTrait", "secondFakeTrait"});
      const openassetio::trait::TraitsDatas threeTraitsDatas{
          unexpectedTraitsData, expectedTraitsData, unexpectedTraitsData};

      // With error callback side effect
      REQUIRE_CALL(mockManagerInterface, register_(threeRefs, threeTraitsDatas, publishingAccess,
                                                   context, hostSession, _, _))
          .LR_SIDE_EFFECT(_7(1, expectedError))
          .LR_SIDE_EFFECT(FAIL_CHECK("Exception should have short-circuited this"));

      WHEN("register is called with kException errorPolicyTag") {
        THEN("an exception is thrown") {
          try {
            manager->register_(threeRefs, threeTraitsDatas, publishingAccess, context,
                               hostApi::Manager::BatchElementErrorPolicyTag::kException);
            FAIL_CHECK("Exception not thrown");
          } catch (const ExpectedExceptionType& exc) {
            TestType::assertExceptionData(exc, HasDataFor::kEntityReference |
                                                   HasDataFor::kTraitSet |
                                                   HasDataFor::kTraitsData);
            CHECK(exc.index == 1);
          }
        }
      }
    }
  }
}
