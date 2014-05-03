// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

global.ConfigMode.keymap = new Map();
global.CxxMode.keymap = new Map();
global.HaskellMode.keymap = new Map();
global.JavaMode.keymap = new Map();
global.JavaScriptMode.keymap = new Map();
global.LispMode.keymap = new Map();
global.MasonMode.keymap = new Map();
global.PerlMode.keymap = new Map();
global.PlainTextMode.keymap = new Map();
global.PythonMode.keymap = new Map();
global.XmlMode.keymap = new Map();

Object.defineProperty(Mode, 'defaultMode', {
  /** @type {!Mode} */
  value: new PlainTextMode()
});

Object.defineProperty(Mode, 'extensionMap', {
  value: (function() {
    var map = /** @type{!ExtensionToModeMap} */(new Map());
    map.set('asdl', {mode: XmlMode, name: 'ASDL Document'});
    map.set('cc', {mode: CxxMode, name: 'C++ Source'});
    map.set('cfg', {mode: ConfigMode, name: 'Config File'});
    map.set('cl', {mode: CxxMode, name: 'CommonLisp Source'});
    map.set('cpp', {mode: CxxMode, name: 'C++ Source'});
    map.set('css', {mode: CxxMode, name: 'Cascading Style Sheet Document'});
    map.set('cxx', {mode: CxxMode, name: 'C++ Source'});
    map.set('el', {mode: LispMode, name: 'EmacsLisp Source'});
    map.set('gyp', {mode: PythonMode, name: 'GYP Source'});
    map.set('gypi', {mode: PythonMode, name: 'GYP Include Source'});
    map.set('h', {mode: CxxMode, name: 'C/C++ Header'});
    map.set('hs', {mode: HaskellMode, name: 'Haskell Source'});
    map.set('hsc', {mode: HaskellMode, name: 'Haskell Component Source'});
    map.set('hpp', {mode: CxxMode, name: 'C++ Header'});
    map.set('htm', {mode: XmlMode, name: 'HTML Document'});
    map.set('html', {mode: XmlMode, name: 'HTML Document'});
    map.set('hxx', {mode: CxxMode, name: 'C++ Header'});
    map.set('idl', {mode: CxxMode, name: 'Web IDL Source'});
    map.set('java', {mode: JavaMode, name: 'Java Source'});
    map.set('js', {mode: JavaScriptMode, name: 'JavaScript Source'});
    map.set('jsm', {mode: JavaScriptMode, name: 'JavaScript Module Source'});
    map.set('l', {mode: LispMode, name: 'Lisp Source'});
    map.set('lisp', {mode: LispMode, name: 'Lisp Source'});
    map.set('lsp', {mode: LispMode, name: 'Lisp Source'});
    map.set('m', {mode: ConfigMode, name: 'Mason Source'});
    map.set('mi', {mode: ConfigMode, name: 'Mason Include Source'});
    map.set('mk', {mode: ConfigMode, name: 'Config File'});
    map.set('mm', {mode: CxxMode, name: 'Objective-C Source'});
    map.set('pl', {mode: PerlMode, name: 'Perl Source'});
    map.set('pm', {mode: PerlMode, name: 'Perl Module Source'});
    map.set('py', {mode: PythonMode, name: 'Python Source'});
    map.set('scm', {mode: LispMode, name: 'Scheme Source'});
    map.set('stanza', {mode: ConfigMode, name: 'Config File'});
    map.set('t', {mode: PerlMode, name: 'Perl Test Source'});
    map.set('wsdl', {mode: XmlMode, name: 'WSDL Document'});
    map.set('xhtml', {mode: XmlMode, name: 'XHTML Document'});
    map.set('xml', {mode: XmlMode, name: 'XML Document'});
    map.set('xsd', {mode: XmlMode, name: 'XSD Document'});
    map.set('xsl', {mode: XmlMode, name: 'XSLT Document'});
    return map;
  })()
});

Object.defineProperty(Mode, 'nameMap', {
  /** @type {!NameToModeMap} */
  value: (function() {
    var map = new Map();
    map.set('config', ConfigMode);
    map.set('c++', CxxMode);
    map.set('haskell', HaskellMode);
    map.set('java', JavaMode);
    map.set('javascript', JavaScriptMode);
    map.set('lisp', LispMode);
    map.set('mason', MasonMode);
    map.set('perl', PerlMode);
    map.set('plain', PlainTextMode);
    map.set('python', PythonMode);
    map.set('xml', XmlMode);
    return /** @type {!NameToModeMap} */(map);
  })()
});

Object.defineProperty(Mode, 'fileNameMap', {
  /** @type {!NameToModeMap} */
  value: (function() {
      var map = new Map();
      map.set('Makefile', ConfigMode);
      map.set('autohandler', MasonMode);
      map.set('dhandler', MasonMode);
      map.set('makefile', ConfigMode);
      return /** @type {!NameToModeMap} */(map);
    })()
});

Object.defineProperty(Mode, 'chooseMode', {
  /**
   * @param {!Document} document
   * @return {!Mode}
   */
  value: function(document) {
    var mode_name = document.properties.get('mode') ||
                    document.properties.get('Mode');
    if (mode_name !== undefined) {
      var mode_ctor = Mode.nameMap.get(mode_name.toLowerCase());
      if (mode_ctor)
        return new mode_ctor();
    }
    if (document.fileName != '')
      return Mode.chooseModeByFileName(document.fileName);
    return Mode.chooseModeByFileName(document.name);
  }
});

Object.defineProperty(Mode, 'chooseModeByFileName', {
  /**
   * @param {string} fileName
   * @return {!Mode}
   */
  value: function(fileName) {
    var matches = /[.](.+)$/.exec(FilePath.basename(fileName));
    if (!matches) {
      var mode_ctor = Mode.fileNameMap.get(FilePath.basename(fileName));
      if (mode_ctor)
        return new mode_ctor();
      return new Mode.defaultMode.constructor();
    }
    var description = Mode.extensionMap.get(matches[1]);
    if (!description)
      return new Mode.defaultMode.constructor();
    return new description.mode();
  }
});

Object.defineProperty(Mode.prototype, 'doColor', {value:
 /**
  * @this {!Mode}
  * @param {!Document} document
  * @param {number} hint
  */
 function(document, hint) {
   this.doColor_(document, hint);
 }
});

Object.defineProperty(Mode.prototype, 'lexer', {value: null, writable: true});

[
  {mode: CxxMode, lexer: CppLexer},
  {mode: ConfigMode, lexer: ConfigLexer},
  //{mode: IdlMode, lexer: IdlLexer},
  {mode: JavaMode, lexer: JavaLexer},
  {mode: JavaScriptMode, lexer: JavaScriptLexer},
  {mode: PythonMode, lexer: PythonLexer},
].forEach(function(spec) {
  var ctor = spec.lexer;
  Object.defineProperty(spec.mode.prototype, 'doColor', {value:
    /**
     * @this {!Mode}
     * @param {!Document} document
     * @param {number} hint
     */
    function(document, hint) {
     if (!this.lexer)
       this.lexer = new ctor(document);
     this.lexer.doColor(hint);
    }
  });
});
