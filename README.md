# A Movesense Example App

This app is a test project on how one can write functionality for the Movesense device.

## Setup

1. Clone the repo (fetching the submodules too):
  * `git clone --recurse-submodules https://github.com/tuohis/movesense-app.git`
2. Setup the build environment:
  * Install Vagrant and your favorite provider (e.g. VirtualBox)
  * `cd movesense-app && vagrant up`
3. Build:
  * `vagrant ssh -c "source scripts/cmake-build.sh"`
  * Optionally you can also start an interactive session within the virtual machine using `vagrant ssh` and call the build command there.

