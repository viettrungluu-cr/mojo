# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },

  'targets': [
    {
      'target_name': 'accessibility',
      'type': '<(component)',
      'hard_dependency': 1,
      'dependencies': [
        '../../base/base.gyp:base',
        '../gfx/gfx.gyp:gfx',
        '../gfx/gfx.gyp:gfx_geometry',
      ],
      'defines': [
        'ACCESSIBILITY_IMPLEMENTATION',
      ],
      'sources': [
        # All .cc, .h under accessibility, except unittests
        'ax_enums.cc',
        'ax_enums.h',
        'ax_node.cc',
        'ax_node.h',
        'ax_node_data.cc',
        'ax_node_data.h',
        'ax_serializable_tree.cc',
        'ax_serializable_tree.h',
        'ax_text_utils.cc',
        'ax_text_utils.h',
        'ax_tree.cc',
        'ax_tree.h',
        'ax_tree_serializer.cc',
        'ax_tree_serializer.h',
        'ax_tree_source.h',
        'ax_tree_update.cc',
        'ax_tree_update.h',
        'ax_view_state.cc',
        'ax_view_state.h',
        'platform/ax_platform_node.cc',
        'platform/ax_platform_node.h',
        'platform/ax_platform_node_base.cc',
        'platform/ax_platform_node_base.h',
        'platform/ax_platform_node_delegate.h',
        'platform/ax_platform_node_mac.h',
        'platform/ax_platform_node_mac.mm',
      ]
    },
    {
      'target_name': 'accessibility_test_support',
      'type': 'static_library',
      'dependencies': [
        '../../base/base.gyp:base',
        'accessibility'
      ],
      'sources': [
        'tree_generator.cc',
        'tree_generator.h'
      ]
    },
    {
      'target_name': 'accessibility_unittests',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:run_all_unittests',
        '../../testing/gtest.gyp:gtest',
        '../gfx/gfx.gyp:gfx',
        '../gfx/gfx.gyp:gfx_geometry',
        'accessibility',
        'accessibility_test_support',
      ],
      'sources': [
        'ax_generated_tree_unittest.cc',
        'ax_tree_serializer_unittest.cc',
        'ax_tree_unittest.cc',
      ]
    },
  ],
}
