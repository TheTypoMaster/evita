// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

/** @type {function() : !TextWindow } */
TextWindow.prototype.clone = function() {};

/**
 * @param {number} method
 * @param {!TextOffset=} opt_position
 * @param {number=} opt_count
 * @param {Point=} opt_point
* @return {number}
 */
TextWindow.prototype.compute_ = function(method, opt_position, opt_count,
                                         opt_point) {};

/**
 * @override
 * @type {!TextSelection}
 */
TextWindow.prototype.selection;

/** @type {function()} */
TextWindow.prototype.makeSelectionVisible = function() {};

/**
 * @param {number} x
 * @param {number} y
 * @return {!TextOffset}
 */
TextWindow.prototype.mapPointToPosition_ = function(x, y) {};

/**
 * @param {!TextOffset} position
 * @return {!Point}
 */
TextWindow.prototype.mapPositionToPoint_ = function(position) {};

/**
 * @param {!TextOffset} start
 * @param {!TextOffset} end
 */
TextWindow.prototype.reconvert_ = function(start, end) {};

/** @type {!Range}
 * We expose view range for blinking left bracket. Although, we discourage
 * to use this. We should find out another way off the page blinking.
 */
TextWindow.prototype.range_;

/**
 * @param {number} direction
 */
TextWindow.prototype.scroll = function(direction) {};