FROM opensuse/tumbleweed:latest

# SDL3 not shipped by leap 15.6 itself
#RUN zypper addrepo https://download.opensuse.org/repositories/games/15.6/games.repo
#RUN zypper --gpg-auto-import-keys refresh

RUN zypper refresh && zypper install --no-recommends -y git bash
# this is what we need for running a pipeline in the container
RUN zypper refresh && zypper install --no-recommends -y sudo shadow util-linux

RUN zypper refresh && zypper install --no-recommends -y cmake pkg-config make gcc-c++ alsa-devel libjack-devel pipewire-devel readline-devel libsndfile-devel libasan8

RUN zypper refresh && zypper install --no-recommends -y clang

RUN zypper refresh && zypper install --no-recommends -y doxygen astyle gdb sox find awk

RUN zypper refresh && zypper install --no-recommends -y glib2-devel ninja

ENTRYPOINT ["/bin/bash"]
