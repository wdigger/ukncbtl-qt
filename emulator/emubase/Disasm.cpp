/*  This file is part of UKNCBTL.
    UKNCBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    UKNCBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
UKNCBTL. If not, see <http://www.gnu.org/licenses/>. */

/// \file Disasm.cpp
/// \brief Disassembler for KM1801VM2 processor
/// \details See defines in header file Emubase.h

#include "Defines.h"
#include "Emubase.h"

#include <cstring>

// Формат отображения режимов адресации
const char* ADDRESS_MODE_FORMAT[] =
{
    "%s", "(%s)", "(%s)+", "@(%s)+", "-(%s)", "@-(%s)", "%06o(%s)", "@%06o(%s)"
};
// Формат отображения режимов адресации для регистра PC
const char* ADDRESS_MODE_PC_FORMAT[] =
{
    "PC", "(PC)", "#%06o", "@#%06o", "-(PC)", "@-(PC)", "%06o", "@%06o"
};

//   strSrc - at least 24 characters
uint16_t ConvertSrcToString(uint16_t instr, uint16_t addr, char* strSrc, uint16_t code)
{
    const size_t strSrcSize = 24;

    uint8_t reg = GetDigit(instr, 2);
    uint8_t param = GetDigit(instr, 3);

    const char* pszReg = REGISTER_NAME[reg];

    if (reg != 7)
    {
        const char* format = ADDRESS_MODE_FORMAT[param];

        if (param == 6 || param == 7)
        {
            uint16_t word = code;  //TODO: pMemory
            snprintf(strSrc, strSrcSize - 1, format, word, pszReg);
            return 1;
        }
        else
            snprintf(strSrc, strSrcSize - 1, format, pszReg);
    }
    else
    {
        const char* format = ADDRESS_MODE_PC_FORMAT[param];

        if (param == 2 || param == 3)
        {
            uint16_t word = code;  //TODO: pMemory
            snprintf(strSrc, strSrcSize - 1, format, word);
            return 1;
        }
        else if (param == 6 || param == 7)
        {
            uint16_t word = code;  //TODO: pMemory
            snprintf(strSrc, strSrcSize - 1, format, (uint16_t)(addr + word + 2));
            return 1;
        }
        else
            snprintf(strSrc, strSrcSize - 1, format, pszReg);
    }

    return 0;
}

//   strDst - at least 24 characters
uint16_t ConvertDstToString (uint16_t instr, uint16_t addr, char* strDst, uint16_t code)
{
    const size_t strDstSize = 24;

    uint8_t reg = GetDigit(instr, 0);
    uint8_t param = GetDigit(instr, 1);

    const char* pszReg = REGISTER_NAME[reg];

    if (reg != 7)
    {
        const char* format = ADDRESS_MODE_FORMAT[param];

        if (param == 6 || param == 7)
        {
            snprintf(strDst, strDstSize - 1, format, code, pszReg);
            return 1;
        }
        else
            snprintf(strDst, strDstSize - 1, format, pszReg);
    }
    else
    {
        const char* format = ADDRESS_MODE_PC_FORMAT[param];

        if (param == 2 || param == 3)
        {
            snprintf(strDst, strDstSize - 1, format, code);
            return 1;
        }
        else if (param == 6 || param == 7)
        {
            snprintf(strDst, strDstSize - 1, format, (uint16_t)(addr + code + 2));
            return 1;
        }
        else
            snprintf(strDst, strDstSize - 1, format, pszReg);
    }

    return 0;
}

