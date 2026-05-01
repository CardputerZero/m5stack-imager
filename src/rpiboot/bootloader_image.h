/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2025 Raspberry Pi Ltd
 *
 * Read/modify the TLV-section format used by Raspberry Pi 4/5 EEPROM
 * bootloader images (pieeprom.bin).  Mirrors the BootloaderImage class
 * in usbboot/rpi-eeprom/rpi-eeprom-config.
 *
 * Section layout: each section starts with an 8-byte header (32-bit
 * magic, 32-bit length, both big-endian), followed by `length` bytes of
 * payload, padded to 8-byte alignment.  Modifiable named files use
 * FILE_MAGIC and store a 12-byte filename in the first 12 bytes of the
 * payload (followed by 4 bytes of metadata).  Modifiable files live in
 * 4 KiB-aligned sectors with a 4076-byte payload limit; the last 4 KiB
 * of the image is reserved scratch.
 */

#ifndef BOOTLOADER_IMAGE_H
#define BOOTLOADER_IMAGE_H

#include <QByteArray>
#include <QString>
#include <cstdint>
#include <vector>

namespace rpiboot {

class BootloaderImage {
public:
    static constexpr uint32_t MAGIC       = 0x55aaf00f;  // generic section
    static constexpr uint32_t PAD_MAGIC   = 0x55aafeef;  // padding section
    static constexpr uint32_t FILE_MAGIC  = 0x55aaf11f;  // modifiable file
    static constexpr uint32_t MAGIC_MASK  = 0xfffff00f;
    static constexpr int FILE_HDR_LEN     = 20;  // 4 magic + 4 length + 12 filename
    static constexpr int FILENAME_LEN     = 12;
    static constexpr int ERASE_ALIGN_SIZE = 4096;
    static constexpr int MAX_FILE_SIZE    = ERASE_ALIGN_SIZE - FILE_HDR_LEN;

    BootloaderImage();
    ~BootloaderImage();

    // Load an EEPROM image from disk.  Returns false on parse / size error.
    bool load(const QString &filename);

    // Write the (possibly modified) image to a file.  Returns false on I/O error.
    bool save(const QString &filename) const;

    // Return the current in-memory bytes (e.g. for hashing).
    const QByteArray &bytes() const { return _bytes; }

    // Read a file's payload by name (e.g. "bootconf.txt", "pubkey.bin",
    // "bootcode.bin").  Returns empty QByteArray if not found.
    QByteArray getFile(const QString &name) const;

    // Replace a modifiable file's payload.  Returns false if the file
    // isn't in the image, or the replacement is too large for the
    // section's available space.  Internally pads with PAD_MAGIC sections
    // and 0xFF bytes to preserve EEPROM erase-block layout.
    bool updateFile(const QString &name, const QByteArray &payload);

    // Replace the bootcode (first section).  The bootcode section uses
    // MAGIC (not FILE_MAGIC) and has no filename header — it's the
    // 128 KiB block at the start of the image.  Returns false if the
    // signed bootcode is larger than the reserved 128 KiB region.
    bool updateBootcode(const QByteArray &payload);

    QString lastError() const { return _lastError; }

private:
    struct Section {
        uint32_t magic;
        int      offset;
        int      length;
        QString  filename;
    };

    bool parse();

    // Locate the section for a named file (or, when name == "bootcode.bin",
    // the first section regardless of magic).  Outputs the header offset,
    // payload length, whether this is the last section, and the offset of
    // the next non-padding section (or end-of-image-minus-scratch).
    bool findFile(const QString &name,
                  int &outHeaderOffset, int &outLength,
                  bool &outIsLast, int &outNextOffset) const;

    QByteArray              _bytes;
    std::vector<Section>    _sections;
    QString                 _lastError;
};

} // namespace rpiboot

#endif // BOOTLOADER_IMAGE_H
