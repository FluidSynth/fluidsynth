# ✅ Release Checklist

## Preparation

  * Mention that CMAKE 3.13 will be required
  * Write up a changelog in the wiki
  * Check copyright year in fluidsynth.c
  * Update the version in the CMakeLists.txt to reflect the new version
  * Also update the library versions in CMakeLists.txt as per the embedded documentation (i.e., if there is any new C API and what not) (`LIB_VERSION_AGE` and `LIB_VERSION_REVISION`).
  * If there are any new API features or settings, they should be documented in the developers reference document.
  * Update the android build scripts to use the new version
  * Make sure changelog of [API doc](http://www.fluidsynth.org/api) is up-to-date
  * Make sure version of API doc is correct
  * Update man page with any new command line or settings options
  * `git merge 2.1.x` (into master)

## Actual release

  * Do a clean checkout from git and make a lightweight tag 
  * Wait for CI to complete
  * Goto Azure DevOps, select the build of the commit that you intend to release, and press the Release button on top right to publish the Windows binaries to the GitHub release (make sure the GitHub release hasn't been published yet!)

## Announce the release

  * fluidsynth.org 
  * fluid-dev 
  * LAA list - <http://lists.linuxaudio.org/pipermail/linux-audio-announce>
