// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 The Foundry Visionmongers Ltd
#include <utility>
#include <vector>

#include <openassetio/Context.hpp>
#include <openassetio/EntityReference.hpp>
#include <openassetio/errors/exceptions.hpp>
#include <openassetio/hostApi/Manager.hpp>
#include <openassetio/trait/TraitsData.hpp>
#include <openassetio/typedefs.hpp>

#include "../errors/exceptionMessages.hpp"

namespace openassetio {
inline namespace OPENASSETIO_CORE_ABI_VERSION {
namespace hostApi {
// The definitions below are the "convenience" method signatures -
// alternate, often friendlier signatures wrapping the core batch-first
// callback-based member functions found in `Manager.cpp`

/******************************************
 * entityTraits
 ******************************************/

// Singular Except
trait::TraitSet hostApi::Manager::entityTraits(
    const EntityReference &entityReference, const access::EntityTraitsAccess entityTraitsAccess,
    const ContextConstPtr &context,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Exception &errorPolicyTag) {
  trait::TraitSet result;
  entityTraits(
      {entityReference}, entityTraitsAccess, context,
      [&result]([[maybe_unused]] std::size_t index, trait::TraitSet traitSet) {
        result = std::move(traitSet);
      },
      [&entityReference, entityTraitsAccess](std::size_t index, errors::BatchElementError error) {
        auto msg = errors::createBatchElementExceptionMessage(
            error, index, entityReference,
            static_cast<internal::access::Access>(entityTraitsAccess));
        throw errors::BatchElementException(index, std::move(error), msg);
      });

  return result;
}

// Singular variant
std::variant<errors::BatchElementError, trait::TraitSet> hostApi::Manager::entityTraits(
    const EntityReference &entityReference, const access::EntityTraitsAccess entityTraitsAccess,
    const ContextConstPtr &context,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Variant &errorPolicyTag) {
  std::variant<errors::BatchElementError, trait::TraitSet> result;
  entityTraits(
      {entityReference}, entityTraitsAccess, context,
      [&result]([[maybe_unused]] std::size_t index, trait::TraitSet traitSet) {
        result = std::move(traitSet);
      },
      [&result]([[maybe_unused]] std::size_t index, errors::BatchElementError error) {
        result = std::move(error);
      });

  return result;
}

// Multi except
std::vector<trait::TraitSet> hostApi::Manager::entityTraits(
    const EntityReferences &entityReferences, const access::EntityTraitsAccess entityTraitsAccess,
    const ContextConstPtr &context,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Exception &errorPolicyTag) {
  std::vector<trait::TraitSet> results;
  results.resize(entityReferences.size());

  entityTraits(
      entityReferences, entityTraitsAccess, context,
      [&results](std::size_t index, trait::TraitSet traitSet) {
        results[index] = std::move(traitSet);
      },
      [&entityReferences, entityTraitsAccess](std::size_t index, errors::BatchElementError error) {
        // Implemented as if FAILFAST is true.
        auto msg = errors::createBatchElementExceptionMessage(
            error, index, entityReferences[index],
            static_cast<internal::access::Access>(entityTraitsAccess));
        throw errors::BatchElementException(index, std::move(error), msg);
      });

  return results;
}

// Multi variant
std::vector<std::variant<errors::BatchElementError, trait::TraitSet>>
hostApi::Manager::entityTraits(
    const EntityReferences &entityReferences, const access::EntityTraitsAccess entityTraitsAccess,
    const ContextConstPtr &context,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Variant &errorPolicyTag) {
  std::vector<std::variant<errors::BatchElementError, trait::TraitSet>> results;
  results.resize(entityReferences.size());
  entityTraits(
      entityReferences, entityTraitsAccess, context,
      [&results](std::size_t index, trait::TraitSet traitSet) {
        results[index] = std::move(traitSet);
      },
      [&results](std::size_t index, errors::BatchElementError error) {
        results[index] = std::move(error);
      });

  return results;
}

/******************************************
 * resolve
 ******************************************/

// Singular Except
trait::TraitsDataPtr hostApi::Manager::resolve(
    const EntityReference &entityReference, const trait::TraitSet &traitSet,
    const access::ResolveAccess resolveAccess, const ContextConstPtr &context,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Exception &errorPolicyTag) {
  trait::TraitsDataPtr resolveResult;
  resolve(
      {entityReference}, traitSet, resolveAccess, context,
      [&resolveResult]([[maybe_unused]] std::size_t index, trait::TraitsDataPtr data) {
        resolveResult = std::move(data);
      },
      [&entityReference, resolveAccess](std::size_t index, errors::BatchElementError error) {
        auto msg = errors::createBatchElementExceptionMessage(
            error, index, entityReference, static_cast<internal::access::Access>(resolveAccess));
        throw errors::BatchElementException(index, std::move(error), msg);
      });

  return resolveResult;
}

// Singular variant
std::variant<errors::BatchElementError, trait::TraitsDataPtr> hostApi::Manager::resolve(
    const EntityReference &entityReference, const trait::TraitSet &traitSet,
    const access::ResolveAccess resolveAccess, const ContextConstPtr &context,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Variant &errorPolicyTag) {
  std::variant<errors::BatchElementError, trait::TraitsDataPtr> resolveResult;
  resolve(
      {entityReference}, traitSet, resolveAccess, context,
      [&resolveResult]([[maybe_unused]] std::size_t index, trait::TraitsDataPtr data) {
        resolveResult = std::move(data);
      },
      [&resolveResult]([[maybe_unused]] std::size_t index, errors::BatchElementError error) {
        resolveResult = std::move(error);
      });

  return resolveResult;
}

// Multi except
std::vector<trait::TraitsDataPtr> hostApi::Manager::resolve(
    const EntityReferences &entityReferences, const trait::TraitSet &traitSet,
    const access::ResolveAccess resolveAccess, const ContextConstPtr &context,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Exception &errorPolicyTag) {
  std::vector<trait::TraitsDataPtr> resolveResult;
  resolveResult.resize(entityReferences.size());

  resolve(
      entityReferences, traitSet, resolveAccess, context,
      [&resolveResult](std::size_t index, trait::TraitsDataPtr data) {
        resolveResult[index] = std::move(data);
      },
      [&entityReferences, resolveAccess](std::size_t index, errors::BatchElementError error) {
        // Implemented as if FAILFAST is true.
        auto msg = errors::createBatchElementExceptionMessage(
            error, index, entityReferences[index],
            static_cast<internal::access::Access>(resolveAccess));
        throw errors::BatchElementException(index, std::move(error), msg);
      });

  return resolveResult;
}

// Multi variant
std::vector<std::variant<errors::BatchElementError, trait::TraitsDataPtr>>
hostApi::Manager::resolve(
    const EntityReferences &entityReferences, const trait::TraitSet &traitSet,
    const access::ResolveAccess resolveAccess, const ContextConstPtr &context,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Variant &errorPolicyTag) {
  std::vector<std::variant<errors::BatchElementError, trait::TraitsDataPtr>> resolveResult;
  resolveResult.resize(entityReferences.size());
  resolve(
      entityReferences, traitSet, resolveAccess, context,
      [&resolveResult](std::size_t index, trait::TraitsDataPtr data) {
        resolveResult[index] = std::move(data);
      },
      [&resolveResult](std::size_t index, errors::BatchElementError error) {
        resolveResult[index] = std::move(error);
      });

  return resolveResult;
}

/******************************************
 * preflight
 ******************************************/

EntityReference Manager::preflight(
    const EntityReference &entityReference, const trait::TraitsDataPtr &traitsHint,
    const access::PublishingAccess publishingAccess, const ContextConstPtr &context,
    [[maybe_unused]] const Manager::BatchElementErrorPolicyTag::Exception &errorPolicyTag) {
  EntityReference result{""};
  preflight(
      {entityReference}, {traitsHint}, publishingAccess, context,
      [&result]([[maybe_unused]] std::size_t index, EntityReference preflightedRef) {
        result = std::move(preflightedRef);
      },
      [&entityReference, publishingAccess](std::size_t index, errors::BatchElementError error) {
        auto msg = errors::createBatchElementExceptionMessage(
            error, index, entityReference,
            static_cast<internal::access::Access>(publishingAccess));
        throw errors::BatchElementException(index, std::move(error), msg);
      });

  return result;
}

std::variant<errors::BatchElementError, EntityReference> Manager::preflight(
    const EntityReference &entityReference, const trait::TraitsDataPtr &traitsHint,
    const access::PublishingAccess publishingAccess, const ContextConstPtr &context,
    [[maybe_unused]] const Manager::BatchElementErrorPolicyTag::Variant &errorPolicyTag) {
  std::variant<errors::BatchElementError, EntityReference> result;
  preflight(
      {entityReference}, {traitsHint}, publishingAccess, context,
      [&result]([[maybe_unused]] std::size_t index, EntityReference preflightedRef) {
        result = std::move(preflightedRef);
      },
      [&result]([[maybe_unused]] std::size_t index, errors::BatchElementError error) {
        result = std::move(error);
      });

  return result;
}

EntityReferences Manager::preflight(
    const EntityReferences &entityReferences, const trait::TraitsDatas &traitsHints,
    access::PublishingAccess publishingAccess, const ContextConstPtr &context,
    [[maybe_unused]] const Manager::BatchElementErrorPolicyTag::Exception &errorPolicyTag) {
  EntityReferences results;
  results.resize(entityReferences.size(), EntityReference{""});

  preflight(
      entityReferences, traitsHints, publishingAccess, context,
      [&results](std::size_t index, EntityReference preflightedRef) {
        results[index] = std::move(preflightedRef);
      },
      [&entityReferences, publishingAccess](std::size_t index, errors::BatchElementError error) {
        // Implemented as if FAILFAST is true.
        auto msg = errors::createBatchElementExceptionMessage(
            error, index, entityReferences[index],
            static_cast<internal::access::Access>(publishingAccess));
        throw errors::BatchElementException(index, std::move(error), msg);
      });

  return results;
}

std::vector<std::variant<errors::BatchElementError, EntityReference>> Manager::preflight(
    const EntityReferences &entityReferences, const trait::TraitsDatas &traitsHints,
    const access::PublishingAccess publishingAccess, const ContextConstPtr &context,
    [[maybe_unused]] const Manager::BatchElementErrorPolicyTag::Variant &errorPolicyTag) {
  std::vector<std::variant<errors::BatchElementError, EntityReference>> results;
  results.resize(entityReferences.size());
  preflight(
      entityReferences, traitsHints, publishingAccess, context,
      [&results](std::size_t index, EntityReference entityReference) {
        results[index] = std::move(entityReference);
      },
      [&results](std::size_t index, errors::BatchElementError error) {
        results[index] = std::move(error);
      });

  return results;
}

/******************************************
 * register_
 ******************************************/

// Singular Except
EntityReference hostApi::Manager::register_(
    const EntityReference &entityReference, const trait::TraitsDataPtr &entityTraitsData,
    const access::PublishingAccess publishingAccess, const ContextConstPtr &context,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Exception &errorPolicyTag) {
  EntityReference result("");
  register_(
      {entityReference}, {entityTraitsData}, publishingAccess, context,
      [&result]([[maybe_unused]] std::size_t index, EntityReference registeredRef) {
        result = std::move(registeredRef);
      },
      [&entityReference, publishingAccess](std::size_t index, errors::BatchElementError error) {
        auto msg = errors::createBatchElementExceptionMessage(
            error, index, entityReference,
            static_cast<internal::access::Access>(publishingAccess));
        throw errors::BatchElementException(index, std::move(error), msg);
      });

  return result;
}

// Singular variant
std::variant<errors::BatchElementError, EntityReference> hostApi::Manager::register_(
    const EntityReference &entityReference, const trait::TraitsDataPtr &entityTraitsData,
    const access::PublishingAccess publishingAccess, const ContextConstPtr &context,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Variant &errorPolicyTag) {
  std::variant<errors::BatchElementError, EntityReference> result;
  register_(
      {entityReference}, {entityTraitsData}, publishingAccess, context,
      [&result]([[maybe_unused]] std::size_t index, EntityReference registeredRef) {
        result = std::move(registeredRef);
      },
      [&result]([[maybe_unused]] std::size_t index, errors::BatchElementError error) {
        result = std::move(error);
      });

  return result;
}

// Multi except
std::vector<EntityReference> hostApi::Manager::register_(
    const EntityReferences &entityReferences, const trait::TraitsDatas &entityTraitsDatas,
    const access::PublishingAccess publishingAccess, const ContextConstPtr &context,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Exception &errorPolicyTag) {
  std::vector<EntityReference> result;
  result.resize(entityReferences.size(), EntityReference{""});

  register_(
      entityReferences, entityTraitsDatas, publishingAccess, context,
      [&result](std::size_t index, EntityReference registeredRef) {
        result[index] = std::move(registeredRef);
      },
      [&entityReferences, publishingAccess](std::size_t index, errors::BatchElementError error) {
        // Implemented as if FAILFAST is true.
        auto msg = errors::createBatchElementExceptionMessage(
            error, index, entityReferences[index],
            static_cast<internal::access::Access>(publishingAccess));
        throw errors::BatchElementException(index, std::move(error), msg);
      });

  return result;
}

// Multi variant
std::vector<std::variant<errors::BatchElementError, EntityReference>> hostApi::Manager::register_(
    const EntityReferences &entityReferences, const trait::TraitsDatas &entityTraitsDatas,
    const access::PublishingAccess publishingAccess, const ContextConstPtr &context,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Variant &errorPolicyTag) {
  std::vector<std::variant<errors::BatchElementError, EntityReference>> result;
  result.resize(entityReferences.size());
  register_(
      entityReferences, entityTraitsDatas, publishingAccess, context,
      [&result](std::size_t index, EntityReference registeredRef) {
        result[index] = std::move(registeredRef);
      },
      [&result](std::size_t index, errors::BatchElementError error) {
        result[index] = std::move(error);
      });

  return result;
}

/******************************************
 * getWithRelationship
 ******************************************/

// Singular Except
EntityReferencePagerPtr hostApi::Manager::getWithRelationship(
    const EntityReference &entityReference, const trait::TraitsDataPtr &relationshipTraitsData,
    size_t pageSize, access::RelationsAccess relationsAccess, const ContextConstPtr &context,
    const trait::TraitSet &resultTraitSet,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Exception &errorPolicyTag) {
  EntityReferencePagerPtr result = nullptr;
  getWithRelationship(
      {entityReference}, relationshipTraitsData, pageSize, relationsAccess, context,
      [&result]([[maybe_unused]] std::size_t index, EntityReferencePagerPtr pager) {
        result = std::move(pager);
      },
      [&entityReference, relationsAccess](std::size_t index, errors::BatchElementError error) {
        auto msg = errors::createBatchElementExceptionMessage(
            error, index, entityReference, static_cast<internal::access::Access>(relationsAccess));
        throw errors::BatchElementException(index, std::move(error), msg);
      },
      resultTraitSet);

  return result;
}

// Singular Variant
std::variant<errors::BatchElementError, EntityReferencePagerPtr>
hostApi::Manager::getWithRelationship(
    const EntityReference &entityReference, const trait::TraitsDataPtr &relationshipTraitsData,
    size_t pageSize, access::RelationsAccess relationsAccess, const ContextConstPtr &context,
    const trait::TraitSet &resultTraitSet,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Variant &errorPolicyTag) {
  std::variant<errors::BatchElementError, EntityReferencePagerPtr> result;
  getWithRelationship(
      {entityReference}, relationshipTraitsData, pageSize, relationsAccess, context,
      [&result]([[maybe_unused]] std::size_t index, EntityReferencePagerPtr pager) {
        result = std::move(pager);
      },
      [&result]([[maybe_unused]] std::size_t index, errors::BatchElementError error) {
        result = std::move(error);
      },
      resultTraitSet);

  return result;
}

// Multi Except
std::vector<EntityReferencePagerPtr> hostApi::Manager::getWithRelationship(
    const EntityReferences &entityReferences, const trait::TraitsDataPtr &relationshipTraitsData,
    size_t pageSize, access::RelationsAccess relationsAccess, const ContextConstPtr &context,
    const trait::TraitSet &resultTraitSet,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Exception &errorPolicyTag) {
  std::vector<EntityReferencePagerPtr> result;
  result.resize(entityReferences.size());

  getWithRelationship(
      entityReferences, relationshipTraitsData, pageSize, relationsAccess, context,
      [&result](std::size_t index, EntityReferencePagerPtr pager) {
        result[index] = std::move(pager);
      },
      [&entityReferences, relationsAccess](std::size_t index, errors::BatchElementError error) {
        auto msg = errors::createBatchElementExceptionMessage(
            error, index, entityReferences[index],
            static_cast<internal::access::Access>(relationsAccess));
        throw errors::BatchElementException(index, std::move(error), msg);
      },
      resultTraitSet);

  return result;
}

// Multi Variant
std::vector<std::variant<errors::BatchElementError, EntityReferencePagerPtr>>
hostApi::Manager::getWithRelationship(
    const EntityReferences &entityReferences, const trait::TraitsDataPtr &relationshipTraitsData,
    size_t pageSize, access::RelationsAccess relationsAccess, const ContextConstPtr &context,
    const trait::TraitSet &resultTraitSet,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Variant &errorPolicyTag) {
  std::vector<std::variant<errors::BatchElementError, EntityReferencePagerPtr>> result;
  result.resize(entityReferences.size(), nullptr);

  getWithRelationship(
      entityReferences, relationshipTraitsData, pageSize, relationsAccess, context,
      [&result](std::size_t index, EntityReferencePagerPtr pager) {
        result[index] = std::move(pager);
      },
      [&result](std::size_t index, errors::BatchElementError error) {
        result[index] = std::move(error);
      },
      resultTraitSet);

  return result;
}

/******************************************
 * getWithRelationships
 ******************************************/

// Singular except
EntityReferencePagerPtr hostApi::Manager::getWithRelationships(
    const EntityReference &entityReference, const trait::TraitsDataPtr &relationshipTraitsData,
    size_t pageSize, access::RelationsAccess relationsAccess, const ContextConstPtr &context,
    const trait::TraitSet &resultTraitSet,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Exception &errorPolicyTag) {
  EntityReferencePagerPtr result = nullptr;
  getWithRelationships(
      entityReference, {relationshipTraitsData}, pageSize, relationsAccess, context,
      [&result]([[maybe_unused]] std::size_t index, EntityReferencePagerPtr pager) {
        result = std::move(pager);
      },
      [&entityReference, relationsAccess](std::size_t index, errors::BatchElementError error) {
        auto msg = errors::createBatchElementExceptionMessage(
            error, index, entityReference, static_cast<internal::access::Access>(relationsAccess));
        throw errors::BatchElementException(index, std::move(error), msg);
      },
      resultTraitSet);

  return result;
}

// Singular variant
std::variant<errors::BatchElementError, EntityReferencePagerPtr>
hostApi::Manager::getWithRelationships(
    const EntityReference &entityReference, const trait::TraitsDataPtr &relationshipTraitsData,
    size_t pageSize, access::RelationsAccess relationsAccess, const ContextConstPtr &context,
    const trait::TraitSet &resultTraitSet,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Variant &errorPolicyTag) {
  std::variant<errors::BatchElementError, EntityReferencePagerPtr> result;
  getWithRelationships(
      entityReference, {relationshipTraitsData}, pageSize, relationsAccess, context,
      [&result]([[maybe_unused]] std::size_t index, EntityReferencePagerPtr pager) {
        result = std::move(pager);
      },
      [&result]([[maybe_unused]] std::size_t index, errors::BatchElementError error) {
        result = std::move(error);
      },
      resultTraitSet);

  return result;
}

// Multi Except
std::vector<EntityReferencePagerPtr> hostApi::Manager::getWithRelationships(
    const EntityReference &entityReference, const trait::TraitsDatas &relationshipTraitsDatas,
    size_t pageSize, access::RelationsAccess relationsAccess, const ContextConstPtr &context,
    const trait::TraitSet &resultTraitSet,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Exception &errorPolicyTag) {
  std::vector<EntityReferencePagerPtr> result;
  result.resize(relationshipTraitsDatas.size(), nullptr);
  getWithRelationships(
      entityReference, relationshipTraitsDatas, pageSize, relationsAccess, context,
      [&result](std::size_t index, EntityReferencePagerPtr pager) {
        result[index] = std::move(pager);
      },
      [&entityReference, relationsAccess](std::size_t index, errors::BatchElementError error) {
        auto msg = errors::createBatchElementExceptionMessage(
            error, index, entityReference, static_cast<internal::access::Access>(relationsAccess));
        throw errors::BatchElementException(index, std::move(error), msg);
      },
      resultTraitSet);

  return result;
}

// Multi Variant
std::vector<std::variant<errors::BatchElementError, EntityReferencePagerPtr>>
hostApi::Manager::getWithRelationships(
    const EntityReference &entityReference, const trait::TraitsDatas &relationshipTraitsDatas,
    size_t pageSize, access::RelationsAccess relationsAccess, const ContextConstPtr &context,
    const trait::TraitSet &resultTraitSet,
    [[maybe_unused]] const BatchElementErrorPolicyTag::Variant &errorPolicyTag) {
  std::vector<std::variant<errors::BatchElementError, EntityReferencePagerPtr>> result;
  result.resize(relationshipTraitsDatas.size());
  getWithRelationships(
      entityReference, relationshipTraitsDatas, pageSize, relationsAccess, context,
      [&result](std::size_t index, EntityReferencePagerPtr pager) {
        result[index] = std::move(pager);
      },
      [&result](std::size_t index, errors::BatchElementError error) {
        result[index] = std::move(error);
      },
      resultTraitSet);

  return result;
}

}  // namespace hostApi
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio
