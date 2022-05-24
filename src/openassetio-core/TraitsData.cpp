// SPDX-License-Identifier: Apache-2.0
// Copyright 2013-2022 The Foundry Visionmongers Ltd

#include <openassetio/TraitsData.hpp>
#include <unordered_map>

namespace openassetio {
inline namespace OPENASSETIO_VERSION {
class TraitsData::Impl {
 public:
  Impl() = default;

  explicit Impl(const TraitSet& traitIds) { addTraits(traitIds); }

  Impl(const Impl& other) = default;

  ~Impl() = default;

  [[nodiscard]] TraitSet traitIds() const {
    TraitSet ids;
    ids.reserve(data_.size());
    for (const auto& item : data_) {
      ids.insert(item.first);
    }
    return ids;
  }

  [[nodiscard]] bool hasTrait(const trait::TraitId& traitId) const {
    return static_cast<bool>(data_.count(traitId));
  }

  void addTrait(const trait::TraitId& traitId) { data_[traitId]; }

  void addTraits(const TraitSet& traitIds) {
    for (const auto& traitId : traitIds) {
      data_[traitId];
    }
  }

  // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
  bool getTraitProperty(trait::property::Value* out, const trait::TraitId& traitId,
                        const trait::property::Key& propertyKey) const {
    // Use `at` deliberately to trigger exception if trait doesn't exist
    const auto& traitDict = data_.at(traitId);

    const auto& iter = traitDict.find(propertyKey);
    if (iter == traitDict.end()) {
      return false;
    }
    *out = iter->second;
    return true;
  }

  void setTraitProperty(const trait::TraitId& traitId, const trait::property::Key& propertyKey,
                        trait::property::Value propertyValue) {
    // Use subscript to ensure the trait is added if it is missing
    data_[traitId][propertyKey] = std::move(propertyValue);
  }

  bool operator==(const Impl& other) const { return data_ == other.data_; }

 private:
  using Properties = std::unordered_map<trait::property::Key, trait::property::Value>;
  using PropertiesByTrait = std::unordered_map<trait::TraitId, Properties>;
  PropertiesByTrait data_;
};

TraitsData::TraitsData() : impl_{std::make_unique<Impl>()} {}

TraitsData::TraitsData(const TraitSet& traitIds) : impl_{std::make_unique<Impl>(traitIds)} {}

TraitsData::TraitsData(const TraitsData& other) : impl_{std::make_unique<Impl>(*other.impl_)} {}

TraitsData::~TraitsData() = default;

TraitsData::TraitSet TraitsData::traitIds() const { return impl_->traitIds(); }

void TraitsData::addTrait(const trait::TraitId& traitId) { impl_->addTrait(traitId); }

void TraitsData::addTraits(const TraitSet& traitIds) { impl_->addTraits(traitIds); }

bool TraitsData::hasTrait(const trait::TraitId& traitId) const { return impl_->hasTrait(traitId); }

bool TraitsData::getTraitProperty(trait::property::Value* out, const trait::TraitId& traitId,
                                  const trait::property::Key& propertyKey) const {
  return impl_->getTraitProperty(out, traitId, propertyKey);
}

void TraitsData::setTraitProperty(const trait::TraitId& traitId,
                                  const trait::property::Key& propertyKey,
                                  trait::property::Value propertyValue) {
  impl_->setTraitProperty(traitId, propertyKey, std::move(propertyValue));
}

bool TraitsData::operator==(const TraitsData& other) const { return *impl_ == *other.impl_; }
}  // namespace OPENASSETIO_VERSION
}  // namespace openassetio
