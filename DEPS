# When you update vars or deps, you should run
#   python tools/deps2git/deps2git.py -w .. -o .DEPS.git
# then commit both DEPS and .DEPS.git

vars = {
  'chromium_git': 'https://chromium.googlesource.com',

  'buildtools_revision':  '565d04e8741429fb1b4f26d102f2c6c3b849edeb',
  'v8_revision': 'f38c251e43cbe0dd927fa1b0350724a12b831050',
}

deps = {
  'src/buildtools':
    Var('chromium_git') + '/chromium/buildtools.git' + '@' +  Var('buildtools_revision'),

  'src/third_party/cygwin':
    Var('chromium_git') + '/chromium/deps/cygwin.git' + '@' + 'c89e446b273697fadf3a10ff1007a97c0b7de6df',

  'src/tools/deps2git':
    Var('chromium_git') + '/chromium/tools/deps2git.git' + '@' + 'f04828eb0b5acd3e7ad983c024870f17f17b06d9',

  'src/testing/gmock':
    Var('chromium_git') + '/external/googlemock.git' + '@' + '0421b6f358139f02e102c9c332ce19a33faf75be', # from svn revision 566

  'src/testing/gtest':
    Var('chromium_git') + '/external/googletest.git' + '@' + '9855a87157778d39b95eccfb201a9dc90f6d61c6', # from svn revision 746

  'src/tools/gyp':
    Var('chromium_git') + '/external/gyp.git' + '@' + '6ee91ad8659871916f9aa840d42e1513befdf638',

  'src/third_party/hunspell':
    Var('chromium_git') + '/chromium/deps/hunspell.git' + '@' + 'c956c0e97af00ef789afb2f64d02c9a5a50e6eb1',

  'src/third_party/icu':
    Var('chromium_git') + '/chromium/deps/icu.git' + '@' + 'bbb24c53b4f46025e87a5973c39fa722c8757595',

  'src/v8':
    Var('chromium_git') + '/v8/v8.git' + '@' +  Var('v8_revision'),

  'src/third_party/yasm/binaries':
    Var('chromium_git') + '/chromium/deps/yasm/binaries.git' + '@' + '52f9b3f4b0aa06da24ef8b123058bb61ee468881',

  'src/third_party/yasm/source/patched-yasm':
    Var('chromium_git') + '/chromium/deps/yasm/patched-yasm.git' + '@' + '4671120cd8558ce62ee8672ebf3eb6f5216f909b',
}

hooks = [
  # Pull GN binaries. This needs to be before running GYP below.
  {
    'name': 'gn_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-gn',
                '-s', 'src/buildtools/win/gn.exe.sha1',
    ],
  },
  # Pull clang-format binaries using checked-in hashes.
  {
    'name': 'clang_format_win',
    'pattern': '.',
    'action': [ 'download_from_google_storage',
                '--no_resume',
                '--platform=win32',
                '--no_auth',
                '--bucket', 'chromium-clang-format',
                '-s', 'src/buildtools/win/clang-format.exe.sha1',
    ],
  },
  {
    'pattern': '.',
    'action': ['src\\build\\gyp_evita.cmd']
  },

  {
    'pattern': '.',
    'action': ['src\\evita\\build\\gn_evita.cmd']
  },
]
