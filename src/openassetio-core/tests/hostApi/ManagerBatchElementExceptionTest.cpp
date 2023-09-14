// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 The Foundry Visionmongers Ltd
#include <fmt/format.h>

#include "./ManagerTest.hpp"

#include <openassetio/access.hpp>

namespace errors = openassetio::errors;
namespace access = openassetio::access;
using ErrorCode = errors::BatchElementError::ErrorCode;

/// Boolean flags to describe what data an API function should provide
/// as data members to a BatchElementException.
enum HasDataFor {
  kEntityReference = 1 << 0,
  kTraitSet = 1 << 1,
  kTraitsData = 1 << 2,
  kAccess = 1 << 3
};

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
  /// Expected access mode to use in tests.
  static constexpr access::Access kExpectedAccess = access::Access::kWrite;
  static inline const openassetio::Str kExpectedAccessName = access::kAccessNames[kExpectedAccess];
  /// Expected trait set to use in tests, where appropriate.
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
  using Base::kExpectedAccess;
  using Base::kExpectedAccessName;
  using Base::kExpectedEntityReference;

  /// Check exception contains entity reference, if available.
  static void assertExceptionData(const T& exc, const int hasDataFor) {
    if (hasDataFor & HasDataFor::kEntityReference) {
      CHECK(exc.what() == fmt::format("{} [{}]", kErrorMessage, kExpectedEntityReference));
      CHECK(exc.entityReference == openassetio::EntityReference{kExpectedEntityReference});
    } else {
      CHECK(!exc.entityReference.has_value());
      Base::assertExceptionData(exc, hasDataFor);
    }
  }
};

/**
 * Parametrization fixture for mapping BatchElementError to an
 * exception, where an access mode can/should be provided as an
 * exception data member.
 */
template <class T, ErrorCode C>
struct EntityAccessBatchElementErrorMapping : EntityBatchElementErrorMapping<T, C> {
  using Base = EntityBatchElementErrorMapping<T, C>;
  using Base::kErrorMessage;
  using Base::kExpectedAccess;
  using Base::kExpectedAccessName;
  using Base::kExpectedEntityReference;

  /// Check exception contains entity reference and access mode, if
  /// available.
  static void assertExceptionData(const T& exc, const int hasDataFor) {
    if (hasDataFor & (HasDataFor::kEntityReference | HasDataFor::kAccess)) {
      CHECK(exc.what() == fmt::format("{} [access={}][{}]", kErrorMessage, kExpectedAccessName,
                                      kExpectedEntityReference));
      CHECK(exc.entityReference == openassetio::EntityReference{kExpectedEntityReference});
      CHECK(exc.access == kExpectedAccess);
    } else {
      if (hasDataFor & HasDataFor::kAccess) {
        CHECK(exc.access == kExpectedAccess);
      } else {
        CHECK(!exc.access.has_value());
      }
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
  static void assertExceptionData(const T& exc, const int hasDataFor) {
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
  static void assertExceptionData(const T& exc, const int hasDataFor) {
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
    EntityBatchElementErrorMapping<errors::EntityResolutionErrorBatchElementException,
                                   ErrorCode::kEntityResolutionError>,
    EntityAccessBatchElementErrorMapping<errors::EntityAccessErrorBatchElementException,
                                         ErrorCode::kEntityAccessError>,
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
  const auto resolveAccess =
      static_cast<openassetio::access::ResolveAccess>(TestType::kExpectedAccess);
  const openassetio::trait::TraitSet traitSet = TestType::kExpectedTraitSet;
  const openassetio::errors::BatchElementError expectedError{errorCode, errorMessage};

  GIVEN("a configured Manager instance") {
    const openassetio::ManagerFixture fixture;
    const auto& manager = fixture.manager;
    auto& mockManagerInterface = fixture.mockManagerInterface;
    const auto& context = fixture.context;
    const auto& hostSession = fixture.hostSession;

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
            TestType::assertExceptionData(
                exc, HasDataFor::kEntityReference | HasDataFor::kTraitSet | HasDataFor::kAccess);
            CHECK(exc.index == 1);
          }
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
  const auto publishingAccess =
      static_cast<openassetio::access::PublishingAccess>(TestType::kExpectedAccess);
  const openassetio::TraitsDataPtr expectedTraitsData = TestType::kExpectedTraitsData;
  const openassetio::errors::BatchElementError expectedError{errorCode, errorMessage};

  GIVEN("a configured Manager instance") {
    const openassetio::ManagerFixture fixture;
    const auto& manager = fixture.manager;
    auto& mockManagerInterface = fixture.mockManagerInterface;
    const auto& context = fixture.context;
    const auto& hostSession = fixture.hostSession;

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
                                                   HasDataFor::kTraitsData | HasDataFor::kAccess);
            CHECK(exc.index == 1);
          }
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
  const auto publishingAccess =
      static_cast<openassetio::access::PublishingAccess>(TestType::kExpectedAccess);
  const openassetio::TraitsDataPtr expectedTraitsData = TestType::kExpectedTraitsData;

  GIVEN("a configured Manager instance") {
    const openassetio::ManagerFixture fixture;
    const auto& manager = fixture.manager;
    auto& mockManagerInterface = fixture.mockManagerInterface;
    const auto& context = fixture.context;
    const auto& hostSession = fixture.hostSession;

    const openassetio::errors::BatchElementError expectedError{errorCode, errorMessage};

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
