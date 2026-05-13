# M5Stack Imager

M5Stack Imager is an M5Stack-oriented fork of Raspberry Pi Imager for writing
operating-system images to M5Stack Raspberry Pi devices such as CM4Stack and
CardputerZero.

This repository currently keeps the upstream Qt/C++ writing engine, removable
drive handling, checksum verification, OS customisation flow, and Compute
Module USB boot support from Raspberry Pi Imager. The first M5Stack-specific
layer is the default branding, telemetry policy, and OS repository structure.

## Preview

![M5Stack Imager screenshot](screenshot_m5stack.png)

## Upstream

The upstream project is `raspberrypi/rpi-imager` and remains available as the
`upstream` git remote:

```sh
git remote -v
git fetch upstream
```

The original upstream README is preserved in `README.upstream.md`. Keep
`license.txt` and upstream copyright notices intact when distributing builds.

## M5Stack Repository Manifest

The app reads an OS list JSON manifest. The default is configured through the
`M5STACK_IMAGER_DEFAULT_REPO_URL` CMake cache variable, with a fallback in
`src/config.h`:

```text
https://cardputer-zero-repo.oss-cn-shenzhen.aliyuncs.com/os-list.json
```

The fallback in `src/config.h` still points at the checked-in example manifest
embedded as `qrc:/m5stack/os-list.json`; use that only for offline test builds.
To force a different manifest at build time, pass:

```sh
cmake -S src -B build \
  -DM5STACK_IMAGER_DEFAULT_REPO_URL=https://cardputer-zero-repo.oss-cn-shenzhen.aliyuncs.com/os-list.json
```

You can also test a local or remote manifest at runtime:

```sh
rpi-imager --repo m5stack/os-list.cm4stack-cardputerzero.example.json
```

That example exposes CM4Stack and CardputerZero in the device picker. The
CardputerZero entry points at the latest Trixie arm64 image:

```text
https://cardputer-zero-repo.oss-cn-shenzhen.aliyuncs.com/cardputerzero-trixie-arm64-latest.img.xz
```

`m5stack/oss/os-list.json` is the production manifest uploaded to
`https://cardputer-zero-repo.oss-cn-shenzhen.aliyuncs.com/os-list.json`.
`m5stack/os-list-template.json` mirrors that production manifest for reference.

OSS upload assets live under `m5stack/oss/`:

- `os-list.json` is the manifest published to the bucket root.
- `icons/*.png` are published to the `icons/` prefix.
- `.env.example` documents the local OSS credentials format.
- `.env` is ignored by git and used only for local uploads.

Run `m5stack/oss/upload-oss.sh` to publish the manifest and icons. The script
does not upload OS image artifacts; image publishing is handled separately.

## Build

Use the upstream build flow:

```sh
cd src
cmake -S . -B ../build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build ../build
```

## GitHub Actions and Releases

The repository includes release workflows for all supported desktop platforms:

- `.github/workflows/windows-release.yml` builds an unsigned Windows x64
  installer and portable zip.
- `.github/workflows/macos-release.yml` builds a macOS arm64 DMG.
- `.github/workflows/linux-release.yml` builds a Linux x64 tarball.

The workflows run on `push`, `pull_request`, and manual `workflow_dispatch`.
Tagged builds also upload artifacts to the matching GitHub Release. The Windows
workflow produces:

- `M5Stack-Imager-<version>-windows-x64-installer.exe`
- `M5Stack-Imager-<version>-windows-x64-portable.zip`

To publish a GitHub Release, push a version tag:

```sh
git tag v0.1.0
git push origin v0.1.0
```

Builds are unsigned until M5Stack code signing credentials are wired into the
workflows, so Windows may show the normal SmartScreen warning and macOS may show
Gatekeeper warnings.

Packaging identifiers such as `com.raspberrypi.rpi-imager`, `rpi-imager`, and
`.rpi-imager-manifest` still need a dedicated M5Stack rename pass before public
release.

## Immediate TODO

- Keep the production M5Stack OS manifest at
  `https://cardputer-zero-repo.oss-cn-shenzhen.aliyuncs.com/os-list.json`
  updated when image metadata changes.
- Fill in the CardputerZero image `extract_size`, `extract_sha256`, and
  `image_download_sha256` once the final image artifact is available.
- Decide whether M5Stack wants its own anonymous metrics endpoint; telemetry is
  disabled by default in this fork.
- Add M5Stack-specific first boot customisation only after the target image
  layout is fixed.
