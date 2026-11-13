# How to build Rebol 3

## Pre-Requisites

1. Clone the Rebol 3 Repo if you haven't already: `git clone https://github.com/Oldes/Rebol3.git`
2. Download the [Siskin build tool](https://github.com/Siskin-framework/Builder/releases/latest) for your platform
3. Place the Siskin binary in the root of the repo (or into one of the `PATH` folders)

macOS users on 10.15+ may need to remove the quarantine setting from the Siskin builder tool after download using:
```
xattr -d -r com.apple.quarantine /path/to/Siskin
```

## Build Walkthrough

1. Enter the `Rebol3` directory you cloned above: `cd ~/Source/Rebol3`
2. Start the build tool: `./Siskin make/rebol3.nest`
3. You should see an interactive CLI tool with a series of numbered options
4. Choose the package you would like to build or test by entering a number and pressing enter
5. Siskin will build Rebol and display detailed output of its process
6. Type `q` to exit Siskin

## Linux x86-64 with glibc 2.39 (Ubuntu 24.04)

With Docker installed and running, build and test the current checkout:

```sh
sh make/docker/build.sh
```

If your user cannot access the Docker socket, run `sudo sh make/docker/build.sh`.
The executable is exported to `build/ubuntu24/rebol3-bulk-linux-x64`.
The build uses the nest's optimized Bulk target, Ubuntu 24.04's compiler and
glibc, and Siskin 0.23.2 verified against its archive SHA-256 checksum.
Existing host binaries and temporary build files are excluded from the context.

Before export, the build checks required glibc symbol versions (maximum 2.39),
runs the executable, builds the native test extension, and runs
`src/tests/run-tests.r3` inside Ubuntu 24.04. Assertion failures and failed test
files fail the build. Some repository tests need Internet access. Test output
is printed in the Docker build output (and retained at `/tmp/rebol-tests.log`
in the test container). External extension integration tests are not included.

To export a binary even when the repository tests fail, use the explicit
build-only mode. It still checks startup and the required glibc symbols:

```sh
sh make/docker/build.sh --build-only
```

This mode skips the unit suite and does not indicate that tests passed.

The executable dynamically links glibc; compatibility with versions older than
2.39 depends on its actual required symbols. This is a repeatable workflow,
not a byte-for-byte reproducible build: Ubuntu package updates and build dates
can change the output.

With Docker BuildKit available, the equivalent direct file export is:

```sh
docker build --platform linux/amd64 -f make/docker/Dockerfile \
  --output type=local,dest=build/ubuntu24 .
```