// Disassemble one instruction
//   pMemory - memory image (we read only words of the instruction)
//   strInstr - instruction mnemonics buffer, at least 8 characters
//   strArg   - instruction arguments buffer, at least 32 characters
//   Return value: number of words in the instruction
uint16_t DisassembleInstruction(const uint16_t* pMemory, uint16_t addr, char* strInstr, char* strArg)
{
    //const size_t strInstrSize = 8;
    const size_t strArgSize = 32;

    *strInstr = 0;
    *strArg = 0;

    uint16_t instr = *pMemory;
    uint16_t length = 1;
    const char* strReg = nullptr;

    const size_t strSrcSize = 24;
    char strSrc[strSrcSize];
    const size_t strDstSize = 24;
    char strDst[strDstSize];

    bool okByte;

    // No fields
    switch (instr)
    {
    case PI_HALT:   strcpy(strInstr, "HALT");   return 1;
    case PI_WAIT:   strcpy(strInstr, "WAIT");   return 1;
    case PI_RTI:    strcpy(strInstr, "RTI");    return 1;
    case PI_BPT:    strcpy(strInstr, "BPT");    return 1;
    case PI_IOT:    strcpy(strInstr, "IOT");    return 1;
    case PI_RESET:  strcpy(strInstr, "RESET");  return 1;
    case PI_RTT:    strcpy(strInstr, "RTT");    return 1;
    case PI_NOP:    strcpy(strInstr, "NOP");    return 1;
    case PI_CLC:    strcpy(strInstr, "CLC");    return 1;
    case PI_CLV:    strcpy(strInstr, "CLV");    return 1;
    case PI_CLVC:   strcpy(strInstr, "CLVC");   return 1;
    case PI_CLZ:    strcpy(strInstr, "CLZ");    return 1;
    case PI_CLZC:   strcpy(strInstr, "CLZC");   return 1;
    case PI_CLZV:   strcpy(strInstr, "CLZV");   return 1;
    case PI_CLZVC:  strcpy(strInstr, "CLZVC");  return 1;
    case PI_CLN:    strcpy(strInstr, "CLN");    return 1;
    case PI_CLNC:   strcpy(strInstr, "CLNC");   return 1;
    case PI_CLNV:   strcpy(strInstr, "CLNV");   return 1;
    case PI_CLNVC:  strcpy(strInstr, "CLNVC");  return 1;
    case PI_CLNZ:   strcpy(strInstr, "CLNZ");   return 1;
    case PI_CLNZC:  strcpy(strInstr, "CLNZC");  return 1;
    case PI_CLNZV:  strcpy(strInstr, "CLNZV");  return 1;
    case PI_CCC:    strcpy(strInstr, "CCC");    return 1;
    case PI_NOP260: strcpy(strInstr, "NOP260"); return 1;
    case PI_SEC:    strcpy(strInstr, "SEC");    return 1;
    case PI_SEV:    strcpy(strInstr, "SEV");    return 1;
    case PI_SEVC:   strcpy(strInstr, "SEVC");   return 1;
    case PI_SEZ:    strcpy(strInstr, "SEZ");    return 1;
    case PI_SEZC:   strcpy(strInstr, "SEZC");   return 1;
    case PI_SEZV:   strcpy(strInstr, "SEZV");   return 1;
    case PI_SEZVC:  strcpy(strInstr, "SEZVC");  return 1;
    case PI_SEN:    strcpy(strInstr, "SEN");    return 1;
    case PI_SENC:   strcpy(strInstr, "SENC");   return 1;
    case PI_SENV:   strcpy(strInstr, "SENV");   return 1;
    case PI_SENVC:  strcpy(strInstr, "SENVC");  return 1;
    case PI_SENZ:   strcpy(strInstr, "SENZ");   return 1;
    case PI_SENZC:  strcpy(strInstr, "SENZC");  return 1;
    case PI_SENZV:  strcpy(strInstr, "SENZV");  return 1;
    case PI_SCC:    strcpy(strInstr, "SCC");    return 1;

        // Спецкоманды режима HALT ВМ2
    case PI_START:  strcpy(strInstr, "START");  return 1;
    case PI_STEP:   strcpy(strInstr, "STEP");   return 1;
    case PI_RSEL:   strcpy(strInstr, "RSEL");   return 1;
    case PI_MFUS:   strcpy(strInstr, "MFUS");   return 1;
    case PI_RCPC:   strcpy(strInstr, "RCPC");   return 1;
    case PI_RCPS:   strcpy(strInstr, "RCPS");   return 1;
    case PI_MTUS:   strcpy(strInstr, "MTUS");   return 1;
    case PI_WCPC:   strcpy(strInstr, "WCPC");   return 1;
    case PI_WCPS:   strcpy(strInstr, "WCPS");   return 1;
    }

    // One field
    if ((instr & ~(uint16_t)7) == PI_RTS)
    {
        if (GetDigit(instr, 0) == 7)
        {
            strcpy(strInstr, "RETURN");
            return 1;
        }

        strcpy(strInstr, "RTS");
        strcpy(strArg, REGISTER_NAME[GetDigit(instr, 0)]);
        return 1;
    }

    // FIS
    switch (instr & ~(uint16_t)7)
    {
    case PI_FADD:  strcpy(strInstr, "FADD");  strcpy(strArg, REGISTER_NAME[GetDigit(instr, 0)]);  return 1;
    case PI_FSUB:  strcpy(strInstr, "FSUB");  strcpy(strArg, REGISTER_NAME[GetDigit(instr, 0)]);  return 1;
    case PI_FMUL:  strcpy(strInstr, "FMUL");  strcpy(strArg, REGISTER_NAME[GetDigit(instr, 0)]);  return 1;
    case PI_FDIV:  strcpy(strInstr, "FDIV");  strcpy(strArg, REGISTER_NAME[GetDigit(instr, 0)]);  return 1;
    }

    // Two fields
    length += ConvertDstToString(instr, addr + 2, strDst, pMemory[1]);

    switch (instr & ~(uint16_t)077)
    {
    case PI_JMP:    strcpy(strInstr, "JMP");   strcpy(strArg, strDst);  return length;
    case PI_SWAB:   strcpy(strInstr, "SWAB");  strcpy(strArg, strDst);  return length;
    case PI_MARK:   strcpy(strInstr, "MARK");  strcpy(strArg, strDst);  return length;
    case PI_SXT:    strcpy(strInstr, "SXT");   strcpy(strArg, strDst);  return length;
    case PI_MTPS:   strcpy(strInstr, "MTPS");  strcpy(strArg, strDst);  return length;
    case PI_MFPS:   strcpy(strInstr, "MFPS");  strcpy(strArg, strDst);  return length;
    }

    okByte = (instr & 0100000) != 0;

    switch (instr & ~(uint16_t)0100077)
    {
    case PI_CLR:  strcpy(strInstr, okByte ? "CLRB" : "CLR");  strcpy(strArg, strDst);  return length;
    case PI_COM:  strcpy(strInstr, okByte ? "COMB" : "COM");  strcpy(strArg, strDst);  return length;
    case PI_INC:  strcpy(strInstr, okByte ? "INCB" : "INC");  strcpy(strArg, strDst);  return length;
    case PI_DEC:  strcpy(strInstr, okByte ? "DECB" : "DEC");  strcpy(strArg, strDst);  return length;
    case PI_NEG:  strcpy(strInstr, okByte ? "NEGB" : "NEG");  strcpy(strArg, strDst);  return length;
    case PI_ADC:  strcpy(strInstr, okByte ? "ADCB" : "ADC");  strcpy(strArg, strDst);  return length;
    case PI_SBC:  strcpy(strInstr, okByte ? "SBCB" : "SBC");  strcpy(strArg, strDst);  return length;
    case PI_TST:  strcpy(strInstr, okByte ? "TSTB" : "TST");  strcpy(strArg, strDst);  return length;
    case PI_ROR:  strcpy(strInstr, okByte ? "RORB" : "ROR");  strcpy(strArg, strDst);  return length;
    case PI_ROL:  strcpy(strInstr, okByte ? "ROLB" : "ROL");  strcpy(strArg, strDst);  return length;
    case PI_ASR:  strcpy(strInstr, okByte ? "ASRB" : "ASR");  strcpy(strArg, strDst);  return length;
    case PI_ASL:  strcpy(strInstr, okByte ? "ASLB" : "ASL");  strcpy(strArg, strDst);  return length;
    }

    length = 1;
    snprintf(strDst, strDstSize - 1, "%06ho", (uint16_t)(addr + ((short)(char)(uint8_t)(instr & 0xff) * 2) + 2));

    // Branches & interrupts
    switch (instr & ~(uint16_t)0377)
    {
    case PI_BR:   strcpy(strInstr, "BR");   strcpy(strArg, strDst);  return 1;
    case PI_BNE:  strcpy(strInstr, "BNE");  strcpy(strArg, strDst);  return 1;
    case PI_BEQ:  strcpy(strInstr, "BEQ");  strcpy(strArg, strDst);  return 1;
    case PI_BGE:  strcpy(strInstr, "BGE");  strcpy(strArg, strDst);  return 1;
    case PI_BLT:  strcpy(strInstr, "BLT");  strcpy(strArg, strDst);  return 1;
    case PI_BGT:  strcpy(strInstr, "BGT");  strcpy(strArg, strDst);  return 1;
    case PI_BLE:  strcpy(strInstr, "BLE");  strcpy(strArg, strDst);  return 1;
    case PI_BPL:  strcpy(strInstr, "BPL");  strcpy(strArg, strDst);  return 1;
    case PI_BMI:  strcpy(strInstr, "BMI");  strcpy(strArg, strDst);  return 1;
    case PI_BHI:  strcpy(strInstr, "BHI");  strcpy(strArg, strDst);  return 1;
    case PI_BLOS:  strcpy(strInstr, "BLOS");  strcpy(strArg, strDst);  return 1;
    case PI_BVC:  strcpy(strInstr, "BVC");  strcpy(strArg, strDst);  return 1;
    case PI_BVS:  strcpy(strInstr, "BVS");  strcpy(strArg, strDst);  return 1;
    case PI_BHIS:  strcpy(strInstr, "BHIS");  strcpy(strArg, strDst);  return 1;
    case PI_BLO:  strcpy(strInstr, "BLO");  strcpy(strArg, strDst);  return 1;
    }

    snprintf(strDst, strDstSize - 1, "%06o", (uint8_t)(instr & 0xff));

    switch (instr & ~(uint16_t)0377)
    {
    case PI_EMT:   strcpy(strInstr, "EMT");   strcpy(strArg, strDst + 3);  return 1;
    case PI_TRAP:  strcpy(strInstr, "TRAP");  strcpy(strArg, strDst + 3);  return 1;
    }

    // Three fields
    switch (instr & ~(uint16_t)0777)
    {
    case PI_JSR:
        if (GetDigit(instr, 2) == 7)
        {
            strcpy(strInstr, "CALL");
            length += ConvertDstToString (instr, addr + 2, strDst, pMemory[1]);
            snprintf(strArg, strArgSize - 1, "%s", strDst);  // strArg = strDst;
        }
        else
        {
            strcpy(strInstr, "JSR");
            strReg = REGISTER_NAME[GetDigit(instr, 2)];
            length += ConvertDstToString (instr, addr + 2, strDst, pMemory[1]);
            snprintf(strArg, strArgSize - 1, "%s, %s", strReg, strDst);  // strArg = strReg + ", " + strDst;
        }
        return length;
    case PI_MUL:
        strcpy(strInstr, "MUL");
        strReg = REGISTER_NAME[GetDigit(instr, 2)];
        length += ConvertDstToString (instr, addr + 2, strDst, pMemory[1]);
        snprintf(strArg, strArgSize - 1, "%s, %s", strDst, strReg);  // strArg = strDst + ", " + strReg;
        return length;
    case PI_DIV:
        strcpy(strInstr, "DIV");
        strReg = REGISTER_NAME[GetDigit(instr, 2)];
        length += ConvertDstToString (instr, addr + 2, strDst, pMemory[1]);
        snprintf(strArg, strArgSize - 1, "%s, %s", strDst, strReg);  // strArg = strDst + ", " + strReg;
        return length;
    case PI_ASH:
        strcpy(strInstr, "ASH");
        strReg = REGISTER_NAME[GetDigit(instr, 2)];
        length += ConvertDstToString (instr, addr + 2, strDst, pMemory[1]);
        snprintf(strArg, strArgSize - 1, "%s, %s", strDst, strReg);  // strArg = strDst + ", " + strReg;
        return length;
    case PI_ASHC:
        strcpy(strInstr, "ASHC");
        strReg = REGISTER_NAME[GetDigit(instr, 2)];
        length += ConvertDstToString (instr, addr + 2, strDst, pMemory[1]);
        snprintf(strArg, strArgSize - 1, "%s, %s", strDst, strReg);  // strArg = strDst + ", " + strReg;
        return length;
    case PI_XOR:
        strcpy(strInstr, "XOR");
        strReg = REGISTER_NAME[GetDigit(instr, 2)];
        length += ConvertDstToString (instr, addr + 2, strDst, pMemory[1]);
        snprintf(strArg, strArgSize - 1, "%s, %s", strReg, strDst);  // strArg = strReg + ", " + strDst;
        return length;
    case PI_SOB:
        strcpy(strInstr, "SOB");
        strReg = REGISTER_NAME[GetDigit(instr, 2)];
        snprintf(strDst, strDstSize - 1, "%06o", addr - (GetDigit(instr, 1) * 8 + GetDigit(instr, 0)) * 2 + 2);
        snprintf(strArg, strArgSize - 1, "%s, %s", strReg, strDst);  // strArg = strReg + ", " + strDst;
        return 1;
    }

    // Four fields

    okByte = (instr & 0100000) != 0;

    length += ConvertSrcToString(instr, addr + 2, strSrc, pMemory[1]);
    length += ConvertDstToString(instr, (uint16_t)(addr + 2 + (length - 1) * 2), strDst, pMemory[length]);

    switch (instr & ~(uint16_t)0107777)
    {
    case PI_MOV:
        if (!okByte && GetDigit(instr, 0) == 6 && GetDigit(instr, 1) == 4)
        {
            strcpy(strInstr, "PUSH");
            snprintf(strArg, strArgSize - 1, "%s", strSrc);  // strArg = strSrc;
            return length;
        }
        if (!okByte && GetDigit(instr, 2) == 6 && GetDigit(instr, 3) == 2)
        {
            strcpy(strInstr, "POP");
            snprintf(strArg, strArgSize - 1, "%s", strDst);  // strArg = strDst;
            return length;
        }
        strcpy(strInstr, okByte ? "MOVB" : "MOV");
        snprintf(strArg, strArgSize - 1, "%s, %s", strSrc, strDst);  // strArg = strSrc + ", " + strDst;
        return length;
    case PI_CMP:
        strcpy(strInstr, okByte ? "CMPB" : "CMP");
        snprintf(strArg, strArgSize - 1, "%s, %s", strSrc, strDst);  // strArg = strSrc + ", " + strDst;
        return length;
    case PI_BIT:
        strcpy(strInstr, okByte ? "BITB" : "BIT");
        snprintf(strArg, strArgSize - 1, "%s, %s", strSrc, strDst);  // strArg = strSrc + ", " + strDst;
        return length;
    case PI_BIC:
        strcpy(strInstr, okByte ? "BICB" : "BIC");
        snprintf(strArg, strArgSize - 1, "%s, %s", strSrc, strDst);  // strArg = strSrc + ", " + strDst;
        return length;
    case PI_BIS:
        strcpy(strInstr, okByte ? "BISB" : "BIS");
        snprintf(strArg, strArgSize - 1, "%s, %s", strSrc, strDst);  // strArg = strSrc + ", " + strDst;
        return length;
    }

    switch (instr & ~(uint16_t)0007777)
    {
    case PI_ADD:
        strcpy(strInstr, "ADD");
        snprintf(strArg, strArgSize - 1, "%s, %s", strSrc, strDst);  // strArg = strSrc + ", " + strDst;
        return length;
    case PI_SUB:
        strcpy(strInstr, "SUB");
        snprintf(strArg, strArgSize - 1, "%s, %s", strSrc, strDst);  // strArg = strSrc + ", " + strDst;
        return length;
    }

    strcpy(strInstr, "unknown");
    snprintf(strArg, strArgSize - 1, "%06o", instr);
    return 1;
}

