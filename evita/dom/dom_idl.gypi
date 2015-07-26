# Copyright (c) 2013-2014 Project Vogue. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'interface_idl_files': [
      'clipboard/DataTransfer.idl',
      'clipboard/DataTransferItem.idl',
      'clipboard/DataTransferItemList.idl',
      'Editor.idl',
      'encodings/TextDecoder.idl',
      'encodings/TextEncoder.idl',
      'events/CompositionEvent.idl',
      'events/CompositionSpan.idl',
      'events/DocumentEvent.idl',
      'events/Event.idl',
      'events/EventTarget.idl',
      'events/UiEvent.idl',
      'events/FocusEvent.idl',
      'events/FormEvent.idl',
      'events/KeyboardEvent.idl',
      'events/MouseEvent.idl',
      'events/UiEvent.idl',
      'events/ViewEventTarget.idl',
      'events/WheelEvent.idl',
      'events/WindowEvent.idl',
      'forms/ButtonControl.idl',
      'forms/CheckboxControl.idl',
      'forms/Form.idl',
      'forms/FormControl.idl',
      'forms/FormWindow.idl',
      'forms/LabelControl.idl',
      'forms/RadioButtonControl.idl',
      'forms/TextFieldControl.idl',
      'forms/TextFieldSelection.idl',
      'os/AbstractFile.idl',
      'os/File.idl',
      'os/Process.idl',
      'text/modes/Mode.idl',
      'text/MutationObserver.idl',
      'text/MutationRecord.idl',
      'text/RegExp.idl',
      'timers/OneShotTimer.idl',
      'timers/RepeatingTimer.idl',
      'timers/Timer.idl',
      'windows/DocumentWindow.idl',
      'windows/EditorWindow.idl',
      'windows/Point.idl',
      'windows/Rect.idl',
      'windows/Selection.idl',
      'windows/TableSelection.idl',
      'windows/TableWindow.idl',
      'windows/TextSelection.idl',
      'windows/TextWindow.idl',
      'windows/Window.idl',
    ], # idl_files

    'dictionary_idl_files': [
      'encodings/TextDecodeOptions.idl',
      'encodings/TextDecoderOptions.idl',
      'encodings/TextEncodeOptions.idl',
      'events/CompositionEventInit.idl',
      'events/DocumentEventInit.idl',
      'events/EventInit.idl',
      'events/FocusEventInit.idl',
      'events/FormEventInit.idl',
      'events/KeyboardEventInit.idl',
      'events/MouseEventInit.idl',
      'events/UiEventInit.idl',
      'events/WheelEventInit.idl',
      'events/WindowEventInit.idl',
      'forms/FormWindowInit.idl',
      'os/MoveFileOptions.idl',
      'text/MutationObserverInit.idl',
      'text/RegExpInit.idl',
    ], # dictionary_idl_files

    'generated_glue_files': [
      '<(glue_dir)/v8_glue_AbstractFile.cc',
      '<(glue_dir)/v8_glue_AbstractFile.h',
      '<(glue_dir)/v8_glue_ButtonControl.cc',
      '<(glue_dir)/v8_glue_ButtonControl.h',
      '<(glue_dir)/v8_glue_CheckboxControl.cc',
      '<(glue_dir)/v8_glue_CheckboxControl.h',
      '<(glue_dir)/v8_glue_DataTransfer.cc',
      '<(glue_dir)/v8_glue_DataTransfer.h',
      '<(glue_dir)/v8_glue_DataTransferItem.cc',
      '<(glue_dir)/v8_glue_DataTransferItem.h',
      '<(glue_dir)/v8_glue_DataTransferItemList.cc',
      '<(glue_dir)/v8_glue_DataTransferItemList.h',
      '<(glue_dir)/v8_glue_CompositionEvent.cc',
      '<(glue_dir)/v8_glue_CompositionEvent.h',
      '<(glue_dir)/v8_glue_CompositionEventInit.cc',
      '<(glue_dir)/v8_glue_CompositionEventInit.h',
      '<(glue_dir)/v8_glue_CompositionSpan.cc',
      '<(glue_dir)/v8_glue_CompositionSpan.h',
      '<(glue_dir)/v8_glue_DocumentEvent.cc',
      '<(glue_dir)/v8_glue_DocumentEvent.h',
      '<(glue_dir)/v8_glue_DocumentEventInit.cc',
      '<(glue_dir)/v8_glue_DocumentEventInit.h',
      '<(glue_dir)/v8_glue_DocumentWindow.cc',
      '<(glue_dir)/v8_glue_DocumentWindow.h',
      '<(glue_dir)/v8_glue_Editor.cc',
      '<(glue_dir)/v8_glue_Editor.h',
      '<(glue_dir)/v8_glue_EditorWindow.cc',
      '<(glue_dir)/v8_glue_EditorWindow.h',
      '<(glue_dir)/v8_glue_Event.cc',
      '<(glue_dir)/v8_glue_Event.h',
      '<(glue_dir)/v8_glue_EventInit.cc',
      '<(glue_dir)/v8_glue_EventInit.h',
      '<(glue_dir)/v8_glue_FocusEvent.cc',
      '<(glue_dir)/v8_glue_FocusEvent.h',
      '<(glue_dir)/v8_glue_FormEvent.cc',
      '<(glue_dir)/v8_glue_FormEvent.h',
      '<(glue_dir)/v8_glue_Form.cc',
      '<(glue_dir)/v8_glue_Form.h',
      '<(glue_dir)/v8_glue_FormControl.cc',
      '<(glue_dir)/v8_glue_FormControl.h',
      '<(glue_dir)/v8_glue_FormEventInit.cc',
      '<(glue_dir)/v8_glue_FormEventInit.h',
      '<(glue_dir)/v8_glue_FormWindow.cc',
      '<(glue_dir)/v8_glue_FormWindow.h',
      '<(glue_dir)/v8_glue_FormWindowInit.cc',
      '<(glue_dir)/v8_glue_FormWindowInit.h',
      '<(glue_dir)/v8_glue_EventTarget.cc',
      '<(glue_dir)/v8_glue_EventTarget.h',
      '<(glue_dir)/v8_glue_File.cc',
      '<(glue_dir)/v8_glue_File.h',
      '<(glue_dir)/v8_glue_FocusEventInit.cc',
      '<(glue_dir)/v8_glue_FocusEventInit.h',
      '<(glue_dir)/v8_glue_KeyboardEvent.cc',
      '<(glue_dir)/v8_glue_KeyboardEvent.h',
      '<(glue_dir)/v8_glue_KeyboardEventInit.cc',
      '<(glue_dir)/v8_glue_KeyboardEventInit.h',
      '<(glue_dir)/v8_glue_LabelControl.cc',
      '<(glue_dir)/v8_glue_LabelControl.h',
      '<(glue_dir)/v8_glue_MouseEvent.cc',
      '<(glue_dir)/v8_glue_MouseEvent.h',
      '<(glue_dir)/v8_glue_MouseEventInit.cc',
      '<(glue_dir)/v8_glue_MouseEventInit.h',
      '<(glue_dir)/v8_glue_MoveFileOptions.cc',
      '<(glue_dir)/v8_glue_MoveFileOptions.h',
      '<(glue_dir)/v8_glue_MutationObserver.cc',
      '<(glue_dir)/v8_glue_MutationObserver.h',
      '<(glue_dir)/v8_glue_MutationObserverInit.cc',
      '<(glue_dir)/v8_glue_MutationObserverInit.h',
      '<(glue_dir)/v8_glue_MutationRecord.cc',
      '<(glue_dir)/v8_glue_MutationRecord.h',
      '<(glue_dir)/v8_glue_OneShotTimer.cc',
      '<(glue_dir)/v8_glue_OneShotTimer.h',
      '<(glue_dir)/v8_glue_Process.cc',
      '<(glue_dir)/v8_glue_Process.h',
      '<(glue_dir)/v8_glue_RadioButtonControl.cc',
      '<(glue_dir)/v8_glue_RadioButtonControl.h',
      '<(glue_dir)/v8_glue_RegExp.cc',
      '<(glue_dir)/v8_glue_RegExp.h',
      '<(glue_dir)/v8_glue_RegExpInit.cc',
      '<(glue_dir)/v8_glue_RegExpInit.h',
      '<(glue_dir)/v8_glue_RepeatingTimer.cc',
      '<(glue_dir)/v8_glue_RepeatingTimer.h',
      '<(glue_dir)/v8_glue_Selection.cc',
      '<(glue_dir)/v8_glue_Selection.h',
      '<(glue_dir)/v8_glue_TableSelection.cc',
      '<(glue_dir)/v8_glue_TableSelection.h',
      '<(glue_dir)/v8_glue_TableWindow.cc',
      '<(glue_dir)/v8_glue_TableWindow.h',
      '<(glue_dir)/v8_glue_TextDecodeOptions.h',
      '<(glue_dir)/v8_glue_TextDecodeOptions.cc',
      '<(glue_dir)/v8_glue_TextDecoder.h',
      '<(glue_dir)/v8_glue_TextDecoder.cc',
      '<(glue_dir)/v8_glue_TextDecoderOptions.h',
      '<(glue_dir)/v8_glue_TextDecoderOptions.cc',
      '<(glue_dir)/v8_glue_TextEncoder.h',
      '<(glue_dir)/v8_glue_TextEncoder.cc',
      '<(glue_dir)/v8_glue_TextEncodeOptions.h',
      '<(glue_dir)/v8_glue_TextEncodeOptions.cc',
      '<(glue_dir)/v8_glue_TextFieldControl.cc',
      '<(glue_dir)/v8_glue_TextFieldControl.h',
      '<(glue_dir)/v8_glue_TextFieldSelection.cc',
      '<(glue_dir)/v8_glue_TextFieldSelection.h',
      '<(glue_dir)/v8_glue_TextSelection.h',
      '<(glue_dir)/v8_glue_TextSelection.cc',
      '<(glue_dir)/v8_glue_TextWindow.h',
      '<(glue_dir)/v8_glue_TextWindow.cc',
      '<(glue_dir)/v8_glue_Timer.cc',
      '<(glue_dir)/v8_glue_Timer.h',
      '<(glue_dir)/v8_glue_UiEvent.cc',
      '<(glue_dir)/v8_glue_UiEvent.h',
      '<(glue_dir)/v8_glue_UiEventInit.cc',
      '<(glue_dir)/v8_glue_UiEventInit.h',
      '<(glue_dir)/v8_glue_ViewEventTarget.cc',
      '<(glue_dir)/v8_glue_ViewEventTarget.h',
      '<(glue_dir)/v8_glue_WheelEvent.cc',
      '<(glue_dir)/v8_glue_WheelEvent.h',
      '<(glue_dir)/v8_glue_WheelEventInit.cc',
      '<(glue_dir)/v8_glue_WheelEventInit.h',
      '<(glue_dir)/v8_glue_Window.cc',
      '<(glue_dir)/v8_glue_Window.h',
      '<(glue_dir)/v8_glue_WindowEvent.cc',
      '<(glue_dir)/v8_glue_WindowEvent.h',
      '<(glue_dir)/v8_glue_WindowEventInit.cc',
      '<(glue_dir)/v8_glue_WindowEventInit.h',
    ],

    'generated_js_externs_files': [
      '<(js_externs_dir)/AbstractFile_externs.js',
      '<(js_externs_dir)/ButtonControl_externs.js',
      '<(js_externs_dir)/CheckboxControl_externs.js',
      '<(js_externs_dir)/CompositionEvent_externs.js',
      '<(js_externs_dir)/CompositionEventInit_externs.js',
      '<(js_externs_dir)/CompositionSpan_externs.js',
      '<(js_externs_dir)/DataTransfer_externs.js',
      '<(js_externs_dir)/DataTransferItem_externs.js',
      '<(js_externs_dir)/DataTransferItemList_externs.js',
      '<(js_externs_dir)/DocumentEvent_externs.js',
      '<(js_externs_dir)/DocumentEventInit_externs.js',
      '<(js_externs_dir)/DocumentWindow_externs.js',
      '<(js_externs_dir)/Editor_externs.js',
      '<(js_externs_dir)/EditorWindow_externs.js',
      '<(js_externs_dir)/Event_externs.js',
      '<(js_externs_dir)/EventInit_externs.js',
      '<(js_externs_dir)/File_externs.js',
      '<(js_externs_dir)/FocusEvent_externs.js',
      '<(js_externs_dir)/FocusEventInit_externs.js',
      '<(js_externs_dir)/Form_externs.js',
      '<(js_externs_dir)/FormControl_externs.js',
      '<(js_externs_dir)/FormEvent_externs.js',
      '<(js_externs_dir)/FormEventInit_externs.js',
      '<(js_externs_dir)/FormWindow_externs.js',
      '<(js_externs_dir)/FormWindowInit_externs.js',
      '<(js_externs_dir)/KeyboardEvent_externs.js',
      '<(js_externs_dir)/KeyboardEventInit_externs.js',
      '<(js_externs_dir)/LabelControl_externs.js',
      '<(js_externs_dir)/Mode_externs.js',
      '<(js_externs_dir)/MouseEvent_externs.js',
      '<(js_externs_dir)/MouseEventInit_externs.js',
      '<(js_externs_dir)/MutationObserver_externs.js',
      '<(js_externs_dir)/MutationObserverInit_externs.js',
      '<(js_externs_dir)/MutationRecord_externs.js',
      '<(js_externs_dir)/OneShotTimer_externs.js',
      '<(js_externs_dir)/Point_externs.js',
      '<(js_externs_dir)/Process_externs.js',
      '<(js_externs_dir)/RadioButtonControl_externs.js',
      '<(js_externs_dir)/Rect_externs.js',
      '<(js_externs_dir)/RegExp_externs.js',
      '<(js_externs_dir)/RegExpInit_externs.js',
      '<(js_externs_dir)/RepeatingTimer_externs.js',
      '<(js_externs_dir)/Selection_externs.js',
      '<(js_externs_dir)/TableSelection_externs.js',
      '<(js_externs_dir)/TableWindow_externs.js',
      '<(js_externs_dir)/TextDecodeOptions_externs.js',
      '<(js_externs_dir)/TextDecoder_externs.js',
      '<(js_externs_dir)/TextDecoderOptions_externs.js',
      '<(js_externs_dir)/TextEncoder_externs.js',
      '<(js_externs_dir)/TextEncodeOptions_externs.js',
      '<(js_externs_dir)/TextFieldControl_externs.js',
      '<(js_externs_dir)/TextFieldSelection_externs.js',
      '<(js_externs_dir)/TextSelection_externs.js',
      '<(js_externs_dir)/TextWindow_externs.js',
      '<(js_externs_dir)/Timer_externs.js',
      '<(js_externs_dir)/UiEvent_externs.js',
      '<(js_externs_dir)/UiEventInit_externs.js',
      '<(js_externs_dir)/ViewEventTarget_externs.js',
      '<(js_externs_dir)/WheelEventInit_externs.js',
      '<(js_externs_dir)/Window_externs.js',
      '<(js_externs_dir)/WindowEvent_externs.js',
      '<(js_externs_dir)/WindowEventInit_externs.js',
    ], # generated_js_externs_files

    'static_js_externs_files': [
      'errors_externs.js',
      'types_externs.js',
      'bracket_externs.js',
      'clipboard/clipboard_externs.js',
      'commander_externs.js',
      'commands/find_and_replace_externs.js',
      'commands/spell_checker_externs.js',
      'console_externs.js',
      'text/document_externs.js',

      'file_path_externs.js',
      'js_console_externs.js',
      'key_names_externs.js',
      'lexers/lexer_externs.js',
      'lexers/lexers_externs.js',
      'text/modes/modes_externs.js',
      'os/os_externs.js',
      'text/range_externs.js',
      'windows/document_state_externs.js',
      'css/style_externs.js',
      'windows/tab_data_externs.js',
      'text/text_position_externs.js',
      'jslib/editors_externs.js',
      'jslib/ordered_set_externs.js',
      'jslib/windows_externs.js',
    ], # static_js_externs_files

    # Computed variables

    'glue_dir': '<(SHARED_INTERMEDIATE_DIR)/evita/bindings',
    'interface_idl_file_list': '<|(interface_idl_file.txt <@(interface_idl_files))',
    'idl_files': '<@(interface_idl_files) <@(dictionary_idl_files)',
    'idl_file_list': '<|(idl_file.txt <@(idl_files))',
    'idl_work_dir': '<(SHARED_INTERMEDIATE_DIR)/evita',
    'js_externs_dir': '<(SHARED_INTERMEDIATE_DIR)/evita/js_externs',
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
      'target_name': 'dom_aggregate_js_extern_files',
      'type': 'none',
      'dependencies': [
        'dom_generated_js_extern_files',
      ],
      'actions': [{
        'action_name': 'dom_aggregate_js_extern_files',
        'inputs': [
          '<(DEPTH)/tools/idl_to_js_externs/aggregate_js_externs.py',
          '<(idl_file_list)',
          '<@(generated_js_externs_files)',
          '<@(static_js_externs_files)',
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

    # DOM bindings library
    {
      'target_name': 'dom_aggregate_glue_files',
      'type': 'static_library',
      'dependencies': [
        '<(DEPTH)/gin/gin.gyp:gin',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        'dom_generated_glue_files',
        'v8_glue',
      ], # dependencies
      'include_dirs+': [ '<(SHARED_INTERMEDIATE_DIR)/evita/bindings' ],
      'sources': [
        '../precomp.cpp',
        '<@(generated_glue_files)',
      ], # sources
    },

    # Generate glue C++ source code and header file from IDL file.
    {
      'target_name': 'dom_generated_glue_files',
      'type': 'none',
      'dependencies': [
        'dom_interfaces_info',
      ],
      'sources': [ '<@(idl_files)' ],
      'rules': [{
        'rule_name': 'dom_idl_to_glue_file',
        'extension': 'idl',
        'msvs_external_rule': 1,
        'inputs': [
          'bindings/idl_to_glue.py',
          'bindings/dictionary.cc',
          'bindings/dictionary.h',
          'bindings/interface.cc',
          'bindings/interface.h',
          '<(idl_work_dir)/InterfacesInfo.pickle',
        ], # inputs
        'outputs': [
          '<(glue_dir)/v8_glue_<(RULE_INPUT_ROOT).cc',
          '<(glue_dir)/v8_glue_<(RULE_INPUT_ROOT).h',
        ],
        'action': [
          'python',
          '<(DEPTH)/evita/dom/bindings/idl_to_glue.py',
          '--output-dir', '<(glue_dir)',
          '--interfaces-info', '<(idl_work_dir)/InterfacesInfo.pickle',
          '--write-file-only-if-changed', '<@(write_file_only_if_changed)',
          '<(RULE_INPUT_PATH)',
        ], # action
      }], # rules
    }, # 'dom_generated_glue_files'

    # Generate JavaScript Externs from IDL files.
    {
      'target_name': 'dom_generated_js_extern_files',
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
          '<(idl_work_dir)/InterfacesInfo.pickle',
        ], # inputs
        'outputs': [
          '<(js_externs_dir)/<(RULE_INPUT_ROOT)_externs.js',
        ],
        'action': [
          'python',
          '<(DEPTH)/tools/idl_to_js_externs/idl_to_js_externs.py',
          '--output-dir', '<(js_externs_dir)',
          '--interfaces-info', '<(idl_work_dir)/InterfacesInfo.pickle',
          '--write-file-only-if-changed', '<@(write_file_only_if_changed)',
          '<(RULE_INPUT_PATH)',
        ], # action
      }], # rules
    }, # 'dom_generated_js_extern_files'

    # Interfaces information for IDL compiler
    {
      'target_name': 'dom_interfaces_info',
      'type': 'none',
      'actions': [{
        'action_name': 'compute_dom_interfaces_info',
        'inputs': [
          '<(DEPTH)/third_party/blink_idl_parser/compute_interfaces_info.py',
          '<@(interface_idl_files)',
        ],
        'outputs': [
          '<(idl_work_dir)/InterfacesInfo.pickle',

        ],
        'action': [
          'python',
          '<(DEPTH)/third_party/blink_idl_parser/compute_interfaces_info.py',
          '--idl-files-list', '<(interface_idl_file_list)',
          '--interfaces-info-file', '<(idl_work_dir)/InterfacesInfo.pickle',
          '--write-file-only-if-changed', '<(write_file_only_if_changed)',
        ], # action
        'message': 'Computing global information about IDL files',
      }], # actions
    }, # dom_interfaces_info
  ], # targets
}
