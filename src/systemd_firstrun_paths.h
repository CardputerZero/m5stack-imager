/*
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2026 Raspberry Pi Ltd
 */

#ifndef SYSTEMD_FIRSTRUN_PATHS_H
#define SYSTEMD_FIRSTRUN_PATHS_H

#include <QByteArray>
#include <QDate>
#include <QString>

namespace rpi_imager {

struct SystemdFirstrunPaths {
    QString firstrunPath;
    QString cmdlinePath;
};

inline bool usesFirmwareBootMount(const QString& releaseDateString)
{
    const QDate releaseDate = QDate::fromString(releaseDateString, QStringLiteral("yyyy-MM-dd"));
    return releaseDate.isValid() && releaseDate >= QDate(2023, 10, 11);
}

inline SystemdFirstrunPaths systemdFirstrunPathsForReleaseDate(const QString& releaseDateString)
{
    if (usesFirmwareBootMount(releaseDateString)) {
        return { QStringLiteral("/boot/firmware/firstrun.sh"),
                 QStringLiteral("/boot/firmware/cmdline.txt") };
    }

    return { QStringLiteral("/boot/firstrun.sh"),
             QStringLiteral("/boot/cmdline.txt") };
}

inline QByteArray systemdFirstrunCmdlineAppend(const QString& firstrunPath)
{
    return QByteArray(" systemd.run=") + firstrunPath.toUtf8()
           + QByteArray(" systemd.run_success_action=reboot systemd.unit=kernel-command-line.target");
}

} // namespace rpi_imager

#endif // SYSTEMD_FIRSTRUN_PATHS_H