bool Disasm_CheckForJump(const uint16_t* memory, int* pDelta)
{
    uint16_t instr = *memory;

    // BR, BNE, BEQ, BGE, BLT, BGT, BLE
    // BPL, BMI, BHI, BLOS, BVC, BVS, BHIS, BLO
    if (((instr & 0177400) >= 0000400 && (instr & 0177400) < 0004000) ||
        ((instr & 0177400) >= 0100000 && (instr & 0177400) < 0104000))
    {
        *pDelta = ((int)(char)(instr & 0xff)) + 1;
        return true;
    }

    // SOB
    if ((instr & ~(uint16_t)0777) == PI_SOB)
    {
        *pDelta = -(GetDigit(instr, 1) * 8 + GetDigit(instr, 0)) + 1;
        return true;
    }

    // CALL, JMP
    if (instr == 0004767 || instr == 0000167)
    {
        *pDelta = ((short)(memory[1]) + 4) / 2;
        return true;
    }

    return false;
}

// Prepare "Jump Hint" string, and also calculate condition for conditional jump
// Returns: jump prediction flag: true = will jump, false = will not jump
bool Disasm_GetJumpConditionHint(
    const uint16_t* memory, const CProcessor * pProc, const CMemoryController * pMemCtl, char* buffer)
{
    const size_t buffersize = 32;
    *buffer = 0;
    uint16_t instr = *memory;
    uint16_t psw = pProc->GetPSW();

    if (instr >= 0001000 && instr <= 0001777)  // BNE, BEQ
    {
        snprintf(buffer, buffersize - 1, "Z=%c", (psw & PSW_Z) ? '1' : '0');
        // BNE: IF (Z == 0)
        // BEQ: IF (Z == 1)
        bool value = ((psw & PSW_Z) != 0);
        return ((instr & 0400) == 0) ? !value : value;
    }
    if (instr >= 0002000 && instr <= 0002777)  // BGE, BLT
    {
        snprintf(buffer, buffersize - 1, "N=%c, V=%c", (psw & PSW_N) ? '1' : '0', (psw & PSW_V) ? '1' : '0');
        // BGE: IF ((N xor V) == 0)
        // BLT: IF ((N xor V) == 1)
        bool value = (((psw & PSW_N) != 0) != ((psw & PSW_V) != 0));
        return ((instr & 0400) == 0) ? !value : value;
    }
    if (instr >= 0003000 && instr <= 0003777)  // BGT, BLE
    {
        snprintf(buffer, buffersize - 1, "N=%c, V=%c, Z=%c", (psw & PSW_N) ? '1' : '0', (psw & PSW_V) ? '1' : '0', (psw & PSW_Z) ? '1' : '0');
        // BGT: IF (((N xor V) or Z) == 0)
        // BLE: IF (((N xor V) or Z) == 1)
        bool value = ((((psw & PSW_N) != 0) != ((psw & PSW_V) != 0)) || ((psw & PSW_Z) != 0));
        return ((instr & 0400) == 0) ? !value : value;
    }
    if (instr >= 0100000 && instr <= 0100777)  // BPL, BMI
    {
        snprintf(buffer, buffersize - 1, "N=%c", (psw & PSW_N) ? '1' : '0');
        // BPL: IF (N == 0)
        // BMI: IF (N == 1)
        bool value = ((psw & PSW_N) != 0);
        return ((instr & 0400) == 0) ? !value : value;
    }
    if (instr >= 0101000 && instr <= 0101777)  // BHI, BLOS
    {
        snprintf(buffer, buffersize - 1, "C=%c, Z=%c", (psw & PSW_C) ? '1' : '0', (psw & PSW_Z) ? '1' : '0');
        // BHI:  IF ((С or Z) == 0)
        // BLOS: IF ((С or Z) == 1)
        bool value = (((psw & PSW_C) != 0) || ((psw & PSW_Z) != 0));
        return ((instr & 0400) == 0) ? !value : value;
    }
    if (instr >= 0102000 && instr <= 0102777)  // BVC, BVS
    {
        snprintf(buffer, buffersize - 1, "V=%c", (psw & PSW_V) ? '1' : '0');
        // BVC: IF (V == 0)
        // BVS: IF (V == 1)
        bool value = ((psw & PSW_V) != 0);
        return ((instr & 0400) == 0) ? !value : value;
    }
    if (instr >= 0103000 && instr <= 0103777)  // BCC/BHIS, BCS/BLO
    {
        snprintf(buffer, buffersize - 1, "C=%c", (psw & PSW_C) ? '1' : '0');
        // BCC/BHIS: IF (C == 0)
        // BCS/BLO:  IF (C == 1)
        bool value = ((psw & PSW_C) != 0);
        return ((instr & 0400) == 0) ? !value : value;
    }
    if (instr >= 0077000 && instr <= 0077777)  // SOB
    {
        int reg = (instr >> 6) & 7;
        uint16_t regvalue = pProc->GetReg(reg);
        snprintf(buffer, buffersize - 1, "R%d=%06o", reg, regvalue);  // "RN=XXXXXX"
        return (regvalue != 1);
    }

    if (instr >= 004000 && instr <= 004677)  // JSR (except CALL)
    {
        int reg = (instr >> 6) & 7;
        uint16_t regvalue = pProc->GetReg(reg);
        snprintf(buffer, buffersize - 1, "R%d=%06o", reg, regvalue);  // "RN=XXXXXX"
        return true;
    }
    if (instr >= 000200 && instr <= 000207)  // RTS / RETURN
    {
        uint16_t spvalue = pProc->GetSP();
        int addrtype;
        uint16_t value = pMemCtl->GetWordView(spvalue, pProc->IsHaltMode(), false, &addrtype);
        if (instr == 000207)  // RETURN
            snprintf(buffer, buffersize - 1, "(SP)=%06o", value);  // "(SP)=XXXXXX"
        else  // RTS
        {
            int reg = instr & 7;
            uint16_t regvalue = pProc->GetReg(reg);
            snprintf(buffer, buffersize - 1, "R%d=%06o, (SP)=%06o", reg, regvalue, value);  // "RN=XXXXXX, (SP)=XXXXXX"
        }
        return true;
    }

    if (instr == 000002 || instr == 000006)  // RTI, RTT
    {
        uint16_t spvalue = pProc->GetSP();
        int addrtype;
        uint16_t value = pMemCtl->GetWordView(spvalue, pProc->IsHaltMode(), false, &addrtype);
        snprintf(buffer, buffersize - 1, "(SP)=%06o", value);  // "(SP)=XXXXXX"
        return true;
    }
    if (instr == 000003 || instr == 000004 ||  // IOT, BPT
        (instr >= 0104000 && instr <= 0104777))  // TRAP, EMT
    {
        uint16_t intvec;
        if (instr == 000003) intvec = 000014;
        else if (instr == 000004) intvec = 000020;
        else if (instr < 0104400) intvec = 000030;
        else intvec = 000034;

        int addrtype;
        uint16_t value = pMemCtl->GetWordView(intvec, pProc->IsHaltMode(), false, &addrtype);
        snprintf(buffer, buffersize - 1, "(%06o)=%06o", intvec, value);  // "(VVVVVV)=XXXXXX"
        return true;
    }

    return true;  // All other jumps are non-conditional
}

