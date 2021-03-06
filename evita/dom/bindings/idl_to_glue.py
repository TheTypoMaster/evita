#!/usr/bin/python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Compile an .idl file to glue/ .cc and .h."""

import os
import sys

module_path = os.path.dirname(os.path.realpath(__file__))
third_party_path = os.path.normpath(os.path.join(
    module_path, os.pardir, os.pardir, os.pardir, 'third_party'))
sys.path.insert(1, third_party_path)
idl_compiler_path = os.path.normpath(os.path.join(
    third_party_path, 'blink_idl_parser'))
sys.path.insert(1, idl_compiler_path)

from idl_compiler import idl_filename_to_interface_name, parse_options, \
                         IdlCompiler
from code_generator_glue import CodeGeneratorGlue
from utilities import write_file

PREFIX = 'v8_glue_'

class IdlCompilerGlue(IdlCompiler):
    # It seems pylint doesn't recognize what IdlCompiler defined.
    # pylint: disable=E1101
    def __init__(self, *args, **kwargs):
        IdlCompiler.__init__(self, *args, **kwargs)
        self.code_generator = CodeGeneratorGlue(self.interfaces_info,
                                                self.output_directory)

    def compile_file(self, idl_filename):
        definitions = self.reader.read_idl_definitions(idl_filename)

        files = self.code_generator.generate_code(definitions)

        for file_data in files:
            file_name = os.path.join(self.output_directory,
                                     PREFIX + file_data['file_name'])
            write_file(file_data['contents'], file_name, self.only_if_changed)


def main():
    options, idl_filename = parse_options()
    idl_compiler = IdlCompilerGlue(
        options.output_directory,
        interfaces_info_filename=options.interfaces_info_file,
        only_if_changed=options.write_file_only_if_changed)
    idl_compiler.compile_file(idl_filename)


if __name__ == '__main__':
    sys.exit(main())
