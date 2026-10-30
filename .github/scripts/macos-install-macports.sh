#!/usr/bin/env bash
# Install universal CI dependencies into /opt/local on a fresh macOS runner.
# Requires gh (authenticated with GH_TOKEN) and jq, as provided by hosted runners.
set -euo pipefail

# Resolve the installer from the pinned release so OS codenames and per-OS
# checksums do not have to be maintained in this repository.
download_installer() {
    local version=$1 os_major=$2 directory=$3
    local asset url digest
    gh api "repos/macports/macports-base/releases/tags/v$version" > "$directory/release.json"
    asset=$(jq -er --arg prefix "MacPorts-$version-$os_major-" '
        [.assets[] | select(.name | startswith($prefix) and endswith(".pkg"))]
        | if length == 1 then
            .[0] | [.browser_download_url, .digest] | @tsv
          else
            error("Expected one installer for \($prefix), found \(length)")
          end
    ' "$directory/release.json")
    IFS=$'\t' read -r url digest <<< "$asset"
    if [[ ! $digest =~ ^sha256:[[:xdigit:]]{64}$ ]]; then
        printf 'Installer has no valid SHA-256 in release metadata: %s\n' "$url" >&2
        return 1
    fi
    curl --fail --location --retry 3 -o "$directory/base.pkg" "$url"
    printf '%s  %s\n' "${digest#sha256:}" "$directory/base.pkg" | shasum -a 256 -c -
}

install_base() {
    local installer=$1
    # The installer syncs the ports tree; select its official HTTPS source first.
    sudo mkdir -p /opt/local/etc/macports
    printf '%s [default]\n' https://distfiles.macports.org/ports.tar.gz \
        | sudo tee /opt/local/etc/macports/sources.conf >/dev/null
    sudo installer -pkg "$installer" -target /
}

install_universal_dependencies() {
    local jobs=$1
    printf '+universal\n' | sudo tee /opt/local/etc/macports/variants.conf >/dev/null
    sudo tee -a /opt/local/etc/macports/macports.conf >/dev/null <<CONFIG
buildfromsource ifneeded
universal_archs arm64 x86_64
ui_interactive no
buildmakejobs $jobs
CONFIG
    sudo /opt/local/bin/port -N install libsndfile readline
    # Cache only the installed SDK; leave it readable by the cache action.
    sudo chown -R "$(id -u):$(id -g)" /opt/local
}

main() (
    if [[ $# != 1 ]]; then
        printf 'Usage: %s <MacPorts-version>\n' "$0" >&2
        exit 2
    fi
    version=$1
    if [[ ! $version =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
        printf 'Expected a MacPorts release version, got: %s\n' "$version" >&2
        exit 2
    fi
    if [[ -e /opt/local/bin/port ]]; then
        echo 'MacPorts installation requires a fresh /opt/local; a cache hit must skip this script.' >&2
        exit 1
    fi
    os_version=$(sw_vers -productVersion)
    work_dir=$(mktemp -d "${TMPDIR:-/tmp}/fluidsynth-macports.XXXXXX")
    trap 'rm -rf "$work_dir"' EXIT

    download_installer "$version" "${os_version%%.*}" "$work_dir"
    install_base "$work_dir/base.pkg"
    install_universal_dependencies "$(sysctl -n hw.logicalcpu)"
)

main "$@"
