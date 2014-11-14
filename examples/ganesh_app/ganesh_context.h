// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXAMPLES_GANESH_APP_GANESH_CONTEXT_H_
#define EXAMPLES_GANESH_APP_GANESH_CONTEXT_H_

#include "base/basictypes.h"
#include "mojo/public/c/gles2/gles2.h"
#include "skia/ext/refptr.h"
#include "third_party/skia/include/gpu/GrContext.h"

namespace mojo {
namespace examples {

class GaneshContext {
 public:
  // The gl_context must outlive the GaneshContext.
  explicit GaneshContext(MojoGLES2Context gl_context);
  ~GaneshContext();

  GrContext* context() const { return context_.get(); }

 private:
  skia::RefPtr<GrContext> context_;

  DISALLOW_COPY_AND_ASSIGN(GaneshContext);
};

}  // namespace examples
}  // namespace mojo

#endif  // EXAMPLES_GANESH_APP_GANESH_CONTEXT_H_
