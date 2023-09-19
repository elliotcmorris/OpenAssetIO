// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 The Foundry Visionmongers Ltd

#include "exceptionMessages.hpp"
#include <fmt/core.h>

namespace openassetio {
inline namespace OPENASSETIO_CORE_ABI_VERSION {
namespace errors {

Str errorCodeName(BatchElementError::ErrorCode code) {
  switch (code) {
    case BatchElementError::ErrorCode::kUnknown:
      return "unknown";
    case BatchElementError::ErrorCode::kInvalidEntityReference:
      return "invalidEntityReference";
    case BatchElementError::ErrorCode::kMalformedEntityReference:
      return "malformedEntityReference";
    case BatchElementError::ErrorCode::kEntityAccessError:
      return "entityAccessError";
    case BatchElementError::ErrorCode::kEntityResolutionError:
      return "entityResolutionError";
    case BatchElementError::ErrorCode::kInvalidPreflightHint:
      return "invalidPreflightHint";
    case BatchElementError::ErrorCode::kInvalidTraitSet:
      return "invalidTraitSet";
  }

  return "Unknown ErrorCode";
}

std::string createBatchElementExceptionMessage(const BatchElementError &err, size_t index,
                                               std::optional<EntityReference> entityReference,
                                               std::optional<access::Access> access) {
  /*
   * BatchElementException messages consist of five parts.
   * 1. The name of the error code.
   * 2. If existing, the message inside the BatchElementError.
   * 3. The index that the batch error relates to.
   * 4. If existing, the access.
   * 5. If existing, the entity reference.
   *
   * Ends up looking something like : "entityAccessError: Could not
   * access Entity [index=2] [access=read] [entity=bal:///entityRef]"
   */
  return fmt::format(
      "{}{}{}{}{}", fmt::format("{}:", errorCodeName(err.code)),
      err.message.empty() ? "" : fmt::format(" {}", err.message),
      fmt::format(" [index={}]", std::to_string(index)),
      (access.has_value() ? fmt::format(" [access={}]", access::kAccessNames[access.value()])
                          : ""),
      (entityReference.has_value()
           ? fmt::format(" [entity={}]", entityReference.value().toString())
           : ""));
}
}  // namespace errors
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio
