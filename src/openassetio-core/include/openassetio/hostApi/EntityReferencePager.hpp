// SPDX-License-Identifier: Apache-2.0
// Copyright 2013-2023 The Foundry Visionmongers Ltd

#pragma once

#include <memory>

#include <openassetio/managerApi/EntityReferencePagerInterface.hpp>
#include <openassetio/typedefs.hpp>

OPENASSETIO_FWD_DECLARE(managerApi, HostSession)

namespace openassetio {
inline namespace OPENASSETIO_CORE_ABI_VERSION {
namespace hostApi {

OPENASSETIO_DECLARE_PTR(EntityReferencePager)

/**
 * The EntityReferencePager is the Host facing representation of a
 * @fqref{managerApi.PagerInterface} implementation.
 * The Manager class shouldn't be directly constructed by the host.
 * The EntityReferencePager allows for the retrieval and traversal of large datasets
 * in a paginated manner.
 *
 * Due to the variance of backends, construction, `hasNext`, `get` and
 * `next` may all reasonably need to perform non trivial, networked
 * operations, and thus performance characteristics should not be
 * assumed.singleEntityReferencePager<Elem>
 *
 * This object falling out of scope is a signal to the manager that the
 * connection query is finished. For this reason you should avoid
 * keeping hold of this object for longer than necessary.
 */
class OPENASSETIO_CORE_EXPORT EntityReferencePager {
 public:
  OPENASSETIO_ALIAS_PTR(EntityReferencePager)
  using Page = managerApi::EntityReferencePagerInterface::Page;

  [[nodiscard]] static EntityReferencePager::Ptr make(
      managerApi::EntityReferencePagerInterface::Ptr pagerInterface,
      managerApi::HostSessionPtr hostSession);

  /**
   * EntityReferencePager cannot be copied, as each object represents a single
   * paginated Query.
   * Destruction of this object is tantamount to closing the query.
   */
  EntityReferencePager(const EntityReferencePager&) = delete;
  EntityReferencePager& operator=(const EntityReferencePager&) = delete;
  EntityReferencePager(EntityReferencePager&&) noexcept = default;
  EntityReferencePager& operator=(EntityReferencePager&&) noexcept = default;
  ~EntityReferencePager() = default;

  /**
   * Return whether or not there is more data accessible by advancing
   * the page.
   */
  bool hasNext();

  /**
   * Return the current page data.
   */
  Page get();

  /**
   * Advance the page.
   */
  void next();

 private:
  EntityReferencePager(managerApi::EntityReferencePagerInterface::Ptr pagerInterface,
                       managerApi::HostSessionPtr hostSession);

  managerApi::EntityReferencePagerInterface::Ptr pagerInterface_;
  managerApi::HostSessionPtr hostSession_;
};
}  // namespace hostApi
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio
