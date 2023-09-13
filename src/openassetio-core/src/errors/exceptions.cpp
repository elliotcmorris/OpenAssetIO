// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 The Foundry Visionmongers Ltd
#include <fmt/format.h>

#include <openassetio/EntityReference.hpp>
#include <openassetio/errors/exceptions.hpp>

namespace openassetio {
inline namespace OPENASSETIO_CORE_ABI_VERSION {
namespace errors {

BatchElementEntityReferenceException::BatchElementEntityReferenceException(
    std::size_t idx, BatchElementError err, std::optional<EntityReference> causedByEntityReference)
    : BatchElementException(
          idx, err,
          fmt::format("{} [{}]", err.message,
                      causedByEntityReference
                          ? (*causedByEntityReference).toString()
                          : openassetio::Str{"<entity reference not provided>"})),

      entityReference{std::move(causedByEntityReference)} {}
}  // namespace errors
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio
