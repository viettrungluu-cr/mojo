// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXAMPLES_GANESH_APP_MOJO_GL_CONTEXT_H_
#define EXAMPLES_GANESH_APP_MOJO_GL_CONTEXT_H_

#include "base/basictypes.h"
#include "mojo/public/c/gles2/gles2.h"
#include "mojo/services/public/interfaces/gpu/gpu.mojom.h"

namespace mojo {
class Shell;

namespace examples {

class MojoGLContext {
 public:
  explicit MojoGLContext(Shell* shell);
  ~MojoGLContext();

  MojoGLES2Context context() const { return context_; }

 private:
  GpuPtr gpu_service_;
  MojoGLES2Context context_;

  DISALLOW_COPY_AND_ASSIGN(MojoGLContext);
};

}  // namespace examples
}  // namespace mojo

#endif  // EXAMPLES_GANESH_APP_MOJO_GL_CONTEXT_H_
