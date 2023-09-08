// SPDX-License-Identifier: Apache-2.0
// Copyright 2022 The Foundry Visionmongers Ltd
#pragma once

#include <stdexcept>
#include <string>
#include <utility>

#include <openassetio/export.h>
#include <openassetio/errors/BatchElementError.hpp>
#include <openassetio/typedefs.hpp>

namespace openassetio {
inline namespace OPENASSETIO_CORE_ABI_VERSION {
namespace errors {
/**
 * @name OpenassetIO Exceptions
 *
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

/**
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
 * Exception base that ties together a @ref BatchElementError and an
 * index.
 *
 * When thrown from a function, indicates that a particular
 * element has caused an error. The specific element that has errored
 * is indicated by the index attribute, relative to the input container.
 */
struct OPENASSETIO_CORE_EXPORT BatchElementException : std::runtime_error {
  BatchElementException(std::size_t idx, BatchElementError err)
      : std::runtime_error{err.message}, index{idx}, error{std::move(err)} {}

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
 * @ref BatchElementError.ErrorCode.kUnknown
 */
struct OPENASSETIO_CORE_EXPORT UnknownBatchElementException : BatchElementException {
  using BatchElementException::BatchElementException;
};

/**
 * Exception equivalent of
 * @ref BatchElementError.ErrorCode.kInvalidEntityReference
 */
struct OPENASSETIO_CORE_EXPORT InvalidEntityReferenceBatchElementException
    : BatchElementException {
  using BatchElementException::BatchElementException;
};

/**
 * Exception equivalent of
 * @ref BatchElementError.ErrorCode.kMalformedEntityReference
 */
struct OPENASSETIO_CORE_EXPORT MalformedEntityReferenceBatchElementException
    : BatchElementException {
  using BatchElementException::BatchElementException;
};

/**
 * Exception equivalent of
 * @ref BatchElementError.ErrorCode.kEntityAccessError
 */
struct OPENASSETIO_CORE_EXPORT EntityAccessErrorBatchElementException : BatchElementException {
  using BatchElementException::BatchElementException;
};

/**
 * Exception equivalent of
 * @ref BatchElementError.ErrorCode.kEntityResolutionError
 */
struct OPENASSETIO_CORE_EXPORT EntityResolutionErrorBatchElementException : BatchElementException {
  using BatchElementException::BatchElementException;
};

/**
 * Exception equivalent of
 * @ref BatchElementError.ErrorCode.kInvalidPreflightHint
 */
struct OPENASSETIO_CORE_EXPORT InvalidPreflightHintBatchElementException : BatchElementException {
  using BatchElementException::BatchElementException;
};

/**
 * Exception equivalent of
 * @ref BatchElementError.ErrorCode.kInvalidTraitSet
 */
struct OPENASSETIO_CORE_EXPORT InvalidTraitSetBatchElementException : BatchElementException {
  using BatchElementException::BatchElementException;
};

/**
 * @}
 */
}  // namespace errors
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio
