/*  This file is part of UKNCBTL.
    UKNCBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    UKNCBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
UKNCBTL. If not, see <http://www.gnu.org/licenses/>. */

/// \file Floppy.cpp
/// \brief Floppy controller and drives emulation
/// \details See defines in header file Emubase.h

#include "Floppy.h"

#include "Debug.h"

#include <sys/stat.h>
#include <cstring>
#include <cassert>

//////////////////////////////////////////////////////////////////////

// Mask of all flags stored in m_flags
const uint16_t FLOPPY_CMD_MASKSTORED =
    FLOPPY_CMD_CORRECTION250 | FLOPPY_CMD_CORRECTION500 | FLOPPY_CMD_SIDEUP | FLOPPY_CMD_DIR | FLOPPY_CMD_SKIPSYNC |
    FLOPPY_CMD_ENGINESTART;

static void EncodeTrackData(const uint8_t* pSrc, uint8_t* data, uint8_t* marker, uint16_t track, uint16_t side);
static bool DecodeTrackData(const uint8_t* pRaw, uint8_t* pDest);

//////////////////////////////////////////////////////////////////////

/// \brief Floppy drive (one of four drives in the floppy controller)
/// \sa CFloppyController
class CFloppyDriveImage: public CFloppyDrive
{
protected:
    FILE* fpFile;       ///< File pointer of the disk image file
    bool okNetRT11Image;  ///< true - .rtd image, false - .dsk image
    bool okReadOnly;    ///< Write protection flag
    uint16_t dataptr;       ///< Data offset within m_data - "head" position
    uint16_t datatrack;     ///< Track number of data in m_data array
    uint16_t dataside;      ///< Disk side of data in m_data array
    uint8_t data[FLOPPY_RAWTRACKSIZE];  ///< Raw track image for the current track
    uint8_t marker[FLOPPY_RAWMARKERSIZE];  ///< Marker positions
    
public:
    CFloppyDriveImage();
    
    virtual void Reset();       ///< Reset the device
    virtual void Step();
    virtual uint16_t Read();
    virtual void Write(uint16_t word);
    virtual void SetMarker();
    virtual void RemoveMarker();
    virtual bool ReadTrack(uint16_t track, uint16_t side);
    virtual bool WriteTrack();
    
    /// \brief Attach the image to the drive -- insert disk
    virtual bool AttachImage(const char *sFileName);
    /// \brief Detach image from the drive -- remove disk
    virtual void DetachImage();
    /// \brief Check if the drive has an image attached
    virtual bool IsAttached() const { return fpFile != nullptr; }
    /// \brief Check if the drive's attached image is read-only
    virtual bool IsReadOnly() const { return okReadOnly; }
    
    virtual bool IsInIndex() const { return dataptr < FLOPPY_INDEXLENGTH; }
    
    virtual bool HasMarker() const { return marker[dataptr / 2] != 0; }
    
    virtual uint16_t GetTrack() const { return datatrack; }
    
    virtual uint16_t GetSide() const { return dataside; }
};

CFloppyDriveImage::CFloppyDriveImage()
{
    fpFile = nullptr;
    okNetRT11Image = false;
    okReadOnly = false;
    datatrack = 0;
    dataside = 0;
    dataptr = 0;
    memset(data, 0, FLOPPY_RAWTRACKSIZE);
    memset(marker, 0, FLOPPY_RAWMARKERSIZE);
}

void CFloppyDriveImage::Reset()
{
    datatrack = 0;
    dataside = 0;
    dataptr = 0;
}

bool CFloppyDriveImage::AttachImage(const char *sFileName)
{
    // If image attached - detach one first
    if (IsAttached())
        DetachImage();

    // Detect if this is a .dsk image or .rtd image, using the file extension
    okNetRT11Image = false;
    const char *sFileNameExt = strchr(sFileName, '.');
    if (sFileNameExt != nullptr && strcasecmp(sFileNameExt, ".rtd") == 0)
        okNetRT11Image = true;

    // Open file
    okReadOnly = false;
    fpFile = ::fopen(sFileName, "r+b");
    if (fpFile == nullptr)
    {
        okReadOnly = true;
        fpFile = ::fopen(sFileName, "rb");
    }
    if (fpFile == nullptr)
        return false;

    datatrack = 0;
    dataside = 0;
    dataptr = 0;

    return true;
}

