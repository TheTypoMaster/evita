// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
'use strict';

(function() {
  global.Range.Case = {
    CAPITALIZED_TEXT: 'CAPITALIZED_TEXT', // "This is capitalized."
    CAPITALIZED_WORDS: 'CAPITALIZED_WORDS', // "This Is Capitalized Words."
    LOWER: 'LOWER', // "this is lower."
    MIXED: 'MIXED', // "ThisIsMixed."
    UPPER: 'UPPER' // "THIS IS UPPER."
  };

  /**
   * @return {!Range.Case}
   */
  Range.prototype.analyzeCase = function() {
    /** @enum{string} */
    var State = {
      FIRST_CAP_IN_WORD: 'FIRST_CAP_IN_WORD',
      FIRST_CAP_NOT_WORD: 'FIRST_CAP_NOT_WORD',
      FIRST_CAP_SECOND: 'FIRST_CAP_SECOND',
      LOWER: 'LOWER',
      REST_CAP_IN_WORD: 'REST_CAP_IN_WORD',
      REST_CAP_NOT_WORD: 'REST_CAP_NOT_WORD',
      REST_CAP_REST: 'REST_CAP_REST',
      START: 'START',
      UPPER: 'UPPER',
    };
    var document = this.document;
    var start = this.start;
    var end = this.end;
    var string_case = Range.Case.MIXED;
    var state = State.START;
    for (var offset = start; offset < end; ++offset) {
      var char_code = document.charCodeAt_(offset);
      var ucd = Unicode.UCD[char_code];
      var lower_case = ucd.category == Unicode.Category.Ll ||
                       ucd.category == Unicode.Category.Lt;
      var upper_case = ucd.category == Unicode.Category.Lu;
      switch (state) {
        case State.START:
          if (upper_case) {
            string_case = Range.Case.CAPITALIZED_WORDS;
            state = State.FIRST_CAP_SECOND;
          } else if (lower_case) {
            string_case = Range.Case.LOWER;
            state = State.LOWER;
          }
          break;
        case State.FIRST_CAP_IN_WORD:
          if (upper_case) {
            // We found "FoB".
            return Range.Case.MIXED;
          } else if (lower_case) {
            // We found "Foo".
          } else {
            // We found "Foo+".
            state = State.FIRST_CAP_NOT_WORD;
          }
          break;
        case State.FIRST_CAP_NOT_WORD:
          if (upper_case) {
            // We found "Foo B".
            state = State.REST_CAP_IN_WORD;
          } else if (lower_case) {
            // We found "Foo b"
            string_case = Range.Case.CAPITALIZED_TEXT;
            state = State.LOWER;
          }
          break;
        case State.FIRST_CAP_SECOND:
          if (upper_case) {
            // We found "FO"
            string_case = Range.Case.UPPER;
            state = State.UPPER;
          } else if (lower_case) {
            // We found "Fo"
            state = State.FIRST_CAP_IN_WORD;
          } else {
            // We see "F+"
            state = State.FIRST_CAP_NOT_WORD;
          }
          break;
        case State.LOWER:
          if (upper_case) {
            // We found "foB"
            return Range.Case.MIXED;
          }
          break;
        case State.REST_CAP_IN_WORD:
          if (upper_case) {
            // We found "Foo Bar BaZ"
            return Range.Case.MIXED;
          }
          if (!lower_case) {
            // We found "Foo Bar+"
            state = State.REST_CAP_NOT_WORD;
          }
          break;
        case State.REST_CAP_NOT_WORD:
          if (lower_case) {
            // We found "Foo Bar+b"
            return Range.Case.MIXED;
          }
          if (upper_case) {
            // We found "Foo Bar+B"
            state = State.REST_CAP_IN_WORD;
          }
          break;
        case State.UPPER:
          if (lower_case) {
            // We found "FOo"
            return Range.Case.MIXED;
          }
          break;
        default:
          throw 'Unexepcted state ' + state;
      }
    }
    return string_case;
  };

  /**
   * Capitalize range.
   * @this {!Range}
   * @return {!Range}
   */
  Range.prototype.capitalize = function() {
    var text = this.text;
    for (var i = 0; i < text.length; ++i) {
      var data = Unicode.UCD[text.charCodeAt(i)];
      if (data.category == Unicode.Category.Lu ||
          data.category == Unicode.Category.Ll ||
          data.category == Unicode.Category.Lt) {
        this.text = text.substr(0, i) + text.charAt(i).toLocaleUpperCase() +
                    text.substr(i + 1).toLocaleLowerCase();
        break;
      }
    }
    return this;
  };

  /**
   * Delete
   * @param {Unit} unit
   * @param {number=} opt_count, default is one.
   * @return {!Range}
   */
  Range.prototype.delete = function(unit, opt_count) {
    var count = arguments.length >= 2 ? opt_count : 1;
    var delta = unit == Unit.CHARACTER && this.start != this.end ? 1 : 0;
    if (count < 0)
      this.moveStart(unit, count + delta);
    else if (count > 0)
      this.moveEnd(unit, count - delta);
    this.text = '';
    return this;
  };

  /**
   * Move end position of Range at end of specified unit.
   * @this {!Range}
   * @param {Unit} unit.
   * @param {Alter=} opt_alter, default is Alter.MOVE.
   * @return {!Range}
   */
  Range.prototype.endOf = function(unit, opt_alter) {
    var alter = arguments.length >= 2 ? opt_alter : Alter.MOVE;
    this.end = this.document.computeEndOf_(unit, this.end);
    switch (alter) {
      case Alter.EXTEND:
        break;
      case Alter.MOVE:
        this.start = this.end;
        break;
      default:
        throw TypeError('Invalid alter: ' + alter);
    }
    return this;
  };

  /**
   * @param {!Editor.RegExp} regexp
   * @return {?Array.<string>}
   */
  Range.prototype.match = function(regexp) {
    var range = this;
    var start = range.start;
    var end = range.end;
    var matches = range.document.match_(regexp, start, end);
    if (!matches)
      return null;
    var strings = matches.map(function(match) {
      return range.document.slice(match.start, match.end);
    });
    range.collapseTo(start);
    range.end = end;
    return strings;
  };

  /**
   * @this {!Range}
   * @param {Unit} unit.
   * @param {number=} opt_count, defualt is one.
   * @return {!Range}
   */
  Range.prototype.move = function(unit, opt_count) {
    var count = arguments.length >= 2 ? opt_count : 1;
    var position = count > 0 ? this.end : this.start;
    this.collapseTo(this.document.computeMotion_(unit, count, position));
    return this;
  };

  /**
   * @this {!Range}
   * @param {Unit} unit.
   * @param {number=} opt_count, defualt is one.
   * @return {!Range}
   */
  Range.prototype.moveEnd = function(unit, opt_count) {
    var count = arguments.length >= 2 ? opt_count : 1;
    var position = this.document.computeMotion_(unit, count, this.end);
    if (position >= this.start)
      this.end = position;
    else
      this.collapseTo(position);
    return this;
  };

  /**
   * @param {string} char_set
   * @param {number=} opt_count, default is Count.FORWARD
   * @return {!Range}
   */
  Range.prototype.moveEndWhile = function(char_set, opt_count) {
    var count = arguments.length >= 2 ? /** @type{number} */(opt_count) :
        Count.FORWARD;
    var position = this.document.computeWhile_(char_set, count, this.end);
    if (position < this.start)
      this.collapseTo(position);
    else
      this.end = position;
    return this
  };

  /**
   * @this {!Range}
   * @param {Unit} unit.
   * @param {number=} opt_count, defualt is one.
   * @return {!Range}
   */
  Range.prototype.moveStart = function(unit, opt_count) {
    var count = arguments.length >= 2 ? opt_count : 1;
    var position = this.document.computeMotion_(unit, count, this.start);
    if (position <= this.end)
      this.start = position;
    else
      this.collapseTo(position);
    return this;
  };

  /**
   * @param {string} char_set
   * @param {number=} opt_count, default is Count.FORWARD
   * @return {!Range}
   */
  Range.prototype.moveStartWhile = function(char_set, opt_count) {
    var count = arguments.length >= 2 ? /** @type{number} */(opt_count) :
        Count.FORWARD;
    var position = this.document.computeWhile_(char_set, count, this.end);
    if (position > this.end)
      this.collapseTo(position);
    else
      this.start = position;
    return this
  };

  /**
   * Move start position of Range at start of specified unit.
   * @param {Unit} unit.
   * @param {Alter=} opt_alter, default is Alter.MOVE.
   * @return {!Range}
   */
  Range.prototype.startOf = function(unit, opt_alter) {
    var alter = arguments.length >= 2 ? opt_alter : Alter.MOVE;
    this.start = this.document.computeStartOf_(unit, this.start);
    switch (alter) {
      case Alter.EXTEND:
        break;
      case Alter.MOVE:
        this.end = this.start;
        break;
      default:
        throw TypeError('Invalid alter: ' + alter);
    }
    return this;
  };

  /**
   * @return {!Range}
   */
  Range.prototype.toLocaleLowerCase = function() {
    this.text = this.text.toLocaleLowerCase();
    return this;
  };
  
  /**
   * @return {!Range}
   */
  Range.prototype.toLocaleUpperCase = function() {
    this.text = this.text.toLocaleUpperCase();
    return this;
  };
  
  /**
   * @return {!Range}
   */
  Range.prototype.toLowerCase = function() {
    this.text = this.text.toLowerCase();
    return this;
  };
  
  /**
   * @return {!Range}
   */
  Range.prototype.toUpperCase = function() {
    this.text = this.text.toUpperCase();
    return this;
  };
})();