// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @type{Window} */
Window.focus;

/**
 * @param {!Event} event
 */
Window.handleEvent = function(event) {};

/** @type {number} */
Window.prototype.bottom;

/** @type {Array.<!Window>} */
Window.prototype.children;

/** @type {Window} */
Window.prototype.firstChild;

/** @type {number} */
Window.prototype.focusTick_;

/** @type {number} */
Window.prototype.id;

/** @type {Window} */
Window.prototype.lastChild;

/** @type {number} */
Window.prototype.left;

/** @type {Window} */
Window.prototype.nextSibling;

/** @type {?Window} */
Window.prototype.parent;

/** @type {Window} */
Window.prototype.previousSibling;

/** @type {number} */
Window.prototype.right;

/** @type {string} */
Window.prototype.state;

/**
 * @type {string}
 * setter displays status text in status bar if possible.
 */
Window.prototype.status;

/** @type {number} */
Window.prototype.top;

/** @param {!Window} window */
Window.prototype.appendChild = function(window) {};

/** @param {!Window} window */
Window.prototype.changeParent = function(window) {};

Window.prototype.destroy = function() {};
Window.prototype.focus = function() {};
Window.prototype.realize = function() {};

/** @type {function()} */
Window.prototype.releaseCapture = function() {};

/** @param {!Window} window */
Window.prototype.removeChild = function(window) {};

/** @type {function()} */
Window.prototype.setCapture = function() {};

/**
 * @param {string} type
 * @param {!EventListener} listener
 * @param {boolean=} opt_capture
 */
Window.prototype.addEventListener = function(type, listener, opt_capture) {};

/**
 * @param {!Event} event
 */
Window.prototype.dispatchEvent = function(event) {};

/**
 * @param {string} type
 * @param {!EventListener} listener
 * @param {boolean=} opt_capture
 */
Window.prototype.removeEventListener = function(type, listener,
                                                opt_capture) {};

/** @param {!Window} new_right_window */
Window.prototype.splitHorizontally = function(new_right_window) {};

/** @param {!Window} new_below_window */
Window.prototype.splitVertically = function(new_below_window) {};