void CFloppyDriveImage::DetachImage()
{
    if (fpFile != nullptr)
    {
        ::fclose(fpFile);
        fpFile = nullptr;
    }
    okNetRT11Image = false;
    okReadOnly = false;
    Reset();
}

void CFloppyDriveImage::Step()
{
    dataptr += 2;
    if (dataptr >= FLOPPY_RAWTRACKSIZE)
        dataptr = 0;
}

uint16_t CFloppyDriveImage::Read()
{
    return (data[dataptr] << 8) | data[dataptr + 1];
}

void CFloppyDriveImage::Write(uint16_t word)
{
    data[dataptr] = (uint8_t)(word & 0xff);
    data[dataptr + 1] = (uint8_t)((word >> 8) & 0xff);
}

void CFloppyDriveImage::SetMarker()
{
    marker[dataptr / 2] = 1;
}

void CFloppyDriveImage::RemoveMarker()
{
    marker[dataptr / 2] = 0;
}

bool CFloppyDriveImage::ReadTrack(uint16_t track, uint16_t side)
{
    datatrack = track;
    dataside = side;

    if (fpFile == nullptr)
        return false;

    // Track has 10 sectors, 512 bytes each; offset of the file is === ((Track<<1)+SIDE)*5120
    long foffset = ((datatrack * 2) + (dataside)) * 5120;
    if (okNetRT11Image) foffset += 256;  // Skip .RTD image header
    //DebugPrintFormat(_T("floppy file offset %d  for trk %d side %d\r\n"), foffset, m_track, m_side);

    uint8_t data[5120] = {0};

    ::fseek(fpFile, foffset, SEEK_SET);
    if (::fread(data, 1, 5120, fpFile) != 5120)
        return false;

    // Fill m_data array and m_marker array with marked data
    EncodeTrackData(data, this->data, marker, datatrack, dataside);

    ////DEBUG: Test DecodeTrackData()
    //uint8_t data2[5120];
    //bool parsed = DecodeTrackData(data, data2);
    //ASSERT(parsed);
    //bool tested = true;
    //for (int i = 0; i < 5120; i++)
    //    if (data[i] != data2[i])
    //    {
    //        tested = false;
    //        break;
    //    }
    //ASSERT(tested);

    return true;
}

bool CFloppyDriveImage::WriteTrack()
{
    uint8_t data[5120] = {0};

    if (!DecodeTrackData(this->data, data))  // Write to the file only if the track was correctly decoded from raw data
    {
        return false;
    }

    // Track has 10 sectors, 512 bytes each; offset of the file is === ((Track<<1)+SIDE)*5120
    long foffset = ((datatrack * 2) + (dataside)) * 5120;
    if (okNetRT11Image) foffset += 256;  // Skip .RTD image header

    // Check file length
    ::fseek(fpFile, 0, SEEK_END);
    uint32_t currentFileSize = (uint32_t)::ftell(fpFile);
    while (currentFileSize < (uint32_t)(foffset + 5120))
    {
        uint8_t datafill[512];  ::memset(datafill, 0, 512);
        uint32_t bytesToWrite = ((uint32_t)(foffset + 5120) - currentFileSize) % 512;
        if (bytesToWrite == 0) bytesToWrite = 512;
        ::fwrite(datafill, 1, bytesToWrite, fpFile);
        //TODO: Check for writing error
        currentFileSize += bytesToWrite;
    }

    // Save data into the file
    ::fseek(fpFile, foffset, SEEK_SET);
    return 5120 == ::fwrite(data, 1, 5120, fpFile);
    //TODO: Check for writing error
}

//////////////////////////////////////////////////////////////////////


CFloppyController::CFloppyController()
{
    for (int i = 0; i < sizeof(m_drivedata) / sizeof(m_drivedata[0]); i++)
    {
        m_drivedata[i] = new CFloppyDriveImage();
    }
    m_drive = m_side = m_track = 0;
    m_pDrive = m_drivedata[0];
    m_datareg = m_writereg = m_shiftreg = 0;
    m_writing = m_searchsync = m_writemarker = m_crccalculus = false;
    m_writeflag = m_shiftflag = m_shiftmarker = false;
    m_trackchanged = false;
    m_status = FLOPPY_STATUS_TRACK0 | FLOPPY_STATUS_WRITEPROTECT;
    m_flags = FLOPPY_CMD_CORRECTION500 | FLOPPY_CMD_SIDEUP | FLOPPY_CMD_DIR | FLOPPY_CMD_SKIPSYNC;
    m_okTrace = 0;
}

CFloppyController::~CFloppyController()
{
    for (int drive = 0; drive < 4; drive++)
        DetachImage(drive);
}

void CFloppyController::Reset()
{
    if (m_okTrace) DebugLog("Floppy RESET\r\n");

    FlushChanges();

    m_drive = m_side = m_track = 0;
    m_pDrive = m_drivedata[0];
    m_datareg = m_writereg = m_shiftreg = 0;
    m_writing = m_searchsync = m_writemarker = m_crccalculus = false;
    m_writeflag = m_shiftflag = false;
    m_trackchanged = false;
    m_status = (m_pDrive->IsReadOnly()) ? FLOPPY_STATUS_TRACK0 | FLOPPY_STATUS_WRITEPROTECT : FLOPPY_STATUS_TRACK0;
    m_flags = FLOPPY_CMD_CORRECTION500 | FLOPPY_CMD_SIDEUP | FLOPPY_CMD_DIR | FLOPPY_CMD_SKIPSYNC;

    PrepareTrack();
}

bool CFloppyController::AttachImage(unsigned int drive, const char *sFileName)
{
    assert(sFileName != nullptr);

    // If image attached - detach one first
    if (!m_drivedata[drive]->AttachImage(sFileName))
        return false;

    m_side = m_track = 0;
    m_datareg = m_writereg = m_shiftreg = 0;
    m_writing = m_searchsync = m_writemarker = m_crccalculus = false;
    m_writeflag = m_shiftflag = false;
    m_trackchanged = false;
    m_status = m_pDrive->IsReadOnly() ? FLOPPY_STATUS_TRACK0 | FLOPPY_STATUS_WRITEPROTECT : FLOPPY_STATUS_TRACK0;
    m_flags = FLOPPY_CMD_CORRECTION500 | FLOPPY_CMD_SIDEUP | FLOPPY_CMD_DIR | FLOPPY_CMD_SKIPSYNC;

    PrepareTrack();

    return true;
}

void CFloppyController::DetachImage(unsigned int drive)
{
    if (!m_drivedata[drive]->IsAttached()) return;

    FlushChanges();

    m_drivedata[drive]->DetachImage();
}

//////////////////////////////////////////////////////////////////////


uint16_t CFloppyController::GetState(void)
{
    if (m_track == 0)
        m_status |= FLOPPY_STATUS_TRACK0;
    else
        m_status &= ~FLOPPY_STATUS_TRACK0;
    if (m_pDrive->IsInIndex())
        m_status |= FLOPPY_STATUS_INDEXMARK;
    else
        m_status &= ~FLOPPY_STATUS_INDEXMARK;

    uint16_t res = m_status;

    if (!m_drivedata[m_drive]->IsAttached())
        res |= FLOPPY_STATUS_MOREDATA;

//    if (res & FLOPPY_STATUS_MOREDATA)
//    {
//        TCHAR oct2[7];  PrintOctalValue(oct2, res);
//        DebugLogFormat(_T("Floppy GET STATE %s\r\n"), oct2);
//    }

    return res;
}

