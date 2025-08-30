# Windows Build Migration Guide

## Overview

The Windows build pipeline has been migrated from Azure Pipeline to GitHub Actions to provide better integration with GitHub releases and automated website announcements.

## Setup Instructions

### 1. Configure Repository Variables

In your GitHub repository, go to **Settings → Secrets and variables → Actions → Variables** and add:

| Variable Name | Description | Required |
|---------------|-------------|----------|
| `GTK_BUNDLE_X86` | URL to GTK bundle for x86 builds | Yes |
| `GTK_BUNDLE_X64` | URL to GTK bundle for x64 builds | Yes |
| `LIBSNDFILE_URL_X86` | URL to libsndfile for x86 builds | Yes |
| `LIBSNDFILE_URL_X64` | URL to libsndfile for x64 builds | Yes |
| `MINGW_URL_X64` | URL to MinGW toolchain for x64 builds | Yes |

### 2. Build Matrix

The new workflow includes:

- **MSVC Builds**: x86/x64 with regular and cpp11 OSAL variants
- **MSVC Additional**: Various configuration tests (debug, profiling, static libs)
- **MinGW Builds**: x64 shared and static library builds
- **MSYS2 Builds**: Legacy compatibility builds

### 3. Release Automation

#### Artifact Publishing
When a GitHub release is published:
1. All build artifacts are automatically downloaded
2. Compressed archives (.tar.gz and .zip) are created
3. Archives are uploaded to the GitHub release

#### Website Announcements
When a GitHub release is published:
1. A PR is created in `FluidSynth/fluidsynth.github.io`
2. A new blog post announcing the release is added
3. If PR creation fails, an issue is created instead

### 4. Selective Publishing

You can control which builds get published by:
1. Using workflow_dispatch to manually trigger builds
2. Modifying the `publish-release` job conditions
3. Only specific artifact names will be published (those starting with "fluidsynth")

## Migration Status

- ✅ Azure Pipeline jobs migrated to GitHub Actions
- ✅ Azure Pipeline disabled (trigger: none, jobs: [])
- ✅ Release automation implemented
- ✅ Website integration implemented
- ✅ Validation tests created

## Troubleshooting

### Missing Dependencies
If builds fail due to missing dependencies, ensure all repository variables are set correctly.

### LibInstPatch Artifacts
The workflow attempts to download libinstpatch artifacts from previous Azure Pipeline runs. This may fail initially but won't break the build.

### Website PR Creation
If website PR creation fails, check:
1. Repository permissions for cross-repo PRs
2. GitHub token permissions
3. The fallback issue creation should work as an alternative

For questions, consult the workflow file at `.github/workflows/windows.yml` or the migration documentation.