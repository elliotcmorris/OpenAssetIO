// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 The Foundry Visionmongers Ltd
#pragma once

#include <stdexcept>
#include <string>
#include <utility>

#include <openassetio/export.h>
#include <openassetio/EntityReference.hpp>
#include <openassetio/TraitsData.hpp>
#include <openassetio/access.hpp>
#include <openassetio/errors/BatchElementError.hpp>
#include <openassetio/trait/collection.hpp>
#include <openassetio/typedefs.hpp>

namespace openassetio {
inline namespace OPENASSETIO_CORE_ABI_VERSION {
/**
 * This namespace contains types related to error handling.
 *
 * Generally there are two types of error handling, "standard" non-batch
 * error handling, which is exception based, and batch errors, which are
 * based upon the @ref BatchElementError type.
 *
 * All exceptions in OpenAssetIO are derived from the @ref
 * OpenAssetIOException type, the idea being a host can use this as a
 * catch-all exception type when attempting mitigative exception
 * handling.
 *
 * Batch error handling with @ref BatchElementError is not
 * exceptional, However, OpenAssetIO provides convenience wrappers
 * around some batch functions that makes them exceptional, therefore
 * each @ref BatchElementError also has a
 * @ref BatchElementException twin, also found in this namespace.
 */
namespace errors {

/**
 * @name OpenassetIO Exceptions
 * @{
 */

/**
 * Exception base for all OpenAssetIO exceptions.
 *
 * Should normally not be constructed directly, favour the more fully
 * derived exceptions instead.
 */
struct OPENASSETIO_CORE_EXPORT OpenAssetIOException : std::runtime_error {
  using std::runtime_error::runtime_error;
};

/**
 * Thrown whenever the input to a public API function is invalid for the
 * requested operation.
 */
struct OPENASSETIO_CORE_EXPORT InputValidationException : OpenAssetIOException {
  using OpenAssetIOException::OpenAssetIOException;
};

/**
 * A special case of InputValidationException for cases where the input
 * comes from external config, rather than function arguments.
 *
 * Thrown whenever a procedure must abort due to misconfigured
 * user-provided configuration, often relating to the plugin system.
 */
struct OPENASSETIO_CORE_EXPORT ConfigurationException : InputValidationException {
  using InputValidationException::InputValidationException;
};

/*
 * Thrown whenever a procedure must abort due to not being implemented.
 * Many methods in OpenAssetIO are optionally implementable, and some
 * may throw this exception to indicate that calling them constitutes
 * an error.
 */
struct OPENASSETIO_CORE_EXPORT NotImplementedException : OpenAssetIOException {
  using OpenAssetIOException::OpenAssetIOException;
};

/**
 * Exceptions emitted from manager plugins that are not handled will
 * be converted to this type and re-thrown when the exception passes
 * through the OpenAssetIO middleware.
 */
struct OPENASSETIO_CORE_EXPORT UnhandledException : OpenAssetIOException {
  using OpenAssetIOException::OpenAssetIOException;
};
/**
 * @}
 */

/**
 * @name Batch Element Exceptions
 *
 * @{
 */

/**
 * Exception base that ties together a @ref errors.BatchElementError and
 * an index.
 *
 * When thrown from a function, indicates that a particular
 * element has caused an error. The specific element that has errored
 * is indicated by the index attribute, relative to the input container.
 */
struct OPENASSETIO_CORE_EXPORT BatchElementException : OpenAssetIOException {
  BatchElementException(std::size_t idx, BatchElementError err)
      : OpenAssetIOException{err.message}, index{idx}, error{std::move(err)} {}

  BatchElementException(const std::string& msg, std::size_t idx, BatchElementError err)
      : OpenAssetIOException{msg}, index{idx}, error{std::move(err)} {}

  /**
   * Index describing which batch element has caused an error.
   */
  std::size_t index;

  /**
   * Object describing the nature of the specific error.
   */
  BatchElementError error;
};

/**
 * Exception equivalent of
 * @ref errors.BatchElementError.ErrorCode.kUnknown
 */
struct OPENASSETIO_CORE_EXPORT UnknownBatchElementException : BatchElementException {
  using BatchElementException::BatchElementException;
};

/**
 * Intermediate base for BatchElementExceptions where the batch axis
 * is along entity references,
 *
 * These errors are therefore capable of providing the individual @ref
 * EntityReference in the batch that caused the failure.
 */
struct OPENASSETIO_CORE_EXPORT BatchElementEntityReferenceException : BatchElementException {
  BatchElementEntityReferenceException(std::size_t idx, BatchElementError err,
                                       std::optional<EntityReference> causedByEntityReference);

  /**
   * Entity that the error relates to, if available.
   */
  std::optional<EntityReference> entityReference;
};

/**
 * Exception equivalent of
 * @ref errors.BatchElementError.ErrorCode.kInvalidEntityReference
 */
struct OPENASSETIO_CORE_EXPORT InvalidEntityReferenceBatchElementException
    : BatchElementEntityReferenceException {
  using BatchElementEntityReferenceException::BatchElementEntityReferenceException;
};

/**
 * Exception equivalent of
 * @ref errors.BatchElementError.ErrorCode.kMalformedEntityReference
 */
struct OPENASSETIO_CORE_EXPORT MalformedEntityReferenceBatchElementException
    : BatchElementEntityReferenceException {
  using BatchElementEntityReferenceException::BatchElementEntityReferenceException;
};

/**
 * Exception equivalent of
 * @ref errors.BatchElementError.ErrorCode.kEntityResolutionError
 */
struct OPENASSETIO_CORE_EXPORT EntityResolutionErrorBatchElementException
    : BatchElementEntityReferenceException {
  using BatchElementEntityReferenceException::BatchElementEntityReferenceException;
};

/**
 * Exception equivalent of
 * @ref errors.BatchElementError.ErrorCode.kEntityAccessError
 */
struct OPENASSETIO_CORE_EXPORT EntityAccessErrorBatchElementException : BatchElementException {
  EntityAccessErrorBatchElementException(std::size_t idx, BatchElementError err,
                                         std::optional<EntityReference> maybeEntityReference,
                                         std::optional<access::Access> causedByAccess);
  /**
   * Entity that the error relates to, if available.
   */
  std::optional<EntityReference> entityReference;
  /**
   * Access mode that the error relates to, if available.
   */
  std::optional<access::Access> access;
};

/**
 * Exception equivalent of
 * @ref errors.BatchElementError.ErrorCode.InvalidTraitsData
 *
 * Although the batch axis is along TraitsData and not  @ref
 * EntityReference, these errors may optionally be able to provide a
 * contextual @ref EntityReference.
 */
struct OPENASSETIO_CORE_EXPORT InvalidTraitsDataBatchElementException : BatchElementException {
  InvalidTraitsDataBatchElementException(std::size_t idx, BatchElementError err,
                                         std::optional<EntityReference> maybeEntityReference,
                                         std::optional<TraitsDataPtr> causedByTraitsData);
  /**
   * Entity that the error relates to, if available.
   */
  std::optional<EntityReference> entityReference;
  /**
   * Traits and properties that the error relates to, if available.
   */
  std::optional<TraitsDataPtr> traitsData;
};

/**
 * Exception equivalent of
 * @ref errors.BatchElementError.ErrorCode.kInvalidPreflightHint
 */
struct OPENASSETIO_CORE_EXPORT InvalidPreflightHintBatchElementException
    : InvalidTraitsDataBatchElementException {
  using InvalidTraitsDataBatchElementException::InvalidTraitsDataBatchElementException;
};

/**
 * Exception equivalent of
 * @ref errors.BatchElementError.ErrorCode.kInvalidTraitSet
 */
struct OPENASSETIO_CORE_EXPORT InvalidTraitSetBatchElementException : BatchElementException {
  InvalidTraitSetBatchElementException(std::size_t idx, BatchElementError err,
                                       std::optional<EntityReference> maybeEntityReference,
                                       std::optional<trait::TraitSet> causedByTraitSet);

  /**
   * Entity that the error relates to, if available.
   */
  std::optional<EntityReference> entityReference;
  /**
   * Trait set that the error relates to, if available.
   */
  std::optional<trait::TraitSet> traitSet;
};

/**
 * @}
 */
}  // namespace errors
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio
