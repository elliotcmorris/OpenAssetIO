// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 The Foundry Visionmongers Ltd
#include <string_view>

#include <fmt/format.h>

#include <openassetio/EntityReference.hpp>
#include <openassetio/errors/exceptions.hpp>
#include <openassetio/typedefs.hpp>

namespace openassetio {
inline namespace OPENASSETIO_CORE_ABI_VERSION {
namespace errors {

namespace {
std::string constructEntityErrorMessage(
    const openassetio::Str& batchElementErrorMessage,
    const std::optional<EntityReference>& maybeEntityReference) {
  return maybeEntityReference
             ? fmt::format("{} [{}]", batchElementErrorMessage, (*maybeEntityReference).toString())
             : batchElementErrorMessage;
}
std::string constructEntityAccessErrorMessage(
    const openassetio::Str& batchElementErrorMessage,
    const std::optional<EntityReference>& maybeEntityReference,
    const std::optional<access::Access>& maybeAccess) {
  return (maybeEntityReference && maybeAccess)
             ? fmt::format("{} [access={}][{}]", batchElementErrorMessage,
                           access::kAccessNames[*maybeAccess], (*maybeEntityReference).toString())
             : constructEntityErrorMessage(batchElementErrorMessage, maybeEntityReference);
}
}  // namespace

BatchElementEntityReferenceException::BatchElementEntityReferenceException(
    std::size_t idx, BatchElementError err, std::optional<EntityReference> causedByEntityReference)
    : BatchElementException{constructEntityErrorMessage(err.message, causedByEntityReference), idx,
                            std::move(err)},
      entityReference{std::move(causedByEntityReference)} {}

EntityAccessErrorBatchElementException::EntityAccessErrorBatchElementException(
    std::size_t idx, BatchElementError err, std::optional<EntityReference> causedByEntityReference,
    std::optional<access::Access> causedByAccess)
    : BatchElementException{constructEntityAccessErrorMessage(err.message, causedByEntityReference,
                                                              causedByAccess),
                            idx, std::move(err)},
      entityReference{std::move(causedByEntityReference)},
      access{causedByAccess} {}

InvalidTraitsDataBatchElementException::InvalidTraitsDataBatchElementException(
    std::size_t idx, BatchElementError err, std::optional<EntityReference> causedByEntityReference,
    std::optional<TraitsDataPtr> causedByTraitsData)
    : BatchElementException{constructEntityErrorMessage(err.message, causedByEntityReference), idx,
                            std::move(err)},
      entityReference{std::move(causedByEntityReference)},
      traitsData{std::move(causedByTraitsData)} {}

InvalidTraitSetBatchElementException::InvalidTraitSetBatchElementException(
    std::size_t idx, BatchElementError err, std::optional<EntityReference> causedByEntityReference,
    std::optional<trait::TraitSet> causedByTraitSet)
    : BatchElementException{constructEntityErrorMessage(err.message, causedByEntityReference), idx,
                            std::move(err)},
      entityReference{std::move(causedByEntityReference)},
      traitSet{std::move(causedByTraitSet)} {}

}  // namespace errors
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio
