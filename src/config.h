#ifndef CONFIG_H
#define CONFIG_H

/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2020-2025 Raspberry Pi Ltd
 */


/* Repository URL */
#ifndef M5STACK_IMAGER_DEFAULT_REPO_URL
#define M5STACK_IMAGER_DEFAULT_REPO_URL         "qrc:/m5stack/os-list.json"
#endif
#define OSLIST_URL                              M5STACK_IMAGER_DEFAULT_REPO_URL

/* Custom repository manifest file extension (without leading dot) */
#define MANIFEST_EXTENSION                      "rpi-imager-manifest"

/* MIME type for manifest files */
#define MANIFEST_MIME_TYPE                      "application/vnd.raspberrypi.imager-manifest+json"

/* Time synchronization URL (only used on linuxfb QPA platform, URL must be HTTP) */
#define TIME_URL                                "http://downloads.raspberrypi.com/"

/* Phone home the name of images downloaded for image popularity ranking.
 * Disabled by default for the M5Stack fork until an M5Stack endpoint exists.
 */
#define TELEMETRY_URL                           ""

/* Hash algorithm for verifying (uncompressed image) checksum */
#define OSLIST_HASH_ALGORITHM                   QCryptographicHash::Sha256

/* Update progressbar every 0.1 second */
#define PROGRESS_UPDATE_INTERVAL                100

/* Default block size for buffer allocation (dynamically adjusted at runtime) */
#define IMAGEWRITER_BLOCKSIZE                   1*1024*1024

/* Enable caching */
#define IMAGEWRITER_ENABLE_CACHE_DEFAULT        true

/* Do not cache if it would bring free disk space under 5 GB */
#define IMAGEWRITER_MINIMAL_SPACE_FOR_CACHING   5*1024*1024*1024ll

#endif // CONFIG_H