void CFloppyController::SetCommand(uint16_t cmd)
{
    if (m_okTrace) DebugLog("Floppy COMMAND %06o\r\n", cmd);

    bool okPrepareTrack = false;  // Is it needed to load the track into the buffer

    // Check if the current drive was changed or not
    uint16_t newdrive = (cmd & 3) ^ 3;
    if ((cmd & 02000) != 0 && m_drive != newdrive)
    {
        FlushChanges();

        DebugLog("Floppy DRIVE %hu\r\n", newdrive);

        m_drive = newdrive;
        m_pDrive = m_drivedata[m_drive];
        okPrepareTrack = true;
    }
    cmd &= ~3;  // Remove the info about the current drive

    // Copy new flags to m_flags
    m_flags &= ~FLOPPY_CMD_MASKSTORED;
    m_flags |= cmd & FLOPPY_CMD_MASKSTORED;

    // Check if the side was changed
    if (m_flags & FLOPPY_CMD_SIDEUP)  // Side selection: 0 - down, 1 - up
    {
        if (m_side == 0) { m_side = 1;  okPrepareTrack = true; }
    }
    else
    {
        if (m_side == 1) { m_side = 0;  okPrepareTrack = true; }
    }

    if (cmd & FLOPPY_CMD_STEP)  // Move head for one track to center or from center
    {
        if (m_okTrace) DebugLog("Floppy STEP\r\n");

        m_side = (m_flags & FLOPPY_CMD_SIDEUP) ? 1 : 0; // DO WE NEED IT HERE?

        if (m_flags & FLOPPY_CMD_DIR)
        {
            if (m_track < 79) { m_track++;  okPrepareTrack = true; }
        }
        else
        {
            if (m_track >= 1) { m_track--;  okPrepareTrack = true; }
        }
    }
    if (okPrepareTrack)
    {
        PrepareTrack();

//    	DebugLogFormat(_T("Floppy DRIVE %hu TR %hu SD %hu\r\n"), m_drive, m_track, m_side);
    }

    if (cmd & FLOPPY_CMD_SEARCHSYNC) // Search for marker
    {
//        DebugLog(_T("Floppy SEARCHSYNC\r\n"));

        m_flags &= ~FLOPPY_CMD_SEARCHSYNC;
        m_searchsync = true;
        m_crccalculus = true;
        m_status &= ~FLOPPY_STATUS_CHECKSUMOK;
    }

    if (m_writing && (cmd & FLOPPY_CMD_SKIPSYNC))  // Writing marker
    {
//        DebugLog(_T("Floppy MARKER\r\n"));

        m_writemarker = true;
        m_status &= ~FLOPPY_STATUS_CHECKSUMOK;
    }
}

uint16_t CFloppyController::GetData(void)
{
    if (m_okTrace) DebugLog("Floppy READ\t\t%04x\r\n", m_datareg);

    m_status &= ~FLOPPY_STATUS_MOREDATA;
    m_writing = m_searchsync = false;
    m_writeflag = m_shiftflag = false;

    return m_datareg;
}

void CFloppyController::WriteData(uint16_t data)
{
//        DebugLogFormat(_T("Floppy WRITE\t\t%04x\r\n"), data);

    m_writing = true;  // Switch to write mode if not yet
    m_searchsync = false;

    if (!m_writeflag && !m_shiftflag)  // Both registers are empty
    {
        m_shiftreg = data;
        m_shiftflag = true;
        m_status |= FLOPPY_STATUS_MOREDATA;
    }
    else if (!m_writeflag && m_shiftflag)  // Write register is empty
    {
        m_writereg = data;
        m_writeflag = true;
        m_status &= ~FLOPPY_STATUS_MOREDATA;
    }
    else if (m_writeflag && !m_shiftflag)  // Shift register is empty
    {
        m_shiftreg = m_writereg;
        m_shiftflag = true;
        m_writereg = data;
        m_status &= ~FLOPPY_STATUS_MOREDATA;
    }
    else  // Both registers are not empty
    {
        m_writereg = data;
    }
}

