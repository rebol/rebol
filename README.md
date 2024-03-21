[![Rebol CI](https://github.com/Oldes/Rebol3/actions/workflows/main.yml/badge.svg)](https://github.com/Oldes/Rebol3/actions/workflows/main.yml)
[![Build Rebol](https://github.com/Oldes/Rebol3/actions/workflows/build-all.yml/badge.svg)](https://github.com/Oldes/Rebol3/actions/workflows/build-all.yml)
[![Gitter](https://badges.gitter.im/rebol3/community.svg)](https://app.gitter.im/#/room/#Rebol3:gitter.im)
[![Zulip](https://img.shields.io/badge/zulip-join_chat-brightgreen.svg)](https://rebol.zulipchat.com/)
[![Chocolatey](https://raw.githubusercontent.com/Oldes/media/master/install-from-choco.svg)](https://chocolatey.org/packages/rebol3)
[![Replit](https://raw.githubusercontent.com/Oldes/media/master/try-on-replit.svg)](https://replit.com/@Oldes/Rebol-3140?v=1)

# Rebol [R3] Source Code Distribution

Purpose of this **Rebol fork** is to push [Carl's original source](https://github.com/rebol/rebol) to be at least as usable as Rebol 2 while keeping the source code clean and project easy to build. Use [CHANGES.md](https://github.com/Oldes/Rebol3/blob/master/CHANGES.md) file to see changes made in this branch.

### Issue Reporting

Preferred way to report issues is using the [dedicated issue repository](https://github.com/Oldes/Rebol-issues/issues). It's a fork of the original Rebol issue repository, which was filled with issues from the [CureCode issue tracker](https://www.curecode.org/rebol3/view-tickets.rsp), which was used before Rebol was on Github. I'm not using the original Rebol issue repository, because I was not allowed to even add labels to my own issues. It was later moved under Metaeducation account and is used for Ren-C development anyway.

### Precompiled Binaries

There are available precompiled binaries for each [release](https://github.com/Oldes/Rebol3/releases). So far there are 3 main build types:
1. **Base** is a build with minimal additions (not much useful)
2. **Core** includes a little bit more stuff than the **Base**
3. **Bulk** is a build which includes almost everything.

And there is also the Host exe and the DLL - the Rebol library is separated and used from the host application. That is from times before open sourcing Rebol completely. Only host part was open and the library was still closed. In theory you can have one library and many tiny host applications. I'm building just the Core on Windows so far to see, if it is still working.

For macOS users on 10.15+, downloaded precompiled applications (and or extensions bellow) may not work properly unless the quarantine setting is removed:
```
xattr -d -r com.apple.quarantine /path/to/file
```

### Rebol extensions

It is possible to extend Rebol functionality using external modules (native and or written in Rebol itself). Here are links to some of them:
* [Rebol/BCM2835](https://github.com/Oldes/Rebol-BCM2835) - Broadcom BCM 2835 chip extension (for GPIO on RaspberryPI)
* [Rebol/Blend2D](https://github.com/Siskin-framework/Rebol-Blend2D) - Drawing dialect using [Blend2D](https://blend2d.com) as a high performance 2D vector graphics engine
* [Rebol/BlurHash](https://github.com/Siskin-framework/Rebol-BlurHash) - Compact representation of a placeholder for an image
* [Rebol/Easing](https://github.com/Oldes/Rebol-Easing) - Collection of easing functions
* [Rebol/Godot](https://github.com/Oldes/Rebol-Godot) - Extractor of Godot's `.pck` files
* [Rebol/Google](https://github.com/Oldes/Rebol-Google) - Google Web API
* [Rebol/HTTPd](https://github.com/Oldes/Rebol-HTTPd) - Webserver scheme
* [Rebol/IRC](https://github.com/Oldes/Rebol-IRC) - Internet Relay Chat scheme
* [Rebol/MathPresso](https://github.com/Siskin-framework/Rebol-MathPresso) - Mathematical Expression Parser And JIT Compiler
* [Rebol/MiniAudio](https://github.com/Oldes/Rebol-MiniAudio) - An audio playback extension
* [Rebol/Names](https://github.com/Oldes/Rebol-Names) - Random names generator scheme
* [Rebol/Spotify](https://github.com/Oldes/Rebol-Spotify) - Spotify Web API
* [Rebol/OpenCV](https://github.com/Oldes/Rebol-OpenCV) - Computer Vision Library
* [Rebol/SQLite](https://github.com/Siskin-framework/Rebol-SQLite) - SQL database engine
* [Rebol/Triangulate](https://github.com/Siskin-framework/Rebol-Triangulate) - Two-Dimensional Quality Mesh Generator and Delaunay Triangulator
* [Rebol/WebDriver](https://github.com/Oldes/Rebol-WebDriver) - WebDriver scheme for automating Chromium based browser sessions
* [Rebol/WebSocket](https://github.com/Oldes/Rebol-WebSocket) - WebSocket scheme and codec

It should be noted that on macOS it may be required to resign _downloaded native extensions_ using command like:
```
codesign --sign "5D94...EED5" -f -o runtime /path/to/extension.rebx
```
To find a signing identity, use: `security find-identity`


### Building Rebol

Rebol itself is not a compiler (like [Red language](https://www.red-lang.org/)) but just an interpreter. You must have some compiler of your choice to compile Rebol. For Windows you may want to use any of these:
1. [Microsoft Visual Studio](https://visualstudio.microsoft.com/)
2. [CLANG](https://clang.llvm.org/)
3. [GCC (Mingw)](https://www.mingw-w64.org/)

Once you have any of these compilers, you must use [Siskin Builder tool](https://github.com/Siskin-framework/Builder/releases), which is actually a customised Rebol needed to [preprocess it's own sources](https://github.com/Oldes/Rebol3/blob/607572d5485f2d8e44aeea4ffadabf0c7374eee5/make/rebol3.nest#L981). It deserves own documentation as it's not mean to be used only to build Rebol. Meanwhile you may take a look at the source of the Github [workflow file for building all Rebol variants](https://github.com/Oldes/Rebol3/blob/master/.github/workflows/build-all.yml) used in the releases.

For a local use, you just do: `siskin <SOME-NEST-FILE>`, which starts CLI in an interactive mode.

The Siskin builder itself may be used as an example, how to build a custom utility based on Rebol sources.
The specification is [defined in this `*.nest` file](https://github.com/Siskin-framework/Builder/blob/master/tree/rebol/siskin.nest), where important is [`CUSTOM_STARTUP` define](https://github.com/Siskin-framework/Builder/blob/756d9531e2f461c22d626ca5458dad4e0c8bd3cd/tree/rebol/siskin.nest#L36) and [some files](https://github.com/Siskin-framework/Builder/blob/756d9531e2f461c22d626ca5458dad4e0c8bd3cd/tree/rebol/siskin.nest#L22-L30), which should be included and [some optional Rebol parts](https://github.com/Siskin-framework/Builder/blob/756d9531e2f461c22d626ca5458dad4e0c8bd3cd/tree/rebol/siskin.nest#L14-L18) if needed.


### Screenshots

![](https://github.com/Oldes/media/blob/master/screens/rebol-windows-terminal.PNG?raw=true "Rebol in Windows Terminal")

![](https://github.com/Oldes/media/blob/master/screens/rebol-ubuntu-terminal.jpg?raw=true "Rebol in Linux Terminal")

#### Building a customized CLI application using compile DSL:
![](https://raw.githubusercontent.com/Oldes/media/master/screens/build-siskin.gif "Building a Rebol based utility")

### Other Rebol related projects

If you are looking for other _Rebol like languages_, you may want to check also:

* [Arturo](https://github.com/arturo-lang/arturo) language written in Nim
* [Boron](http://urlan.sourceforge.net/boron/) language written in C
* [Red](https://github.com/red/red) language written in Red bootstrapped from Rebol2
* [Red.js](https://github.com/ALANVF/Red.js) web runtime for Red written in Haxe
* [Ren-C](https://github.com/metaeducation/ren-c) another living Rebol3 fork
* [Rye](https://github.com/refaktor/rye) language written in Go
* [Topaz](https://github.com/giesse/Project-SnowBall) experimental Rebol like language being compiled to JS
* [World](https://github.com/Geomol/World) language written in C


There is also [Shinxin's fork](https://github.com/zsx/r3), which I was initially using for _chery-picking_,
as it contains modifications from Atronix and Saphirion. But its use is now limited as it depends on non-public modules and also there is not much life visible recently.
