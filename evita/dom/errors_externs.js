// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!Document} document
 * @param {string=} opt_message
 */
var DocumentError = function(document, opt_message) {};

/** @export @type {!Document} */
DocumentError.prototype.document;

/**
 * @constructor
 * @extends {DocumentError}
 * @param {!Document} document
 * @param {string=} opt_message
 */
var DocumentNotReady = function(document, opt_message) {};

/**
 * @constructor
 * @extends {DocumentError}
 * @param {!Document} document
 * @param {string=} opt_message
 */
var DocumentReadOnly = function(document, opt_message) {};