void CFloppyController::Periodic()
{
    //if (!IsEngineOn()) return;  // Rotate diskettes only if the motor is on

    // Rotating all the disks at once
    for (int drive = 0; drive < 4; drive++)
        m_drivedata[drive]->Step();

    // Then process reading/writing on the current drive
    if (!IsAttached(m_drive)) return;

    if (!m_writing)  // Read mode
    {
        m_datareg = m_pDrive->Read();
        if (m_status & FLOPPY_STATUS_MOREDATA)
        {
            if (m_crccalculus)  // Stop CRC calculation
            {
                m_crccalculus = false;
                //TODO: Compare calculated CRC to m_datareg
                m_status |= FLOPPY_STATUS_CHECKSUMOK;
            }
        }
        else
        {
            if (m_searchsync)  // Search for marker
            {
                if (m_pDrive->HasMarker())  // Marker found
                {
                    m_status |= FLOPPY_STATUS_MOREDATA;
                    m_searchsync = false;
                }
            }
            else  // Just read
                m_status |= FLOPPY_STATUS_MOREDATA;
        }
    }
    else  // Write mode
    {
        if (m_shiftflag)
        {
            m_pDrive->Write(m_shiftreg);
            m_shiftflag = false;
            m_trackchanged = true;

            if (m_shiftmarker)
            {
//            DebugLogFormat(_T("Floppy WRITING %04x MARKER at %04hx SC %hu\r\n"), m_shiftreg, m_pDrive->dataptr, (m_pDrive->dataptr - 0x5e) / 614 + 1);

                m_pDrive->SetMarker();
                m_shiftmarker = false;
                m_crccalculus = true;  // Start CRC calculation
            }
            else
            {
//            DebugLogFormat(_T("Floppy WRITING %04x\r\n"), m_shiftreg);

                m_pDrive->RemoveMarker();
            }

            if (m_writeflag)
            {
                m_shiftreg = m_writereg;
                m_shiftflag = true;
                m_writeflag = false;
                m_shiftmarker = m_writemarker;
                m_writemarker = false;
                m_status |= FLOPPY_STATUS_MOREDATA;
            }
            else
            {
                if (m_crccalculus)  // Stop CRC calclation
                {
                    m_shiftreg = 0x4444;  //STUB
                    m_shiftflag = false; //Should be true, but temporarily disabled
                    m_shiftmarker = false;
                    m_crccalculus = false;
                    m_status |= FLOPPY_STATUS_CHECKSUMOK;
                }
            }
        }
    }
}

// Read track data from file and fill m_data
void CFloppyController::PrepareTrack()
{
    FlushChanges();

    if (m_okTrace) DebugLog("Floppy PREP  %hu TR %hu SD %hu\r\n", m_drive, m_track, m_side);

    //TCHAR buffer[512];

    m_trackchanged = false;
    m_status |= FLOPPY_STATUS_MOREDATA;
    //NOTE: Not changing m_pDrive->dataptr
    if (!m_pDrive->ReadTrack(m_track, m_side))
    {
        //TODO: Check for reading error
    }
}

void CFloppyController::FlushChanges()
{
    if (!IsAttached(m_drive)) return;
    if (!m_trackchanged) return;

    if (m_okTrace) DebugLog("Floppy FLUSH %hu TR %hu SD %hu\r\n", m_drive, m_pDrive->GetTrack(), m_pDrive->GetSide());

    if (!m_pDrive->WriteTrack())  // Write to the file only if the track was correctly decoded from raw data
    {
        if (m_okTrace) DebugLog("Floppy FLUSH FAILED\r\n");
    }

    m_trackchanged = false;

    ////DEBUG: Save raw m_data/m_marker into rawdata.bin
    //HANDLE hRawFile = CreateFile(_T("rawdata.bin"),
    //            GENERIC_WRITE, FILE_SHARE_READ, nullptr,
    //            CREATE_ALWAYS, 0, nullptr);
}


//////////////////////////////////////////////////////////////////////


