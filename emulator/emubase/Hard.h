/*  This file is part of UKNCBTL.
 UKNCBTL is free software: you can redistribute it and/or modify it under the terms
 of the GNU Lesser General Public License as published by the Free Software Foundation,
 either version 3 of the License, or (at your option) any later version.
 UKNCBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU Lesser General Public License for more details.
 You should have received a copy of the GNU Lesser General Public License along with
 UKNCBTL. If not, see <http://www.gnu.org/licenses/>. */

/// \file Hard.h  Header for use of floppy classes

#include <cstdint>
#include <cstdio>

#define IDE_DISK_SECTOR_SIZE      512

/// \brief UKNC IDE hard drive
class CHardDrive
{
public:
    virtual ~CHardDrive() {}
    /// \brief Reset the device.
    virtual void Reset() = 0;
    /// \brief Attach HDD image file to the device
    virtual bool AttachImage(const char* sFileName) = 0;
    /// \brief Detach HDD image file from the device
    virtual void DetachImage() = 0;
    /// \brief Check if the attached hard drive image is read-only
    virtual bool IsReadOnly() const = 0;
    
public:
    /// \brief Read word from the device port
    virtual uint16_t ReadPort(uint16_t port) = 0;
    /// \brief Write word th the device port
    virtual void WritePort(uint16_t port, uint16_t data) = 0;
    /// \brief Rotate disk
    virtual void Periodic() = 0;
};

//////////////////////////////////////////////////////////////////////

extern CHardDrive *ProduceHardDriveImage();
