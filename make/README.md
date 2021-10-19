# How to build Rebol 3

## Pre-Requisites

1. Clone the Rebol 3 Repo if you haven't already: `git clone https://github.com/Oldes/Rebol3.git`
2. Download the [Siskin build tool](https://github.com/Siskin-framework/Builder/releases/latest) for your platform
3. Place the Siskin binary in the root of the repo (there seems to a bug in Siskin where it cannot find itself from the `PATH` and dies)

## Build Walkthrough

1. Enter the `Rebol3` directory you cloned above: `cd ~/Source/Rebol3`
2. Start the build tool: `./Siskin make/rebol3.nest`
3. You should see an interactive CLI tool with a series of numbered options
4. Choose the package you would like to build or test by entering a number and pressing enter
5. Siskin will build Rebol and display detailed output of its process
6. Type `q` to exit Siskin
