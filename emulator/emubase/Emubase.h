/*  This file is part of UKNCBTL.
    UKNCBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    UKNCBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
UKNCBTL. If not, see <http://www.gnu.org/licenses/>. */

/// \file Emubase.h  Header for use of all emubase classes

#pragma once

#include "Board.h"
#include "Processor.h"
#include "Memory.h"

#include <cstdio>

//////////////////////////////////////////////////////////////////////
// Disassembler

/// \brief Disassemble one instruction of KM1801VM2 processor
/// \param[in]  pMemory Memory image (we read only words of the instruction)
/// \param[in]  addr    Address of the instruction
/// \param[out] sInstr  Instruction mnemonics buffer - at least 8 characters
/// \param[out] sArg    Instruction arguments buffer - at least 32 characters
/// \return  Number of words in the instruction
uint16_t DisassembleInstruction(const uint16_t* pMemory, uint16_t addr, char* sInstr, char* sArg);

bool Disasm_CheckForJump(const uint16_t* memory, int* pDelta);

// Prepare "Jump Hint" string, and also calculate condition for conditional jump
// Returns: jump prediction flag: true = will jump, false = will not jump
bool Disasm_GetJumpConditionHint(
    const uint16_t* memory, const CProcessor * pProc, const CMemoryController * pMemCtl, char* buffer);

// Prepare "Instruction Hint" for a regular instruction (not a branch/jump one)
// buffer, buffer2 - buffers for 1st and 2nd lines of the instruction hint, min size 42
// Returns: number of hint lines; 0 = no hints
int Disasm_GetInstructionHint(
    const uint16_t* memory, const CProcessor * pProc,
    const CMemoryController * pMemCtl,
    char* buffer, char* buffer2);

//////////////////////////////////////////////////////////////////////
// CHardDrive

#define IDE_DISK_SECTOR_SIZE      512

/// \brief UKNC IDE hard drive
class CHardDrive
{
protected:
    FILE*   m_fpFile;           ///< File pointer for the attached HDD image
    bool    m_okReadOnly;       ///< Flag indicating that the HDD image file is read-only
    bool    m_okInverted;       ///< Flag indicating that the HDD image has inverted bits
    uint8_t m_status;           ///< IDE status register, see IDE_STATUS_XXX constants
    uint8_t m_error;            ///< IDE error register, see IDE_ERROR_XXX constants
    uint8_t m_command;          ///< Current IDE command, see IDE_COMMAND_XXX constants
    int     m_numcylinders;     ///< Cylinder count
    int     m_numheads;         ///< Head count
    int     m_numsectors;       ///< Sectors per track
    int     m_curcylinder;      ///< Current cylinder number
    int     m_curhead;          ///< Current head number
    int     m_cursector;        ///< Current sector number
    int     m_curheadreg;       ///< Current head number
    int     m_sectorcount;      ///< Sector counter for read/write operations
    uint8_t m_buffer[IDE_DISK_SECTOR_SIZE];  ///< Sector data buffer
    int     m_bufferoffset;     ///< Current offset within sector: 0..511
    int     m_timeoutcount;     ///< Timeout counter to wait for the next event
    int     m_timeoutevent;     ///< Current stage of operation, see TimeoutEvent enum

public:
    CHardDrive();
    ~CHardDrive();
    /// \brief Reset the device.
    void Reset();
    /// \brief Attach HDD image file to the device
    bool AttachImage(const char* sFileName);
    /// \brief Detach HDD image file from the device
    void DetachImage();
    /// \brief Check if the attached hard drive image is read-only
    bool IsReadOnly() const { return m_okReadOnly; }

public:
    /// \brief Read word from the device port
    uint16_t ReadPort(uint16_t port);
    /// \brief Write word th the device port
    void WritePort(uint16_t port, uint16_t data);
    /// \brief Rotate disk
    void Periodic();

private:
    uint32_t CalculateOffset() const;  ///< Calculate sector offset in the HDD image
    void HandleCommand(uint8_t command);  ///< Handle the IDE command
    void ReadNextSector();
    void ReadSectorDone();
    void WriteSectorDone();
    void NextSector();          ///< Advance to the next sector, CHS-based
    void ContinueRead();
    void ContinueWrite();
    void IdentifyDrive();       ///< Prepare m_buffer for the IDENTIFY DRIVE command
};


//////////////////////////////////////////////////////////////////////

class CSoundAY
{
protected:
    int index;
    int ready;
    unsigned Regs[16];
    int32_t lastEnable;
    int32_t PeriodA, PeriodB, PeriodC, PeriodN, PeriodE;
    int32_t CountA, CountB, CountC, CountN, CountE;
    uint32_t VolA, VolB, VolC, VolE;
    uint8_t EnvelopeA, EnvelopeB, EnvelopeC;
    uint8_t OutputA, OutputB, OutputC, OutputN;
    int8_t CountEnv;
    uint8_t Hold, Alternate, Attack, Holding;
    int32_t RNG;
protected:
    static unsigned int VolTable[32];

public:
    CSoundAY();
    void Reset();

public:
    void SetReg(int r, int v);
    void Callback(uint8_t* stream, int length);

protected:
    static void BuildMixerTable();
};


//////////////////////////////////////////////////////////////////////
