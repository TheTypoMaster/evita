// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview Document externs.
 * @externs
 */

/**
 * @constructor
 * @extends {EventTarget}
 * @param {string} name
 */
function Document(name) {}

/**
 * @typedef {!function(string, !Document)}
 */
Document.Observer;

/**
 * @param {!Document.Observer} observer
 */
Document.addObserver = function(observer) {};

/**
 * @param {string} name
 * @return {?Document}
 */
Document.find = function(name) {};
/**
 * @param {string} absoluteFileName
 * @return {?Document}
 */
Document.findFile = function(absoluteFileName) {};

/**
 * @type {Array.<!Document>}
 */
Document.list;

/**
 * @param {string} fileName
 * @return {!Document}
 */
Document.open = function(fileName) {};

/**
 * @param {!Document} document
 */
Document.remove = function(document) {};

/**
 * @param {!Document.Observer} observer
 */
Document.removeObserver = function(observer) {};

/** @type {string} */
Document.prototype.encoding;

/** @export  @type {string} */
Document.prototype.fileName;

/** @export  @type {?Keymap} */
Document.prototype.keymap;

/** @type {!Date} */
Document.prototype.lastStatTime_;

/** @type {!Date} */
Document.prototype.lastWriteTime;

/** @export  @type {number} */
Document.prototype.length;

/** @export  @type {!Generator.<string>} */
Document.prototype.lines;

/** @export  @type {?Mode} */
Document.prototype.mode;

/** @export  @type {boolean} */
Document.prototype.modified;

/** @export  @type {string} */
Document.prototype.name;

/** @type {number} */
Document.prototype.newline;

/** @export @type {!Document.Obsolete} */
Document.prototype.obsolete;

/** @export  @type {!Map.<string, *>} */
Document.prototype.properties;

/** @export  @type {boolean} */
Document.prototype.readonly;

/** @type {number} */
Document.prototype.state;

/**
 * @param {string} key_combination
 * @param {Object} command
 */
Document.prototype.bindKey = function (key_combination, command) {};

/**
 * @param {number} index
 * @return {number}
 */
Document.prototype.charCodeAt_ = function(index) {};

/** @type {!function()} */
Document.prototype.clearUndo = function() {};

/** @type {!function()} */
Document.prototype.close = function() {};

/**
 * @param {Unit} unit
 * @param {number} position
 * @return {number} new position.
 */
Document.prototype.computeEndOf_ = function(unit, position) {};

/**
 * @param {Unit} unit
 * @param {number} count
 * @param {number} position
 * @return {number}
 */
Document.prototype.computeMotion_ = function(unit, count, position) {};

/**
 * @param {Unit} unit
 * @param {number} position
 * @return {number} new position.
 */
Document.prototype.computeStartOf_ = function(unit, position) {};

/**
 * @param {string} char_set
 * @param {number} count
 * @param {number} position
 * @return {number}
 */
Document.prototype.computeWhile_ = function(char_set, count, position) {};

/** @param {number} hint */
Document.prototype.doColor_ = function(hint) {};

/**
 * @param {string} name
 */
Document.prototype.endUndoGroup_ = function(name) {};

Document.prototype.forceClose = function() {};

/**
 * @param {number} offset
 * @return {{column: number, lineNumber: number}}
 */
Document.prototype.getLineAndColumn_ = function(offset) {};

/**
 * @return {!Array.<!DocumentWindow>}
 */
Document.prototype.listWindows = function() {};

/**
 * @param {string=} opt_fileName
* @return {!Promise.<number>}
*/
Document.prototype.load = function(opt_fileName) {};

/**
 * @param {!Editor.RegExp} regexp
 * @param {number} start
 * @param {number} end
 * @return {?Array.<!Editor.RegExp.Match>}
 */
Document.prototype.match_ = function(regexp, start, end) {};

/**
 * @return {boolean}.
 */
Document.prototype.needSave = function() {};

/** @export @type {function()} */
Document.prototype.parseFileProperties;

/** @param {!TextOffset} position */
Document.prototype.redo = function(position) {}

/** @param {string} new_name */
Document.prototype.renameTo = function(new_name) {}

/**
 * @param {string=} opt_fileName
 * @return {!Promise.<number>}
 */
Document.prototype.save = function(opt_fileName) {};

/**
 * @param {number} start
 * @param {number=} opt_end
 * @return {string}
 */
Document.prototype.slice = function(start, opt_end) {};

/**
 * @param {string} name
 */
Document.prototype.startUndoGroup_ = function(name) {};

/**
 * @param {!TextOffset} offset
 * @return {string}
 */
Document.prototype.spellingAt = function(offset) {};

/**
 * @param {!TextOffset} position
 * @return {!Style}
 */
Document.prototype.styleAt = function(position) {};

/**
 * @param {!TextOffset} offset
 * @return {number}
 */
Document.prototype.syntaxAt = function(offset) {};

/** @param {!TextOffset} position */
Document.prototype.undo = function(position) {}

/**
 * @param {string} name
 * @param {function()} callback
 * @param {!Object=} opt_receiver
 */
Document.prototype.undoGroup = function(name, callback, opt_receiver) {};
