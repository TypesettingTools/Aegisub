#!/bin/sh

set -eu

if test "$#" -ne 1; then
    echo "Usage: $0 MESON_BUILD_DIR" >&2
    exit 2
fi

if test "$(uname -s)" != "Darwin"; then
    echo "FFTW macOS bootstrap must run on macOS" >&2
    exit 2
fi

BUILD_DIR="$(cd "$1" && pwd)"
DEPLOYMENT_TARGET="$(meson introspect --buildoptions "${BUILD_DIR}" | python3 -c '
import json
import sys

options = json.load(sys.stdin)
matches = [option["value"] for option in options if option["name"] == "macos_deployment_target"]
if len(matches) != 1:
    raise SystemExit("macos_deployment_target is missing from the Meson build")
print(matches[0])
')"

FFTW_VERSION="3.3.11"
FFTW_SHA256="5630c24cdeb33b131612f7eb4b1a9934234754f9f388ff8617458d0be6f239a1"
FFTW_URL="https://fftw.org/fftw-${FFTW_VERSION}.tar.gz"
FFTW_PREFIX="${BUILD_DIR}/fftw-prefix"
FFTW_STAMP="${FFTW_PREFIX}/.aegisub-build"
ARCH="$(meson introspect --machines "${BUILD_DIR}" | python3 -c '
import json
import sys

machine = json.load(sys.stdin)["host"]
if machine["system"] != "darwin":
    raise SystemExit("FFTW macOS bootstrap requires a Darwin host")
print({"aarch64": "arm64", "x86_64": "x86_64"}.get(machine["cpu_family"], machine["cpu_family"]))
')"
BUILD_ARCH="$(uname -m)"
EXPECTED_STAMP="fftw=${FFTW_VERSION} arch=${ARCH} build=${BUILD_ARCH} macos=${DEPLOYMENT_TARGET}"

if test -f "${FFTW_STAMP}" && test "$(cat "${FFTW_STAMP}")" = "${EXPECTED_STAMP}"; then
    echo "Using existing FFTW build: ${EXPECTED_STAMP}"
    exit 0
fi

case "${ARCH}" in
    x86_64)
        ARCH_ARGS="--enable-sse2 --enable-avx --enable-avx2"
        ;;
    arm64)
        ARCH_ARGS="--enable-armv8-cntvct-el0"
        ;;
    *)
        echo "Unsupported macOS architecture: ${ARCH}" >&2
        exit 2
        ;;
esac

WORK_DIR="$(mktemp -d "${TMPDIR:-/tmp}/aegisub-fftw.XXXXXX")"
trap 'rm -rf "${WORK_DIR}"' EXIT HUP INT TERM

ARCHIVE="${WORK_DIR}/fftw.tar.gz"
curl --fail --location --retry 3 --output "${ARCHIVE}" "${FFTW_URL}"
echo "${FFTW_SHA256}  ${ARCHIVE}" | shasum -a 256 --check
tar -xzf "${ARCHIVE}" -C "${WORK_DIR}"

rm -rf "${FFTW_PREFIX}"
mkdir -p "${FFTW_PREFIX}"

cd "${WORK_DIR}/fftw-${FFTW_VERSION}"
export CC=/usr/bin/clang
export CFLAGS="-O3 -arch ${ARCH} -mmacosx-version-min=${DEPLOYMENT_TARGET}"
export LDFLAGS="-arch ${ARCH} -mmacosx-version-min=${DEPLOYMENT_TARGET}"

# Keep runtime probes enabled for native builds.
set -- "--build=$(./config.guess)"
if test "${ARCH}" != "${BUILD_ARCH}"; then
    set -- "$@" "--host=${ARCH}-apple-darwin"
fi

# Aegisub only uses FFTW's single-threaded, double-precision API.
# shellcheck disable=SC2086
./configure "$@" \
    --prefix="${FFTW_PREFIX}" \
    --disable-shared \
    --enable-static \
    --disable-fortran \
    --disable-doc \
    --disable-mpi \
    --disable-openmp \
    --disable-threads \
    ${ARCH_ARGS}

make -s -j"$(sysctl -n hw.logicalcpu)"
make -s install
if test "$(lipo -archs "${FFTW_PREFIX}/lib/libfftw3.a")" != "${ARCH}"; then
    echo "FFTW library architecture does not match Meson's host: ${ARCH}" >&2
    exit 1
fi
printf '%s\n' "${EXPECTED_STAMP}" > "${FFTW_STAMP}"

echo "Built FFTW ${FFTW_VERSION} for macOS ${DEPLOYMENT_TARGET} (${ARCH})"
echo "pkg-config path: ${FFTW_PREFIX}/lib/pkgconfig"
