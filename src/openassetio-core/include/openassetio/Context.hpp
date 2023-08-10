// SPDX-License-Identifier: Apache-2.0
// Copyright 2013-2022 The Foundry Visionmongers Ltd
#pragma once
#include <array>
#include <cstddef>
#include <memory>

#include <openassetio/export.h>
#include <openassetio/typedefs.hpp>

OPENASSETIO_FWD_DECLARE(TraitsData)
OPENASSETIO_FWD_DECLARE(managerApi, ManagerStateBase)

namespace openassetio {
inline namespace OPENASSETIO_CORE_ABI_VERSION {
OPENASSETIO_DECLARE_PTR(Context)

/**
 *  The Context object is used to convey information about the calling
 *  environment to a @ref manager. It encapsulates several key access
 *  properties, as well as providing additional information about the
 *  @ref host that may be useful to the @ref manager.
 *
 *  A Manager will also use this information to ensure it presents the
 *  correct UI, or behavior.
 *
 *  The Context is passed to many calls in this API, and it may, or may
 *  not need to be used directly.
 *
 *  @warning Contexts should never be directly constructed. Hosts should
 *  use @fqref{hostApi.Manager.createContext} "createContext" or
 *  @fqref{hostApi.Manager.createChildContext} "createChildContext". A
 *  Manager implementation should never need to create a context of it's
 *  own, one will always be supplied through the ManagerInterface entry
 *  points.
 */
class OPENASSETIO_CORE_EXPORT Context final {
 public:
  OPENASSETIO_ALIAS_PTR(Context)

  /**
   * @name Access Pattern
   */
  enum class Access {
    /**
     * Host intends to read data
     */
    kRead,
    /**
     * Host intends to write data. This should be the default choice for
     * write, unless the conditions for @ref Access.kCreateRelated are
     * met.
     */
    kWrite,
    /**
     * Hosts intends to write related data against a reference to
     * another entity. This is a specialization of `kWrite`, and should
     * be used when the host knows up front that it wishes to to publish
     * a new related entity, and not an update to an existing entity.
     * The canonical motivating example for this is that host may know
     * it wishes to publish a new subfolder inside an existing folder,
     * and not an update to said existing folder.
     */
    kCreateRelated,
    /// Unknown Access Pattern
    kUnknown
  };

  static constexpr std::array kAccessNames{"read", "write", "createRelated", "unknown"};
  /// @}

  /**
   * @name Data Retention
   */
  enum class Retention {
    /// Data will not be used
    kIgnored,
    /// Data will be re-used during a particular action
    kTransient,
    /// Data will be stored and re-used for the session
    kSession,
    /// Data will be permanently stored in the document
    kPermanent
  };

  static constexpr std::array kRetentionNames{"ignored", "transient", "session", "permanent"};
  /// @}

  /**
   * Describes what the @ref host is intending to do with the data.
   *
   * For example, when passed to resolve, it specifies if the @ref host
   * is about to read or write. When configuring a BrowserWidget, then
   * it will hint as to whether the Host is wanting to choose a new file
   * name to save, or open an existing one.
   *
   * When the access mode is one of the write patterns, the manager is
   * expected to abide by the following procedure:
   *
   * 1. When the reference points to a non-existant entity, that entity
   * should be created.
   *
   * 2. When the reference points to an existing entity:
   *    * a. When the access is @ref Access.kCreateRelated a new
   *      entity is created in relation to the target entity where
   *      logical (eg: a child), or error is emitted.
   *
   *    * b. When the access is @ref Access.kWrite :
   *        * i. When the trait set of the existing entity matches that
   *          of the new entity, the entity should be updated (possibly
   *          by versioning up). If this is not permitted, an error
   *          should be emitted.
   *        * ii. When the trait set of the existing entity does not
   *          match, behave as per 2a.
   */
  Access access;

  /**
   * A concession to the fact that it's not always possible to fully
   * implement the spec of this API within a @ref host.
   *
   * For example, @fqref{managerApi.ManagerInterface.register_}
   * "Manager.register()" can return an @ref entity_reference that
   * points to the newly published @ref entity. This is often not the
   * same as the reference that was passed to the call. The Host is
   * expected to store this new reference for future use. For example
   * in the case of a Scene File added to an 'open recent' menu. A
   * Manager may rely on this to ensure a reference that points to a
   * specific version is used in the future.
   *
   * In some cases - such as batch rendering of an image sequence,
   * it may not be possible to store this final reference, due to
   * constraints of the distributed natured of such a render.
   * Often, it is not actually of consequence. To allow the @ref manager
   * to handle these situations correctly, Hosts are required to set
   * this property to reflect their ability to persist this information.
   */
  Retention retention;

  /**
   * In many situations, the @ref trait_set of the desired @ref entity
   * itself is not entirely sufficient information to realize many
   * functions that a @ref manager wishes to implement. For example,
   * when determining the final file path for an Image that is about
   * to be published - knowing it came from a render catalog, rather
   * than a 'Write node' from a comp tree could result in different
   * behavior.
   *
   * The Locale uses a @fqref{TraitsData} "TraitsData" to describe in
   * more detail, what specific part of a @ref host is requesting an
   * action. In the case of a file browser for example, it may also
   * include information such as whether or not multi-selection is
   * required.
   */
  TraitsDataPtr locale;

  /**
   * The opaque state token owned by the @ref manager, used to
   * correlate all API calls made using this context.
   *
   * @see @ref stable_resolution
   */
  managerApi::ManagerStateBasePtr managerState;

  /**
   * Constructs a new context.
   *
   * @warning This method should never be called directly by host code -
   * @fqref{hostApi.Manager.createContext} "Manager.createContext"
   * should always be used instead.
   */
  [[nodiscard]] static ContextPtr make(Access access = Access::kUnknown,
                                       Retention retention = Retention::kTransient,
                                       TraitsDataPtr locale = nullptr,
                                       managerApi::ManagerStateBasePtr managerState = nullptr);
  /**
   * @return `true` if the context is a 'Read' based access
   * pattern. If the access is unknown (Access::kUnknown), then `false`
   * is returned.
   */
  [[nodiscard]] inline bool isForRead() const { return access == Access::kRead; }

  /**
   * @return `true` if the context is a 'Write' based access
   * pattern. If the access is unknown (Access::kUnknown), then `false`
   * is returned.
   */
  [[nodiscard]] inline bool isForWrite() const {
    return access == Access::kWrite || access == Access::kCreateRelated;
  }

 private:
  Context(Access access, Retention retention, TraitsDataPtr locale,
          managerApi::ManagerStateBasePtr managerState);
};
}  // namespace OPENASSETIO_CORE_ABI_VERSION
}  // namespace openassetio
