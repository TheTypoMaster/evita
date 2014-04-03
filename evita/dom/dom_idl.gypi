# Copyright (c) 2013-2014 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'idl_files': [
      'clipboard/DataTransfer.idl',
      'clipboard/DataTransferItem.idl',
      'clipboard/DataTransferItemList.idl',
    ], # idl_files

    'static_js_externs_files': [
      'errors_externs.js',
      'types_externs.js',
      'bracket_externs.js',
      'commander_externs.js',
      'commands/find_and_replace_externs.js',
      'commands/spell_checker_externs.js',
      'console_externs.js',
      'text/document_externs.js',
      'windows/document_window_externs.js',
      'events/document_event_externs.js',
      'events/event_externs.js',
      'events/event_target_externs.js',
      'events/focus_event_externs.js',
      'events/form_event_externs.js',
      'events/keyboard_event_externs.js',
      'events/mouse_event_externs.js',
      'events/wheel_event_externs.js',
      'events/ui_event_externs.js',
      'events/window_event_externs.js',
      'editor_externs.js',
      'windows/editor_window_externs.js',

      'encoding/text_decoder_externs.js',
      'encoding/text_encoder_externs.js',

      'forms/button_control_externs.js',
      'forms/checkbox_control_externs.js',
      'forms/form_externs.js',
      'forms/form_control_externs.js',
      'forms/form_window_externs.js',
      'forms/label_control_externs.js',
      'forms/radio_button_control_externs.js',
      'forms/text_field_control_externs.js',
      'forms/text_field_selection_externs.js',

      'file_path_externs.js',
      'js_console_externs.js',
      'key_names_externs.js',
      'text/modes/modes_externs.js',
      'text/mutation_observer_externs.js',
      'os/file_externs.js',
      'os/process_externs.js',
      'windows/point_externs.js',
      'text/range_externs.js',
      'text/regexp_externs.js',
      'windows/document_state_externs.js',
      'windows/selection_externs.js',
      'css/style_externs.js',
      'windows/table_selection_externs.js',
      'windows/table_window_externs.js',
      'windows/tab_data_externs.js',
      'text/text_position_externs.js',
      'windows/text_selection_externs.js',
      'windows/text_window_externs.js',
      'timer_externs.js',
      'windows/window_externs.js',
      'jslib/editors_externs.js',
      'jslib/windows_externs.js',
    ], # static_js_externs_files

    # Computed variables
    'idl_file_list': '<|(idl_file.txt <@(idl_files))',
    'js_externs_dir': '<(SHARED_INTERMEDIATE_DIR)/evita',
    'static_file_list': '<|(static_js_externs.txt <@(static_js_externs_files))',

    'conditions': [
        ['"<(GENERATOR)"=="ninja"', {
          'write_file_only_if_changed': 1,
        },{
          'write_file_only_if_changed': 1,
        }],
      ], # conditions
  }, # variables

  'targets': [
    {
      'target_name': 'dom_aggregate_js_externs',
      'type': 'none',
      'dependencies': [
        'dom_idl_js_externs',
      ],
      'actions': [{
        'action_name': 'dom_aggregate_js_externs',
        'inputs': [
          '<(DEPTH)/tools/idl_to_js_externs/aggregate_js_externs.py',
          '<(idl_file_list)',
        ], # inputs
        'outputs': [
          '<(js_externs_dir)/evita_js_externs.js',
        ],
        'action': [
          'python',
          '<(DEPTH)/tools/idl_to_js_externs/aggregate_js_externs.py',
          '<(js_externs_dir)/evita_js_externs.js',
          '<(DEPTH)/tools/idl_to_js_externs/header.js',
          '<(js_externs_dir)', '<(idl_file_list)',
          '<(DEPTH)/evita', '<(static_file_list)',
        ], # action
      }], # actions
    },

    # Generate JavaScript Externs from IDL files.
    {
      'target_name': 'dom_idl_js_externs',
      'type': 'none',
      'dependencies': [
        'dom_interfaces_info',
      ],
      'sources': [ '<@(idl_files)' ],
      'rules': [{
        'rule_name': 'dom_idl_to_js_externs',
        'extension': 'idl',
        'msvs_external_rule': 1,
        'inputs': [
          '<(DEPTH)/tools/idl_to_js_externs/idl_to_js_externs.py',
          '<(js_externs_dir)/InterfacesInfo.pickle',
        ], # inputs
        'outputs': [
          '<(js_externs_dir)/<(RULE_INPUT_ROOT)_externs.js',
        ],
        'action': [
          'python',
          '<(DEPTH)/tools/idl_to_js_externs/idl_to_js_externs.py',
          '--output-dir', '<(js_externs_dir)',
          '--interfaces-info', '<(js_externs_dir)/InterfacesInfo.pickle',
          '--write-file-only-if-changed', '<@(write_file_only_if_changed)',
          '<(RULE_INPUT_PATH)',
        ], # action
      }], # rules
    }, # 'dom_idl_js_externs'
    {
      'target_name': 'dom_interfaces_info',
      'type': 'none',
      'actions': [{
        'action_name': 'compute_dom_interfaces_info',
        'inputs': [
          '<(DEPTH)/third_party/blink_idl_parser/compute_interfaces_info.py',
        ],
        'outputs': [
          '<(js_externs_dir)/InterfacesInfo.pickle',
        ],
        'action': [
          'python',
          '<(DEPTH)/third_party/blink_idl_parser/compute_interfaces_info.py',
          '--idl-files-list', '<(idl_file_list)',
          '--interfaces-info-file', '<(js_externs_dir)/InterfacesInfo.pickle',
          '--write-file-only-if-changed', '<(write_file_only_if_changed)',
        ], # action
        'message': 'Computing global information about IDL files',
      }], # actions
    }, # dom_interfaces_info
  ], # targets
}
