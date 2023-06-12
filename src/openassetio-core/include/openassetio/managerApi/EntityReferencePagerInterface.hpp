// SPDX-License-Identifier: Apache-2.0
// Copyright 2013-2023 The Foundry Visionmongers Ltd

#pragma once

#include <memory>
#include <vector>

#include <openassetio/export.h>
#include <openassetio/EntityReference.hpp>
#include <openassetio/typedefs.hpp>

OPENASSETIO_FWD_DECLARE(managerApi, HostSession)
OPENASSETIO_FWD_DECLARE(Context)

namespace openassetio {
inline namespace OPENASSETIO_CORE_ABI_VERSION {
namespace managerApi {

OPENASSETIO_DECLARE_PTR(EntityReferencePagerInterface)

/**
 * Deals with the retrieval of paginated data from the backend at the
 * behest of the host.
 *
 * The manager is expected to extend this type, and store data
 * data necessary to perform the paging operations on the extended
 * object, utilizing caching when possible to reduce redundant
 * queries.
 *
 * This object does not time out until the host gives up ownership. A
 * manager should implement the destructor if they wish to close any
 * open connections in response to this.
 *
 * To support as wide array of possible backends as possible,
 * OpenAssetIO places no restraints on the behaviour of this type
 * concerning performance, however, it is considered friendly to
 * document the performance characteristics of your Pager implementation.
 */
class OPENASSETIO_CORE_EXPORT EntityReferencePagerInterface {
 public:
  OPENASSETIO_ALIAS_PTR(EntityReferencePagerInterface)
  using Page = std::vector<EntityReference>;

  // Explicitly disallow copying.
  EntityReferencePagerInterface() = default;
  explicit EntityReferencePagerInterface(const EntityReferencePagerInterface&) = delete;
  EntityReferencePagerInterface& operator=(const EntityReferencePagerInterface&) = delete;
  // Allow moving.
  EntityReferencePagerInterface(EntityReferencePagerInterface&&) noexcept = default;
  EntityReferencePagerInterface& operator=(EntityReferencePagerInterface&&) noexcept = default;

  /**
   * Manager should override destructor to be notified when query has
   * finished.
   */
  virtual ~EntityReferencePagerInterface() = default;

  /**
   * Returns whether or not there is more data accessible by advancing
   * the page.
   *
   * The mechanism to acquire this information is variable, and left up
   * to the specifics of the backend implementation.
   */
  virtual bool hasNext(const HostSessionPtr&) = 0;

  /**
   * Return the current page data.
   */
  virtual Page get(const HostSessionPtr&) = 0;

  /**
   * Advance the page.
   */
  virtual void next(const HostSessionPtr&) = 0;
};
}  // namespace managerApi
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio
