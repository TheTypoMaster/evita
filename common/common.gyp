# Copyright (c) 2013 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'evita_code': 1,
  }, # variables

  'includes': [
    '../build/win_precompile.gypi',
  ], # includes

  'targets': [
    {
      'target_name': 'common',
      'type': '<(component)',
      'defines': [ 'COMMON_IMPLEMENTATION' ],
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
      ], # dependencies
      'sources': [
        'timer/timer.cc',
        'win/native_window.cc',
        'win/rect.cc',
      ], # sources
    }, # common

    {
      'target_name': 'common_unittests',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/base.gyp:run_all_unittests',
        '<(DEPTH)/common/common.gyp:common',
        '<(DEPTH)/testing/gtest.gyp:gtest',
      ], # dependencies
      'include_dirs+' : [
        '<(DEPTH)/testing/gtest/include/',
      ], # include_dirs
      'sources': [
        'castable_unittest.cc',
        'memory/ref_counted_unittest.cc',
        'memory/scoped_refptr_unittest.cc',
        'tree/node_unittest.cc',
      ], # sources
    }, # common_test
  ], # targets

  'target_defaults': {
    'msvs_disabled_warnings': [
      # C4127: conditional expression is constant
      4127, # for DCHECK_XX
      # C4310: cast truncates constant value
      4310, # for base/basictypes.h
     ],
  }, # target_defaults
}
