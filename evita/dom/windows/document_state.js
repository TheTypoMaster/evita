// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @constructor
 * @param {!Document} document
 */
global.DocumentState = function(document) {
  this.filename = document.filename;
  // TODO(yosi) We should specify |DocumentState.prototype.icon| from
  // IconSet.
  this.icon = -2;
  this.lastWriteTime = document.lastWriteTime;
  this.modified = document.modified;
  this.name = document.name;
  this.state = document.state;
};

/**
 * @param {!DocumentState} other
 * @return {boolean}
 */
global.DocumentState.prototype.equals = function(other) {
  return this.filename == other.filename &&
         this.icon == other.icon &&
         this.lastWriteTime == other.lastWriteTime &&
         this.name == other.name &&
         this.state == other.state;
};

(function() {
  /**
   * @type {!Map.<!Document, !DocumentState>}
   */
  var documentStateMap = new Map();

  /**
   * @type {!Array.<!DocumentStateCallback>}
   */
  var observers = [];

  /**
   * @type {!function(!Document)} callback
   */
  global.DocumentState.addObserver = function(callback) {
    observers.push(callback);
  };

  /**
   * For testing purpose only.
   * @param {!Document} document
   * @return {!DocumentState}
   */
  global.DocumentState.get = function(document) {
    return documentStateMap.get(document);
  };

  /**
   * Updates document state by |document.modified| property. This function is
   * intended to be called from "idle" event handler to update |modified|
   * property of |DocumentState|.
   *
   * @param {!Document} document
   */
  global.DocumentState.update = function(document) {
    var state = documentStateMap.get(document);
    if (!state)
      return;
    if (state.modified == document.modified)
      return;
    observers.forEach(function(observer) {
      observer.call(this, document, state);
    });
  };

  /**
   * Set initial tab data for |TextWindow|.
   * @param {!DocumentEvent} event
   */
  function didAttachWindow(event) {
    if (!(event.view instanceof TextWindow))
      return;
    var document = /** @type{!Document} */(event.target);
    var state = documentStateMap.get(document);
    if (!state)
      return;
    TabData.update(event.view, state);
  }

  /**
   * @param {!DocumentEvent} event
   */
  function didDocumentLoadSave(event) {
    var document = /** @type{!Document} */(event.target);
    var state = new DocumentState(document);
    documentStateMap.set(documentStateMap, state);
    observers.forEach(function(observer) {
      observer.call(this, document, state);
    });
  }

  /**
   * @param {!Document} document
   */
  function startTracking(document) {
    console.log('DocumentState startTracking', document);
    documentStateMap.set(document, new DocumentState(document));
    document.addEventListener('attach', didAttachWindow);
    document.addEventListener('load', didDocumentLoadSave);
    document.addEventListener('save', didDocumentLoadSave);
  }

  Document.addObserver(function(type, document) {
    if (type == 'add')
      startTracking(document);
  });
})();