void Disasm_RegisterHint(const CProcessor * pProc, const CMemoryController * pMemCtl,
        char* hint1, char* hint2,
        int regnum, int regmod, bool byteword, uint16_t indexval)
{
    const size_t hintsize = 20;
    int addrtype = 0;
    uint16_t regval = pProc->GetReg(regnum);
    uint16_t srcval2 = 0;

    snprintf(hint1, hintsize - 1, "%s=%06o", REGISTER_NAME[regnum], regval);  // "RN=XXXXXX"
    switch (regmod)
    {
    case 1:
    case 2:
        srcval2 = pMemCtl->GetWordView(regval, pProc->IsHaltMode(), false, &addrtype);
        if (byteword)
        {
            srcval2 = (regval & 1) ? (srcval2 >> 8) : (srcval2 & 0xff);
            snprintf(hint2, hintsize - 1, "(%s)=%03o", REGISTER_NAME[regnum], srcval2);  // "(RN)=XXX"
        }
        else
        {
            snprintf(hint2, hintsize - 1, "(%s)=%06o", REGISTER_NAME[regnum], srcval2);  // "(RN)=XXXXXX"
        }
        break;
    case 3:
        srcval2 = pMemCtl->GetWordView(regval, pProc->IsHaltMode(), false, &addrtype);
        snprintf(hint2, hintsize - 1, "(%s)=%06o", REGISTER_NAME[regnum], srcval2);  // "(RN)=XXXXXX"
        //TODO: Show the real value in hint line 3
        break;
    case 4:
        if (byteword)
        {
            srcval2 = (regval & 1) ?
                    ((pMemCtl->GetWordView(regval - 1, pProc->IsHaltMode(), false, &addrtype)) & 0xff) :
                    ((pMemCtl->GetWordView(regval - 2, pProc->IsHaltMode(), false, &addrtype)) >> 8);
            snprintf(hint2, hintsize - 1, "(%s-1)=%03o", REGISTER_NAME[regnum], srcval2);  // "(RN-1)=XXX"
        }
        else
        {
            srcval2 = pMemCtl->GetWordView(regval - 2, pProc->IsHaltMode(), false, &addrtype);
            snprintf(hint2, hintsize - 1, "(%s-2)=%06o", REGISTER_NAME[regnum], srcval2);  // "(RN-2)=XXXXXX"
        }
        break;
    case 5:
        srcval2 = pMemCtl->GetWordView(regval - 2, pProc->IsHaltMode(), false, &addrtype);
        snprintf(hint2, hintsize - 1, "(%s-2)=%06o", REGISTER_NAME[regnum], srcval2);  // "(RN+2)=XXXXXX"
        //TODO: Show the real value in hint line 3
        break;
    case 6:
        {
            uint16_t addr2 = regval + indexval;
            srcval2 = pMemCtl->GetWordView(addr2 & ~1, pProc->IsHaltMode(), false, &addrtype);
            if (byteword)
            {
                srcval2 = (addr2 & 1) ? (srcval2 >> 8) : (srcval2 & 0xff);
                snprintf(hint2, hintsize - 1, "(%s+%06o)=%03o", REGISTER_NAME[regnum], indexval, srcval2);  // "(RN+NNNNNN)=XXX"
            }
            else
            {
                snprintf(hint2, hintsize - 1, "(%s+%06o)=%06o", REGISTER_NAME[regnum], indexval, srcval2);  // "(RN+NNNNNN)=XXXXXX"
            }
            break;
        }
    case 7:
        srcval2 = pMemCtl->GetWordView(regval + indexval, pProc->IsHaltMode(), false, &addrtype);
        snprintf(hint2, hintsize - 1, "(%s+%06o)=%06o", REGISTER_NAME[regnum], indexval, srcval2);  // "(RN+NNNNNN)=XXXXXX"
        //TODO: Show the real value in hint line 3
        break;
    }
}

