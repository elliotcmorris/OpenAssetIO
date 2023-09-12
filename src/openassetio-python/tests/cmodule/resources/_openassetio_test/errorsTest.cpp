// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 The Foundry Visionmongers Ltd}
/**
 * Bindings used for testing errors behaviour.
 * Specifically, the conversion from cpp -> python.
 */

#include <pybind11/pybind11.h>

#include <openassetio/errors/errorCodes.h>
#include <openassetio/EntityReference.hpp>
#include <openassetio/TraitsData.hpp>
#include <openassetio/errors/exceptions.hpp>
#include <openassetio/trait/collection.hpp>

namespace py = pybind11;

namespace {
using openassetio::errors::BatchElementError;
using ErrorCode = BatchElementError::ErrorCode;

constexpr const char* kExceptionMessage = "Explosion!";
constexpr const char* kEntityReference = "bogus:///entity_reference";

void throwException(const std::string& exceptionName, bool populateArgs = true) {
  const auto entityRef = populateArgs ? openassetio::EntityReference{kEntityReference}
                                      : std::optional<openassetio::EntityReference>{};
  const auto traitSet = populateArgs ? openassetio::trait::TraitSet{"trait1", "trait2"}
                                     : std::optional<openassetio::trait::TraitSet>{};
  const auto traitsData = populateArgs ? openassetio::TraitsData::make(traitSet.value())
                                       : std::optional<openassetio::TraitsDataPtr>{};

  if (exceptionName == "OpenAssetIOException") {
    throw openassetio::errors::OpenAssetIOException(kExceptionMessage);
  }
  if (exceptionName == "InputValidationException") {
    throw openassetio::errors::InputValidationException(kExceptionMessage);
  }
  if (exceptionName == "ConfigurationException") {
    throw openassetio::errors::ConfigurationException(kExceptionMessage);
  }
  if (exceptionName == "NotImplementedException") {
    throw openassetio::errors::NotImplementedException(kExceptionMessage);
  }
  if (exceptionName == "UnhandledException") {
    throw openassetio::errors::UnhandledException(kExceptionMessage);
  }

  if (exceptionName == "BatchElementException") {
    throw openassetio::errors::BatchElementException(1, {ErrorCode::kUnknown, kExceptionMessage});
  }
  if (exceptionName == "BatchElementEntityReferenceException") {
    throw openassetio::errors::BatchElementEntityReferenceException(
        1,
        {
            ErrorCode::kInvalidEntityReference,
            kExceptionMessage,
        },
        entityRef);
  }
  if (exceptionName == "UnknownBatchElementException") {
    throw openassetio::errors::UnknownBatchElementException(
        1, {ErrorCode::kUnknown, kExceptionMessage});
  }
  if (exceptionName == "InvalidTraitSetBatchElementException") {
    throw openassetio::errors::InvalidTraitSetBatchElementException(
        1, {ErrorCode::kInvalidTraitSet, kExceptionMessage}, traitSet, entityRef);
  }
  if (exceptionName == "InvalidTraitsDataBatchElementException") {
    throw openassetio::errors::InvalidTraitsDataBatchElementException(
        1, {ErrorCode::kInvalidTraitsData, kExceptionMessage}, traitsData, entityRef);
  }
  if (exceptionName == "EntityAccessErrorBatchElementException") {
    throw openassetio::errors::EntityAccessErrorBatchElementException(
        1, {ErrorCode::kEntityAccessError, kExceptionMessage}, entityRef);
  }
  if (exceptionName == "InvalidEntityReferenceBatchElementException") {
    throw openassetio::errors::InvalidEntityReferenceBatchElementException(
        1, {ErrorCode::kInvalidEntityReference, kExceptionMessage}, entityRef);
  }
  if (exceptionName == "MalformedEntityReferenceBatchElementException") {
    throw openassetio::errors::MalformedEntityReferenceBatchElementException(
        1, {ErrorCode::kMalformedEntityReference, kExceptionMessage}, entityRef);
  }
  if (exceptionName == "EntityResolutionErrorBatchElementException") {
    throw openassetio::errors::EntityResolutionErrorBatchElementException(
        1, {ErrorCode::kEntityResolutionError, kExceptionMessage}, entityRef);
  }
  if (exceptionName == "InvalidPreflightHintBatchElementException") {
    throw openassetio::errors::InvalidPreflightHintBatchElementException(
        1, {ErrorCode::kInvalidPreflightHint, kExceptionMessage}, traitsData, entityRef);
  }
}
}  // namespace

void registerExceptionThrower(py::module_& mod) {
  mod.def("throwExceptionWithPopulatedArgs",
          [](const std::string& exceptionName) { throwException(exceptionName, true); });
  mod.def("throwExceptionWithUnpopulatedArgs",
          [](const std::string& exceptionName) { throwException(exceptionName, false); });
}
