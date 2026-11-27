/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * X11 host access for the elevated instance: the in-process equivalent of
 * `xhost +SI:localuser:<user>`.
 *
 * Deliberately free of Qt, X11 and xcb.  The part that talks to the X server
 * runs in a forked child of a multi-threaded process, where only
 * async-signal-safe calls are permitted, so all parsing and allocation happens
 * in the parent, before fork().
 */
#ifndef X11HOSTACCESS_H
#define X11HOSTACCESS_H

#include <cstddef>
#include <sys/types.h>

namespace X11HostAccess {

constexpr int kMaxSocketPaths = 2;   /* filesystem socket, then the abstract socket */
constexpr int kMaxReason = 512;

/* Everything the forked child needs, prepared by prepareGrant(). */
struct GrantPlan {
    /* Unix sockets to try, in order (the abstract socket has a leading '\0'). */
    char socketPath[kMaxSocketPaths][108];
    int socketCount = 0;
    /* Connection setup: xConnClientPrefix plus the authorization strings, padded. */
    unsigned char setup[256];
    size_t setupLength = 0;
    /* ChangeHosts(Insert, FamilyServerInterpreted, "localuser\0<user>\0"). */
    unsigned char changeHosts[64];
    size_t changeHostsLength = 0;
    /* The server-interpreted address the host access list must contain afterwards. */
    char target[64];
    size_t targetLength = 0;
    /* Report the host access list without changing it (diagnostics, tests). */
    bool listOnly = false;
    /* How long to wait for the X server before giving up. */
    unsigned timeoutSeconds = 5;
};

/* Child exit codes; only Ok means the user may connect. */
enum ExitCode : int {
    Ok = 0,
    PrivilegeDropFailed = 1,
    CouldNotConnect = 10,
    SetupRejected,
    IoError,
    Timeout,
    ChangeHostsRefused,
    ListHostsRefused,
    ProtocolError,
    NotGranted,
};

/* Resolves DISPLAY and, when there is one, its MIT-MAGIC-COOKIE-1 entry.
 * xauthority falls back to "<homeDirectory>/.Xauthority" when null.
 * Returns false, filling reason, when DISPLAY is unset, malformed or not a
 * local display: host access lists only govern local connections. */
bool prepareGrant(const char* display, const char* xauthority, const char* homeDirectory,
                  const char* targetUser, GrantPlan& plan, char* reason, size_t reasonCapacity);

/* Child side, to be called after dropping back to the original user.  Asks
 * the X server to add targetUser to its host access list and verifies the
 * result by reading the list back.  Async-signal-safe; never returns.
 * When reportFd >= 0, a single-line NUL-terminated report is written there
 * first. */
[[noreturn]] void runGrant(const GrantPlan& plan, int reportFd);

/* Parent side: turns the child's wait() status and report into a verdict and
 * a human-readable reason. */
bool describeOutcome(int waitStatus, const char* report, char* reason, size_t reasonCapacity);

}   // namespace X11HostAccess

#endif   // X11HOSTACCESS_H