void Disasm_RegisterHintPC(const CProcessor * pProc, const CMemoryController * pMemCtl,
        char* hint1, char* /*hint2*/,
        int regmod, bool byteword, uint16_t curaddr, uint16_t value)
{
    const size_t hintsize = 20;
    int addrtype = 0;
    uint16_t srcval2 = 0;

    //TODO: else if (regmod == 2)
    if (regmod == 3)
    {
        srcval2 = pMemCtl->GetWordView(value, pProc->IsHaltMode(), false, &addrtype);
        if (byteword)
        {
            srcval2 = (value & 1) ? (srcval2 >> 8) : (srcval2 & 0xff);
            snprintf(hint1, hintsize - 1, "(%06o)=%03o", value, srcval2);  // "(NNNNNN)=XXX"
        }
        else
        {
            snprintf(hint1, hintsize - 1, "(%06o)=%06o", value, srcval2);  // "(NNNNNN)=XXXXXX"
        }
    }
    else if (regmod == 6)
    {
        uint16_t addr2 = curaddr + value;
        srcval2 = pMemCtl->GetWordView(addr2, pProc->IsHaltMode(), false, &addrtype);
        if (byteword)
        {
            srcval2 = (addr2 & 1) ? (srcval2 >> 8) : (srcval2 & 0xff);
            snprintf(hint1, hintsize - 1, "(%06o)=%03o", addr2, srcval2);  // "(NNNNNN)=XXX"
        }
        else
        {
            snprintf(hint1, hintsize - 1, "(%06o)=%06o", addr2, srcval2);  // "(NNNNNN)=XXXXXX"
        }
    }
    //TODO: else if (regmod == 7)
}