// Fill data array and marker array with marked data
static void EncodeTrackData(const uint8_t* pSrc, uint8_t* data, uint8_t* marker, uint16_t track, uint16_t side)
{
    memset(data, 0, FLOPPY_RAWTRACKSIZE);
    memset(marker, 0, FLOPPY_RAWMARKERSIZE);
    uint32_t count;
    int ptr = 0;
    int gap = 34;
    for (int sect = 0; sect < 10; sect++)
    {
        // GAP
        for (count = 0; count < (uint32_t) gap; count++) data[ptr++] = 0x4e;
        // sector header
        for (count = 0; count < 12; count++) data[ptr++] = 0x00;
        // marker
        marker[ptr / 2] = 1;  // ID marker; start CRC calculus
        data[ptr++] = 0xa1;  data[ptr++] = 0xa1;  data[ptr++] = 0xa1;
        data[ptr++] = 0xfe;

        data[ptr++] = (uint8_t) track;  data[ptr++] = (side != 0);
        data[ptr++] = (uint8_t)sect + 1;  data[ptr++] = 2; // Assume 512 bytes per sector;
        // crc
        //TODO: Calculate CRC
        data[ptr++] = 0x12;  data[ptr++] = 0x34;  // CRC stub

        // sync
        for (count = 0; count < 24; count++) data[ptr++] = 0x4e;
        // data header
        for (count = 0; count < 12; count++) data[ptr++] = 0x00;
        // marker
        marker[ptr / 2] = 1;  // Data marker; start CRC calculus
        data[ptr++] = 0xa1;  data[ptr++] = 0xa1;  data[ptr++] = 0xa1;
        data[ptr++] = 0xfb;
        // data
        for (count = 0; count < 512; count++)
            data[ptr++] = pSrc[(sect * 512) + count];
        // crc
        //TODO: Calculate CRC
        data[ptr++] = 0x43;  data[ptr++] = 0x21;  // CRC stub

        gap = 38;
    }
    // fill GAP4B to the end of the track
    while (ptr < FLOPPY_RAWTRACKSIZE) data[ptr++] = 0x4e;
}

// Decode track data from raw data
// pRaw is array of FLOPPY_RAWTRACKSIZE bytes
// pDest is array of 5120 bytes
// Returns: true - decoded, false - parse error
static bool DecodeTrackData(const uint8_t* pRaw, uint8_t* pDest)
{
    uint16_t dataptr = 0;  // Offset in m_data array
    uint16_t destptr = 0;  // Offset in data array
    for (;;)
    {
        while (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0x4e) dataptr++;  // Skip GAP1 or GAP3
        if (dataptr >= FLOPPY_RAWTRACKSIZE) break;  // End of track or error
        while (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0x00) dataptr++;  // Skip sync
        if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong

        if (pRaw[dataptr] == 0xa1) dataptr++;
        if (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0xa1) dataptr++;
        if (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0xa1) dataptr++;
        if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong
        if (pRaw[dataptr++] != 0xfe)
            return false;  // Marker not found

        uint8_t sectno = 0;
        if (dataptr < FLOPPY_RAWTRACKSIZE) sectno  = pRaw[dataptr++];
        if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong

        int sectorsize;
        if (sectno == 1) sectorsize = 256;
        else if (sectno == 2) sectorsize = 512;
        else if (sectno == 3) sectorsize = 1024;
        else return false;  // Something wrong: unknown sector size
        // crc
        dataptr++;
        if (dataptr < FLOPPY_RAWTRACKSIZE) dataptr++;

        while (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0x4e) dataptr++;  // Skip GAP2
        if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong
        while (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0x00) dataptr++;  // Skip sync
        if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong

        if (pRaw[dataptr] == 0xa1) dataptr++;
        if (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0xa1) dataptr++;
        if (dataptr < FLOPPY_RAWTRACKSIZE && pRaw[dataptr] == 0xa1) dataptr++;
        if (dataptr >= FLOPPY_RAWTRACKSIZE) return false;  // Something wrong
        if (pRaw[dataptr++] != 0xfb)
            return false;  // Marker not found

        for (int count = 0; count < sectorsize; count++)  // Copy sector data
        {
            if (destptr >= 5120) break;  // End of track or error
            pDest[destptr++] = pRaw[dataptr++];
            if (dataptr >= FLOPPY_RAWTRACKSIZE)
                return false;  // Something wrong
        }
        if (dataptr >= FLOPPY_RAWTRACKSIZE)
            return false;  // Something wrong
        // crc
        dataptr++;
        if (dataptr < FLOPPY_RAWTRACKSIZE) dataptr++;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////

