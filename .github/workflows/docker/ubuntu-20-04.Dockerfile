# This image is pushed up to Dockerhub on every master commit in order to save
# time during builds installing packages.
#
# In the event that a push/pull-request modifies the list of packages we install, the
# list of packages will be reinstalled ontop of this image.
#
# This is done to prevent surprises when a PR is merged, that it either breaks master builds
# or was never going to build properly initially, before the merge.
#
# This dockerfile is assumed to be built from the root directory

FROM ubuntu:20.04

COPY .github/ .github/

# Pass in - PLATFORM <x86|x64> to the docker build command
ARG ENV_PLATFORM=x86
ENV PLATFORM=$ENV_PLATFORM

# Install dependencies not directly related to building / using pcsx2
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get -qq update
RUN apt-get install -y git

RUN chmod +x .github/workflows/scripts/linux/install-packages.sh
RUN USE_SUDO=false ./.github/workflows/scripts/linux/install-packages.sh