void Disasm_InstructionHint(const uint16_t* memory, const CProcessor * pProc, const CMemoryController * pMemCtl,
        char* buffer, char* buffer2,
        int srcreg, int srcmod, int dstreg, int dstmod)
{
    const size_t buffersize = 42;
    const size_t hintsize = 20;
    char srchint1[hintsize] = { 0 }, dsthint1[hintsize] = { 0 };
    char srchint2[hintsize] = { 0 }, dsthint2[hintsize] = { 0 };
    bool byteword = ((*memory) & 0100000) != 0;  // Byte mode (true) or Word mode (false)
    const uint16_t* curmemory = memory + 1;
    uint16_t curaddr = pProc->GetPC() + 2;
    uint16_t indexval = 0;

    if (srcreg >= 0)
    {
        if (srcreg == 7)
        {
            uint16_t value = *(curmemory++);  curaddr += 2;
            Disasm_RegisterHintPC(pProc, pMemCtl, srchint1, srchint2, srcmod, byteword, curaddr, value);
        }
        else
        {
            if (srcmod == 6 || srcmod == 7) { indexval = *(curmemory++);  curaddr += 2; }
            Disasm_RegisterHint(pProc, pMemCtl, srchint1, srchint2, srcreg, srcmod, byteword, indexval);
        }
    }
    if (dstreg >= 0)
    {
        if (dstreg == 7)
        {
            uint16_t value = *(curmemory++);  curaddr += 2;
            Disasm_RegisterHintPC(pProc, pMemCtl, dsthint1, dsthint2, dstmod, byteword, curaddr, value);
        }
        else
        {
            if (dstmod == 6 || dstmod == 7) { indexval = *(curmemory++);  curaddr += 2; }
            Disasm_RegisterHint(pProc, pMemCtl, dsthint1, dsthint2, dstreg, dstmod, byteword, indexval);
        }
    }

    // Prepare 1st line of the instruction hint
    if (*srchint1 != 0 && *dsthint1 != 0)
    {
        if (strcmp(srchint1, dsthint1) != 0)
            snprintf(buffer, buffersize - 1, "%s, %s", srchint1, dsthint1);
        else
        {
            strcpy(buffer, srchint1);
            *dsthint1 = 0;
        }
    }
    else if (*srchint1 != 0)
        strcpy(buffer, srchint1);
    else if (*dsthint1 != 0)
        strcpy(buffer, dsthint1);

    // Prepare 2nd line of the instruction hint
    if (*srchint2 != 0 && *dsthint2 != 0)
    {
        if (strcmp(srchint2, dsthint2) == 0)
            strcpy(buffer2, srchint2);
        else
            snprintf(buffer2, buffersize - 1, "%s, %s", srchint2, dsthint2);
    }
    else if (*srchint2 != 0)
        strcpy(buffer2, srchint2);
    else if (*dsthint2 != 0)
    {
        if (*srchint1 == 0 || *dsthint1 == 0)
            strcpy(buffer2, dsthint2);
        else
        {
            // Special case: we have srchint1, dsthint1 and dsthint2, but not srchint2 - let's align dsthint2 to dsthint1
            size_t hintpos = strlen(srchint1) + 2;
            for (size_t i = 0; i < hintpos; i++) buffer2[i] = ' ';
            strcpy(buffer2 + hintpos, dsthint2);
        }
    }
}

