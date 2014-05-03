// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

(function() {
  /** @enum{number} */
  global.Document.Obsolete = {
    CHECKING: -2,
    UNKNOWN: -1,
    NO: 0,
    YES: 1,
  };

  Object.defineProperty(Document.prototype, 'lastStatTime_', {
    value: new Date(0),
    writable: true
  });

  Object.defineProperty(Document.prototype, 'obsolete', {
    value: Document.Obsolete.UNKNOWN,
    writable: true
  });

  /**
   * @param {string} absoluteFileName
   * @return {Document}
   */
  Document.findFile = function(absoluteFileName) {
    var canonical_fileName = absoluteFileName.toLocaleLowerCase();
    return /** @type{Document} */ (Document.list.find(
        function(document) {
          return document.fileName.toLocaleLowerCase() == canonical_fileName;
        }));
  }

  /**
   * @type {!function()}
   */
  Document.prototype.close = function() {
    var document = this;
    if (!document.needSave()) {
      document.forceClose();
      return;
    }
    Editor.messageBox(null,
        Editor.localizeText(Strings.IDS_ASK_SAVE, {name: document.name}),
        MessageBox.ICONWARNING | MessageBox.YESNOCANCEL)
      .then(function(response_code) {
        switch (response_code) {
          case DialogItemId.NO:
            document.forceClose();
            break;
          case DialogItemId.YES:
            Editor.getFileNameForSave(null, document.fileName)
              .then(function(fileName) {
                document.save(fileName).then(function() {
                  document.forceClose();
                });
              });
            break;
        }
      });
  };

  /**
   * @type {!function()}
   */
  Document.prototype.forceClose = function() {
    this.listWindows().forEach(function(window) {
      window.destroy();
    });
    Document.remove(this);
  };

  /**
   * @param {string} fileName A backing store file of document.
   * @return {!Document} A Document bound to fileName
   */
  Document.open = function(fileName) {
    var absoluteFileName = FilePath.fullPath(fileName);
    var present = Document.findFile(absoluteFileName);
    if (present)
      return present;
    var document = new Document(FilePath.basename(fileName));
    document.fileName = absoluteFileName;
    document.mode = Mode.chooseModeByFileName(fileName);
    return document;
  };

  /**
   * @return {boolean}
   */
  Document.prototype.needSave = function() {
    // TODO: We should use |document.notForSave|.
    return this.modified && !this.name.startsWith('*') &&
           FilePath.isValidFileName(this.fileName);
  };

  /**
   * This function handles Emacs "File Variables" in the first line.
   * TODO(yosi) Support "Local Variables: ... End:".
   */
  Document.prototype.parseFileProperties = function() {
    var document = this;
    var first_line = new Range(document);
    first_line.endOf(Unit.LINE, Alter.EXTEND);
    var file_vars_matches = /-\*-\s+(.+?)\s+-\*-/.exec(first_line.text);
    if (!file_vars_matches)
      return;
    file_vars_matches[1].split(';').forEach(function(var_def) {
      var matches = /^\s*([^:\s]+)\s*:\s*(.+?)\s*$/.exec(var_def);
      if (!matches)
        return;
      document.properties.set(matches[1], matches[2]);
    });
  };

})();
