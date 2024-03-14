/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2023 Chris Harlow
 * Copyright © 2023 Peter Akers
 * Copyright © 2023 Peter Cole
 *
 * This work is licensed under the Creative Commons Attribution-ShareAlike
 * 4.0 International License. To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-sa/4.0/ or send a letter to
 * Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
 *
 * Attribution — You must give appropriate credit, provide a link to the
 * license, and indicate if changes were made. You may do so in any
 * reasonable manner, but not in any way that suggests the licensor
 * endorses you or your use.
 *
 * ShareAlike — If you remix, transform, or build upon the material, you
 * must distribute your contributions under the same license as the
 * original.
 *
 * All other rights reserved.
 *
 */

// This is a demonstration script for a simplified <> command parser
// It accepts <> command inputs and breaks it down to values

// The dump() function is used to list the parameters obtained.
// so this is the best place to look for how to access the results.
#include "DCCEXInbound.h"
#include <Arduino.h>

// Internal stuff for the parser and getters.
const int32_t QUOTE_FLAG = 0x77777000;
const int32_t QUOTE_FLAG_AREA = 0xFFFFF000;
enum splitState : byte {
  FIND_START,
  SET_OPCODE,
  SKIP_SPACES,
  CHECK_SIGN,
  BUILD_PARAM,
  SKIPOVER_TEXT,
  COMPLETE_i_COMMAND
};

int16_t DCCEXInbound::_maxParams = 0;
int16_t DCCEXInbound::_parameterCount = 0;
byte DCCEXInbound::_opcode = 0;
int32_t *DCCEXInbound::_parameterValues = nullptr;
char *DCCEXInbound::_cmdBuffer = nullptr;

// Public methods

void DCCEXInbound::setup(int16_t maxParameterValues) {
  _parameterValues = (int32_t *)realloc(_parameterValues, maxParameterValues * sizeof(int32_t));
  _maxParams = maxParameterValues;
  _parameterCount = 0;
  _opcode = 0;
}

void DCCEXInbound::cleanup() {
  if (_parameterValues) {
    free(_parameterValues);
    _parameterValues = nullptr;
  }
}

byte DCCEXInbound::getOpcode() { return _opcode; }

int16_t DCCEXInbound::getParameterCount() { return _parameterCount; }

int32_t DCCEXInbound::getNumber(int16_t parameterNumber) {
  if (parameterNumber < 0 || parameterNumber >= _parameterCount)
    return 0;
  if (_isTextInternal(parameterNumber))
    return 0;
  return _parameterValues[parameterNumber];
}

bool DCCEXInbound::isTextParameter(int16_t parameterNumber) {
  if (parameterNumber < 0 || parameterNumber >= _parameterCount)
    return false;
  return _isTextInternal(parameterNumber);
}

char *DCCEXInbound::getTextParameter(int16_t parameterNumber) {
  if (parameterNumber < 0 || parameterNumber >= _parameterCount)
    return 0;
  if (!_isTextInternal(parameterNumber))
    return 0;
  return _cmdBuffer + (_parameterValues[parameterNumber] & ~QUOTE_FLAG_AREA);
}

char *DCCEXInbound::copyTextParameter(int16_t parameterNumber) {
  char *unsafe = getTextParameter(parameterNumber);
  if (!unsafe)
    return unsafe; // bad parameter number probably
  char *safe = (char *)malloc(strlen(unsafe) + 1);
  strcpy(safe, unsafe);
  return safe;
}

bool DCCEXInbound::parse(char *command) {
  _parameterCount = 0;
  _opcode = 0;
  _cmdBuffer = command;

  int32_t runningValue = 0;
  char *remainingCmd = command;
  bool signNegative = false;
  splitState state = FIND_START;

  while (_parameterCount < _maxParams) {
    byte hot = *remainingCmd;
    if (hot == 0)
      return false; // no > on end of command.

    // In this switch, break will go on to next char but continue will
    // rescan the current char.
    switch (state) {
    case FIND_START: // looking for <
      if (hot == '<')
        state = SET_OPCODE;
      break;
    case SET_OPCODE:
      _opcode = hot;
      if (_opcode == 'i') {
        // special case <iDCCEX stuff > breaks all normal rules
        _parameterValues[_parameterCount] = QUOTE_FLAG | (remainingCmd - _cmdBuffer + 1);
        _parameterCount++;
        state = COMPLETE_i_COMMAND;
        break;
      }
      state = SKIP_SPACES;
      break;

    case SKIP_SPACES: // skipping spaces before a param
      if (hot == ' ')
        break; // ignore
      if (hot == '>')
        return true;
      state = CHECK_SIGN;
      continue;

    case CHECK_SIGN: // checking sign or quotes start param.
      if (hot == '"') {
        // for a string parameter, the value is the offset of the first char in the cmd.
        _parameterValues[_parameterCount] = QUOTE_FLAG | (remainingCmd - _cmdBuffer + 1);
        _parameterCount++;
        state = SKIPOVER_TEXT;
        break;
      }
      runningValue = 0;
      state = BUILD_PARAM;
      signNegative = hot == '-';
      if (signNegative)
        break;
      continue;

    case BUILD_PARAM: // building a parameter
      if (hot >= '0' && hot <= '9') {
        runningValue = 10 * runningValue + (hot - '0');
        break;
      }
      if (hot >= 'a' && hot <= 'z')
        hot = hot - 'a' + 'A'; // uppercase a..z

      if (hot == '_' || (hot >= 'A' && hot <= 'Z')) {
        // Super Kluge to turn keywords into a hash value that can be recognised later
        runningValue = ((runningValue << 5) + runningValue) ^ hot;
        break;
      }
      // did not detect 0-9 or keyword so end of parameter detected
      _parameterValues[_parameterCount] = runningValue * (signNegative ? -1 : 1);
      _parameterCount++;
      state = SKIP_SPACES;
      continue;

    case SKIPOVER_TEXT:
      if (hot == '"') {
        *remainingCmd = '\0'; // overwrite " in command buffer with the end-of-string
        state = SKIP_SPACES;
      }
      break;
    case COMPLETE_i_COMMAND:
      if (hot == '>') {
        *remainingCmd = '\0'; // overwrite > in command buffer with the end-of-string
        return true;
      }
      break;
    }
    remainingCmd++;
  }
  return false; // we ran out of max parameters
}

void DCCEXInbound::dump(Print *out) {
  out->print(F("\nDCCEXInbound Opcode='"));
  if (_opcode)
    out->write(_opcode);
  else
    out->print(F("\\0"));
  out->println('\'');

  for (int i = 0; i < getParameterCount(); i++) {
    if (isTextParameter(i)) {
      out->print(F("getTextParameter("));
      out->print(i);
      out->print(F(")=\""));
      out->print(getTextParameter(i));
      out->println('"');
    } else {
      out->print(F("getNumber("));
      out->print(i);
      out->print(F(")="));
      out->println(getNumber(i));
    }
  }
}

// Private methods

bool DCCEXInbound::_isTextInternal(int16_t n) { return ((_parameterValues[n] & QUOTE_FLAG_AREA) == QUOTE_FLAG); }
