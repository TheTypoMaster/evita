// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/**
 * @fileoverview Types
 * @externs
 */

/**
 * @type {Object.<string>}
 */
var global = {};

/**
 * @constructor
 * @extends {EventTarget}
 * @param {string} name
 * @param {!Mode=} opt_mode
 */
var Document = function(name, opt_mode) {};

/** @typedef {!Function|!Object} Object which has |handleEvent(event)| */
var EventListener;

var FilePath;

/**
 * @constructor
 */
var JsConsole = function() {};

/**
 * Note: Since, Closure compiler doesn't support recursive type, we use
 * |!Map| instead of |!Keymap|.
 * @typedef {Map.<number, !Function|!Map>}


 */
var Keymap;

/**
 * @typedef {!Object}
 *  backward: boolean,
 *  global: boolean,
 *  ignoreCase: boolean,
 *  matchExact: boolean,
 *  matchWord: boolean,
 *  multiline: boolean,
 *  sticky: boolean,
 */
Editor.RegExpInit;

/**
 * @constructor
 * @param {string} source
 * @param {Editor.RegExpInit=} opt_init_dict
 */
Editor.RegExp = function(source, opt_init_dict) {};

/**
 * @typedef {{name: string, start: number, end: number}}
 */
Editor.RegExp.Match;

/** @typedef {number} */
var TextOffset;

/**
 * @constructor
 * @param {!Document} document
 * @param {!TextOffset} offset
 */
var TextPosition = function(document, offset) {};

/**
 * @constructor
 * @param {!Document|!Range} document_or_range
 * @param {number=} opt_start
 * @param {number=} opt_end
 */
var Range = function(document_or_range, opt_start, opt_end) {};

/**
 * @constructor
 */
var Style = function() {};

/**
 * @constructor
 * @extends {DocumentWindow}
 * @param {Document} document
 */
var TableWindow = function(document) {};

/**
 * @constructor
 * @extends {DocumentWindow}
 * @param {Range} range
 */
var TextWindow = function(range) {};

/**
 * @constructor
 * @extends {Selection}
 * You can't create |TextSelection| object directly. You can obtain
 * |TextSelection| object via |TextWindow.prototype.selection|.
 */
var TextSelection = function() {};

/**
 * @final
 * @constructor
 * @extends {Event}
 * @param {string} type
 * @param {KeyboardEventInit=} opt_init_dict
 */
var KeyboardEvent = function(type, opt_init_dict) {};

/**
 * @constructor
 * @extends {Event}
 * @param {string} type
 * @param {MouseEventInit=} opt_init_dict
 */
var MouseEvent = function(type, opt_init_dict) {};

/**
 * @constructor
 * @extends {Event}
 * @param {string} type
 * @param {WindowEventInit=} opt_init_dict
 */
var WindowEvent = function(type, opt_init_dict) {};

//////////////////////////////////////////////////////////////////////
//
// Timer
//
/**
 * @constructor
 */
var Timer = function() {};

/**
 * @constructor
 * @extends {Timer}
 */
var OneShotTimer = function() {};

/**
 * @constructor
 * @extends {Timer}
 */
var RepeatingTimer = function() {};

//////////////////////////////////////////////////////////////////////
//
// Editor.idl
//
/**
 * @typedef {Map.<string, Map.<number,function(number=)>>}
 */
var KeyBindingMapMap;

/**
 * @typedef {{
 *  column: number,
 *  functionName: string,
 *  lineNumber: number,
 *  scriptName: string
 * }}
 */
var StackFrame;

/**
 * @typedef {Object.<string, string>}
 */
var StringDict;

/**
 * @typedef {{
 *  exception: Error,
 *  value: *,
 *  stackTrace: Array.<!StackFrame>,
 *  stackTraceString: string,
 *  lineNumber: number,
 *  start: number,
 *  end: number,
 *  startColumn: number,
 *  endColumn: number
 * }}
 */
var RunScriptResult;

/** @typedef {boolean|number|string} */
var SwitchValue;

//////////////////////////////////////////////////////////////////////
//
// Unicode
//
var Unicode;

/** @const @type{Array.<string>} */
Unicode.CATEGORY_SHORT_NAMES;

/** @enum{string} */
Unicode.Category = {
  Cc: "Cc", Cf: "Cf", Cn: "Cn", Co: "Co", Cs: "Cs",
  Ll: "Ll", Lm: "Lm", Lo: "Lo", Lt: "Lt", Lu: "Lu", Mc: "Mc"
};

/** @const @type{Array.<{bidi: string, category: string}>} */
Unicode.UCD;
