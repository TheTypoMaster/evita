// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /** @type {?CheckboxControl} */
  var case_preserve_replace_checkbox;

  /** @type {?Form} */
  var form = null;

  /** @type {?FormWindow} */
  var form_window = null;

  /** @type {?CheckboxControl} */
  var match_case_checkbox;

  /** @type {?CheckboxControl} */
  var match_whole_word_checkbox;

  /** @type {?ButtonControl} */
  var find_next_button;

  /** @type {?ButtonControl} */
  var find_previous_button;

  /** @type {?TextFieldControl} */
  var find_what_text;

  /** @type {?ButtonControl} */
  var replace_all_button;

  /** @type {?ButtonControl} */
  var replace_one_button;

  /** @type {?TextFieldControl} */
  var replace_with_text;

  /** @type {?CheckboxControl} */
  var use_regexp_checkbox;

  /**
   * @param {!Window} window
   */
  function doFindNext(window) {
    var text_window = ensureTextWindow(window);
    console.log('doFindNext', text_window);
    if (!text_window)
      return;
    FindAndReplace.find(text_window, find_what_text.value,
                        makeFindOptions(Direction.FORWARD));
  }

  /**
   * @param {!Window} window
   */
  function doFindPrevious(window) {
    var text_window = ensureTextWindow(window);
    if (!text_window)
      return;
    FindAndReplace.find(text_window, find_what_text.value,
                        makeFindOptions(Direction.BACKWARD));
  }

  /**
   * @param {!Window} window
   */
  function doReplaceOne(window) {
    var text_window = ensureTextWindow(window);
    if (!text_window)
      return;
    FindAndReplace.replaceOne(text_window, find_what_text.value,
                              replace_with_text.value,
                              makeFindOptions(Direction.FORWARD));
  }

  /**
   * @param {!Window} window
   */
  function doReplaceAll(window) {
    var text_window = ensureTextWindow(window);
    if (!text_window)
      return;
    FindAndReplace.replaceAll(text_window, find_what_text.value,
                              replace_with_text.value,
                              makeFindOptions(Direction.FORWARD));
  }

  function ensureForm() {
    if (form_window)
      return;

    form = new Form('DummyName');

    var BUTTON_MARGIN = 5;
    var CONTROL_HEIGHT = 26;
    var LINE_MARGIN = 5;
    var PADDING_LEFT = 5;
    var PADDING_TOP = 5;

    var control_left = PADDING_LEFT;
    var control_top = PADDING_TOP;
    var dummy_id = 2000;

    function add(control, width) {
      control.clientLeft = control_left;
      control.clientTop = control_top;
      control.clientWidth = width;
      control.clientHeight = CONTROL_HEIGHT;
      control_left += control.clientWidth;
      form.add(control);
      return control;
    }

    function newline() {
      control_left = PADDING_LEFT;
      control_top += CONTROL_HEIGHT + LINE_MARGIN;
    }

    form.title = 'Find and Replace - evita';
    form.width = 350;
    form.height = 235;

    // Text fields
    add(new LabelControl('Find what:'), 75);
    find_what_text = add(new TextFieldControl(1), 265);
    find_what_text.accessKey = 'N';
    newline();
    add(new LabelControl('Replace with:'), 75);
    replace_with_text = add(new TextFieldControl(2), 265);
    replace_with_text.accessKey = 'P';
    replace_with_text.disabled = true;
    newline();

    // Checkboxes
    function addCheckbox(text) {
      var checkbox = new CheckboxControl(++dummy_id);
      add(checkbox, 20);
      add(new LabelControl(text), text.length * 10);
      return checkbox;
    }
    CONTROL_HEIGHT = 20;
    match_case_checkbox = addCheckbox('Match case');
    newline();
    match_whole_word_checkbox = addCheckbox('Match whole word');
    newline();
    use_regexp_checkbox = addCheckbox('Use regular expression');
    newline();
    case_preserve_replace_checkbox = addCheckbox('Case preserve replacement');
    case_preserve_replace_checkbox.checked = true;
    newline();

    // Buttons
    CONTROL_HEIGHT = 26;
    function addButton(text, accessKey) {
      var button = new ButtonControl(++dummy_id);
      button.accessKey = accessKey;
      button.text = text;
      add(button, 100);
      control_left += BUTTON_MARGIN;
      return button;
    }
    control_left = 140;
    replace_one_button = addButton('Replace', 'R');
    find_previous_button = addButton('Find Previous', 'I');
    newline();
    control_left = 140;
    replace_all_button = addButton('Replace All', 'A');
    find_next_button = addButton('Find Next', 'F');

    function updateUiByFindWhat() {
      var can_find = find_what_text.value != '';
      find_next_button.disabled = !can_find;
      find_previous_button.disabled = !can_find;
      replace_one_button.disabled = !can_find;
      replace_all_button.disabled = !can_find;
      replace_with_text.disabled = !can_find;
    }

    // TODO(yosi) We should handle both "change" and "input".
    find_what_text.addEventListener('change', updateUiByFindWhat);
    find_what_text.addEventListener('input', updateUiByFindWhat);

    find_what_text.addEventListener('keydown', function(event) {
      console.log('keydown', event.code.toString(16));
      if (event.code == 0x10D)
        doFindNext(/** @type {!FormWindow} */(form_window));
      else if (event.code == 0x11B)
        form_window.hide();
    });
    replace_with_text.addEventListener('keydown', function(event) {
      if (event.code == 0x10D)
        doFindNext(/** @type {!FormWindow} */(form_window));
      else if (event.code == 0x11B)
        form_window.hide();
    });

    function installButton(button, handler) {
      button.addEventListener('click', function(event) {
        console.log('button click', event);
        if (!Window.focus)
          return;
        handler(/**@type{!Window}*/(Window.focus));
      });
    }

    installButton(find_next_button, doFindNext);
    installButton(find_previous_button, doFindPrevious);
    installButton(replace_one_button, doReplaceOne);
    installButton(replace_all_button, doReplaceAll);

    updateUiByFindWhat();
    form_window = new FormWindow(form);
    form_window.realize();
    // TODO(yosi) |global.findForm| is only for debugging purpose.
    global.findFormWindow = form_window;
  }

  /**
   * @param {!Window} window
   * @return {?TextWindow}
   */
  function ensureTextWindow(window) {
    if (window instanceof TextWindow)
      return window;
    var text_window = null;
    EditorWindow.list.forEach(function(editor_window) {
      editor_window.children.forEach(function(window) {
        if (!(window instanceof TextWindow))
          return;
        if (!text_window || text_window.focusTick_ < window.focusTick_)
          text_window = window;
      });
    });
    return text_window;
  }

  /**
   * @param {string} text
   * @return {boolean}
   */
  function hasUpperCase(text) {
    for (var i = 0; i < text.length; ++i) {
      var data = Unicode.UCD[text.charCodeAt(i)];
      if (data.category == Unicode.Category.Lu ||
          data.category == Unicode.Category.Lt) {
        return true;
      }
    }
    return false;
  }

  /**
   * @param {!Direction} direction,
   * @return {!FindAndReplaceOptions}
   */
  function makeFindOptions(direction) {
    return {
      backward: direction == Direction.BACKWARD,
      casePreserveReplacement: case_preserve_replace_checkbox.checked,
      ignoreCase: shouldIgnoreCase(find_what_text.value),
      matchWholeWord: match_whole_word_checkbox.checked,
      useRegExp: use_regexp_checkbox.checked
   };
  }

  function showFindFormCommand() {
    ensureForm();
    form_window.show();
    form_window.focus();
  }

  /**
   * @param {string} text
   * @return {boolean}
   */
  function shouldIgnoreCase(text) {
    return !hasUpperCase(text) && !match_case_checkbox.checked;
  }

  Editor.bindKey(TextWindow, 'Ctrl+F', showFindFormCommand);
  Editor.bindKey(TextWindow, 'Ctrl+H', showFindFormCommand);

  Editor.bindKey(TextWindow, 'F3', function() {
    ensureForm();
    doFindNext(this);
  });

  Editor.bindKey(TextWindow, 'Shift+F3', function() {
    ensureForm();
    doFindPrevious(this);
  });
})();
