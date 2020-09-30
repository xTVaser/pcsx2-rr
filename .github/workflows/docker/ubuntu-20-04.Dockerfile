# This image is pushed up to Dockerhub on every master commit in order to save
# time during builds installing packages.
#
# In the event that a push/pull-request modifies the list of packages we install, the
# list of packages will be reinstalled ontop of this image.
#
# This is done to prevent surprises when a PR is merged, that it either breaks master builds
# or was never going to build properly initially, before the merge.
#
# This dockerfile is assumed to be built in the .github/workflows/ directory
FROM ubuntu:20.04

COPY . .

# Pass in -e PLATFORM <x86|x64> to the docker build command

RUN chmod +x ./scripts/linux/install-packages.sh
RUN ./scripts/linux/install-packages.sh
