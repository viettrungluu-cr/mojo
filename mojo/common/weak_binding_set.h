// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "mojo/public/cpp/bindings/binding.h"

namespace mojo {

template <typename Interface>
class WeakBinding;

// Use this class to manage a set of weak pointers to bindings each of which is
// owned by the pipe they are bound to.
template <typename Interface>
class WeakBindingSet {
 public:
  WeakBindingSet() {}
  ~WeakBindingSet() { CloseAllBindings(); }

  void AddBinding(Interface* impl, InterfaceRequest<Interface> request) {
    auto binding = new WeakBinding<Interface>(impl, request.Pass());
    bindings_.push_back(binding->GetWeakPtr());
    ClearNullBindings();
  }

  void CloseAllBindings() {
    for (const auto& it : bindings_) {
      if (it)
        it->Close();
    }
    bindings_.clear();
  }

  template <typename FunctionType>
  void ForAllBindings(FunctionType function) {
    for (const auto& it : bindings_) {
      if (it)
        function(it.get()->client());
    }
    ClearNullBindings();
  }

 private:
  void ClearNullBindings() {
    bindings_.erase(
        std::remove_if(bindings_.begin(), bindings_.end(),
                       [](const base::WeakPtr<WeakBinding<Interface>>& p) {
          return p.get() == nullptr;
        }),
        bindings_.end());
  }

  std::vector<base::WeakPtr<WeakBinding<Interface>>> bindings_;

  DISALLOW_COPY_AND_ASSIGN(WeakBindingSet);
};

template <typename Interface>
class WeakBinding : public ErrorHandler {
 public:
  WeakBinding(Interface* impl, InterfaceRequest<Interface> request)
      : binding_(impl, request.Pass()), weak_ptr_factory_(this) {
    binding_.set_error_handler(this);
  }

  ~WeakBinding() override {}

  typename Interface::Client* client() { return binding_.client(); }

  base::WeakPtr<WeakBinding> GetWeakPtr() {
    return weak_ptr_factory_.GetWeakPtr();
  }

  void Close() { binding_.Close(); }

  // ErrorHandler implementation.
  void OnConnectionError() override { delete this; }

 private:
  mojo::Binding<Interface> binding_;
  base::WeakPtrFactory<WeakBinding> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(WeakBinding);
};

}  // namespace mojo
