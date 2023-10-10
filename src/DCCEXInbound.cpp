// This is a demonstration script for a simplified <> command parser
//It accepts <> command inputs and breaks it down to values

// The dump() function is used to list the parameters obtained. 
// so this is the best place to look for how to access the results. 
#include <Arduino.h>
#include "DCCEXInbound.h"


// Internal stuff for the parser and getters. 
const int32_t QUOTE_FLAG=0x77777000;
const int32_t QUOTE_FLAG_AREA=0xFFFFF000;
enum splitState:byte {FIND_START,SET_OPCODE,SKIP_SPACES,CHECK_SIGN,
                      BUILD_PARAM,SKIPOVER_TEXT, COMPLETE_i_COMMAND};


    int16_t DCCEXInbound::maxParams;
    int16_t DCCEXInbound::parameterCount;
    byte DCCEXInbound::opcode;
    int32_t * DCCEXInbound::parameterValues;
    char * DCCEXInbound::cmdBuffer;

    void DCCEXInbound::setup(int16_t maxParameterValues) {
         parameterValues=(int32_t*)malloc(maxParameterValues * sizeof(int32_t));
         maxParams=maxParameterValues;
         parameterCount=0;
         opcode=0;
    }


    byte DCCEXInbound::getOpcode() {
        return opcode; 
    }

    int16_t DCCEXInbound::getParameterCount() {
        return parameterCount;
    }

    int32_t DCCEXInbound::getNumber(int16_t parameterNumber) {
        if (parameterNumber<0 || parameterNumber>=parameterCount) return 0;
        if (isTextInternal(parameterNumber)) return 0;
        return parameterValues[parameterNumber];
    }

    bool DCCEXInbound::isTextParameter(int16_t parameterNumber) {
        if (parameterNumber<0 || parameterNumber>=parameterCount) return false;
        return  isTextInternal(parameterNumber);
    }
    char * DCCEXInbound::getText(int16_t parameterNumber) {
     if (parameterNumber<0 || parameterNumber>=parameterCount) return 0;
        if (!isTextInternal(parameterNumber)) return 0;
        return cmdBuffer + (parameterValues[parameterNumber] & ~QUOTE_FLAG_AREA);;
    }
    
    bool DCCEXInbound::isTextInternal(int16_t n) {
        return  ((parameterValues[n] & QUOTE_FLAG_AREA)==QUOTE_FLAG);
    }

bool DCCEXInbound::parse(char* command) {

    parameterCount = 0;
    opcode=0;
    cmdBuffer=command; 
    
    int32_t runningValue = 0;
    char *remainingCmd = command; 
    bool signNegative = false;
    splitState state = FIND_START;
    
    
    while (parameterCount < maxParams)
    {
        byte hot = *remainingCmd;
        if (hot==0) return false;  // no > on end of command.

        // In this switch, break will go on to next char but continue will
        // rescan the current char. 
        switch (state)
        {
        case FIND_START: // looking for <
            if (hot=='<') state=SET_OPCODE;
            break;
        case SET_OPCODE:
             opcode=hot;
             if (opcode=='i') {
                // special case <iDCCEX stuff > breaks all normal rules
                parameterValues[parameterCount] = QUOTE_FLAG | (remainingCmd-cmdBuffer+1);
                parameterCount++;
                state=COMPLETE_i_COMMAND;
                break;
             }
             state=SKIP_SPACES;
             break;     
        case SKIP_SPACES: // skipping spaces before a param
            if (hot == ' ') break; // ignore
            if (hot == '>') return true;
            state = CHECK_SIGN;
            continue;

        case CHECK_SIGN: // checking sign or quotes start param.
            if (hot=='"') {
              // for a string parameter, the value is the offset of the first char in the cmd.
              parameterValues[parameterCount] = QUOTE_FLAG | (remainingCmd-cmdBuffer+1);
              parameterCount++;
              state=SKIPOVER_TEXT;
              break;
            }
            runningValue = 0;
            state = BUILD_PARAM;
            signNegative = hot=='-';
            if (signNegative) break;
            continue; 
        
        case BUILD_PARAM: // building a parameter
            if (hot >= '0' && hot <= '9')
            {
                runningValue = 10 * runningValue + (hot - '0');
                break;
            }
            if (hot >= 'a' && hot <= 'z') hot=hot-'a'+'A'; // uppercase a..z
            
            if (hot=='_' || (hot >= 'A' && hot <= 'Z'))
            {
                // Super Kluge to turn keywords into a hash value that can be recognised later
                runningValue = ((runningValue << 5) + runningValue) ^ hot;
                break;
            }
            // did not detect 0-9 or keyword so end of parameter detected 
            parameterValues[parameterCount] = runningValue * (signNegative ? -1 : 1);
            parameterCount++;
            state = SKIP_SPACES;
            continue;
        
        case SKIPOVER_TEXT:
            if (hot=='"') {
              *remainingCmd='\0';         // overwrite " in command buffer with the end-of-string
              state=SKIP_SPACES;   
            }
            break;
        case COMPLETE_i_COMMAND:
            if (hot=='>') {
              *remainingCmd='\0';         // overwrite > in command buffer with the end-of-string
              return true;
            }
            break;  
        }
        remainingCmd++;
    }
    return false; // we ran out of max parameters
}