// Prepare "Instruction Hint" for a regular instruction (not a branch/jump one)
// buffer, buffer2 - buffers for 1st and 2nd lines of the instruction hint, min size 42
// Returns: number of hint lines; 0 = no hints
int Disasm_GetInstructionHint(const uint16_t* memory, const CProcessor * pProc,
        const CMemoryController * pMemCtl,
        char* buffer, char* buffer2)
{
    const size_t buffersize = 42;
    *buffer = 0;  *buffer2 = 0;
    uint16_t instr = *memory;

    // Source and Destination
    if ((instr & ~(uint16_t)0107777) == PI_MOV || (instr & ~(uint16_t)0107777) == PI_CMP ||
        (instr & ~(uint16_t)0107777) == PI_BIT || (instr & ~(uint16_t)0107777) == PI_BIC || (instr & ~(uint16_t)0107777) == PI_BIS ||
        (instr & ~(uint16_t)0007777) == PI_ADD || (instr & ~(uint16_t)0007777) == PI_SUB)
    {
        int srcreg = (instr >> 6) & 7;
        int srcmod = (instr >> 9) & 7;
        int dstreg = instr & 7;
        int dstmod = (instr >> 3) & 7;
        Disasm_InstructionHint(memory, pProc, pMemCtl, buffer, buffer2, srcreg, srcmod, dstreg, dstmod);
    }

    // Register and Destination
    if ((instr & ~(uint16_t)0777) == PI_MUL || (instr & ~(uint16_t)0777) == PI_DIV ||
        (instr & ~(uint16_t)0777) == PI_ASH || (instr & ~(uint16_t)0777) == PI_ASHC ||
        (instr & ~(uint16_t)0777) == PI_XOR)
    {
        int srcreg = (instr >> 6) & 7;
        int dstreg = instr & 7;
        int dstmod = (instr >> 3) & 7;
        Disasm_InstructionHint(memory, pProc, pMemCtl, buffer, buffer2, srcreg, 0, dstreg, dstmod);
    }

    // Destination only
    if ((instr & ~(uint16_t)0100077) == PI_CLR || (instr & ~(uint16_t)0100077) == PI_COM ||
        (instr & ~(uint16_t)0100077) == PI_INC || (instr & ~(uint16_t)0100077) == PI_DEC || (instr & ~(uint16_t)0100077) == PI_NEG ||
        (instr & ~(uint16_t)0100077) == PI_TST ||
        (instr & ~(uint16_t)0100077) == PI_ASR || (instr & ~(uint16_t)0100077) == PI_ASL ||
        (instr & ~(uint16_t)077) == PI_SWAB || (instr & ~(uint16_t)077) == PI_SXT ||
        (instr & ~(uint16_t)077) == PI_MTPS || (instr & ~(uint16_t)077) == PI_MFPS)
    {
        int dstreg = instr & 7;
        int dstmod = (instr >> 3) & 7;
        Disasm_InstructionHint(memory, pProc, pMemCtl, buffer, buffer2, -1, -1, dstreg, dstmod);
    }

    // ADC, SBC, ROR, ROL: destination only, and also show C flag
    if ((instr & ~(uint16_t)0100077) == PI_ADC || (instr & ~(uint16_t)0100077) == PI_SBC ||
        (instr & ~(uint16_t)0100077) == PI_ROR || (instr & ~(uint16_t)0100077) == PI_ROL)
    {
        int dstreg = instr & 7;
        int dstmod = (instr >> 3) & 7;
        if (dstreg != 7)
        {
            char tempbuf[buffersize];
            Disasm_InstructionHint(memory, pProc, pMemCtl, tempbuf, buffer2, -1, -1, dstreg, dstmod);
            uint16_t psw = pProc->GetPSW();
            snprintf(buffer, buffersize - 1, "%s, C=%c", tempbuf, (psw & PSW_C) ? '1' : '0');  // "..., C=X"
        }
    }

    // CLC..CCC, SEC..SCC -- show flags
    if (instr >= 0000241 && instr <= 0000257 || instr >= 0000261 && instr <= 0000277)
    {
        uint16_t psw = pProc->GetPSW();
        snprintf(buffer, buffersize - 1, "C=%c, V=%c, Z=%c, N=%c",
                (psw & PSW_C) ? '1' : '0', (psw & PSW_V) ? '1' : '0', (psw & PSW_Z) ? '1' : '0', (psw & PSW_N) ? '1' : '0');
    }

    // JSR, JMP -- show non-trivial cases only
    if ((instr & ~(uint16_t)0777) == PI_JSR && (instr & 077) != 067 && (instr & 077) != 037 ||
        (instr & ~(uint16_t)077) == PI_JMP && (instr & 077) != 067 && (instr & 077) != 037)
    {
        int dstreg = instr & 7;
        int dstmod = (instr >> 3) & 7;
        Disasm_InstructionHint(memory, pProc, pMemCtl, buffer, buffer2, -1, -1, dstreg, dstmod);
    }

    // HALT mode commands
    if (instr == PI_MFUS)
    {
        snprintf(buffer, buffersize - 1, "R5=%06o, R0=%06o", pProc->GetReg(5), pProc->GetReg(0));  // "R5=XXXXXX, R0=XXXXXX"
    }
    else if (instr == PI_MTUS)
    {
        snprintf(buffer, buffersize - 1, "R0=%06o, R5=%06o", pProc->GetReg(0), pProc->GetReg(5));  // "R0=XXXXXX, R5=XXXXXX"
    }
    else if (instr == 0000022 || instr == 0000023)  // RCPC / MFPC
    {
        snprintf(buffer, buffersize - 1, "PC'=%06o, R0=%06o", pProc->GetCPC(), pProc->GetReg(0));  // "PC'=XXXXXX, R0=XXXXXX"
    }
    else if (instr >= 0000024 && instr <= 0000027)  // RCPS / MFPS
    {
        snprintf(buffer, buffersize - 1, "PS'=%06o, R0=%06o", pProc->GetCPSW(), pProc->GetReg(0));  // "PS'=XXXXXX, R0=XXXXXX"
    }
    else if (instr == PI_RSEL)
    {
        snprintf(buffer, buffersize - 1, "SEL=%06o, R0=%06o", pMemCtl->GetSelRegister(), pProc->GetReg(0));  // "SEL=XXXXXX, R0=XXXXXX"
    }

    if ((instr & ~(uint16_t)077) == PI_MARK)
    {
        uint16_t regval = pProc->GetReg(6);
        snprintf(buffer, buffersize - 1, "SP=%06o, R5=%06o", regval, pProc->GetReg(5));  // "SP=XXXXXX, R5=XXXXXX"
        int addrtype = 0;
        uint16_t srcval2 = pMemCtl->GetWordView(regval, pProc->IsHaltMode(), false, &addrtype);
        snprintf(buffer2, buffersize - 1, "(SP)=%06o", srcval2);  // "(SP)=XXXXXX"
    }

    int result = 0;
    if (*buffer != 0)
        result = 1;
    if (*buffer2 != 0)
        result = 2;
    return result;
}
