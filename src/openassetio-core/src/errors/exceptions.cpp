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
std::string constructErrorMessage(const openassetio::Str& batchElementErrorMessage,
                                  const std::optional<EntityReference>& maybeEntityReference) {
  return maybeEntityReference
             ? fmt::format("{} [{}]", batchElementErrorMessage, (*maybeEntityReference).toString())
             : batchElementErrorMessage;
}
}  // namespace

BatchElementEntityReferenceException::BatchElementEntityReferenceException(
    std::size_t idx, BatchElementError err, std::optional<EntityReference> causedByEntityReference)
    : BatchElementException{constructErrorMessage(err.message, causedByEntityReference), idx,
                            std::move(err)},
      entityReference{std::move(causedByEntityReference)} {}

EntityAccessErrorBatchElementException::EntityAccessErrorBatchElementException(
    std::size_t idx, BatchElementError err, std::optional<EntityReference> causedByEntityReference)
    : BatchElementException{constructErrorMessage(err.message, causedByEntityReference), idx,
                            std::move(err)},
      entityReference{std::move(causedByEntityReference)} {}

InvalidTraitsDataBatchElementException::InvalidTraitsDataBatchElementException(
    std::size_t idx, BatchElementError err, std::optional<EntityReference> causedByEntityReference,
    std::optional<TraitsDataPtr> causedByTraitsData)
    : BatchElementException{constructErrorMessage(err.message, causedByEntityReference), idx,
                            std::move(err)},
      entityReference{std::move(causedByEntityReference)},
      traitsData{std::move(causedByTraitsData)} {}

InvalidTraitSetBatchElementException::InvalidTraitSetBatchElementException(
    std::size_t idx, BatchElementError err, std::optional<EntityReference> causedByEntityReference,
    std::optional<trait::TraitSet> causedByTraitSet)
    : BatchElementException{constructErrorMessage(err.message, causedByEntityReference), idx,
                            std::move(err)},
      entityReference{std::move(causedByEntityReference)},
      traitSet{std::move(causedByTraitSet)} {}

}  // namespace errors
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio
