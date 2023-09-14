// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 The Foundry Visionmongers Ltd
#include <tuple>
#include <type_traits>

#include <catch2/catch.hpp>

#include <openassetio/errors/exceptions.hpp>

namespace errors = openassetio::errors;
using AllBatchElementExceptions =
    std::tuple<errors::BatchElementException, errors::UnknownBatchElementException,
               errors::BatchElementEntityReferenceException,
               errors::InvalidEntityReferenceBatchElementException,
               errors::MalformedEntityReferenceBatchElementException,
               errors::EntityAccessErrorBatchElementException,
               errors::EntityResolutionErrorBatchElementException,
               errors::InvalidTraitsDataBatchElementException,
               errors::InvalidPreflightHintBatchElementException,
               errors::InvalidTraitSetBatchElementException>;

using EntityBatchElementExceptions =
    std::tuple<errors::BatchElementEntityReferenceException,
               errors::InvalidEntityReferenceBatchElementException,
               errors::MalformedEntityReferenceBatchElementException,
               errors::EntityResolutionErrorBatchElementException>;

using NonEntityBatchElementExceptions =
    std::tuple<errors::UnknownBatchElementException,
               errors::EntityAccessErrorBatchElementException,
               errors::InvalidTraitsDataBatchElementException,
               errors::InvalidPreflightHintBatchElementException,
               errors::InvalidTraitSetBatchElementException>;

TEMPLATE_LIST_TEST_CASE("Batch element exception hierarchy", "", AllBatchElementExceptions) {
  STATIC_REQUIRE(std::is_base_of_v<errors::OpenAssetIOException, TestType>);
  STATIC_REQUIRE(std::is_base_of_v<errors::BatchElementException, TestType>);
}

TEMPLATE_LIST_TEST_CASE("Entity batch element exception hierarchy", "",
                        EntityBatchElementExceptions) {
  STATIC_REQUIRE(std::is_base_of_v<errors::BatchElementEntityReferenceException, TestType>);
}

TEMPLATE_LIST_TEST_CASE("Non-entity batch element exception hierarchy", "",
                        NonEntityBatchElementExceptions) {
  STATIC_REQUIRE(!std::is_base_of_v<errors::BatchElementEntityReferenceException, TestType>);
}

TEST_CASE("TraitsData batch element exception hierarchy") {
  STATIC_REQUIRE(std::is_base_of_v<errors::InvalidTraitsDataBatchElementException,
                                   errors::InvalidPreflightHintBatchElementException>);
}
