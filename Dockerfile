FROM opensuse/tumbleweed:latest

# SDL3 not shipped by leap 15.6 itself
#RUN zypper addrepo https://download.opensuse.org/repositories/games/15.6/games.repo
#RUN zypper --gpg-auto-import-keys refresh

# this is what we need for running a pipeline in the container
RUN zypper refresh && zypper install --no-recommends -y sudo shadow util-linux

# fluidsynths core dependencies + sox and awk for regression tests
RUN zypper refresh && zypper --non-interactive install --no-recommends \
  git bash findutils gawk cmake pkg-config make ninja gcc-c++ clang libasan8 libgomp1 \
  alsa-devel libjack-devel pipewire-devel ladspa-devel readline-devel libsndfile-devel SDL3-devel portaudio-devel libpulse-devel dbus-1-devel \
  doxygen astyle gdb sox gcovr \
  glib2-devel libinstpatch-devel # allowing builds of legacy fluidsynth versions for regression testing, git bisecting, etc.

# Documentation build dependencies:
#   python3 + pipx     -- for zensical
#   libxslt-tools      -- provides xsltproc for the XSLT pipeline
RUN zypper refresh && zypper install --no-recommends -y python3 python311-pipx libxslt-tools xsltproc
RUN pipx install zensical && pipx ensurepath

ENV PATH="/root/.local/bin:${PATH}"

ENTRYPOINT ["/bin/bash"]
