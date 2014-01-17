// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

'use strict';

(function() {
  /**
   * @param {Document} document.
   */
  function newEditorWindow(document) {
    var editorWindow = new EditorWindow();
    newTextWindow(editorWindow, document);
    editorWindow.realize();
  }

  /**
   * @param {Window} document.
   * @param {Document} document.
   */
  function newTextWindow(parent, document) {
    parent.add(new TextWindow(new Range(document)));
  }

  // Show document in new editor window.
  Editor.bindKey('Ctrl+Shift+3', function() {
    var text_window = new TextWindow(this.selection.range);
    var editor_window = new EditorWindow();
    editor_window.add(text_window);
    editor_window.realize();
    text_window.makeSelectionVisible();
    text_window.focus();
  });

  // Show document in new editor window and close current editor window.
  Editor.bindKey('Ctrl+Shift+4', function() {
    var current = this.selection.window.parent;
    // When editor window has only one tab, we don't ignore this command,
    // since result of this command isn't useful.
    if (current.children.length == 1)
      return;
    var text_window = new TextWindow(this.selection.range);
    var editor_window = new EditorWindow();
    editor_window.add(text_window);
    editor_window.realize();
    text_window.makeSelectionVisible();
    text_window.focus();
    current.destroy();
  });

  // Select all contents
  Editor.bindKey('Ctrl+A', function() {
    var range = this.selection.range;
    range.start = 0
    range.end = range.document.length;
    this.selection.startIsActive = false;
  });

  function changeCase(selection, changer) {
    if (selection.start == selection.end) {
      selection.range.startOf(Unit.WORD, Alter.EXTEND);
      selection.range.endOf(Unit.WORD, Alter.EXTEND);
    }
    changer(selection);
  }

  Editor.bindKey('Ctrl+Shift+C', function() {
    changeCase(this.selection, function(selection) {
      selection.range.capitalize();
    });
  });

  Editor.bindKey('Ctrl+Shift+D', function() {
    changeCase(this.selection, function(selection) {
      selection.range.toLowerCase();
    });
  });

  // TODO(yosi) We should display dialog box to prompt enter line number and
  // list of functions.
  Editor.bindKey('Ctrl+G', function(arg) {
    if (!arg)
      return;
    this.selection.range.startOf(Unit.DOCUMENT);
    this.selection.range.move(Unit.PARAGRAPH, arg - 1);
  });

  // Make selection visible
  Editor.bindKey('Ctrl+L', function() {
    this.makeSelectionVisible();
  });

  // Open new document in new window in current editor window.
  Editor.bindKey('Ctrl+N', function(arg) {
    var editorWindow = this.parent;
    if (arg !== undefined) {
      newTextWindow(editorWindow, new Document('untitled.txt'));
      return;
    }

    Editor.getFilenameForSave(this, this.selection.document.filename)
        .then(function(filename) {
          newTextWindow(editorWindow, new Document(filename))
        });
  });

  // Open new document in new editor window.
  Editor.bindKey('Ctrl+Shift+N', function(arg) {
    if (arg !== undefined) {
      newEditorWindow(new Document('untitled.txt'));
      return;
    }

    Editor.getFilenameForSave(this, this.selection.document.filename)
        .then(function(filename) {
          newEditorWindow(new Document(filename));
        });
  });

  // Open document in new or existing window in current editor window.
  Editor.bindKey('Ctrl+O', function() {
    var editorWindow = this.parent;
    Editor.getFilenameForLoad(this, this.selection.document.filename)
        .then(function(filename) {
          // TODO(yosi) When editorWindow has a window for filename, we should
          // activate it.
          newTextWindow(editorWindow, Document.load(filename));
        });
  });

  // Open document in new editor window.
  Editor.bindKey('Ctrl+Shift+O', function() {
    Editor.getFilenameForLoad(this, this.selection.document.filename)
        .then(function(filename) {
          newEditorWindow(Document.load(filename));
        });
  });

  // Save file
  Editor.bindKey('Ctrl+S', function(arg) {
    var document = this.selection.document;
    if (!arg && document.filename != '') {
      document.save(document.filename);
      return;
    }

    Editor.getFilenameForSave(this, document.filename)
        .then(function(filename) {
          document.save(filename);
        });
  });

  Editor.bindKey('Ctrl+W', function(arg) {
    var document = this.selection.document;
    document.close();
  });

  Editor.bindKey('Ctrl+Shift+U', function() {
    changeCase(this.selection, function(selection) {
      selection.range.toUpperCase();
    });
  });

  Editor.bindKey('Ctrl+Shift+W', function(arg) {
    if (arg)
      Editor.forceExit();
    else
      Editor.exit();
  });

  Editor.bindKey('Ctrl+Y', function() {
    var selection = this.selection;
    var position = selection.document.redo(selection.active);
    if (position < 0) {
      Editor.messageBox(this,
          Editor.localizeText(Strings.IDS_NO_MORE_REDO),
          Editor.localizeText(Strings.IDS_APP_TITLE),
          MessageBox.ICONWARNING);
      return;
    }
    selection.range.collapseTo(position);
  });

  Editor.bindKey('Ctrl+Z', function() {
    var selection = this.selection;
    var position = selection.document.undo(selection.active);
    if (position < 0) {
      Editor.messageBox(this,
          Editor.localizeText(Strings.IDS_NO_MORE_UNDO),
          Editor.localizeText(Strings.IDS_APP_TITLE),
          MessageBox.ICONWARNING);
      return;
    }
    selection.range.collapseTo(position);
  });
})();
