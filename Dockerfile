FROM opensuse/tumbleweed:latest

# SDL3 not shipped by leap 15.6 itself
#RUN zypper addrepo https://download.opensuse.org/repositories/games/15.6/games.repo
#RUN zypper --gpg-auto-import-keys refresh

RUN zypper refresh && zypper install --no-recommends -y git bash findutils gawk
# this is what we need for running a pipeline in the container
RUN zypper refresh && zypper install --no-recommends -y sudo shadow util-linux

RUN zypper refresh && zypper install --no-recommends -y cmake pkg-config make gcc-c++ alsa-devel libjack-devel pipewire-devel readline-devel libsndfile-devel libasan8

RUN zypper refresh && zypper install --no-recommends -y clang

RUN zypper refresh && zypper install --no-recommends -y doxygen astyle gdb sox

RUN zypper refresh && zypper install --no-recommends -y glib2-devel ninja

# Documentation build dependencies:
#   python3 + pipx     -- for zensical
#   libxslt-tools      -- provides xsltproc for the XSLT pipeline
RUN zypper refresh && zypper install --no-recommends -y python3 python311-pipx libxslt-tools xsltproc
RUN pipx install zensical && pipx ensurepath

ENV PATH="/root/.local/bin:${PATH}"

ENTRYPOINT ["/bin/bash"]
