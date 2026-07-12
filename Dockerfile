FROM opensuse/tumbleweed:latest

# SDL3 not shipped by leap 15.6 itself
#RUN zypper addrepo https://download.opensuse.org/repositories/games/15.6/games.repo
#RUN zypper --gpg-auto-import-keys refresh

# this is what we need for running a pipeline in the container
RUN zypper refresh && zypper install --no-recommends -y sudo shadow util-linux

# fluidsynths core dependencies + sox and awk for regression tests
RUN zypper refresh && zypper --non-interactive install --no-recommends \
  git bash findutils gawk ladspa-devel readline-devel \
  cmake pkg-config make gcc-c++ clang alsa-devel libjack-devel pipewire-devel readline-devel libsndfile-devel SDL3-devel libasan8 \
  doxygen astyle gdb sox ninja \
  glib2-devel # allowing builds of legacy fluidsynth versions

# Documentation build dependencies:
#   python3 + pipx     -- for zensical
#   libxslt-tools      -- provides xsltproc for the XSLT pipeline
RUN zypper refresh && zypper install --no-recommends -y python3 python311-pipx libxslt-tools xsltproc
RUN pipx install zensical && pipx ensurepath

ENV PATH="/root/.local/bin:${PATH}"

ENTRYPOINT ["/bin/bash"]
