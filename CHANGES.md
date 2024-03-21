# Rebol 3 (Oldes' branch) Changes

This is just generated output from commits in [this repository](https://github.com/Oldes/Rebol3). For full log use [GitHub commits](https://github.com/Oldes/Rebol3/commits/master).


## 2024 February

### Features:

* [`f25ddf`](https://github.com/Oldes/Rebol3/commit/f25ddfa9239befaab611b209d5b64146b61b75a1) Implemented optional `XXH3`, `XXH32`, `XXH64` and `XXH128` checksums
* [`ab42ca`](https://github.com/Oldes/Rebol3/commit/ab42ca5231abdbc73f11ece0bc8b0dd6953f553b) Tiny optimization
* [`da4718`](https://github.com/Oldes/Rebol3/commit/da4718b39c376b323799ecabe1491fae899ad036) Not using hashing for small enough maps (reduces memory usage)

### Changes:

* [`3c2b93`](https://github.com/Oldes/Rebol3/commit/3c2b93afa872334effe9c840813ed789ce1dbd0c) **Switching map and construction syntax**

  `map!` values now use syntax like: `#[key: value]`
  construction syntax now looks like: `#(true)` or `#(i32! [1 2 3])`
  
  The included [`map-conv.reb` script](https://github.com/Oldes/Rebol3/blob/master/make/tools/map-conv.reb) can be used for converting existing code.

* [`2ba01e`](https://github.com/Oldes/Rebol3/commit/2ba01ead3b5da1ab609fa25a0c675b30e96bb163) `unset` value in `any` and `all` is transparent now


### Fixes:

* [`01a68c`](https://github.com/Oldes/Rebol3/commit/01a68cfbedebd86fda68ce287efba85a0f199660) Compiler warnings
* [`695636`](https://github.com/Oldes/Rebol3/commit/695636065bb7876478dbf70e7f5f4560eb986370) Correct fix of regression when creating 64 bit vectors
* [`238ecb`](https://github.com/Oldes/Rebol3/commit/238ecbcedda7b583ce0d96d4c111dc5d6d1bd330) Regression when creating 64 bit vectors
* [`820687`](https://github.com/Oldes/Rebol3/commit/8206872e89a080622b1d6dc15abfd746aa4d1de8) `checksum` with `file!` argument not supporting all available methods
* [`d4364d`](https://github.com/Oldes/Rebol3/commit/d4364df0e4f48944cb36beba1fdbc32d6d109e9f) Saving a file into a zip archive

## 2024 January

### Features:

* [`e401ed`](https://github.com/Oldes/Rebol3/commit/e401eddfb87ed47243984392bb445409b6fc390a) Added `sum` and `average` functions
* [`27611e`](https://github.com/Oldes/Rebol3/commit/27611e9d02764593484a1c8c9aac151c907145cc) Including source of `webdriver` and `websocket` modules

### Changes:

* [`da1905`](https://github.com/Oldes/Rebol3/commit/da1905820e0091c4e27a7746e66f3ca5877a4ca7) Updated Mbed-TLS sources to version 3.5.2
* [`52efc0`](https://github.com/Oldes/Rebol3/commit/52efc0caca758dfa9fc74256ee26757738c4c115) Register `.swc` file extension with `zip` codec
* [`7c84b6`](https://github.com/Oldes/Rebol3/commit/7c84b60c8a2ba2cbd9c57fb416d87a563ad82713) Allow issues like `###` or `#a#` (Rebol2/Red compatibility)
* [`9bb910`](https://github.com/Oldes/Rebol3/commit/9bb91086c266e190121c075b0b412c29e9c06651) Zlib source updated to version 1.3.0.1

### Fixes:

* [`1bfcbb`](https://github.com/Oldes/Rebol3/commit/1bfcbbcf83827b846ef7f3fa508bd1afc0796de6) Bbcode codec - reset row-width of an image gallery tag
* [`307b24`](https://github.com/Oldes/Rebol3/commit/307b24a9012fdbb37d1d3e29031e7e96ca7dca2d) Windows JPEG codec unexpected property type error
* [`592454`](https://github.com/Oldes/Rebol3/commit/592454e4090018accedbc4730dc79b7cc7320992) LF to CRLF conversion when using `write` with `string` input
* [`fbad12`](https://github.com/Oldes/Rebol3/commit/fbad12f1a426e2802ac16898a88cff287270101b) Joining path/url using path notation not respecting index position
* [`aa7aa4`](https://github.com/Oldes/Rebol3/commit/aa7aa41114ba158d9e55fd3e291099229e3d15fb) Throw an error when converting `1.#NaN` value to integer
* [`45b358`](https://github.com/Oldes/Rebol3/commit/45b358f075215b19490104891c2d8f64a578628e) Close `httpd` server on request even when there are pending client connections

## 2023 December

### Features:

* [`ffa41e`](https://github.com/Oldes/Rebol3/commit/ffa41e0b0d48cc5371e32215c35b3a2d73d47149) `bbcode` image gallery emitter
* [`e39581`](https://github.com/Oldes/Rebol3/commit/e39581578b98b3eeb121f47e99224e8dda2be424) `bbcode` csv table emitter

### Changes:

* [`7a9e11`](https://github.com/Oldes/Rebol3/commit/7a9e11000d4275e95940116593ec0a731289d5e0) Updated `httpd` scheme to version 0.9.0

### Fixes:

* [`9715d1`](https://github.com/Oldes/Rebol3/commit/9715d1205bc45b0c195f4ab95f1cdd8c151abe43) Code simplified
* [`aa227f`](https://github.com/Oldes/Rebol3/commit/aa227fc7f8a5571ca3307255a93f0804a174c5b1) Avoid an error when loading an unset value
* [`051670`](https://github.com/Oldes/Rebol3/commit/051670ad6e5b2ec6cee6e95c002a43a06f128a97) Using correct path to images in the generated `bbcode` image gallery
* [`8ee972`](https://github.com/Oldes/Rebol3/commit/8ee972c64ef81df1f58e42e31d7876c6635a8c2a) `bbcode` codec must convert binary input to string before decoding
* [`b0f684`](https://github.com/Oldes/Rebol3/commit/b0f6840ceb23f0f7752f9f7edae25edaced34bbe) Reading from virtual `/proc` files on Linux using `read/part`

## 2023 November

### Features:

* [`963d08`](https://github.com/Oldes/Rebol3/commit/963d083a5e7fbf2c96ef9a60be386af7ff4402c0) Allow `word!`, `get-word!` and `get-path!` values inside a `call` argument
* [`39c7e2`](https://github.com/Oldes/Rebol3/commit/39c7e22efb9237a1e86e72973ebf0582d3d5e981) Allow any-string value as a `call` argument
* [`a38e2a`](https://github.com/Oldes/Rebol3/commit/a38e2a5c5622641df7ca291a396d4937f871e589) Added possibility to set/clear BRK, RTS and DTR serial port flags on Posix
* [`1ff2d9`](https://github.com/Oldes/Rebol3/commit/1ff2d94d69bce3dfb6a55091b246689b8c8e987a) Optional support for SHA3 family checksums (included in Bulk)
* [`3dc6f6`](https://github.com/Oldes/Rebol3/commit/3dc6f6aa27408ef814cc90897dedb751eacda703) New platform for TurrisOS
* [`feaaef`](https://github.com/Oldes/Rebol3/commit/feaaef940490166984aaff4c9a654324703fbc4b) Better info when native extension fails to load its library
* [`a1b79b`](https://github.com/Oldes/Rebol3/commit/a1b79b72cff7b1e0707aae080a22736111899bcf) `wait-for-key` which waits for a single key press
* [`855adc`](https://github.com/Oldes/Rebol3/commit/855adc457744f140311c45dca0bbfa54f2593578) Struct reflection code reduction
* [`4332e1`](https://github.com/Oldes/Rebol3/commit/4332e1b614327cfd3908e94a5a69048b9fc827b4) Implemented `body-of` struct reflection (returning struct's key/value pairs)
* [`8c111a`](https://github.com/Oldes/Rebol3/commit/8c111aafe1460b4e071bd8c4f9a414ca819b04a2) Implemented `values-of` and `words-of` struct reflections
* [`b6640e`](https://github.com/Oldes/Rebol3/commit/b6640ecb6b6f61d7735b862bb61b054270079b8e) Allow changing binary series with tuple values

### Changes:

* [`bf4999`](https://github.com/Oldes/Rebol3/commit/bf49996cccaec38302cd6481856bc63dd0ed6bb4) Modified `launch` function not to use shell and to handle all optional script arguments
* [`abc6ec`](https://github.com/Oldes/Rebol3/commit/abc6ecc0b2f6a52a5fdc8724d9c5483f7d9a0757) Using system error codes when opening a MIDI device on Windows
* [`35c1b6`](https://github.com/Oldes/Rebol3/commit/35c1b610e7d1a6df74437ee3a2918ff96577a3b0) Updated Mbed-TLS sources to version 3.5.1
* [`4dc546`](https://github.com/Oldes/Rebol3/commit/4dc546e52076f3e88064ccf99039f1a0c00d7cd1) Using line breaks in molded `struct!` values for better readability
* [`68bf82`](https://github.com/Oldes/Rebol3/commit/68bf82e5d71403b5345611219c40b5293573557e) Moved `STATIC_ASSERT` macro to `reb-c.h` file and used it in a few more cases
* [`ad12a6`](https://github.com/Oldes/Rebol3/commit/ad12a68069d13953f9fb11c9eb68611a3caf7b43) Don't try to validate baud rate for a serial port

### Fixes:

* [`a73014`](https://github.com/Oldes/Rebol3/commit/a730144ae9cb4d607d6725c5ea0044760e335c08) Accept block input to `call` function on Windows
* [`56a39e`](https://github.com/Oldes/Rebol3/commit/56a39e6267e828add4be1e5d2d1741a452b45128) Warning: implicit declaration of function ‘wcwidth’
* [`2ea23b`](https://github.com/Oldes/Rebol3/commit/2ea23b1a25f076acaa3f0bdd662ffed58c869de6) Ignore warning: ‘__builtin___strncpy_chk’ specified bound depends on the length of the source argument
* [`a84671`](https://github.com/Oldes/Rebol3/commit/a84671a2d4477e51649884020e4e0993b438a003) Warning: ignoring return value of ‘read’, declared with attribute warn_unused_result
* [`98c54f`](https://github.com/Oldes/Rebol3/commit/98c54fc5eac64ce2b717c39660faca77f90a1a9a) Warning: specified bound depends on the length of the source argument
* [`45832b`](https://github.com/Oldes/Rebol3/commit/45832b91fbf9c82907ecb6806ebeee7a42e00e17) Trying to fix build for Haiku OS
* [`da13e8`](https://github.com/Oldes/Rebol3/commit/da13e819a8cf94f79dd40a26523ecb5b219295d4) Trying to fix build for Haiku OS
* [`7f8108`](https://github.com/Oldes/Rebol3/commit/7f81085e563a65e2e71e0ee326e56c081ef98d66) Trying to fix build for Haiku OS
* [`0be75d`](https://github.com/Oldes/Rebol3/commit/0be75dc3a360f2eaab92debb06605f1a2287ef76) Timeout when reading some HTTPS URLs
* [`b63745`](https://github.com/Oldes/Rebol3/commit/b63745d321ba99b8d9392341e909c9ec47b8a9e2) Invalid length of `checksum:sha224` port result
* [`a87e66`](https://github.com/Oldes/Rebol3/commit/a87e667ab9e195a7d4becf4df14a411ef16ed551) Redundant message when logging an error value
* [`821a5a`](https://github.com/Oldes/Rebol3/commit/821a5a96c22b909c4500391795e73fc255bc78d9) Better `struct!` initialization; empty struct not allowed
* [`7aaa05`](https://github.com/Oldes/Rebol3/commit/7aaa058af6f72f311c103c5e208391db850907ae) Early validate input data type (else there could be free before alloc call)
* [`76c8c0`](https://github.com/Oldes/Rebol3/commit/76c8c09cdc12e96aa1dbf12d1375299603ddacab) Signed/unsigned mismatch warning
* [`96ece9`](https://github.com/Oldes/Rebol3/commit/96ece90bb5518c5e85fc6a9df47664394b419d65) Not using `NZ` macro

## 2023 October

### Features:

* [`455f04`](https://github.com/Oldes/Rebol3/commit/455f0458184bda051a4ef0ecb3b3fd98cca5d432) `function` and `closure` accepting also `module!` for its `object` argument
* [`bee8e0`](https://github.com/Oldes/Rebol3/commit/bee8e09beea85d95a82d58a4db7d14b409693781) Unicode utils module with `decode-utf8` function

### Fixes:

* [`45749f`](https://github.com/Oldes/Rebol3/commit/45749f7b3b837dd1fd7e2888b5ab1380cc080cba) Not initialized local variable
* [`1871d9`](https://github.com/Oldes/Rebol3/commit/1871d95412a9b2defa54c0b20609ff4b311bcae5) Only using paren char escaping for chars in range `0x7F - 0x9F`
* [`3a7282`](https://github.com/Oldes/Rebol3/commit/3a72825a8313d7c4dadbf24fa323ad2b68a7a512) Using replacement char for all chars outside the Basic Multilingual Plane
* [`0976da`](https://github.com/Oldes/Rebol3/commit/0976daf110bd35f8c506c3320465ed7097a47112) Formed/molded output with ASCII chars using always a special char escaping

## 2023 September

### Features:

* [`3b731a`](https://github.com/Oldes/Rebol3/commit/3b731af5cc2a02e21b9a7ac5c0ccee337c7757ad) Extension library API for conversion of Rebol file to OS file and back
* [`ca45cf`](https://github.com/Oldes/Rebol3/commit/ca45cf2ca598c61f5e3470cb406275f18bec8a13) Allow extension to mark handle's context on free call and so prevent it from releasing
* [`a0ff1b`](https://github.com/Oldes/Rebol3/commit/a0ff1bb7e35b269ce0b0ad8488d34e463fef6923) Allow `write` action on the console port
* [`be97ce`](https://github.com/Oldes/Rebol3/commit/be97ceed342043230fe8b0a4f7fd6a3393103b0f) Included link to Rebol/MiniAudio extension version 1.0.0
* [`d4878d`](https://github.com/Oldes/Rebol3/commit/d4878d9a45e50351670337d14506a68db8134fe4) Allow custom `mold` callback for context handles to provide additional handle's info, if wanted
* [`a74b26`](https://github.com/Oldes/Rebol3/commit/a74b261bba1d22f55bd9500d99319c5bf3d6364d) Provide info, when molding a released handle, that it is already released
* [`b048bd`](https://github.com/Oldes/Rebol3/commit/b048bd7e3e240e17fc55922a0697c23b0a225eaa) Allow to pass handle objects to external handle's free function and dispose handle objects before freeing any series

### Changes:

* [`f5aaa5`](https://github.com/Oldes/Rebol3/commit/f5aaa582896ed192f0cd32ee2281ef331c88f019) Including RXA_UINT64 macro
* [`aab9d4`](https://github.com/Oldes/Rebol3/commit/aab9d4b98beb052a6eb4c3a6c0a6d985cfee7a3d) Extended possible number of extension command arguments to 15
* [`2c4565`](https://github.com/Oldes/Rebol3/commit/2c4565195e33d48d8437d238baa133234bf5f0c0) Extended possible number of extension command arguments from 7 to 11
* [`928761`](https://github.com/Oldes/Rebol3/commit/928761be6d7704952b52249d51514d7bd1e3c0f7) Using lower MiniAudio extension version
* [`b24999`](https://github.com/Oldes/Rebol3/commit/b2499948f88618a90fab055aa2f5c2452e83b4bb) Downloading extensions into `system/options/modules` directory instead of the current dir
* [`3f7972`](https://github.com/Oldes/Rebol3/commit/3f7972a0c3eddc40e309ef69ff9defb15ef5dd4b) Including `uint64` in the RXIARG value union
* [`c87321`](https://github.com/Oldes/Rebol3/commit/c87321e881b7e298fd3eb7fe46e7b497f7fe706e) Deprecated `system/options/module-paths` replaced with `system/options/modules`

### Fixes:

* [`e1cc08`](https://github.com/Oldes/Rebol3/commit/e1cc08c101b7c493c5c35344aa16034f6048b734) Correctly terminate truncated series
* [`af865e`](https://github.com/Oldes/Rebol3/commit/af865e2f3eb742c44375924d1e8a2e21482eae18) Included support for some rare codepages in `iconv`
* [`ea71e7`](https://github.com/Oldes/Rebol3/commit/ea71e78ac989cbf99036d577f8db84b2ba0c8694) Unreferenced local variable warnings
* [`f29b6b`](https://github.com/Oldes/Rebol3/commit/f29b6ba37a0c62248cbadd5e784b5f66f4f54ed2) Using correct module name (not using arch and `.rebx` extension) in case, that url to module contains file name
* [`c7d328`](https://github.com/Oldes/Rebol3/commit/c7d3281ca88865369012b6a8a1d32240f8783e9a) Crash when trying to initialise multi-dimension struct value
* [`9fffc0`](https://github.com/Oldes/Rebol3/commit/9fffc0b4c32892b66a3a790f2ceb03404764b3f5) Invalid function name when used help on function using a path
* [`54bb9d`](https://github.com/Oldes/Rebol3/commit/54bb9d9fd9f8dc233b31978c7976d861aa18e9e3) Properly implemented `List_Func_Types` function
* [`158080`](https://github.com/Oldes/Rebol3/commit/158080a4627575033cad833ac3195b40376aad37) Make the modules directory, if does not exists
* [`db8971`](https://github.com/Oldes/Rebol3/commit/db89718117f2e1cef0d74a6e58e930df34c87bd8) Lower security policy on the application data directory (when running a script)
* [`56eac4`](https://github.com/Oldes/Rebol3/commit/56eac4ef36ca940d599ad58155c2be1100a16670) Missing optional extension handle `mold` callback registration
* [`0837df`](https://github.com/Oldes/Rebol3/commit/0837df62d38f9b7dfbc729626c564e85c291c53f) Warning: pointer type mismatch
* [`48e772`](https://github.com/Oldes/Rebol3/commit/48e7726b668270d5ea65e4b3121f618becd61e10) Using proper type for extension handle mold callback function argument
* [`5413ff`](https://github.com/Oldes/Rebol3/commit/5413ff1b0b221626dcceb09848d08d6cb1ed69e8) `list-dir` stopping its output on symbolic link to not existing file
* [`922ac0`](https://github.com/Oldes/Rebol3/commit/922ac0654ffa3a9b5ce139693a98c1b584094d90) Use handle's context path access only if the handle is still in use (not already released)
* [`41a265`](https://github.com/Oldes/Rebol3/commit/41a26582c0a3fdf54462c292de4761fbc879d349) Correctly check for a handle's context flag on free

## 2023 August

### Features:

* [`93e07d`](https://github.com/Oldes/Rebol3/commit/93e07d61304cbbe019dbb19e713811a7e2cc0602) Added possibility to register extension handles with custom setters/getters
* [`3e53cf`](https://github.com/Oldes/Rebol3/commit/3e53cfde4efb308dffddb3a7bf638041309ed987) Handle SHIFT and CONTROL flags with arrow keys on Posix for char based terminal input
* [`4bcc98`](https://github.com/Oldes/Rebol3/commit/4bcc98d23b65223ceb215e815940e572b68161db) Simple `wait-for-key` function test file
* [`e1c7da`](https://github.com/Oldes/Rebol3/commit/e1c7da7f86ba72806381ae98c98772ac74e0e57c) Using `poll` instead of `select` to query events on Posix
* [`37145b`](https://github.com/Oldes/Rebol3/commit/37145b00d9729a6ed7bfe2b1a13571d54ceb60c0) Code reduction
* [`84ae96`](https://github.com/Oldes/Rebol3/commit/84ae9604c0d98eba0e21e912f872b573897c6d82) Key presses with unprintable chars are now reported as `control` and `control-up` events on Windows (on console with turned off line mode and from GUI as well)
* [`3f57b5`](https://github.com/Oldes/Rebol3/commit/3f57b5c619292d724d92dcce2afe45da7c300061) Preliminary support for char based input on Posix
* [`37971d`](https://github.com/Oldes/Rebol3/commit/37971d0e5582b7426772c09e8659e41cb8b44b78) Access to `Decode_UTF8_Char` core function from host's sources

### Changes:

* [`2f7998`](https://github.com/Oldes/Rebol3/commit/2f799831f83d21066cbc5e85230bea825af5badf) Debug print function `??` not throwing an error on invalid paths
* [`f4dd93`](https://github.com/Oldes/Rebol3/commit/f4dd931060cf459eeb74df976c078c9f79560b9a) Restrict `value?` to word arguments only
* [`ea3cd4`](https://github.com/Oldes/Rebol3/commit/ea3cd49398f22f40d080acd56274964457672410) Moved `wait-list` from `system/ports` to `system/state`
* [`efbfa4`](https://github.com/Oldes/Rebol3/commit/efbfa4e3c1d5447d7d0a278810862b68983b0bff) Moved `event-keys` and `event-types` from `system/view` to `system/catalog`, because these are used not only in GUI

### Fixes:

* [`ba824b`](https://github.com/Oldes/Rebol3/commit/ba824b1a789bab1a7a26f137fed4e0f99a873ed7) Allow extension context handles to reference other Rebol resources and so keep it safe from GC
* [`ad9863`](https://github.com/Oldes/Rebol3/commit/ad986332f018a15e39329267c24c32e4e7d0f327) Incorrectly molded NBSP char
* [`639ad7`](https://github.com/Oldes/Rebol3/commit/639ad74ad391668b156b9cb95c989510fb2f4edb) LOAD can't open JSON over HTTP
* [`511a4e`](https://github.com/Oldes/Rebol3/commit/511a4eab39a4f6d3f16619130c648482c89f751b) Unspecified calling convention fails in 32 bit build
* [`2cde09`](https://github.com/Oldes/Rebol3/commit/2cde09b0d160011b54c53712a7b75233d6e30e6f) Not all control paths return a value warning
* [`90fabd`](https://github.com/Oldes/Rebol3/commit/90fabdac2efdeaa2db663d8b0632cd4c48ca777d) Signed/unsigned mismatch
* [`227f8d`](https://github.com/Oldes/Rebol3/commit/227f8d5ec92ae28e65a01bbcd7cded4cc3900042) Unreferenced local variable
* [`f57e4f`](https://github.com/Oldes/Rebol3/commit/f57e4f23acda7f509f64a2c4fb0ab4676ab78243) Swapped location of the virtual key conversion table (host-event.c is not used in Base builds)
* [`d22923`](https://github.com/Oldes/Rebol3/commit/d22923343da7d85f7d1cbf62e3186044aa0d938e) Moving the newly introduced host lib function to the tail (to keep compatibility with old extensions)
* [`65a33d`](https://github.com/Oldes/Rebol3/commit/65a33ded795e8fef44aff457470de7038a5494af) Hide `X-goog-api-key` header field from HTTP logs
* [`bd91b5`](https://github.com/Oldes/Rebol3/commit/bd91b598d88fdfcc86cfba55312189895437d222) `mold` emitting invisible chars in some cases

## 2023 July

### Features:

* [`d423e9`](https://github.com/Oldes/Rebol3/commit/d423e91d6407e0f4e8d1e2508a20f324ddbd93e1) Support for optional Brotli encoding in HTTP scheme
* [`7d24ad`](https://github.com/Oldes/Rebol3/commit/7d24ad85e992b45208322e532401682fcc4e2393) Updated Spotify module to version 0.2.1 (test file moved to the test folder)
* [`1b74d4`](https://github.com/Oldes/Rebol3/commit/1b74d4d58c969d413f9ef971af02d3c0523df888) Updated Spotify module to version 0.2.0
* [`05b700`](https://github.com/Oldes/Rebol3/commit/05b700462061d9af2506d2b0bcef4e86928507be) Shortened code of `rejoin` function
* [`82682e`](https://github.com/Oldes/Rebol3/commit/82682e192e21724acc02ff684554b305aa62c09b) Enhanced `ajoin` native function for merging values into a string types
* [`fd5656`](https://github.com/Oldes/Rebol3/commit/fd5656dd19f1f0f1fcaa8e188f8e834a760b46df) Initial audio device prototype for Windows (using XAudio2)

### Changes:

* [`8eb643`](https://github.com/Oldes/Rebol3/commit/8eb64355ad3092dd3fcde5f4302db0f167d787f1) Modified verbosity of the `gzip` codec
* [`f49d96`](https://github.com/Oldes/Rebol3/commit/f49d969e28610014d5fb1fb0bbffb088373385b7) Renamed `Signal_Device` to `OS_Signal_Device`
* [`473bb0`](https://github.com/Oldes/Rebol3/commit/473bb097e0ef3810d38f13a6669a3b7ba1564f45) Updated Mbed-TLS sources to version 3.4.0

### Fixes:

* [`a7c2cc`](https://github.com/Oldes/Rebol3/commit/a7c2ccde3419376517ed6cd3c32388571c73260b) Include only included optional compressions in `system/catalog/compressions`
* [`786e33`](https://github.com/Oldes/Rebol3/commit/786e3334d9b0e66bb4527027ec9eb5a8d17d5b3f) `form` ignores leading empty strings and unset values
* [`8115be`](https://github.com/Oldes/Rebol3/commit/8115be79e5ea7b4b1342dc8e3a636e9e210badf9) Invalid console input
* [`03a2cd`](https://github.com/Oldes/Rebol3/commit/03a2cd0f4c8fc8c6cd1a34e19416460357b598a0) `)` not allowed as a first character in tag
* [`1a1706`](https://github.com/Oldes/Rebol3/commit/1a1706839b25f53b0a26faf237f25d0f01fc471a) Not able to use `list-dir` without any argument
* [`1712f2`](https://github.com/Oldes/Rebol3/commit/1712f28bb51710dd21488af62fe870f0ab72af7a) Removed forgotten probe
* [`8e5e1b`](https://github.com/Oldes/Rebol3/commit/8e5e1b3af89ddbb3b5f5d2dbe49168aa1fdce5cc) Removed unwanted line break after `ask` function call
* [`4e8d95`](https://github.com/Oldes/Rebol3/commit/4e8d95a621c670ce6611c2a8ef3afd16f75d3d5b) Enable wide character support on Posix
* [`8d7a6f`](https://github.com/Oldes/Rebol3/commit/8d7a6fe6b6ab264c783136491522f79fc44373b1) `~` folder name expansion does not work in all cases
* [`290720`](https://github.com/Oldes/Rebol3/commit/2907201e23d60b5a42a13c7a09e8862ea21e199b) Wrong directory names when diacritics is used
* [`dc1bcd`](https://github.com/Oldes/Rebol3/commit/dc1bcd05e23b6e1803c509bd7286dec366e34d3a) Suppress compiler warnings
* [`6af517`](https://github.com/Oldes/Rebol3/commit/6af517e4f5b7f2ff99de66c3dcc8ce0f68fb0e83) Not all control paths return a value warning
* [`908362`](https://github.com/Oldes/Rebol3/commit/90836286d180ee16dbae06ac8f6d6e9542b59bbc) Suppress compiler warnings
* [`2c23ea`](https://github.com/Oldes/Rebol3/commit/2c23ea097ff4cd6be9c5b959da037e5be1f376e9) Possible loss of data warnings
* [`ad1f2a`](https://github.com/Oldes/Rebol3/commit/ad1f2a0a2c628895a73fa5e3b81e8e58b1c4dc3f) Signed/unsigned mismatch warning
* [`23b8c8`](https://github.com/Oldes/Rebol3/commit/23b8c810323af9d2b3300fa9347a9ee36bc430a1) Unreferenced label
* [`5d036f`](https://github.com/Oldes/Rebol3/commit/5d036ff62879a84b55f0d015766f57998a58717a) Unreferenced local variables

## 2023 June

### Features:

* [`5d9af5`](https://github.com/Oldes/Rebol3/commit/5d9af51c244900b6187d3c3e408b069deb19368a) Dynamic function refinements
* [`869708`](https://github.com/Oldes/Rebol3/commit/869708abb961ba6872b5dc7103323aae14ce3df9) Optional support for Brotli de/compression
* [`311223`](https://github.com/Oldes/Rebol3/commit/311223c9487d8af88e4de2993644231610a6373a) Added possibility to set/clear BREAK, DATA-TERMINAL-READY and REQUEST-TO-SEND serial signals (on Windows so far)
* [`083c0d`](https://github.com/Oldes/Rebol3/commit/083c0dec04d31461510ede291a4f2e6a9a8b8f0c) Implemented `attempt/safer` to catch also thrown exceptions
* [`6edaaa`](https://github.com/Oldes/Rebol3/commit/6edaaa08d5dccf0e2801665291e367472c8d29c2) Implemented `try/all` to catch also thrown exceptions
* [`059643`](https://github.com/Oldes/Rebol3/commit/059643d3e79f2b207f4f46c0566632dbbfee5ab1) Allowed function for a recovery of a caught throw
* [`e3c224`](https://github.com/Oldes/Rebol3/commit/e3c224773bc9b19639756e4b6f9f0f4fd2495f9b) Implemented `catch/all` which may be used to catch all throws (named and unnamed)
* [`b91715`](https://github.com/Oldes/Rebol3/commit/b917152bccfd7828e8d29114b5a7ae1a34478210) New `/recover` option of the `catch` function
* [`808276`](https://github.com/Oldes/Rebol3/commit/80827617f6a508bd9af37b1d34e7505a51ce7a29) New `request-password` native which reads line with hidden input and avoids command history
* [`2494ff`](https://github.com/Oldes/Rebol3/commit/2494ff1d9b2b8be7d9d0c9127170a230b6201338) Dispatch file argument to `checksum` native to `file-checksum` function
* [`b91f64`](https://github.com/Oldes/Rebol3/commit/b91f6456dfd03ddfe4e17c97e1c71457d3326826) Included function `file-checksum` returning checksum of a given file's content
* [`b522a2`](https://github.com/Oldes/Rebol3/commit/b522a22e3bbe194bb53867de129538c2e990655d) Anonymize authentication tokens in log
* [`b6aedc`](https://github.com/Oldes/Rebol3/commit/b6aedca279a1c211d1cddb29316a98828d442b36) Direct conversion between date and decimal now uses Modified Julian Date
* [`483db9`](https://github.com/Oldes/Rebol3/commit/483db913fae92fe1033bb81d6bea307a215d1387) Conversion from date to decimal now returns Julian date as well as `/julian` date accessor
* [`b18dc5`](https://github.com/Oldes/Rebol3/commit/b18dc57d726d58642e78b077ba6c7ecbe2fce7a9) `to-date` mezzanine function accepting input in format like: `1-1-2000 00:00:00`
* [`0401ce`](https://github.com/Oldes/Rebol3/commit/0401ce5c4be3fbd3ef13b47e03622978c6a5afed) Allowed specifying MIDI devices using its names

### Changes:

* [`467ae8`](https://github.com/Oldes/Rebol3/commit/467ae8d5e587a020d819b2d2612214e8e6d6ab51) Using more lightweight dynamic function refinements instead of `either` where possible
* [`a14bf5`](https://github.com/Oldes/Rebol3/commit/a14bf53047e09e7588dd4bd37c2127da429a1c8b) Using more lightweight dynamic function refinements instead of `apply` where possible
* [`fbbb20`](https://github.com/Oldes/Rebol3/commit/fbbb200dbb86975173b1b0d66b2ff6c5ab73418a) Value of false dynamic function refinement is `none`, not `false` (as native code expects `none`, which is cheaper to compare)
* [`3fa4a0`](https://github.com/Oldes/Rebol3/commit/3fa4a0b63c21e739e0ac2a7e4ed41a03e744acbf) List Brotli compression in `system/catalog/compressions`
* [`3d23ed`](https://github.com/Oldes/Rebol3/commit/3d23ed3b26247109ac8a9763d9ac92f01f45e274) Using lower default compression level for Brotli compression
* [`6ed8da`](https://github.com/Oldes/Rebol3/commit/6ed8da9122686fb88cf1c371ee265f0c79c40f59) Deprecated `try/except`. Using `try/with` instead.
* [`519fd4`](https://github.com/Oldes/Rebol3/commit/519fd4df4844cae73c437adccbfa141b9223c81e) Using `catch/with` instead of `catch/recover`
* [`0cb8bd`](https://github.com/Oldes/Rebol3/commit/0cb8bda3f920612d0eac35f9584b1de33054e235) Include JSON codec in Core builds
* [`a2ac18`](https://github.com/Oldes/Rebol3/commit/a2ac189e13778010e52d1fc982b8272b3ddac80d) Include date mezzanine functions in Core builds
* [`58b604`](https://github.com/Oldes/Rebol3/commit/58b60433493bb0c3466b6db933c9f7daf7e9d54e) Not using HostileFork's CPP casting macros
* [`8cfcc7`](https://github.com/Oldes/Rebol3/commit/8cfcc7ab0fdfe063ab453e10e71d018e0a3a3ac5) Moved the event port location from `system/view/event-port` to `system/ports/event`

### Fixes:

* [`5fdac4`](https://github.com/Oldes/Rebol3/commit/5fdac4c9c4216a5bb4dc7970471037ccea4cbef7) Regression (code reduced too much)
* [`777da7`](https://github.com/Oldes/Rebol3/commit/777da715bb39ddbe30cf9bce560019c9b00c4f37) Non-void function does not return a value in all control paths
* [`8d4fed`](https://github.com/Oldes/Rebol3/commit/8d4fed3980f2c04c70e78d9643df190c1ded39b0) Variable used without being initialized
* [`4dc5b4`](https://github.com/Oldes/Rebol3/commit/4dc5b417ea8b9344aef824b1f8abc69687efbc98) Path evaluation does not process THROWN values as THROWN
* [`842939`](https://github.com/Oldes/Rebol3/commit/84293975eced8a2815707e6702913b215a4c662e) MAKE ERROR! does not process THROWN values as THROWN
* [`7e2585`](https://github.com/Oldes/Rebol3/commit/7e258561b2c7273639a17c99918b24731bd2f085) Better error messages for not caught throws
* [`6e0011`](https://github.com/Oldes/Rebol3/commit/6e00115ccab18b81406e76fdf1d682a297b79b9c) Using wrong constants
* [`2bbd3e`](https://github.com/Oldes/Rebol3/commit/2bbd3ebd511dd98d5cbb056a998c4db7eb5113a3) `catch/quit/name` catches only quit or halt, but not any named throw
* [`0605fd`](https://github.com/Oldes/Rebol3/commit/0605fd57a8ded92fce095071b3d16a427748d05e) Equality of objects with hidden values
* [`dc85ac`](https://github.com/Oldes/Rebol3/commit/dc85ac5f0bbf9b7c2621b9ce543ebeeef62d7019) Regression in `1.#NaN` comparison
* [`8bb611`](https://github.com/Oldes/Rebol3/commit/8bb6115bcbe2b0faf5bc9e8826d73e1682a49668) Proper handling wide Unicode characters in console input on Posix systems
* [`e5219e`](https://github.com/Oldes/Rebol3/commit/e5219e8f8637240a0f8259e03542db71a6dce95d) Code cleanup
* [`bbbabb`](https://github.com/Oldes/Rebol3/commit/bbbabb0dc803a862d9838f041fdcc96f921a4aed) Simplified `request-password` native code and fixed Win32 version
* [`ce2711`](https://github.com/Oldes/Rebol3/commit/ce2711a77a6798c487ca1b4fd908ca013f722d3d) Allow releasing a system user with `none` value
* [`7c96ba`](https://github.com/Oldes/Rebol3/commit/7c96ba54fa4c53d537b9b37d71cef26d9c94d46e) Potentially uninitialized local pointer variable
* [`8d4879`](https://github.com/Oldes/Rebol3/commit/8d4879f2527bf7267045a100fd7f9d21b4a22855) Throw an error when `checksum` is used with refines on a file argument
* [`b62428`](https://github.com/Oldes/Rebol3/commit/b6242808cfc44c3cfbad8666a9ba2e21b031b2c6) Checksum port crashes when input is not a series value
* [`91f574`](https://github.com/Oldes/Rebol3/commit/91f574ba4757cb4ef2ba388d74703af055c94527) `parse/case` parsing words and paths
* [`44e7f8`](https://github.com/Oldes/Rebol3/commit/44e7f80cd9e21ae6a10541e5e8b15470098d7675) Invalid output when converting date to integer without a time value
* [`a5c247`](https://github.com/Oldes/Rebol3/commit/a5c247982ec5cb36b5cef06c75bd3bc0e36fa5da) Incompatible-pointer-types-discards-qualifiers warning
* [`828473`](https://github.com/Oldes/Rebol3/commit/828473fb3a7e790b12be34029baeef1587dc63d2) Provide error numbers instead of hard-coded traces from the MIDI device
* [`cae3ef`](https://github.com/Oldes/Rebol3/commit/cae3efe160d4731630caa54b16f8ad344ba0e476) Changes of the MIDI setup not recognized on macOS
* [`9dbc8e`](https://github.com/Oldes/Rebol3/commit/9dbc8e9cc8e7801e9701c8ac2ec1f91d043a8f28) MIDI implementation for macOS so it works same like on Windows
* [`952d76`](https://github.com/Oldes/Rebol3/commit/952d76f8c239d469222523b4abaab8ae075f6622) Ignore the 3rd MIDI message byte in the MIDI test (it can have value on macOS)
* [`67e519`](https://github.com/Oldes/Rebol3/commit/67e519ad82a2d7578ab2f423b6d6a79cb06204ea) MIDI port using old path (test script included)

## 2023 May

### Features:

* [`8319d5`](https://github.com/Oldes/Rebol3/commit/8319d5cc77f4930e1a44112c6f1b680e7c821e0e) Special integer notation (bit, octal, decimal and hexadecimal)
* [`eeb381`](https://github.com/Oldes/Rebol3/commit/eeb381ec3910d471b5e7dd0323ee098e6db24129) Allow `gob!` to be used as a source for `move` function
* [`df0907`](https://github.com/Oldes/Rebol3/commit/df09076d1cd1b8597c5d9f36554396e297ece8ad) Simplified enbase-64 padding code
* [`422ac0`](https://github.com/Oldes/Rebol3/commit/422ac0f6618d11c59ee843016b7050bc72988f47) Store user's persistent data before releasing existing user

### Changes:

* [`11fc8b`](https://github.com/Oldes/Rebol3/commit/11fc8b250adb629eefdf39f520b5528ce9e83936) Make `halt` catchable
* [`7bce10`](https://github.com/Oldes/Rebol3/commit/7bce104413f263059f1327099ea3a2ba1538cc8d) Removed the `quit/now` option
* [`476a74`](https://github.com/Oldes/Rebol3/commit/476a74bc52481766d861bc1dcba4f2eb4e5793cd) More aggressive `recycle` native
* [`8657fb`](https://github.com/Oldes/Rebol3/commit/8657fb981b80a00c4e8f42a1e5c17095d24189bc) Do not delay JSON codec module as it is used in HTTP protocol

### Fixes:

* [`fd780c`](https://github.com/Oldes/Rebol3/commit/fd780c541ebb45a7968e69667bfcfa96bac9f7f3) `access-os` function was missing because of the old native definition used
* [`d063af`](https://github.com/Oldes/Rebol3/commit/d063af761e1c7617ae788456954a986d91dde600) Corrupted output from `trace` on Posix systems
* [`0c4fad`](https://github.com/Oldes/Rebol3/commit/0c4fadfd06f93104b6e201d1c57ac2ec758c79d2) Missing string definition for `map`
* [`de59c1`](https://github.com/Oldes/Rebol3/commit/de59c1f7c704a62a343bdd4937d70f6e36094785) Finer decimal `equiv?` comparison than `equal?`
* [`ddcedf`](https://github.com/Oldes/Rebol3/commit/ddcedfe4c0c78e90a97c68f841409ce14ec2aaed) Do not pad output when used `enbase/url`
* [`cbd626`](https://github.com/Oldes/Rebol3/commit/cbd6262772335cd8b1b82c1f3290b4c12e60e111) Correct decoding escaped Unicode chars in the JSON codec"
* [`c77e99`](https://github.com/Oldes/Rebol3/commit/c77e99513cded993810b3880833fff8723daefdf) Correct decoding escaped Unicode chars in the JSON codec
* [`e92f1f`](https://github.com/Oldes/Rebol3/commit/e92f1f52c54519e446a1b58adc5aad80be1f20d7) Correctly handle not existing `user` argument for the `set-user` function
* [`107df0`](https://github.com/Oldes/Rebol3/commit/107df095f5343a091b7dd4766fe101c1054abd50) Handle decoding escaped Unicode chars in the JSON codec

## 2023 April

### Features:

* [`4d9318`](https://github.com/Oldes/Rebol3/commit/4d93180f5ef862229e9f2e88ecede95ba0db8881) Including optional support for serial device as was used in the Atronix's branch

### Fixes:

* [`a65ef4`](https://github.com/Oldes/Rebol3/commit/a65ef43c71ac53c847c90c2ac84caceef0317e1c) Posting large data over TLS
* [`6c458f`](https://github.com/Oldes/Rebol3/commit/6c458fda398beeef644056e47aef97afbd4387c5) Typo in previous commits
* [`b0412a`](https://github.com/Oldes/Rebol3/commit/b0412a6ce746f01bab0afcab2012b593114e7102) Posix error: too many arguments to function ‘Open_StdIO’
* [`d87348`](https://github.com/Oldes/Rebol3/commit/d87348290c788be185383d17a88f9618ccd1fec5) Don't set `READ_LINE` mode when started with `--cgi` option
* [`358b73`](https://github.com/Oldes/Rebol3/commit/358b734dc8a8abdda70a94b799624faff1e8b1b1) Removing trailing LF from console's input tail on Posix only when in READ_LINE mode
* [`96eb7d`](https://github.com/Oldes/Rebol3/commit/96eb7da28db4bcf5c1ae8a2d5da973f02f0b718d) Handle 1.INF and 1.#NaN cases of its percent type
* [`be1edd`](https://github.com/Oldes/Rebol3/commit/be1edd25db0b959f7ddaa234adcabd629f893280) Dealing with index past tail scenarios
* [`b47bbd`](https://github.com/Oldes/Rebol3/commit/b47bbdee7e18e12679a16513819cb5d9386ba87a) Missing path value in serial port's specification
* [`122d3d`](https://github.com/Oldes/Rebol3/commit/122d3df5293fd32d352114ae1851042a4f55e188) Including serial port related files taken from Atronix's branch

## 2023 March

### Features:

* [`3761b7`](https://github.com/Oldes/Rebol3/commit/3761b7d3a88c9e212fe4bdb0a9bbd856128f465e) Allow use of series index in parse's `remove` command
* [`7f46bc`](https://github.com/Oldes/Rebol3/commit/7f46bc6c1b92341c58a9b374a939853aae5abff4) Implemented a raw string syntax in a form `%{...}%`
* [`9fa0f5`](https://github.com/Oldes/Rebol3/commit/9fa0f546117d6c38d77365063296fdf55a5115da) Native for computation of a difference between two images
* [`dd1807`](https://github.com/Oldes/Rebol3/commit/dd1807dc3979dceef4e0aaeaa3800d713c030be9) Native for human perception weighted Euclidean distance between two RGB colors
* [`dbdb4d`](https://github.com/Oldes/Rebol3/commit/dbdb4d39502ad32855b154c2999feeef66f7b841) Allow comments in automatically collected REBNATIVE code specifications
* [`d7d85c`](https://github.com/Oldes/Rebol3/commit/d7d85cef7fc2119e9f2a6e3899a29fb52b5c4fbc) Included link to Rebol/BlurHash extension version 1.0.0

### Fixes:

* [`73a481`](https://github.com/Oldes/Rebol3/commit/73a481eef14f5eb109144ecad17252218ab0cea0) TCP connection not working on Haiku OS

## 2023 February

### Features:

* [`8a44ea`](https://github.com/Oldes/Rebol3/commit/8a44eaeea79a7a8879af2b011a2f9f6cbcb7b159) Implemented passing a struct value to an extension

### Changes:

* [`9c9fef`](https://github.com/Oldes/Rebol3/commit/9c9fef14719c5b9aaa356260ac4108984117e6d6) Updated mbedTLS code to version 3.3.0
* [`9a327d`](https://github.com/Oldes/Rebol3/commit/9a327d3aea2106e9b4584965baf26134402d3e58) Simplified use of TO_<os> defines and included Linux armv6 targets (and using armv7 instead of arm32 now)
* [`6e7fa5`](https://github.com/Oldes/Rebol3/commit/6e7fa57c93b1510e230dbaa5bff7830dc3e3952f) Including `system/build/os-version` information (if known)
* [`1dbefc`](https://github.com/Oldes/Rebol3/commit/1dbefcdf4b3730e9737bf04bc71ad57cae477a38) Using `system/platform` instead of `system/build/os` to get an extension url
* [`0d5d91`](https://github.com/Oldes/Rebol3/commit/0d5d91ec4a6955a3d89cd66a9ce209efa84750ab) Better information in `system/build/os` about build on Linux
* [`759609`](https://github.com/Oldes/Rebol3/commit/759609ad00f30a6c512fedb8196af85a1c4a26bf) Modified version info
* [`937c97`](https://github.com/Oldes/Rebol3/commit/937c972c9deb0433597fc48fe06af234501f6bdf) Included `system/build/libc` info on systems where available
* [`555f89`](https://github.com/Oldes/Rebol3/commit/555f89ef710bc36e31782ea63c65898c1be25ae1) Display directories before files in the `list-dir` output
* [`f5016c`](https://github.com/Oldes/Rebol3/commit/f5016c11e91a9c7e082d7380524b0b3f1451c0a5) JSON codec code cleanup
* [`3faac7`](https://github.com/Oldes/Rebol3/commit/3faac7d016b1d45dc77d7f5ee032c3e77adb826a) Including download link to Rebol/MathPresso extension
* [`5f4f43`](https://github.com/Oldes/Rebol3/commit/5f4f43414f50daedc5d3247c1b473a6e78eddb69) Replaced crash report url

### Fixes:

* [`f5f6ae`](https://github.com/Oldes/Rebol3/commit/f5f6aee5706eb153370f7b3982c1014a669f60ab) Make sure that os-info is always loadable
* [`4070cf`](https://github.com/Oldes/Rebol3/commit/4070cfc30c71fb90aa54a39a82c2fa21a935654e) Missing define from the previous change
* [`b99a78`](https://github.com/Oldes/Rebol3/commit/b99a783349c4c6113299346344da1942e5d2ce95) 32bit builds crashes when struct is being recycled
* [`de73f7`](https://github.com/Oldes/Rebol3/commit/de73f73bfb1f7375190589c7a66411c9e84c5cfd) Crash when resolving file name containing a colon char on Posix systems
* [`1bc5cd`](https://github.com/Oldes/Rebol3/commit/1bc5cdba7dc16fbd86f5a8fe7162fafe9641bc04) Avoid overwriting `sys`
* [`417992`](https://github.com/Oldes/Rebol3/commit/417992081c5608cc55c07df3a98f858bc67e750c) Make `delta-time` a little bit more precise
* [`29ada6`](https://github.com/Oldes/Rebol3/commit/29ada63f27d702b0f4c707e4ca173df3c122fc50) Regression related to the recent series changes

## 2023 January

### Features:

* [`5b6e84`](https://github.com/Oldes/Rebol3/commit/5b6e8404d019b898fff296a1fdc49dd0c877037b) Including API_EXPORT define for the external extension use
* [`43c02f`](https://github.com/Oldes/Rebol3/commit/43c02f95d29485feb470181de718bb0aea454023) Handle's context accessible from an external extension
* [`e6c778`](https://github.com/Oldes/Rebol3/commit/e6c778cf07823804548ede0213d100561a268d40) New script which amalgamate Rebol extension related include files into a single one

### Changes:

* [`4ef198`](https://github.com/Oldes/Rebol3/commit/4ef1980812fe921c233f409cfe6d3cf5b7833494) Enhanced number of possible series flags from 8 to 32 (could be lowered in the future and use some of these bits for something else)
* [`a27822`](https://github.com/Oldes/Rebol3/commit/a27822c3b0a6f13ff55f2f6f086d75ebc981c912) Including download link to Rebol/Easing extension and updated Rebol/SQLite
* [`e34c37`](https://github.com/Oldes/Rebol3/commit/e34c3749439605b3a4993a103875e400ace7f4ea) Updated ZLIB code to version 1.2.13
* [`71e7aa`](https://github.com/Oldes/Rebol3/commit/71e7aa976f5ef82727c0abbb94420dd2b0f9f87b) Not using _To_Upper_Case and _To_Lower_Case functions in debug buids
* [`053f84`](https://github.com/Oldes/Rebol3/commit/053f84ef387313772f195582a639bee092a6d719) Necessary changes related to having `import`'s module argument not evaluated
* [`bd09ce`](https://github.com/Oldes/Rebol3/commit/bd09ce0ed5ab3a4b4aa9ce6312b6406d5e096599) Using lit-word as a `module` argument of the `import` function, so one can just use `import module-name` (no need to use lit-word for the module's name)
* [`79c3e6`](https://github.com/Oldes/Rebol3/commit/79c3e6c32ac56624d09918ea8c2d61a8ce7cfb5c) Associate `p7s` file suffix with the `DER` codec
* [`7680e4`](https://github.com/Oldes/Rebol3/commit/7680e4c335cb7a7545b3e6e0b2637568b0822ff7) Allow plain words as a log `id` argument

### Fixes:

* [`4bac74`](https://github.com/Oldes/Rebol3/commit/4bac74eed0c33b5ae343a58988d4b6df69bfccd0) Removed unnecessary casting
* [`ee9b2a`](https://github.com/Oldes/Rebol3/commit/ee9b2a5e177fb993793c08c0d2c7fb44e2408d98) Using `read` with a wildcard path without any result was throwing an error on macOS instead of an empty block
* [`8f9e3b`](https://github.com/Oldes/Rebol3/commit/8f9e3bfbecf319c24dc8a5e975b8ec9d94186f9a) Regression in `crush` compression
* [`db3818`](https://github.com/Oldes/Rebol3/commit/db38189cfca5228823402140d5691414d02ac97b) Invalid data in loaded images with an alpha channel on macOS
* [`ff5e13`](https://github.com/Oldes/Rebol3/commit/ff5e131f6b376bc51edecc65ae520893315aff98) Safely handle a case when path to the exe is not resolved
* [`0cdee8`](https://github.com/Oldes/Rebol3/commit/0cdee8cbb26ad1143a99611a090822314ef4f6a5) Regression in importing an unnamed module
* [`3725ef`](https://github.com/Oldes/Rebol3/commit/3725ef700f30c850da283dec7684d261974249de) It was possible to modify a hidden value using `extend`
* [`78f683`](https://github.com/Oldes/Rebol3/commit/78f683bf542313def17d1c008e2aabb042de39ab) Regression in debase 36
* [`eae5e6`](https://github.com/Oldes/Rebol3/commit/eae5e64dbc667cb47c4c66e5815ce47bca98d670) The `info` struct member is now called `sizes`
* [`6fb6e0`](https://github.com/Oldes/Rebol3/commit/6fb6e0f6bee975c7d8009409fbb41c42a617300b) Disallowed loop variable context access
* [`8503e0`](https://github.com/Oldes/Rebol3/commit/8503e07d58a8a1f91cb06806d047a0b26ceed233) Clang warning: function declared 'noreturn' should not return
* [`7af6eb`](https://github.com/Oldes/Rebol3/commit/7af6ebe5009f83afca6e299f44d9a0f965c0730a) Crash when using `foreach` with a set-word and values not in a block
* [`87e010`](https://github.com/Oldes/Rebol3/commit/87e010b9adca5be997836fe1b34786620e4f6131) Not using `REB_NORETURN` define inside a struct
* [`13e361`](https://github.com/Oldes/Rebol3/commit/13e361a4a37ba571d7813b66241e6422e496a7a3) Mark OS_Exit and OS_Crash functions as `noreturn`
* [`0a0757`](https://github.com/Oldes/Rebol3/commit/0a0757e359a1e6667c18d99e4314ef1931edaa3e) Warning: unreferenced local variable
* [`f7a9e9`](https://github.com/Oldes/Rebol3/commit/f7a9e94c92da9f6b107b9846633f6ae80e880e60) Warning: 'puts' undefined; assuming extern returning
* [`a14b4b`](https://github.com/Oldes/Rebol3/commit/a14b4b70c9cfee2dcc6a47861ee4dd36ae2b140f) Allow bincode/init size value to be zero
* [`dc9401`](https://github.com/Oldes/Rebol3/commit/dc9401a5e5c86b2fb19fac05533897d2247e491d) Warning: 'puts' undefined; assuming extern returning
* [`149789`](https://github.com/Oldes/Rebol3/commit/149789c4d99fb344617ef83f6fbd3bcba3c70b44) Many unsigned/signed variable conflict warnings
* [`1742fa`](https://github.com/Oldes/Rebol3/commit/1742fa869c16d6066f2c37eeb3de9dc288765f7d) Warning: differs in levels of indirection
* [`6e8f18`](https://github.com/Oldes/Rebol3/commit/6e8f18d18c1a5eb3337c99fe0854b2fcfd0eae77) Warning: unreferenced local variable
* [`e28909`](https://github.com/Oldes/Rebol3/commit/e2890977bba7d3e28e9c967c85a590cbcdd6d732) Not including winerror.h as it redefines IS_ERROR
* [`52cf02`](https://github.com/Oldes/Rebol3/commit/52cf025514e9e7fe474ce670ec76bfbca515f735) Warning: unreferenced local variable
* [`e86310`](https://github.com/Oldes/Rebol3/commit/e863106991b85f2e6f240130e07a40f6a3195a32) Msvc warning: conversion to a greater size
* [`6d914c`](https://github.com/Oldes/Rebol3/commit/6d914c970ab82cb97b551242e15bd142af21ff0e) Possible use of uninitialized variable
* [`cacaf5`](https://github.com/Oldes/Rebol3/commit/cacaf51bac24dff074f7202a9016e9b6a9984b34) Avoid redefinition of typedef warnings when using amalgamated extension includes with `-std=c99` compilation flag
* [`e7273e`](https://github.com/Oldes/Rebol3/commit/e7273ea4df84184ffa40b55a24eb4b0023046d11) Have reb-args.h included also in external Rebol extensions
* [`4ebb17`](https://github.com/Oldes/Rebol3/commit/4ebb17a5820cb8bf1e0c1a303a42f3b04235b90a) Make sure that there is REBARGS define as a part of extension include
* [`b5179c`](https://github.com/Oldes/Rebol3/commit/b5179c9cbb3359796e0a4da7f52f64e755fb3fc5) RL defined in host-lib.c file, in all embedded extensions it must be used as extern
* [`47b542`](https://github.com/Oldes/Rebol3/commit/47b542729b9b03f4bc1663fb0d71b49b91048f07) RL defined in host-lib.c file, in all embedded extensions it must be used as extern
* [`16a4ea`](https://github.com/Oldes/Rebol3/commit/16a4ea8b15cadc4f995cc5fc4a69b37e174ac537) Silent compiler's warnings
* [`6d7fa4`](https://github.com/Oldes/Rebol3/commit/6d7fa43060032d45dbcda7340f6c0387685664f4) Warning: incompatible pointer types passing
* [`a12dd9`](https://github.com/Oldes/Rebol3/commit/a12dd92f3a8813ad45a11ec0d4619b73ead14320) Warning C4018: signed/unsigned mismatch
* [`982a42`](https://github.com/Oldes/Rebol3/commit/982a420727a7364064c2e68ecb4c585cf04a97b1) Warning: incompatible pointer types passing
* [`41a828`](https://github.com/Oldes/Rebol3/commit/41a8288d03b1ef9b94403bb92e52f10ee790c36a) Don't redefine MIN/MAX
* [`5eb72e`](https://github.com/Oldes/Rebol3/commit/5eb72eda0ea243be01af982343445aca0778c44d) Silent dereferencing NULL pointer warnings
* [`287398`](https://github.com/Oldes/Rebol3/commit/287398beb052bbaf5dafefe32df972c107407f19) Clang warning: implicit conversion from 'long long' to 'double' changes value from 9223372036854775807 to 9223372036854775808
* [`e347ad`](https://github.com/Oldes/Rebol3/commit/e347ad088d37ecf4afd80d152ce1bff04bab8ad5) Using extern link to RL_LIB in the test extension (GCC was failing without it)
* [`c337fe`](https://github.com/Oldes/Rebol3/commit/c337fe1c4cd8aadadd0b2986d551a46df1b5ddb7) Warning C4003 (not enough arguments for function-like macro invocation)
* [`b95505`](https://github.com/Oldes/Rebol3/commit/b95505310a209125cd82e92ab4f9800aaf324a96) Warning C6308 (possible memory leak)
* [`361b4d`](https://github.com/Oldes/Rebol3/commit/361b4d6037417503a576c29158a54ce2500d1f6c) Silent "The prior call to 'wcsncpy' might not zero-terminate string" warnings
* [`3f3bea`](https://github.com/Oldes/Rebol3/commit/3f3bea809c32c9175d7a60495abdb7d66f100bd6) `find/any` failing in some cases

## 2022 December

### Features:

* [`5337c8`](https://github.com/Oldes/Rebol3/commit/5337c81d6f92a301a73eda059f31c5e0c1dbdd9b) New `swap-endian` native function for swapping binary byte order
* [`1b758a`](https://github.com/Oldes/Rebol3/commit/1b758a2652d516e4de233a78920dbe466ae43143) Register `.docx` file suffix with ZIP codec
* [`9c128d`](https://github.com/Oldes/Rebol3/commit/9c128dc365eb941291210ebdc01d7efd2c3fc74e) Added possibility to keep a track of a Rebol series in a handle's context (guarding this series from GC as long as the handle is referenced)
* [`36828a`](https://github.com/Oldes/Rebol3/commit/36828a25be19a5fd9f9e943607121667c4127abb) Fine tune memory pool sizes
* [`783309`](https://github.com/Oldes/Rebol3/commit/783309eed90523d69f6a978843ef4512927811ed) New `truncate` native for shortening a series
* [`c51876`](https://github.com/Oldes/Rebol3/commit/c51876865cef9106aff51094708021e2a854bbc9) Using built in WebSocket handshake function in the test server
* [`bae6cf`](https://github.com/Oldes/Rebol3/commit/bae6cffe97e662ec9a727a01cdcc1fdd61455ea6) Minimalistic WebSocket support in the `httpd` scheme

### Changes:

* [`f94ebb`](https://github.com/Oldes/Rebol3/commit/f94ebb9e846785248c97afbf3b5d3724e817b8eb) Bitwise operation on binary: the shorter argument is used repeatedly

### Fixes:

* [`76cb96`](https://github.com/Oldes/Rebol3/commit/76cb968402698902698713529397064ea84f039f) Visual Studio compiler not knowing `uint*_t` types
* [`de9a63`](https://github.com/Oldes/Rebol3/commit/de9a63cfccb9bfc43f18c9e7c0263de679b3d5d4) Warning: incompatible pointer types initializing
* [`e8a0f7`](https://github.com/Oldes/Rebol3/commit/e8a0f78f38418c6d2039eff930592bde516b2736) Warning: format specifies type
* [`bda36a`](https://github.com/Oldes/Rebol3/commit/bda36af8f39ab0c2f47c84422f04efdde633512b) Warning: incompatible pointer types assigning
* [`be4656`](https://github.com/Oldes/Rebol3/commit/be4656bfa7b0b2c8284aff5c224cd2343ea7c090) Warning: non-void function does not return a value
* [`05f7f9`](https://github.com/Oldes/Rebol3/commit/05f7f97fff2aeb53817dd45e701afcc0ea8ff01e) Warning: incompatible integer to pointer conversion
* [`aeeb6a`](https://github.com/Oldes/Rebol3/commit/aeeb6a3c1f68d014dfdd8b011a516a2a369b50f0) Pair's `area` was not correctly computed for decimal values
* [`93b62c`](https://github.com/Oldes/Rebol3/commit/93b62c07eac9ace4a5304c50c52787a24e0253ee) Warning: incompatible-pointer-types-discards-qualifiers
* [`a1f8e8`](https://github.com/Oldes/Rebol3/commit/a1f8e85cc93089d5db138659023bca82005c2c98) Warning: for loop has empty body
* [`235ade`](https://github.com/Oldes/Rebol3/commit/235adec2c3b9446ee8e3be79a88d2fb8f67aa383) Warning: expression result unused (in unused asserts)
* [`dba08b`](https://github.com/Oldes/Rebol3/commit/dba08b444c606554a869c1a70710e9338cbf85fd) Regression: failed boot of BASE build versions. Added tests to the Github workflow.
* [`4d3dd3`](https://github.com/Oldes/Rebol3/commit/4d3dd34059f45b5d099b7d4b9d4b3114530d762d) Creating a directory with a name containing Unicode chars on Posix platforms
* [`931ddf`](https://github.com/Oldes/Rebol3/commit/931ddfb954fd832b37d5a8788981f397713c1526) Missing include when used `SHOW_SIZEOFS` define
* [`fb8a03`](https://github.com/Oldes/Rebol3/commit/fb8a03a9b93958d88097dffa8d5fcd090f015850) `httpd` scheme: correct timeout; updated header
* [`269032`](https://github.com/Oldes/Rebol3/commit/269032dbb7d721c4279f678cc1feaa83a940ae07) Unnecessary series initialization
* [`31e72e`](https://github.com/Oldes/Rebol3/commit/31e72ec44b8ec2bdac7e86db80a617ea485caa38) Block-like series must be terminated with the END value

## 2022 October

### Features:

* [`0695bf`](https://github.com/Oldes/Rebol3/commit/0695bfc84a86fe02cad9e47da79abce4499d77fc) Pair's `area` accessor

### Fixes:

* [`34df08`](https://github.com/Oldes/Rebol3/commit/34df085081f4e162ccea2f0d6feed12c9ab2afe3) `find/any` returning incorrect result
* [`9a63e8`](https://github.com/Oldes/Rebol3/commit/9a63e8899d3f84f337af9c9fab86fb18cde9f909) Not available image size when passed in to an extension (regression)

## 2022 September

### Features:

* [`bcaa10`](https://github.com/Oldes/Rebol3/commit/bcaa10870fafceab35abf3f50c5035db317c765e) Implemented set operations on `map!`
* [`603035`](https://github.com/Oldes/Rebol3/commit/603035402c14d8f3eef07f804b0ea99b1f382011) Optional LZW compression method
* [`f8f551`](https://github.com/Oldes/Rebol3/commit/f8f551001069cff83493ed7e70ca6f9bfe61e004) Enable loading exact frame from an image on macOS
* [`381337`](https://github.com/Oldes/Rebol3/commit/381337cd3729250d1e94bf2cbbbd7f9c47f8f987) Cleaned `system/catalog/datatypes` initialization
* [`dc48b3`](https://github.com/Oldes/Rebol3/commit/dc48b33e1b37da706e739527c92c52ec8e440dea) Small code optimization (removing unnecessary SERIES_FULL check)
* [`469ba7`](https://github.com/Oldes/Rebol3/commit/469ba799d59283ff275343b4f35579d5f7c82c0a) New Bincode dialect `CROP` command for shrinking buffers from the left side at the read buffer index position

### Changes:

* [`2b5318`](https://github.com/Oldes/Rebol3/commit/2b5318964c80f98c37b9546afcca24286366834b) Don't allow binary input for set operations (does not work anyway)
* [`421b2f`](https://github.com/Oldes/Rebol3/commit/421b2fed59bcca81940c187bf675434fea7828fa) Updated ZLIB code to version 1.2.12
* [`972dcc`](https://github.com/Oldes/Rebol3/commit/972dcc003fc57480e2b4cd9864234ea65bc28c82) Not setting `datatypes` value in the library context
* [`587c22`](https://github.com/Oldes/Rebol3/commit/587c22362d1b981c2048b83877fc72bcff8998bd) Shorter SMTP title and added one more log message
* [`646cb4`](https://github.com/Oldes/Rebol3/commit/646cb49d4ece9230523f78245a213da1a779a9d3) Using same error type for `to-issue "a a"` and `to-issue "a^Aa"`

### Fixes:

* [`07e09a`](https://github.com/Oldes/Rebol3/commit/07e09a248dedb760e1724c740a5d6147ab8cc6ae) `union/skip` not working correctly in some cases
* [`00f44f`](https://github.com/Oldes/Rebol3/commit/00f44fa2b3a99f0c8987371a09f6a664a01091e9) Codecs/zip/verbose ignored by encode
* [`2f601f`](https://github.com/Oldes/Rebol3/commit/2f601ff2f9068560171898cee41808b3277484a2) TAKE/part of offset gob! returning contents from head of gob!
* [`22b822`](https://github.com/Oldes/Rebol3/commit/22b8224e58b2c1ef4c7a2c878fc321772fd4c727) Lost index information when passing image value between core and extension
* [`d915c0`](https://github.com/Oldes/Rebol3/commit/d915c0aa0c44fbf4292a764782ce734cc1b96bc2) Better port validation when dealing with native port actors
* [`664904`](https://github.com/Oldes/Rebol3/commit/6649047f5cd78aaeb3de1c2e0e3b58f93e5bcc85) Loading unnamed modules was causing failing test
* [`aacacc`](https://github.com/Oldes/Rebol3/commit/aacacc9190f61999e05a1eb91b940addfcecfec7) Incorrectly dealing with vector item's width
* [`22f793`](https://github.com/Oldes/Rebol3/commit/22f79314dcb1fd9c1da584eb1fda8536a445d83e) Reverted leaked unwanted modification in the previous commit
* [`115099`](https://github.com/Oldes/Rebol3/commit/115099497f70cd392f9f8e52f8590833da4a0a0f) Fixed regression with `break` in a `reduce` block


## 2022 August

### Features:

* [`b96911`](https://github.com/Oldes/Rebol3/commit/b96911198472de2af414c9e26caace768eacde64) Added `switch/case` option to make it use case-sensitive comparisons
* [`24faca`](https://github.com/Oldes/Rebol3/commit/24faca96ffe7882a0c6ab82354035f646e4aaa4e) Implemented `reverse` on a `vector!` value
* [`8951d9`](https://github.com/Oldes/Rebol3/commit/8951d94b98c8e1b07a86820e590291365ed904ac) Using Knuth's multiplicative hash for color tuples
* [`5e9b14`](https://github.com/Oldes/Rebol3/commit/5e9b14a94b1502efc598b3f4705ec04e39f6e0bf) Implemented `transcode/line` for getting information about number of lines scanned
* [`fe4495`](https://github.com/Oldes/Rebol3/commit/fe449585a3a2395e7223313fc60bcd386270246c) Implemented `transcode/one` for translating just one value
* [`2e55f6`](https://github.com/Oldes/Rebol3/commit/2e55f6a3757602b7ad838715c41752c06cbe3ff2) Allow string input to `transcode`

### Changes:

* [`fd8188`](https://github.com/Oldes/Rebol3/commit/fd8188d422a1da11bbf1122039baabe704e6c0b8) `transcode` used without any refinement returns only scanned data
* [`e9990a`](https://github.com/Oldes/Rebol3/commit/e9990a08c7e93c88e86eadc6f6332230dabe8c9d) Better construction syntax for typesets
* [`ed5e1e`](https://github.com/Oldes/Rebol3/commit/ed5e1e89a38628b8d82946ef10f08e552c39a437) Better construction syntax for datatypes (compatible with Red language)

### Fixes:

* [`687167`](https://github.com/Oldes/Rebol3/commit/6871674edd1f790190bd9a618335132dc4ad5d9d) Correcting datatype value detection logic
* [`8fb93e`](https://github.com/Oldes/Rebol3/commit/8fb93e8abbcaf3c2b9d5787a805efccce4286a54) Required `find/only` when searching for a datatype value
* [`7c8549`](https://github.com/Oldes/Rebol3/commit/7c8549343663913012eb75921b6e7cdc6bc936ef) `reduce/only` emits `#[unset]` for undefined word instead of an error
* [`67dfe1`](https://github.com/Oldes/Rebol3/commit/67dfe18ca485e380017e84396d113cc8faa5ed6e) Enable `ripemd160` checksum
* [`f229de`](https://github.com/Oldes/Rebol3/commit/f229de601f27c8fd78eadd69b3804b8e9dd7e9e4) `transcode/error` of an invalid serialized constructor triggers the error it should insert
* [`97496e`](https://github.com/Oldes/Rebol3/commit/97496e71165e06dc5547079faca653a16a695fca) Error throwing an error on empty input must be only when used `/next` or `/one` refinements
* [`9ea5fc`](https://github.com/Oldes/Rebol3/commit/9ea5fc1a3de5a11759a36df50145ec36a3c21aaa) Line numbering for error messages doesn't account for header
* [`047f03`](https://github.com/Oldes/Rebol3/commit/047f0328b5aee74143eb30dd1068ca5d248014a9) Throwing an error when used `transcode/next` or `transcode/one` with an empty input
* [`769b5e`](https://github.com/Oldes/Rebol3/commit/769b5e5215a81158300a5a890fcec27ed19945e0) Update `transcode` input position only when needed


## 2022 July

### Features:

* [`1c0004`](https://github.com/Oldes/Rebol3/commit/1c000489c198fca3bd2a15fd5432b278cc5331e8) Using `compose` instead of slower `join`
* [`843fcc`](https://github.com/Oldes/Rebol3/commit/843fcc6c515c871d6dc20d09864468e038a1be5e) Implemented `collect into` and `collect after` parse's command
* [`fe5df3`](https://github.com/Oldes/Rebol3/commit/fe5df3c42199eed6787e0ba35d56248d809695b8) Allow lowering assertions level using `ALEVEL` define
* [`9d919b`](https://github.com/Oldes/Rebol3/commit/9d919b606c7825a7b252876c5d666834fa7cb20e) Implemented `collect set` parse's command
* [`4c14cd`](https://github.com/Oldes/Rebol3/commit/4c14cde521e18279d71474df8a7c18baa52ad777) Initial implementation of parse's `collect` and `keep` commands.
* [`1a46e3`](https://github.com/Oldes/Rebol3/commit/1a46e32eae9998a3a525ad162d4dc2b7fc324ad4) Cherry picked Ladislav Mecir's modifications
* [`e2c232`](https://github.com/Oldes/Rebol3/commit/e2c23202ac56363565bc2257af1d078dc6cdbb2c) `daytime` scheme: initial version of the Daytime Protocol (RFC-867)
* [`01ee80`](https://github.com/Oldes/Rebol3/commit/01ee80d9e730cc5b640bad68183a56c6cae15b88) Using `mime-types` module in `httpd` scheme for more complete list of mime types
* [`099275`](https://github.com/Oldes/Rebol3/commit/0992755a463ef071939e2869c0ab2bceef360f61) Implemented `query/mode` on network ports
* [`d7e55e`](https://github.com/Oldes/Rebol3/commit/d7e55e1445ab72067fa11afd3fb614cec4e5947c) New `flush` action for flushing console output stream buffers
* [`2005a7`](https://github.com/Oldes/Rebol3/commit/2005a79d0238342b7846fc7868923d0fc55848e1) Implemented `read/all` and `write/all` on `url!`, which returns also status code and headers and not just content. It also don't throws an error!
* [`74f3df`](https://github.com/Oldes/Rebol3/commit/74f3df0327ccacbe97e17eea8f84372999dfbc97) Implemented `read/seek` on `url!`
* [`a92ab7`](https://github.com/Oldes/Rebol3/commit/a92ab75fc4d8845dc91c9ba66d50704c08c22d2f) `pop3` scheme: initial version of the Post Office Protocol (v3)
* [`5dab0c`](https://github.com/Oldes/Rebol3/commit/5dab0cde5a77ea2f70ad4cd6f5c2ab61e7b9b66c) Including first version of higher level `mail` scheme for sending emails
  So far it is possible to use Rebol2 like `send` function:
  ```rebol
  import 'mail
  
  ; sending just a plain message:
  send you@example.com "Hello^/ this is just simple message"
  
  ; sending single file:
  send you@example.com %path/to/file
  
  ; sending more files:
  send you@example.com [%path/to/file1 %path/to/file2]
  
  ; sending more complete message:
  send friends [
      subject: "Invitation"
      message: "Hi friends, check the invitation!"
      attach:  %path/to/invitation
  ]
  ```
  It is also possible to use the scheme directly, like:
  ```rebol
  write mail:// [
      From:   ["Oldes" oldes@example.com]
      To:     ["Bob" bob@example.com "Eva" eva@example.com]
      subject: "Invitation"
      message: "Hi friends, check the invitation!"
      attach:  %path/to/invitation
  ]
  
  ```
  In all cases above is expected, that `system/user` is configured and that there is available `smtp` specification (host/user/password) under `system/users/data`!
  
  This is still just the initial version. So far it works only in sync mode, but the idea is, to be able use it in async mode as well (for example from a server instance).
* [`d906bf`](https://github.com/Oldes/Rebol3/commit/d906bf5dc11e58f91643680173096f637858492f) `smtp` scheme: recipient's address validation and possibility to have more than one; Better error handling.
* [`b6108e`](https://github.com/Oldes/Rebol3/commit/b6108ec66c000a4b2d5b368e499988ce223939cb) Allow `email!` as system user's name
* [`ab0c6a`](https://github.com/Oldes/Rebol3/commit/ab0c6af94cfb528d44de649e38ea8212fd11410b) Using `safe` scheme to handle persistent encrypted system user's data
* [`c0ff57`](https://github.com/Oldes/Rebol3/commit/c0ff574a9917aa9fc532cac0a8ea9d127aa9a3e4) Included `safe` scheme as a persistent encrypted key/value data storage
* [`7329f5`](https://github.com/Oldes/Rebol3/commit/7329f550593b322dd18cee00f87944e24466037e) Using `put` action in the `extend` mezzanine (no need of `reduce` call)
* [`a04226`](https://github.com/Oldes/Rebol3/commit/a042264f97fc0050f9191fd2132e2529c2196626) Implemented `put` action on `object!` (extending)
* [`8e00cc`](https://github.com/Oldes/Rebol3/commit/8e00cc92f60e1a3100cfebff9d3158463bdc6ba8) Allow definition of one function to multiple actions in scheme's actor dialect
* [`fcdf59`](https://github.com/Oldes/Rebol3/commit/fcdf59b7e41e22819353159360f329d42e5d8929) Allow `put` action on `port!`
* [`f20211`](https://github.com/Oldes/Rebol3/commit/f2021109c068108b7fb94fec47b5837967dea777) Included `safe` codec (for a simple data storage in encrypted files)
* [`d84882`](https://github.com/Oldes/Rebol3/commit/d84882b9c9cdad15855319dec51cc18f4f71ed38) Allow making a scheme without specifying its title
* [`68cb9d`](https://github.com/Oldes/Rebol3/commit/68cb9d0fbfad43c647a3da22834c79cd55f581a7) Implemented `indexz?`, `at` and `atz` actions on file port
* [`f0ecd1`](https://github.com/Oldes/Rebol3/commit/f0ecd1154e2d6ff7fa313a22c456cd9cd0e041b5) Handle negative part values when reading from a file port
* [`3822b5`](https://github.com/Oldes/Rebol3/commit/3822b56317d7b2902a6f7ce9abb502c01ddd4f38) Less strict `to-date` conversion
* [`00008a`](https://github.com/Oldes/Rebol3/commit/00008ad22698d390b9b0c36d1dad24c3d4700715) Including `mime-field` codec
* [`592c36`](https://github.com/Oldes/Rebol3/commit/592c36329b96c4c34b405265e6985381649c30aa) Including `system/catalog/bitsets/space` bitset (TAB+SP)
* [`cbc7b8`](https://github.com/Oldes/Rebol3/commit/cbc7b88b634f75222c0e3b7a3e1eeafe51ac8646) `reduce paren!` evaluate and generate `paren!`
* [`b5fc6f`](https://github.com/Oldes/Rebol3/commit/b5fc6f204f006f90b62cdff171dbdcf358f04cab) Implemented `case` and `no-case` PARSE keywords
* [`6b47c2`](https://github.com/Oldes/Rebol3/commit/6b47c24ea8059c4fbf58677c3def695883c6ab3f) Implemented `odd?` and `even?` on `pair!`
* [`e4459c`](https://github.com/Oldes/Rebol3/commit/e4459c59ac62835ad83de52c744200d09d4a4eeb) Replaced mezzanine `with` with a lightweight and faster native version

### Changes:

* [`c214c2`](https://github.com/Oldes/Rebol3/commit/c214c22bb56aeb064fbafad17e89104cf448cc5a) Including `comma`, `dot` and `dbl-quote` char constants
* [`f84268`](https://github.com/Oldes/Rebol3/commit/f84268258c37081f0c2b5f64f57d5fe2996fc3a4) Using shorter url for links to Rebol module's sources
* [`46bc61`](https://github.com/Oldes/Rebol3/commit/46bc615d8a7b97b2c8f77adc42675c567fd6a19a) `enbase` using line breaks by default; new `enbase/flat` for old behavior.
* [`7644be`](https://github.com/Oldes/Rebol3/commit/7644beec16f409f38075e62627364fd3f6bdc72f) Always using construction syntax when molding bitsets
* [`98ead9`](https://github.com/Oldes/Rebol3/commit/98ead99a87f54fd3b025edf84bad6c0a459b0a89) Removed limit on `kye` value types in `put` action
* [`b4787d`](https://github.com/Oldes/Rebol3/commit/b4787deb702485011dfbb6be147089d2be16df9a) `system` object reordered + included `user`
* [`901728`](https://github.com/Oldes/Rebol3/commit/901728e72639c02906224b5cc6eb71e31c2417eb) `sys/make-scheme` returns the created scheme's spec instead of `system/schemes`
* [`1a9db6`](https://github.com/Oldes/Rebol3/commit/1a9db606fa6b049803952d08d73ebe7176563e9e) Removed `/all` refine from `parse`
* [`0ee3b5`](https://github.com/Oldes/Rebol3/commit/0ee3b5ffab075fdc5ab3be22e04a1cc3e8879456) Removed series-splitting functionality from `parse`
* [`b7d6db`](https://github.com/Oldes/Rebol3/commit/b7d6dbe4ec7eafff60519eb699014d6904389e25) Decoration removed by `form` and `to-string` from (lit/set/get)-words and (lit/set/get)-paths
* [`4dff72`](https://github.com/Oldes/Rebol3/commit/4dff72357cfda27df12454c97c878f607905e282) `do word!` behavior made compatible with Rebol2 and Red
* [`2d91e5`](https://github.com/Oldes/Rebol3/commit/2d91e515d3d88c19d912a5d8e7a1636029012875) `do path!` behavior made compatible with Rebol2 and Red

### Fixes:

* [`6eff98`](https://github.com/Oldes/Rebol3/commit/6eff98d9ba2b81427659caf4e2884b1aecfedb7d) Crash in nested `collect` with an expression
* [`0ae1ba`](https://github.com/Oldes/Rebol3/commit/0ae1baa157f133e5a0aac07ced319e56c676eaff) Missing new `collect` related errors
* [`29e86d`](https://github.com/Oldes/Rebol3/commit/29e86d5e35af21bcd93d544bfec1ddd328f53b1a) `fail` parse's command must close `collect` if used
* [`8d3e13`](https://github.com/Oldes/Rebol3/commit/8d3e1323278f85f04753e234351d3fa0d40559fb) Improved output of the `??` parse command (limited and flattened)
* [`8549ca`](https://github.com/Oldes/Rebol3/commit/8549cab40e249311584a2afee26a2bbc17c1d2b1) Parse's `reject` working incorrectly
* [`c1ce59`](https://github.com/Oldes/Rebol3/commit/c1ce59f6d74324d0f1d3713527ec6189a250275d) Resolved issue when used `POP3` scheme without console output
* [`5f2b6a`](https://github.com/Oldes/Rebol3/commit/5f2b6a57b67fe190b4ee6295c740b38369cece35) Including network mode words
* [`da4f11`](https://github.com/Oldes/Rebol3/commit/da4f1140324f6b8fd3d19145fb1bda0ade8246c5) Improved `UDP` scheme server/client test scripts
* [`b4d35e`](https://github.com/Oldes/Rebol3/commit/b4d35ebda2b633226afefe7216a6d3a1da1c6ea3) Output buffer tail extension must be done only for UDP
* [`c5e159`](https://github.com/Oldes/Rebol3/commit/c5e159b930a6b509fa9bac2fc69e6cf7790303d2) Missing messages in `UDP` server
* [`40632b`](https://github.com/Oldes/Rebol3/commit/40632bcc52843abe147360858c1f09b056764366) Moving the new `RDC_FLUSH` device command to the tail (so it does not breaks unsupported devices)
* [`90b975`](https://github.com/Oldes/Rebol3/commit/90b97564c64d60bd8391f3d69754e184517ebf33) Invalid data with the _partial_ http response regression
* [`1171af`](https://github.com/Oldes/Rebol3/commit/1171af8cfdfbee2298038a4e487ae8faeb8943d1) There is no `in_addr_t` defined on Windows
* [`fa9920`](https://github.com/Oldes/Rebol3/commit/fa99208f0790e639c682b649679fc4f5e04cd429) Resolved warnings
* [`e6527b`](https://github.com/Oldes/Rebol3/commit/e6527be2ff772228acd1a84501792d7581f998d1) Writing data to `smtp` scheme must use chunks not higher than 16KiB (TLS limitation)
* [`d044c2`](https://github.com/Oldes/Rebol3/commit/d044c26657bfa05f3cb4d03247da7199d304f6d5) Better doc-string for `enbase/flat`
* [`043512`](https://github.com/Oldes/Rebol3/commit/0435129a54caf23c54d348da1e77e07c239d6f26) Used external IP in the SMTP's EHLO message, when domain name is not available
* [`ec9959`](https://github.com/Oldes/Rebol3/commit/ec995952315fb5ffb96ca8ccf8cc90e2671bdda6) Make sure, that values from _safe_ scheme are always copied, if possible
* [`0bf389`](https://github.com/Oldes/Rebol3/commit/0bf389290fcfc23d8a960b4742f1e0fb74ace095) Protect `system/user` value
* [`17b485`](https://github.com/Oldes/Rebol3/commit/17b485504a6c2941700c87ab5083d6d7a66e573f) Using `split-lines` native (until `split` will not be native as well)
* [`5de5f2`](https://github.com/Oldes/Rebol3/commit/5de5f28fa294d5bbf514b640b14fb46bc7cc8914) Regression in `reword` with `none` as an escape value
* [`0bad6f`](https://github.com/Oldes/Rebol3/commit/0bad6fb37a02620ca0edc0993eeebcb89d91c8a0) Equality of extended objects
* [`5bd5af`](https://github.com/Oldes/Rebol3/commit/5bd5af612f70840c73fe0c0b067175b0b51b6aaf) Simplified code using the new `no-case` parse's keyword
* [`9a592d`](https://github.com/Oldes/Rebol3/commit/9a592dc370402ae489697edcd0ee6f390127e102) Removed now unused `parse/all` refine
* [`37b539`](https://github.com/Oldes/Rebol3/commit/37b539ff2f63aac0521f54368767614fb73aab8d) Use of escape char `^` must not be allowed in unquoted file notation
* [`eef408`](https://github.com/Oldes/Rebol3/commit/eef40859aafa4604efa7185c79653b0d20e6710c) Removed series-splitting functionality from `parse`
* [`30569d`](https://github.com/Oldes/Rebol3/commit/30569dbc9063ebaa42d06f3aa521250614776fdf) Don't allow `error!` and `task!` to be a `bind`'s context
* [`01a1f2`](https://github.com/Oldes/Rebol3/commit/01a1f23fdb4311a5816ec1561b1ee5f7f04f9154) Some TLS code optimizations
* [`9e7aa5`](https://github.com/Oldes/Rebol3/commit/9e7aa5b94ca44829425546b98073302db4ef9ffa) Make `trace` output better aligned
* [`1093ae`](https://github.com/Oldes/Rebol3/commit/1093ae165a1f626521cd410a059c1ec055cb54a0) Tinny code optimization
* [`e3a200`](https://github.com/Oldes/Rebol3/commit/e3a200adfcf131d2c6188e62aa092b4bafab349b) `to integer! "00"` failing on MacOS
* [`446a1e`](https://github.com/Oldes/Rebol3/commit/446a1e769831c2680bcfe28a6371660cfe6a7a4d) Crash when decoding image from invalid data on MacOS
* [`0b50d9`](https://github.com/Oldes/Rebol3/commit/0b50d9c37201c2de95bc6fb9a251744fa37c7e87) Better handling odd part values when using `append/part` on `object!` and `map!`
* [`1ca9df`](https://github.com/Oldes/Rebol3/commit/1ca9df5ea664c610901ab730cb483b9288566344) Implemented `append/part` and `append/dup`
* [`4bde88`](https://github.com/Oldes/Rebol3/commit/4bde88a3c6cfb212ea9f18b69ee312e7898d9211) Write file fails were silently ignored; closing port on read errors before throwing
* [`3c43f4`](https://github.com/Oldes/Rebol3/commit/3c43f44838769e1b72c040cd1d9598f230691d1b) Error writing files of exact size 0xffffffff on Windows
* [`d61afe`](https://github.com/Oldes/Rebol3/commit/d61afeb53abf942eea6cc2a274f94c1959ab6e18) Code cleanup
* [`8c92c8`](https://github.com/Oldes/Rebol3/commit/8c92c8c8ae22820ec9c4cd323c67a9df42413eac) Resolved implicit conversion warning

## 2022 June

### Features:

* [`32dd19`](https://github.com/Oldes/Rebol3/commit/32dd199de5bbf590b1cb2dbf14379e05d7637765) New `combine` function as alternative to `rejoin`
* [`9ea810`](https://github.com/Oldes/Rebol3/commit/9ea8105ed2b0565e0e276b4c0710f871ce6c1e9f) Allow `try` to accept code as a `paren!`
* [`435e29`](https://github.com/Oldes/Rebol3/commit/435e298e2321503df3354b78428bf0592869b4f7) `debase` and `enbase` having `/part` handling compatible with `copy/part`
* [`3fd5b6`](https://github.com/Oldes/Rebol3/commit/3fd5b60da40082a13292590aa047b8101fe4d86a) Automatically handle unicode non-binary `dehex` input
* [`c6bbba`](https://github.com/Oldes/Rebol3/commit/c6bbbaee702a0111130c67cf1a19863ef5fdded0) `quoted-printable` codec with special space/no-space options
* [`b24107`](https://github.com/Oldes/Rebol3/commit/b2410769fc426c7831efbad60c76bc36b95efa33) `enhex/uri` and `dehex/uri` for a special short variant of space encoding
* [`394370`](https://github.com/Oldes/Rebol3/commit/394370872993e75e463b04d19fd91b931878104b) Added date/time formatting
* [`53e3db`](https://github.com/Oldes/Rebol3/commit/53e3db0ae5841013d7d66c8767a1d672a11228da) Including `quoted-printable` codec
* [`4b7d14`](https://github.com/Oldes/Rebol3/commit/4b7d14837f3a86561ac717c13f73453db2fb9c82) Updated link to Rebol/Blend2D extension version 0.0.18.1
* [`05a616`](https://github.com/Oldes/Rebol3/commit/05a6160435b4ee4c205725b615f8f7e9d1ab1fca) Including link to Rebol/Triangulate extension
* [`53108a`](https://github.com/Oldes/Rebol3/commit/53108a4f9227a5d27e3fddff6fe088bc03496bcd) Including links to sources of optional modules
* [`33d32b`](https://github.com/Oldes/Rebol3/commit/33d32b088ca0ef2fc06128fda49cd5efc944da1b) Automatic download of the well known extension modules
* [`4a0d81`](https://github.com/Oldes/Rebol3/commit/4a0d814a0dac2b03302d01f9e5ab44eb36c09d3b) Counting an hourly money rate using division of time! with money!
* [`821962`](https://github.com/Oldes/Rebol3/commit/8219628e4183064aebd674cfa8c14de8b2fa2577) Allow multiplication between time! and money! values

### Changes:

* [`1a80f5`](https://github.com/Oldes/Rebol3/commit/1a80f518ad67c54668deefb2ad63d3fb3d86754b) `quoted-printable` codec is delayed (required `import 'quoted-printable` before use)

### Fixes:

* [`6ac01e`](https://github.com/Oldes/Rebol3/commit/6ac01e3a6ec6470b919dbc50f2e65952c766dfea) Not using flattened source code
* [`6005b1`](https://github.com/Oldes/Rebol3/commit/6005b104ca254a7b506250ef6dd02a6da1fa033b) `construct` multiline unfolding missing space
* [`66bf62`](https://github.com/Oldes/Rebol3/commit/66bf6261f9b121c6ba5f6df604ef63570786c1eb) `enhex/uri` and `dehex/uri` with an unicode input
* [`24bb10`](https://github.com/Oldes/Rebol3/commit/24bb10abb2c80e76595f29d1e15d098a64667032) Handle PLAIN SMTP authentication response
* [`dc02d7`](https://github.com/Oldes/Rebol3/commit/dc02d7081c485a377d3a80a4f4eed2415fe9eedd) Correctly handle failed extension downloads
* [`8e0606`](https://github.com/Oldes/Rebol3/commit/8e0606295e7cc8686b2ec1267fb0cae925541348) Using `query` on URL sometimes reports `date: none`
* [`19f638`](https://github.com/Oldes/Rebol3/commit/19f638ea816f25d7fdc603ba4aa0321d5a3c19e5) Loaded/saved image data have reversed RGB colour order on macOS


## 2022 May

### Features:

* [`a63913`](https://github.com/Oldes/Rebol3/commit/a639131f6b0e6b574211cb4e1aa264258b599467) `SMTP` protocol scheme updated to work with the recent TLS changes
* [`d70bab`](https://github.com/Oldes/Rebol3/commit/d70bab13b9257695718a4d108dde4c4052d5b63e) Fine-tunning what crypto features are included in base/core/bulk builds
* [`125fce`](https://github.com/Oldes/Rebol3/commit/125fcec8b5b58583c9d562d33fa6d897ab0e5ff7) Using only cipher modes, which are really supported in the TLS protocol
* [`8e3985`](https://github.com/Oldes/Rebol3/commit/8e39851e12c56c3d9061d78a3b63347292ea6ad8) Converted original CSV-tools script to proper CSV/TSV codecs
* [`4d0841`](https://github.com/Oldes/Rebol3/commit/4d0841b65e0dabab67226d93a8e6350abc879ebc) Including Brian Hawley's CSV handling tools script downloaded from www.rebol.org
* [`c8befe`](https://github.com/Oldes/Rebol3/commit/c8befe733478b675cbfa29f54922db1e078480f1) Added support for SHA384 cipher modes in the TLS protocol
* [`c17863`](https://github.com/Oldes/Rebol3/commit/c17863b30018364f373aeff3e5bc4be07e0ecaf8) Added support for GCM ciphers in the TLS protocol
* [`f1a82b`](https://github.com/Oldes/Rebol3/commit/f1a82b1892a20e2ae0eff3b90bf289bdebafbacd) Added support for authentication in GCM (Galois/Counter mode) cipher mode
* [`f6f7c6`](https://github.com/Oldes/Rebol3/commit/f6f7c60e58c75d3b6db9e0031454f19deaebe1f0) Updated CCM (Counter with CBC-MAC) cipher mode
* [`b08704`](https://github.com/Oldes/Rebol3/commit/b08704c989099d3be0b8398b1360a531922c9377) Added support for CCM (Counter with CBC-MAC) cipher mode
* [`7cb433`](https://github.com/Oldes/Rebol3/commit/7cb4335bf32af1a6907ab98fa2de046d25dcae1f) Added support to upgrade plain SMTP connection with a TLS layer (STARTTLS)
* [`5ae9b0`](https://github.com/Oldes/Rebol3/commit/5ae9b0f242dc9e3bdb7cbb8bfb7aa0da9fec16a2) Tiny code optimization in the HTTP scheme
* [`e89f4b`](https://github.com/Oldes/Rebol3/commit/e89f4b77767e47495fb12e272f193b7cebb8b5b7) Added possibility to start TLS connection reusing existing TCP port

### Changes:

* [`bef541`](https://github.com/Oldes/Rebol3/commit/bef541059d8fa19303928825cba4064f135a7a54) Deprecated `chacha20`, `chacha20poly1305` and `poly1305` natives are now completely removed

### Fixes:

* [`dd3f3e`](https://github.com/Oldes/Rebol3/commit/dd3f3e0debebab2a21e4d377a41d3ea3fe030ece) Host is not using os provided crypt device
* [`3f38fc`](https://github.com/Oldes/Rebol3/commit/3f38fcfe9e5096d3692a940bd73bccdc0ce2380b) Unreferenced local variable warning
* [`22794e`](https://github.com/Oldes/Rebol3/commit/22794e48de0dd89a49170ed2788cc65dd65279c4) Assignment from incompatible pointer type warning
* [`dd9300`](https://github.com/Oldes/Rebol3/commit/dd93003da2c7ff00cddc292d2e47fc778a30ecff) Incompatible pointer types warning
* [`9589ec`](https://github.com/Oldes/Rebol3/commit/9589ec86fec077e20f464ab81d8feca2faf91c10) Always false result warning
* [`5eedc5`](https://github.com/Oldes/Rebol3/commit/5eedc5432fb591a4acef9e44391a4022036595da) Incompatible integer to pointer conversion warning
* [`13c4d3`](https://github.com/Oldes/Rebol3/commit/13c4d3b61cd00dba3e88a6df240813e8e853deb7) Removed unused config include
* [`79ccb3`](https://github.com/Oldes/Rebol3/commit/79ccb3603702d82f866e14eee761ea4a517e436c) Incompatible pointer types warning
* [`b527b1`](https://github.com/Oldes/Rebol3/commit/b527b18795eb19a7d027ff804c3dcb9cc19295e8) `import/check` with an invalid checksum was failing with a wrong error (https://github.com/Oldes/Rebol-issues/issues/1686)
* [`2660ab`](https://github.com/Oldes/Rebol3/commit/2660ab1e6ff643b70c639aca56f615ee4f56ccb1) Updated `ppk` codec using the new crypt port
* [`dd3651`](https://github.com/Oldes/Rebol3/commit/dd3651718d77303341ac413f17fd515b8c36dc4f) Cleanup and better resolving `code-page` charset encoding
* [`5a29c1`](https://github.com/Oldes/Rebol3/commit/5a29c1518b23e3049295d47eee23c3be8b674506) Wrong length of the IV in the GCM cipher mode initialization
* [`e4d80c`](https://github.com/Oldes/Rebol3/commit/e4d80cb3eb58715725b28fbd1236c60548e54b09) Implicit declaration of function and incompatible pointer types
* [`9f3bdc`](https://github.com/Oldes/Rebol3/commit/9f3bdc22435e88e76ca50c237af57a711b509ee4) `CHACHA20-POLY1305` decryption in the TLS protocol was not correctly checking the `mac`
* [`6efd1d`](https://github.com/Oldes/Rebol3/commit/6efd1d47e7d80d897b4d702eb093a94b42b01953) Missing CCM crypt mode source file
* [`6c8d19`](https://github.com/Oldes/Rebol3/commit/6c8d19e411abaadd1b74e1fe8dc727f128c81b2c) ECDHE_ECDSA verification (ec public key may start with a zero byte)
* [`200a42`](https://github.com/Oldes/Rebol3/commit/200a42e096c1a2e947f5bc6c439a74ca25c20732) Trying to get a console width under CI on Windows trows an error
* [`0ca83b`](https://github.com/Oldes/Rebol3/commit/0ca83b31b2ba3c6110e2161ab803a4708e2eff13) Using `init-vector` instead of just `iv` in crypt port specification; `take` on crypt port is now like `read update`; updated crypt port initialization to be compatible with the changes of `decode-url` function.
* [`975eb9`](https://github.com/Oldes/Rebol3/commit/975eb9cfb7b8a14b46f63562101fc5578440bcf3) Comparing blocks with a `1.#NaN` value
* [`b75188`](https://github.com/Oldes/Rebol3/commit/b751885142634a9bee08c30727023b182b897bbd) `1.#NaN` sorting inconsistency

## 2022 April

### Features:

* [`8aa3f5`](https://github.com/Oldes/Rebol3/commit/8aa3f531386b75741785c0898ae80d52722ccdf1) `enbase`/`debase` with base 36 (alphanumeric)
* [`0df0d4`](https://github.com/Oldes/Rebol3/commit/0df0d440a59306f9c1b35ff0a5cf8463e1398c5d) Some useful responses from the test httpd server
* [`266b20`](https://github.com/Oldes/Rebol3/commit/266b20f926f97c76933ae33b5816b235274a2ace) Allow conversion `to word!` from any string datatype (if don't have invalid chars)
* [`81699e`](https://github.com/Oldes/Rebol3/commit/81699eec65e9884c3923bef9e38e99a5ac10062e) Initial rewrite of the old `smtp` protocol
* [`c4547d`](https://github.com/Oldes/Rebol3/commit/c4547d959912c4ebf9effb65b5ad713df2f7ccb4) Included codec for Apple's Provision Profile files
* [`31dae9`](https://github.com/Oldes/Rebol3/commit/31dae9177687af2eda692260afb9ca9101cee780) Added optional codec (decoder) for `plist` files
* [`7447dc`](https://github.com/Oldes/Rebol3/commit/7447dc011461592dee144203b533e0de2a94daa2) Added `uid` attribute as a known OID

### Changes:

* [`98fd68`](https://github.com/Oldes/Rebol3/commit/98fd6888fa3b3b30bc8728db6742ee055d635816) Improved _fingerprint_ checksum counter in the `crt` codec

### Fixes:

* [`a4f4fc`](https://github.com/Oldes/Rebol3/commit/a4f4fc9ebaa8a1122b8b7f85b4f1eed89c49fcb7) Wrong return value
* [`093c89`](https://github.com/Oldes/Rebol3/commit/093c89db647441440e4aa1a0d1464fa027a63ce6) Reset unused parts of the http uri during redirection and some tiny optimizations
* [`e2ef1a`](https://github.com/Oldes/Rebol3/commit/e2ef1a97a3eb8c74b8f939c1c35d64517382a267) Updated http request construction to work with changes in `decode-url`
* [`3dd0df`](https://github.com/Oldes/Rebol3/commit/3dd0dfb6dcf80a2f5801850808728f4f57550d84) Regression related to the new output from `decode-url` function
* [`48a3fc`](https://github.com/Oldes/Rebol3/commit/48a3fc2fd8dc39f9a59f7d3c94f4e82a90dea626) Optimized url/file escape table initialization
* [`92c65f`](https://github.com/Oldes/Rebol3/commit/92c65fc8fd475feae24471cd65337cf2e7936e4d) Special char escaping when molding a file (so it is backwards loadable)
* [`7bf61f`](https://github.com/Oldes/Rebol3/commit/7bf61fafa790c1b1944fd6fd855a98b9bb730f60) Rewritten `decode-url` function, `mold` properly escaping urls
* [`9dc15a`](https://github.com/Oldes/Rebol3/commit/9dc15a0c6f20853f21526c422134e6c56df5c04b) Regression in the `httpd` scheme
* [`347d73`](https://github.com/Oldes/Rebol3/commit/347d73578c6dab7327ffc47df431513b32077400) `split/at` when the delimiter is not found
* [`4fb0ac`](https://github.com/Oldes/Rebol3/commit/4fb0acd4ece5fd3192e2f208b4708764a1d4485c) Using shorter code variant for extending a series at its tail
* [`044c59`](https://github.com/Oldes/Rebol3/commit/044c59d76e2d06cc690f8bbec4373b481bbfddf0) Using set-words in the `dh parameters` parse rule
* [`2ce523`](https://github.com/Oldes/Rebol3/commit/2ce523d1dc6e6525a8ffd926cf088bd901d800e3) Don't silently ignore leading zero byte in the `der` codec data

## 2022 February

### Features:

* [`080cd3`](https://github.com/Oldes/Rebol3/commit/080cd36b1e6a6ecedafeecded22de01734fdc8c1) Support for direct DH params initialization from `.key` files
* [`f0d33e`](https://github.com/Oldes/Rebol3/commit/f0d33e55e2ebf0b1b81168108da3535aa17c15c2) `form-oid` function for conversion of OID binary value into its numeric string format
* [`5868c1`](https://github.com/Oldes/Rebol3/commit/5868c1dda338ac5d58af925da1321d02dab3e7c9) Added (limited) support for a TLS _server_ role
* [`ace3ec`](https://github.com/Oldes/Rebol3/commit/ace3eca70ccaabe6861eedb56508bf45dd881c0a) Providing better info on `port!` values from `help`
* [`8c051b`](https://github.com/Oldes/Rebol3/commit/8c051b1101585bff6b1d43f2b580f9b6489cb79f) Added support to load X.509 encoded private RSA keys
* [`07f79c`](https://github.com/Oldes/Rebol3/commit/07f79c39248d39cbccdac9049d0237abb3600fcf) Allow `read/lines` on urls (HTTP scheme)
* [`ab481c`](https://github.com/Oldes/Rebol3/commit/ab481cc1dc7855d35c65132f52f1baf1a6b4c44d) New `split-lines` native
* [`5ec4bb`](https://github.com/Oldes/Rebol3/commit/5ec4bb22ee5d928abb4d062ef508787bddae0452) Allow char as an argument for writing to a file port
* [`e22657`](https://github.com/Oldes/Rebol3/commit/e22657f3d7f4e57d344079f6580336a79dbb90c8) Added certificate's `fingerprint` calculation
* [`61609d`](https://github.com/Oldes/Rebol3/commit/61609d747e1fd4b7fbf574eb1b03aefb37e324ff) `tls` scheme - using the new cryptography code
* [`9ea657`](https://github.com/Oldes/Rebol3/commit/9ea657c153e4869d7ae0926db336e38a626e584a) Simple `crypt` port test/example file
* [`7d0849`](https://github.com/Oldes/Rebol3/commit/7d0849bb9815d46a36924a2649d4da5cd34d8152) Added support for `GCM` (Galois/Counter Mode) cipher mode
* [`d4614b`](https://github.com/Oldes/Rebol3/commit/d4614b755c694a0de6437b14db09ce1002e32abe) Add `/part` refinement to `remold` function

### Changes:

* [`070c6b`](https://github.com/Oldes/Rebol3/commit/070c6b2133603237115988af724dddf0bf4684d2) Added new `release` native as an universal way how to release internal resources available thru handles. Removed all non universal ways of releasing crypt keys!
* [`3cdba9`](https://github.com/Oldes/Rebol3/commit/3cdba9b08aa1a835c021f44b0fcb8a9547024452) Minor TLS code rearrangement
* [`b2059c`](https://github.com/Oldes/Rebol3/commit/b2059c2cbd7189f3b27201a493814b868845f864) Removed the rest of the old legacy TLS code
* [`f4ed6b`](https://github.com/Oldes/Rebol3/commit/f4ed6b517eeb8ccbadd1d0664a15734d71cf1797) Removed legacy TLS support, first steps to have a server side TLS
* [`f66b9a`](https://github.com/Oldes/Rebol3/commit/f66b9a675593b750efc044338f332ca871c1a318) Replaced `port/spec/port-id` with `port/spec/port`
* [`c54b35`](https://github.com/Oldes/Rebol3/commit/c54b35250de1d1dbc3434cd837b1dd7609c5873e) Replaced `port/locals` with `port/extra`
* [`d19b88`](https://github.com/Oldes/Rebol3/commit/d19b8851d185eec991f4fb0baf11969c073f5d8b) Replaced `load/type` with `load/as`
* [`942d1a`](https://github.com/Oldes/Rebol3/commit/942d1ae8ad00d364b47a8c5d39a19721ff6d6d4c) Renamed `remove/part` argument from `length` to `range` and allowed `remove/part` on bitset when the argument is string, binary, block or char
* [`2d9411`](https://github.com/Oldes/Rebol3/commit/2d94117f4e12e0751bb2d87f44d3b945696c7ec3) Allow `to-decimal "1e"`
* [`356292`](https://github.com/Oldes/Rebol3/commit/356292c3813fc196f4b2bad90183253c779676e4) Using `range` instead of `length` for the `/part` argument, as it may be negative.
* [`6c7fd4`](https://github.com/Oldes/Rebol3/commit/6c7fd45f8a9ae469d49623c399d1b5de4a24a381) Allow making error using error as a make's type

### Fixes:

* [`811d22`](https://github.com/Oldes/Rebol3/commit/811d229e354f6f9c731913dc7351f5460c29f140) `set` with any-block types
* [`18fdf2`](https://github.com/Oldes/Rebol3/commit/18fdf25831108ce57405102686ab43479587b9b1) Move elyptic curves on top (these are the most common now)
* [`60bfc8`](https://github.com/Oldes/Rebol3/commit/60bfc8b146af3a6a9da7692e5f513f9c06d67d13) RSA binary input values were expecting, that series are always at its head
* [`e114d7`](https://github.com/Oldes/Rebol3/commit/e114d7389a66a5e2d1522dbc8ba6b01061014fd3) Warning: incompatible pointer types
* [`c58fba`](https://github.com/Oldes/Rebol3/commit/c58fba246b640f5c15bf4e762e15c2f0b97cbcab) Better error message when using unsupported port action
* [`6ca57f`](https://github.com/Oldes/Rebol3/commit/6ca57f4fa4a9886c603b27eb3d0badb8811182c4) Correct width expansion when appending char in range 128-255
* [`b633ce`](https://github.com/Oldes/Rebol3/commit/b633cea2ce471e0970b385a380fcd3d46cf29404) Leaked local variable
* [`c6efa2`](https://github.com/Oldes/Rebol3/commit/c6efa2ad7815d69539b09f27ad5f2e58d6c1174d) Using correct checksum method in the `fingerprint` calculation
* [`bcb875`](https://github.com/Oldes/Rebol3/commit/bcb875acc1dda247bfe23acd406e52c026b5ab3e) Minor TLS code optimization
* [`580ad3`](https://github.com/Oldes/Rebol3/commit/580ad3060ec241ff8a26acd49026fc4c8dde568f) `http` scheme - situation when server does not provide Content-Length and just closes connection
* [`1b6b88`](https://github.com/Oldes/Rebol3/commit/1b6b88a7c6decaa5833274ab781d3bbbf140adf7) Shortened doc-string
* [`756721`](https://github.com/Oldes/Rebol3/commit/756721af8f2451f24ad69c93fcb6aa48c8644c69) Large integer from string
* [`acd983`](https://github.com/Oldes/Rebol3/commit/acd983699f249335cccd89c44d6f71871293951c) Regression introduced in https://github.com/Oldes/Rebol3/commit/356292c3813fc196f4b2bad90183253c779676e4
* [`8892f7`](https://github.com/Oldes/Rebol3/commit/8892f7d4113aaed4b7e5c13ba668650c533248ae) `make event!` using existing `event!` as a `make` type
* [`56c14d`](https://github.com/Oldes/Rebol3/commit/56c14d99652dd9e32da8ffec1a609c5f37cc8c12) `save` to `string!` needs `mold`

## 2022 January

### Features:

* [`c7e3d7`](https://github.com/Oldes/Rebol3/commit/c7e3d73e5c82a6282629f72cb359a9a1198a7136) Optional definition of the message digest algorithm to sign/verify data using `rsa`
* [`ef0280`](https://github.com/Oldes/Rebol3/commit/ef0280c8c4329597b303721cb8523e45a21d6f37) `profile` function for easy code speed comparisons, like: `profile [[wait 0.1][wait 0.2]]`
* [`ca4ddb`](https://github.com/Oldes/Rebol3/commit/ca4ddbd56d4c4cd61b60b1850b380495e7aacc76) Rewritten crypt port not to depend on mbedTLS's cipher-wrapping; included ARIA cipher and initial `chacha20-poly1305` crypt's implementation (still WIP!)
* [`d4bd52`](https://github.com/Oldes/Rebol3/commit/d4bd5216d47c07c0b7f57744bfe233822e083ab7) Included CHACHA20 cipher with possibility to set initial block counter
* [`a86c3e`](https://github.com/Oldes/Rebol3/commit/a86c3eefa2968fe927532e0a89d26c7e899a0639) Added info about possible output limitation of `probe`
* [`d9c7fb`](https://github.com/Oldes/Rebol3/commit/d9c7fb3b957e6d6f6ee2be9df0490a3c425beedd) Simplified extension test definition
* [`e9ed58`](https://github.com/Oldes/Rebol3/commit/e9ed58a739ad29bfd975796b1e5cb09cb691eaf8) Updated mbedTLS code (used for checksums only so far) to version 3.1.0
* [`aa70ed`](https://github.com/Oldes/Rebol3/commit/aa70ed944fed4b6e80cd2275f231fdc16d06a184) Added FreeBSD support

### Changes:

* [`e74518`](https://github.com/Oldes/Rebol3/commit/e745186cfbec3c1c0c76f197b087a07fd6b9f4db) Crypt port (work in progress)
* [`902eeb`](https://github.com/Oldes/Rebol3/commit/902eeb37be9628b2b96b8bd1c79602b912c47ca4) Report port error reasons as positive integers
* [`f46f9b`](https://github.com/Oldes/Rebol3/commit/f46f9babc37b401a4174bdc4e10e314ce3cd24af) Replaced the old `ECDH` and `ECDSA implementation with code from `mbedTLS`
* [`d6db6f`](https://github.com/Oldes/Rebol3/commit/d6db6f50ae697419bee31b5e443c0902e010227b) Replaced the old `DH` implementation with code from `mbedTLS`
* [`e12f0a`](https://github.com/Oldes/Rebol3/commit/e12f0ae9afb3a30ab79a81c876f2019d3bddc194) Replaced the old `RSA` implementation with code from `mbedTLS`

### Fixes:

* [`77aac4`](https://github.com/Oldes/Rebol3/commit/77aac4fc9c56554128ded21d4d8744bf0ca072d2) Wrong context casting
* [`2077fb`](https://github.com/Oldes/Rebol3/commit/2077fb5b29dfb9ea928cfd71c0d6dc9373f29bb6) Static declaration of 'free_crypt_cipher_context' follows non-static declaration
* [`bb08b5`](https://github.com/Oldes/Rebol3/commit/bb08b5ebe602a80c664937ccac7f7371618c0def) Using unsigned integer
* [`799019`](https://github.com/Oldes/Rebol3/commit/7990196b6adc69162e695c695d7ddbd861accf37) Improved precision of the `profile` function even more
* [`b171da`](https://github.com/Oldes/Rebol3/commit/b171da93f48dc9820f3da263007d39829ae158cc) Improved precision of the `profile` function
* [`73c731`](https://github.com/Oldes/Rebol3/commit/73c731b247f56b0ea58aa151ee26f8946021f282) `mbedtls/common.h` file is only for private use
* [`f4e8e6`](https://github.com/Oldes/Rebol3/commit/f4e8e6b22155fc3a07164a4d2f39850a40be9ccf) Missing free call for ARIA cipher context
* [`95f974`](https://github.com/Oldes/Rebol3/commit/95f974562ba694b4cd01cf43f3c6d6351fc64480) Provide title and type for the QOI codec
* [`710e66`](https://github.com/Oldes/Rebol3/commit/710e6693e995150d093b0ce6de252bdf4dbe5c97) `echo` does not error on bad path
* [`5bbac3`](https://github.com/Oldes/Rebol3/commit/5bbac328eb8618dbf2a09ff074c103eb3333b70a) `trim` on `block!` removing nones only from head and tail, `trim/all` everywhere
* [`6e0d26`](https://github.com/Oldes/Rebol3/commit/6e0d26f3ca68097427ed9ff5681d2b032bc1cfd3) `trim` on `binary!` should deal with NULL char instead of white-spaces like on `string!`
* [`13da59`](https://github.com/Oldes/Rebol3/commit/13da594076618beddcbc0380174d43ef740d9872) Make sure that error message always starts on a new line
* [`c41c33`](https://github.com/Oldes/Rebol3/commit/c41c33d6089fa1d94dee0d46a7556ee11cfe0442) 'for' loop initial declarations
* [`a8fbea`](https://github.com/Oldes/Rebol3/commit/a8fbea0773c1ec49f7a36fc88df0f92a2ce60d0e) 'for' loop initial declarations
* [`9ed043`](https://github.com/Oldes/Rebol3/commit/9ed043ed53470236f21202831ac3dc33c0653cca) Explicitly define all platform names (useful in the bootstrap phase)
* [`c6ea2e`](https://github.com/Oldes/Rebol3/commit/c6ea2e374093f69be4a5703b975d5d8d316572b6) Conflicting types
* [`ecd379`](https://github.com/Oldes/Rebol3/commit/ecd379159f00cd1572454990cc5d8f56230c8bad) Updated banner to year 2022 + including `system/product` info
* [`346412`](https://github.com/Oldes/Rebol3/commit/34641263cb872fa3b86b2d1a48ec84bb8b8d9de0) Failing Base and Core builds
* [`e7f2b6`](https://github.com/Oldes/Rebol3/commit/e7f2b61182784df2b03d1205900cd786205adf7c) Crash while using parse with non-utf-8 data from file
* [`989235`](https://github.com/Oldes/Rebol3/commit/98923506ee770ba1e0085179145455ced6941661) Macro redefinition
* [`a84d39`](https://github.com/Oldes/Rebol3/commit/a84d395ece785b12e74408e5d368798a333cf9f2) Incorrect codepage id for `CP819` alias `IBM819` codepage name
* [`1fdda5`](https://github.com/Oldes/Rebol3/commit/1fdda56befbc7f2f624186145b23079bda1d19a2) `iconv` on FreeBSD does not accept `CP12000` for `UTF-32LE` codepage
* [`0c62ea`](https://github.com/Oldes/Rebol3/commit/0c62ea403d739260dc74c6fec2726180f95486fb) `iconv` not accepting `utf8` codepage on BSD systems
* [`5a8c6e`](https://github.com/Oldes/Rebol3/commit/5a8c6e07b432ef684ebaf77a2dafd70d742caa4a) Make sure that `setenv` is used on FreeBSD
* [`a53862`](https://github.com/Oldes/Rebol3/commit/a53862b1a40d02273bd89187475dab9b605701c0) Executable path not resolved correctly on OpenBSD and FreeBSD platforms
* [`734eda`](https://github.com/Oldes/Rebol3/commit/734edab855fcd3aa76b31c56acdf6e16a0f519db) Occasional HTTPS read fails when server used ECDHE_ECDSA key method
* [`06425a`](https://github.com/Oldes/Rebol3/commit/06425a79c57a599b3cf1f31f4ff0510b68426dda) Occasional HTTPS read fails when server used ECDHE_ECDSA key method


## 2021 December

### Features:

* [`c474d0`](https://github.com/Oldes/Rebol3/commit/c474d052c13b2bbbf55b9f2d604560960ebf9a9a) Updated optional `qoi` (Quite OK Image) codec
* [`b26f9b`](https://github.com/Oldes/Rebol3/commit/b26f9bff05bc8a73657ad62a7827c1d8bc28f370) `COLLECT-WORDS /as type [datatype!]` option
* [`3d83ca`](https://github.com/Oldes/Rebol3/commit/3d83cac73affe6c07ae2fc08665a527ebf257d0c) Errors and debug messages printed to stderr
* [`95d535`](https://github.com/Oldes/Rebol3/commit/95d5354c071290dcf8398d63c51d696cf8c1f4ce) Added possibility to change console output from `stdout` to `stderr` (windows)
* [`9c4036`](https://github.com/Oldes/Rebol3/commit/9c40364ab6c5442ff0361456d6ed89ae60b651cd) Added possibility to change console output from `stdout` to `stderr` (posix)
* [`53b04b`](https://github.com/Oldes/Rebol3/commit/53b04bd93c7d64f7d5f4e8f25308d51081055b96) New `lcm` native for resolving the least common multiple of two integers
* [`12794b`](https://github.com/Oldes/Rebol3/commit/12794b36ca2758599e2f8647b6f68b796de0c5a6) New `gcd` native for resolving the greatest common divisor of two integers
* [`f64cd3`](https://github.com/Oldes/Rebol3/commit/f64cd37ac70d36932215d226a5c85f110e8c973b) Partial `crush` decompression
* [`b7fb49`](https://github.com/Oldes/Rebol3/commit/b7fb49bab839182c73aeacccf366ef4708b7b050) Optional custom LZ77-based compression algorithm (`crush`)
* [`3a3076`](https://github.com/Oldes/Rebol3/commit/3a30769c1f03da4d78bcaace948b266c962b36cc) Bincode - added padding command `PAD`
* [`fa46cb`](https://github.com/Oldes/Rebol3/commit/fa46cb46b1580f322b2007d16c3336defd074faa) BinCode - added commands: `F16LE`, `F16BE`, `F32LE`, `F32BE`, `F64LE` and `F64BE`
* [`15de62`](https://github.com/Oldes/Rebol3/commit/15de62ce49cb6ead3478aa3062cc34e2eb1893df) QOI codec (The "Quite OK Image" format for fast, lossless image compression)
* [`cc1025`](https://github.com/Oldes/Rebol3/commit/cc1025de878aebf9d39f0f8d783de9a99320754b) Added example how to prevent hacking attempts to the http test server
* [`b4789f`](https://github.com/Oldes/Rebol3/commit/b4789fb597f0f14e9c17bd858395360b0875e116) Added example how to server `humans.txt` data from the test server

### Changes:

* [`061773`](https://github.com/Oldes/Rebol3/commit/061773c4b206b650c0a57fc17903231df290b26d) Have `system/ports/output` to be same as `system/ports/input` (instead of `none`)
* [`3139f4`](https://github.com/Oldes/Rebol3/commit/3139f40e72fd34a626674b6d0aad949b7db1f648) Keep `/` at the head of the HTTP request target (`httpd` module)

### Fixes:

* [`c99eb2`](https://github.com/Oldes/Rebol3/commit/c99eb2ee16e1c02497e2aaa3e5ec4c6870b587cc) Better fix of JSON's non-valid-word keys (https://github.com/Oldes/Rebol3/commit/82efc99dca20e8d5b938ea51569cb7c208031b20)
* [`dcdb67`](https://github.com/Oldes/Rebol3/commit/dcdb6711eb087a8122cc31742ee1fb72965d86d3) Correct tuple length struct name
* [`015441`](https://github.com/Oldes/Rebol3/commit/015441e9701d538bdb64386fb25bf83b14204fca) Pass all 12 tuple bytes in the extension argument including info about tuple's size
* [`ea3e47`](https://github.com/Oldes/Rebol3/commit/ea3e47ac39b9e8ca19f521b0c6377cf4a36efaa0) Warning: __VA_ARGS__ can only appear in the expansion of a C99 variadic macro
* [`cf9b34`](https://github.com/Oldes/Rebol3/commit/cf9b34bc7bf7cdbfe33fd7711db475650523e4e1) `split-path` was returning `none` for these paths: `%/`, `%./` and `%../`
* [`82efc9`](https://github.com/Oldes/Rebol3/commit/82efc99dca20e8d5b938ea51569cb7c208031b20) Don't fail when decoding "+1" key from `JSON` data
* [`68679a`](https://github.com/Oldes/Rebol3/commit/68679a1ab82af1ad38ea64c792e9d828def70a65) `read/lines` and `read/string` on `system/ports/input` not working
* [`50caf1`](https://github.com/Oldes/Rebol3/commit/50caf12ba82301211bf4db3413f9891b4f849463) Returning same status code across all systems
* [`993380`](https://github.com/Oldes/Rebol3/commit/9933803576873d1b38eeb2c148f97550f3de16a5) Command line consumes first argument when --script is used
* [`84090f`](https://github.com/Oldes/Rebol3/commit/84090f9651be3d5d58b1fb2539257aea15b1d039) Resolved possible buffer overrun warning
* [`738476`](https://github.com/Oldes/Rebol3/commit/738476556fe9202915859e738ed02cf4135d7772) Using system output to print version info
* [`03a4b6`](https://github.com/Oldes/Rebol3/commit/03a4b6b64171a45eac4606c48e2ffb9979e5249f) Make project compile able with Xcode v13.2
* [`d0aa12`](https://github.com/Oldes/Rebol3/commit/d0aa122b63c36d7a580b84cb62c33bed02dd24cc) Accidentally removed `--do` option storage done in https://github.com/Oldes/Rebol3/commit/ca7c0256d23f159dcebfa25146b4288efbe00cba
* [`ca7c02`](https://github.com/Oldes/Rebol3/commit/ca7c0256d23f159dcebfa25146b4288efbe00cba) Interpreter too eagerly consuming command-line arguments & arguments containing whitespace
* [`7f1343`](https://github.com/Oldes/Rebol3/commit/7f13439bd3f758b053bc5241db23b3bd4e5c7e17) Evaluate `--do` boot argument
* [`6588c3`](https://github.com/Oldes/Rebol3/commit/6588c30fec0c92842f82fd35cb05c04426e0a1d3) `switch` with `ref!` value type
* [`d744ce`](https://github.com/Oldes/Rebol3/commit/d744cee9ad83b76b61566b5de27b7a6e5c80afa6) Memory corruption when used `crush` compression
* [`c33ff2`](https://github.com/Oldes/Rebol3/commit/c33ff26cbc272a863ab8f6de425af67622f50f46) Force garbage collection before measuring `delta-profile` stats
* [`34e0e2`](https://github.com/Oldes/Rebol3/commit/34e0e2c29657dfaa8ebfc01342a4ff9339d23e0e) Missing change for: https://github.com/Oldes/Rebol3/commit/15de62ce49cb6ead3478aa3062cc34e2eb1893df
* [`a09ee2`](https://github.com/Oldes/Rebol3/commit/a09ee2fc455af004f0a7d0f439615f9011ecd32d) `httpd` module: don't keep opened connection if client (like Rebol2) don't want it

## 2021 November

### Features:

* [`5fa5b8`](https://github.com/Oldes/Rebol3/commit/5fa5b83a0ef64a68f0513112f6dc7171b5b2cc3f) Updated ZIP codec
  * Access to comment and extra field of uncompressed data
  * Added support to include file comments, extras or insternal and external attributes
  * Added support to include uncompressed data (useful when making APK targeting version 30 and above)
* [`b49781`](https://github.com/Oldes/Rebol3/commit/b4978111333a9af2228339ed3924bc6cc85c2d5b) Possibility to resolve context handle's type
* [`27ab5a`](https://github.com/Oldes/Rebol3/commit/27ab5a4cbb934459ef2a04e4e06f93bccc137d1c) Make context type handles comparable
* [`e837a8`](https://github.com/Oldes/Rebol3/commit/e837a88efb750bcee354a2306bdec19271782bf0) Allow `insert` on `object!`
* [`cfe7e2`](https://github.com/Oldes/Rebol3/commit/cfe7e2a25534196a39f69866bb12a04829a44fb8) Allow `select` on `module`
* [`5ae487`](https://github.com/Oldes/Rebol3/commit/5ae4871c87b839b8c1392fb3812f5d76c89ca06c) Register `.deb` file extension fith `AR` codec
* [`3a5f9e`](https://github.com/Oldes/Rebol3/commit/3a5f9e50fa084954f76c8adf7bcb2e995d898a47) Including last git commit version in the `system/build/git` and full version info when used `-v` boot argument
* [`4b321e`](https://github.com/Oldes/Rebol3/commit/4b321e3f0178056d1830e2f02b08ad557bdbc5d6) Extended maximum number of tuple values to 12
* [`6f451f`](https://github.com/Oldes/Rebol3/commit/6f451f0c58a802ff598329320b29e8406e6be595) Tweaked doc-string of `clear` action
* [`ab5ae7`](https://github.com/Oldes/Rebol3/commit/ab5ae7dd5573be0a2ef3d3c53e646e7721e10427) Improved doc-string for `at` and `atz` actions

### Changes:

* [`f9fe06`](https://github.com/Oldes/Rebol3/commit/f9fe06397609a60cfe32f66fb15d35ff402f5761) Including some of modules as a delayed (required to `import` them before use)
* [`9458d4`](https://github.com/Oldes/Rebol3/commit/9458d4d6e34b9b263481ba4bf363aeb9ed56cb15) Modified single-line doc string for modules in the `help` output
* [`d4e947`](https://github.com/Oldes/Rebol3/commit/d4e947639dff246565768892e623028eebae63ed) Storing `system/modules` as an `object!`; added support to include delayed modules
* [`04c71d`](https://github.com/Oldes/Rebol3/commit/04c71d11d5f2bba500bd2a658336f33363ac6454) More detailed info about used compilation target; new build output names; rewritten build specification to be better prepared for a future cross-compilation possibility.

### Fixes:

* [`5f9695`](https://github.com/Oldes/Rebol3/commit/5f9695d9defe2bf3bcf6f07e1c0988471028d4c7) Enhanced precision in comparison of time with integer
* [`b1a65b`](https://github.com/Oldes/Rebol3/commit/b1a65ba77e689905e241ac54c41aeaaa14210ea3) Using `call/shell` in the `launch` function
* [`a14d3a`](https://github.com/Oldes/Rebol3/commit/a14d3afa777b6b67046b39a8127229da9347ae04) Use user context filtering only for pattern matches and with `strict-equal?`
* [`74d0c3`](https://github.com/Oldes/Rebol3/commit/74d0c30fa98f5ab84a68748bdaf9641770b27d9d) There must be `get-word!` used in case that value is a function
* [`e26d3d`](https://github.com/Oldes/Rebol3/commit/e26d3d40b955f71d1df2d598bff7880ce71076bf) Include user context values in the `help` output
* [`067fee`](https://github.com/Oldes/Rebol3/commit/067fee59362a62858a99e309e4f4a2b4d3dc9c74) Length-specified script embedding
* [`ef13db`](https://github.com/Oldes/Rebol3/commit/ef13db708605d3c192d2bf6ac9c85340772d8ac1) Improved resolving console width for help output trunctation
* [`43886c`](https://github.com/Oldes/Rebol3/commit/43886c432448ebf82663e21dbd0cde4a2f2b777d) Improved resolving console width for help output trunctation
* [`7a4a44`](https://github.com/Oldes/Rebol3/commit/7a4a441133d82c65364cbd8b5b8eadae354b6481) Improved resolving console width for help output trunctation
* [`a96c89`](https://github.com/Oldes/Rebol3/commit/a96c89285e137795e3b29c86bc751520937aef50) Improved resolving console width for help output trunctation
* [`cd1afe`](https://github.com/Oldes/Rebol3/commit/cd1afe70fe9d83dd33b19c88580341633bd3c9ba) Improved resolving console width for help output trunctation
* [`e3520a`](https://github.com/Oldes/Rebol3/commit/e3520a2902f0efd9ef143ad6d5c9825b74e18cd0) Opening a library (extension import) requires absolute path
* [`2b7b6c`](https://github.com/Oldes/Rebol3/commit/2b7b6cf9588a83351937657c3b473a7171ae3f01) Use default terminal width 80 chars when not resolved from system (CI)


## 2021 October

### Features:

* [`60cc1f`](https://github.com/Oldes/Rebol3/commit/60cc1f98e494239d6c119c24e5a8cf84dc4fd286) Simplified resolving of the path to the executable (as it is now resolved in C using `OS_Get_Boot_Path`)
* [`180dc3`](https://github.com/Oldes/Rebol3/commit/180dc3500f5631a7309adf55073b80b05d9c6bce) Linux version of `OS_Get_Boot_Path` function
* [`39693b`](https://github.com/Oldes/Rebol3/commit/39693bf5302268e0d0af0d7a8a6aa770ff66e2e8) MacOS version of `OS_Get_Boot_Path` function
* [`11b46d`](https://github.com/Oldes/Rebol3/commit/11b46de53915ecb76bb18404f1500021118503fe) Windows version of `OS_Get_Boot_Path` function
* [`d99656`](https://github.com/Oldes/Rebol3/commit/d9965674030d1faf622a19645299d163eb4aeb05) Win32 variant of `to-real-file` native
* [`31e7c2`](https://github.com/Oldes/Rebol3/commit/31e7c24befef74034ec41f3723f0bb9504808ccc) New `to-real-file` native (posix version) for resolving canonicalized filenames (removing `..` and `.` path components, simplification of sequences of multiple slashes, removal of trailing slashes, and the resolution of symbolic links).
* [`e108cf`](https://github.com/Oldes/Rebol3/commit/e108cfc88b98a76cbbb63b957f87d8df31d781ad) Optionally truncate output of `probe` function
* [`2ee6c0`](https://github.com/Oldes/Rebol3/commit/2ee6c0ab964158554a6e44143fb32f14a0ab0b55) Improved `ar` codec to be able decode also System V (or GNU) variant files
* [`aaa70e`](https://github.com/Oldes/Rebol3/commit/aaa70ecb2d2c1688d681893290d7d6086645e303) Minimalistic Unix Archive File (AR) decoder
* [`fb0daa`](https://github.com/Oldes/Rebol3/commit/fb0daad20211a8f5acc9f870c70cf130e011ec19) New `ATz` action returning the seriest at 0-based position (index)
* [`101284`](https://github.com/Oldes/Rebol3/commit/101284b78663859639c4f56a1596d5581dea4af5) New `indexz?` action returning 0-based position (index) of the series
* [`7d3a04`](https://github.com/Oldes/Rebol3/commit/7d3a040091167ae11364d1ea21e938d3cc925f68) PICKZ and POKEZ functions for 0-based indexing
* [`877514`](https://github.com/Oldes/Rebol3/commit/8775144375e930c6c1981d7153367823b0607455) Don't allow `clear` on not opened port, allow `size?` on port

### Changes:

* [`5d1a4c`](https://github.com/Oldes/Rebol3/commit/5d1a4ca4809682cca267db5bfc01ba1a6426bd62) New wording for `repend/only` function doc string
* [`2cd3c8`](https://github.com/Oldes/Rebol3/commit/2cd3c8f19b3c9096e442b7d16f16435b7fd755bd) Rewritten `compress` and `decompress` so it's now compatible with the recent Red language. Now it also correctly handles output buffer expansion. It does not store the uncompress length at the tail of compressed data anymore!
* [`fb0daa`](https://github.com/Oldes/Rebol3/commit/fb0daad20211a8f5acc9f870c70cf130e011ec19) `at` action on image with pair position was 0-based, now is 1-based
* [`c8a593`](https://github.com/Oldes/Rebol3/commit/c8a59362bc977556ed1876b6cb5c78f915cf29dd) Replaced internal `boot-print` and `loud-print` functions with system log calls
* [`117e16`](https://github.com/Oldes/Rebol3/commit/117e1607d7adaa826a55c6f2855fd3393d2a9a8b) Limit output of `help` to single line if used with some value types
* [`c5c3b4`](https://github.com/Oldes/Rebol3/commit/c5c3b45c31465971057edc051d76c7d76a0b8b62) Made colors in console's prompt and result optional
* [`e4b1a8`](https://github.com/Oldes/Rebol3/commit/e4b1a8c33f62c54d05e743be9f064ac3312b6e29) Fine-tune decimal non-strict comparison (to cover also percent datatype)

### Fixes:

* [`502382`](https://github.com/Oldes/Rebol3/commit/50238280dd96bf386f3f4013026bb67bd4237d28) Generated RL defines were not counting with possibility of variadic arguments (like in RL_PRINT)
* [`cd463f`](https://github.com/Oldes/Rebol3/commit/cd463feb183af76795f040108bbf43c773dd07fd) Make sure that TO_OSX define is used even for 64bit builds
* [`5fcb00`](https://github.com/Oldes/Rebol3/commit/5fcb006327687a208bcc982bd2330d94e12bc89f) Using the new `compress` in build scripts
* [`720514`](https://github.com/Oldes/Rebol3/commit/72051415e4a944b7d5042d93aa7db66cf62d655d) Wrong member name when `RAW_MAIN_ARGS` define is used
* [`180264`](https://github.com/Oldes/Rebol3/commit/1802649a37489e208f70b0af4e1528a6b96b700c) Don't treat zero value in `mold/part` as unlimited output
* [`aaed74`](https://github.com/Oldes/Rebol3/commit/aaed7495afe4cb6e9cd65748aca982d48ed28349) Removed debug trace
* [`811bb3`](https://github.com/Oldes/Rebol3/commit/811bb35576964ea98b9df42a9968f4faa81c586f) Missing log traces in boot sequence
* [`14a003`](https://github.com/Oldes/Rebol3/commit/14a003d29f9a102465043b798be92e5360276ebc) Resolve correct canonized absolute path to main process executable
* [`4c6e88`](https://github.com/Oldes/Rebol3/commit/4c6e884252f3299ce2fb523c5e49653d487fdcd2) Non-void function does not return a value
* [`226461`](https://github.com/Oldes/Rebol3/commit/226461eda95012bc60965bb844ba190ea636ed8e) Convert TAB in the terminal input to 4 spaces (posix)
* [`2ac8c1`](https://github.com/Oldes/Rebol3/commit/2ac8c158cf701aaf5509f31ae3833dccf2d8e56e) Update file info after clearing file-port (posix)
* [`f1395c`](https://github.com/Oldes/Rebol3/commit/f1395c880bea0b8b941c90c0705fc4fd551f656b) Update index after `write` into a file-port
* [`c6276d`](https://github.com/Oldes/Rebol3/commit/c6276dd94671523aea55c87b57fc51f4f49ba683) Making sure that image codecs extensions are included after image codecs
* [`22de60`](https://github.com/Oldes/Rebol3/commit/22de60c667f007baaf84faec6c7f462313550689) WRITE on file-port does not update file size (posix fix)
* [`4127a7`](https://github.com/Oldes/Rebol3/commit/4127a7c2256da494326002994853d277ac82f96d) Non-void function does not return a value

## 2021 September

### Features:

* [`dedf26`](https://github.com/Oldes/Rebol3/commit/dedf268038beea1c45fcd4b18211e4d7bbc1dc4a) Allow making op! from action!
* [`d797a9`](https://github.com/Oldes/Rebol3/commit/d797a9324bc3b952781837beee58176e4c34e468) Support `to date! integer!` and `to integer! date!` using unixtime as an integer value
* [`10a5b4`](https://github.com/Oldes/Rebol3/commit/10a5b4d563cee137cc0af82e458cd46225889941) Implemented copy of nested maps
* [`14bc42`](https://github.com/Oldes/Rebol3/commit/14bc42f747104a0e1e9212e55f2a2fa4599b016e) PDF encoder input validation
* [`cb0521`](https://github.com/Oldes/Rebol3/commit/cb0521a502b20f7681a48f90bcc7705de3a1075f) Initial implementation of the PDF codec
* [`770a06`](https://github.com/Oldes/Rebol3/commit/770a06db2285ea173c0c4410bbbf78dfaf1af904) Make PNG filter's native functions optional
* [`e0d567`](https://github.com/Oldes/Rebol3/commit/e0d5675cb0054dec3edc15290771df3884f0e812) PNG's pre-compression filter algorithms exposed as `filter` and `unfilter` natives
* [`2947e8`](https://github.com/Oldes/Rebol3/commit/2947e8b4f8692ed2c6e90f78890df9a795fc943e) Print a human friendly list of available codecs on `help codecs`
* [`f27939`](https://github.com/Oldes/Rebol3/commit/f27939c1e572028601cfe02ed25ecf3be45d0eeb) Define codec's types
* [`53167c`](https://github.com/Oldes/Rebol3/commit/53167c325b1b2d264c1fff6b4862f4f085e6f417) Optimize powerof2 memory allocation
* [`e5868f`](https://github.com/Oldes/Rebol3/commit/e5868f4316882c20075fdd42e5084b6e96b34486) Made parse's `ahead` being a synonym for `and` for better compatibility with Red
* [`19974e`](https://github.com/Oldes/Rebol3/commit/19974e2d6b57b3c035046084c14aa3996451dec2) MacOS support for DirectDraw Surface `dds` encoding/decoding
* [`5bf90d`](https://github.com/Oldes/Rebol3/commit/5bf90d8141beddc414b18e6fbdefe92a77ac2cd9) `codecs/dds/size?` function for resolving dds image size without need of decoding
* [`64734d`](https://github.com/Oldes/Rebol3/commit/64734d53f1e33c1c6a6588f9fe42854f303654e7) `codecs/bmp/size?` function for resolving bmp image size without need of decoding
* [`1d3e2c`](https://github.com/Oldes/Rebol3/commit/1d3e2ce9adc27395754f1c102a971415dcbe6182) `codecs/gif/size?` function for resolving gif image size without need of decoding

### Changes:

* [`093728`](https://github.com/Oldes/Rebol3/commit/093728a7262a9b3ebda7a2d7e18b55bf9ded58db) Appending a `word` into an `object` sets its value to `unset` instead of `none`
* [`8bf1f2`](https://github.com/Oldes/Rebol3/commit/8bf1f2c5a202936d932460f9241d8ac88cc6be22) Allow `copy error!`
* [`9ce9d9`](https://github.com/Oldes/Rebol3/commit/9ce9d9a7321574c365234e7de8a4df4e5f47c8d8) Use plain `text` codec for `.cgi` files
* [`131090`](https://github.com/Oldes/Rebol3/commit/13109036d01bb43243d52aacf977876b1b7e50fe) Merged macOS and Windows OS codecs registrations
* [`62d8db`](https://github.com/Oldes/Rebol3/commit/62d8db79eb72c336e85c6e61356c5aa11c8f94ad) Using Gregg Irwin's `split` updates

### Fixes:

* [`c8198a`](https://github.com/Oldes/Rebol3/commit/c8198a87262ca9e95ad9f31c3f3b2a4f81aab958) Invalid byte-size of protected vector! data
* [`d40897`](https://github.com/Oldes/Rebol3/commit/d408975f13acd2906bae2b01784d5960a3cb6602) Handle loading 24bit WAV files + minor wav codec fixes
* [`b6c9a3`](https://github.com/Oldes/Rebol3/commit/b6c9a389fb0b063d2487926bcc5b01de641a1ab8) Unexpected slowdown when processing deep recursive loops
* [`ad2819`](https://github.com/Oldes/Rebol3/commit/ad281973854eb4012526b01c8a5628229a82aa8a) `resolve/all` bypassing `protect/hide`
* [`552ec9`](https://github.com/Oldes/Rebol3/commit/552ec9bda87a953115cadc2a741c11ed0a43b12b) Failed `decompress` when input data was not at head
* [`85de1d`](https://github.com/Oldes/Rebol3/commit/85de1db567ab46e3cd90e23500e342d36ec5d674) Corrected code for `copy map!`
* [`9d225e`](https://github.com/Oldes/Rebol3/commit/9d225e5b73976db2f82cb566a2697e2bf1b8b144) Resolved signed/unsigned mismatch warning
* [`ab3288`](https://github.com/Oldes/Rebol3/commit/ab32889365e99cffc9ff70d85cf44139e9bebaa6) Handle text encoded stream data
* [`5c7f72`](https://github.com/Oldes/Rebol3/commit/5c7f724c98be7e7994be2f3560d2fa9a5fdc2e59) Regression in `alter` function
* [`7d936d`](https://github.com/Oldes/Rebol3/commit/7d936dfabf8ea1fa3ff286c97b919ea38243623e) Silenced non-prototype definition warnings
* [`c52c47`](https://github.com/Oldes/Rebol3/commit/c52c47df8ee85258a9b3c7c25b76575b25d42449) Silenced possibly uninitialized variables warnings
* [`244c5b`](https://github.com/Oldes/Rebol3/commit/244c5b87d2330894d57481614465eb58285fcb14) Silenced non-prototype definition warnings
* [`de3d53`](https://github.com/Oldes/Rebol3/commit/de3d53caab842062ee4a7177ff4d38623d426cb9) Silenced possibly uninitialized variables warnings
* [`515149`](https://github.com/Oldes/Rebol3/commit/515149ccac5cb4ae0e0cbbbea9cec73ca8f24b86) Removed unused variables
* [`8857d4`](https://github.com/Oldes/Rebol3/commit/8857d44b9b9ca1802a4f0ed569d796c962ee2aeb) Silenced lower precedence warning
* [`f10ea4`](https://github.com/Oldes/Rebol3/commit/f10ea4a392bacef2d76adb06a5846743a0ff265c) Silenced discarding qualifiers warning
* [`431bb3`](https://github.com/Oldes/Rebol3/commit/431bb3daaedaa60e6ced257329cc9a7ca7cc4304) Silenced type specifier missing warning
* [`b83216`](https://github.com/Oldes/Rebol3/commit/b83216937f3b799643c6f1004dbaef0a2333dd3f) Silenced macro redefined warnings
* [`cfb060`](https://github.com/Oldes/Rebol3/commit/cfb060f366648c25508737e987224383caf0fe3c) Silenced incompatible pointer types passing warning
* [`218e5c`](https://github.com/Oldes/Rebol3/commit/218e5c70452a098ce36cacd6b1b5d7570c949510) Silenced discarding qualifiers warning
* [`e97679`](https://github.com/Oldes/Rebol3/commit/e97679fd7b9840db4afb3ad989e49c981fc2d3da) Silenced incompatible function pointer warning
* [`f46546`](https://github.com/Oldes/Rebol3/commit/f465469f1c563357a6d477cf908308c3217dfed2) Silenced incompatible pointer warnings
* [`cf86e5`](https://github.com/Oldes/Rebol3/commit/cf86e55d5f39ff52e7f1f391933cd33a7528105a) Recognize also older GIF87a version

## 2021 August

### Features:

* [`7591f4`](https://github.com/Oldes/Rebol3/commit/7591f49fe3f6326c321633d036ce5f5e46e5e126) Allow FIND binary! char!
* [`547f35`](https://github.com/Oldes/Rebol3/commit/547f357ad356cfb96a3e9228cda4f5af62b5c139) Optimized `parse` when checking `ref!` and `tag!` datatypes
* [`384602`](https://github.com/Oldes/Rebol3/commit/3846021ea486701b528b8636a84415cf4254cc7d) `codecs/jpeg/size?` function for resolving jpeg image size without need of decoding
* [`b35117`](https://github.com/Oldes/Rebol3/commit/b35117a9cca11260874456f89bbacd994b2bdcce) MacOS basic clipboard port implementation
* [`dbe8aa`](https://github.com/Oldes/Rebol3/commit/dbe8aa5737ffde22f199c78adb38b626b15576a2) MacOS image codecs cleanup and added support for `haif` encoding/decoding
* [`894708`](https://github.com/Oldes/Rebol3/commit/894708f6965353a2c95ac507e4d6ba259dda05c6) Using native system image codecs on macOS

### Changes:

* [`db5ebf`](https://github.com/Oldes/Rebol3/commit/db5ebf22fd3bbfa5297e18cc610438cfd03aff89) `FIND bitset! block!` is looking for ALL of the values in the block by default
* [`807740`](https://github.com/Oldes/Rebol3/commit/80774077958b63fda663f558d4dc6c9dcaa00bdd) Make `find/match` not imply `/tail`

### Fixes:

* [`458f59`](https://github.com/Oldes/Rebol3/commit/458f5991f967a75d8541056f4eeb7b3f8ec4ee66) Making `find` case-insensitive by default on BITSET when used with a CHAR
* [`9817d0`](https://github.com/Oldes/Rebol3/commit/9817d01adfb48f710433b1f7f5563c07d907ceb6) FIND string! tag! misses < and >
* [`9e0f24`](https://github.com/Oldes/Rebol3/commit/9e0f2429e23b7a37cbd36547b1a4bf28c69a5ef4) Make `struct!` result visible in console
* [`6f9f15`](https://github.com/Oldes/Rebol3/commit/6f9f15ef6d560c2a3dc46e22a4e710fecd8af7ca) Resolved some of the unused variable warnings
* [`1a3606`](https://github.com/Oldes/Rebol3/commit/1a3606054982f92887146d130ed4f92c1e8229a5) Some of xcode warnings
* [`9f0b07`](https://github.com/Oldes/Rebol3/commit/9f0b072b4793626efa139673dc53c0edfb9b31f7) Resolved some of possibly uninitialized variables warnings
* [`95016d`](https://github.com/Oldes/Rebol3/commit/95016d9db16879dad8290560887eaa193f13d505) Missing native jpeg codec on posix platforms

## 2021 July

### Features:

* [`515c16`](https://github.com/Oldes/Rebol3/commit/515c163df1b2803e20c0f2685be22d3e1f89a348) Conversion of `struct!` to `binary!`
* [`319531`](https://github.com/Oldes/Rebol3/commit/319531e6149cab65dd3066d2c24c414acc34e7c4) Add support for `word!` struct fields
* [`476327`](https://github.com/Oldes/Rebol3/commit/4763277d8d27fe98b92d91da83fa930f2b9a3314) Including Atronix's source of `struct!` datatype implementation
* [`af7772`](https://github.com/Oldes/Rebol3/commit/af77722d5d8f827afd286724147fdf7f2942c92a) Better deal with stack size definition while compilation
* [`f1b74c`](https://github.com/Oldes/Rebol3/commit/f1b74c7ed3741b6c65107954cc2850a0a3b125ae) Store last console result in `system/state/last-result`
* [`4fab4c`](https://github.com/Oldes/Rebol3/commit/4fab4ce4b8ff41475e71b47649f22c85e554d48a) `REFLECT module! 'title` could return the module header title

### Changes:

* [`e564c5`](https://github.com/Oldes/Rebol3/commit/e564c594037aaffe913882999b7a94d3b45eb2e4) Using `!` at the end of struct value type specification
* [`0248e8`](https://github.com/Oldes/Rebol3/commit/0248e8599611ac4ef94136a70887e37023e6276b) Revisited vector construction syntax types
  Now accepting for example `i16!` and `uint16!` instead of previous `si16!` and `ui16!`.
* [`2cc8c7`](https://github.com/Oldes/Rebol3/commit/2cc8c7351c2ead999b73e47cad57d5890050265f) Disable support for Rebol values in struct

### Fixes:

* [`72d0d2`](https://github.com/Oldes/Rebol3/commit/72d0d2fafbd202531aeba81c15c472fec7516b9e) Don't allow code evaluation inside struct construction specification
* [`d135f0`](https://github.com/Oldes/Rebol3/commit/d135f0b34787e7f1e87ee9f443ad7c319e8fb00b) Removed debug traces
* [`49481f`](https://github.com/Oldes/Rebol3/commit/49481f3727dd804e9cd5352f2f1cab4f96e3c841) Allow pool size to be larger than series total length in `evoke` memory check
* [`6901c1`](https://github.com/Oldes/Rebol3/commit/6901c173cb09e08d1462093f488d36ac9a8d5e2a) Validate missing struct initialization field value
* [`1fb779`](https://github.com/Oldes/Rebol3/commit/1fb779f5f0793f08d1c1c76cf56dfc6f915d4166) Possible use of uninitialized variable
* [`fb6624`](https://github.com/Oldes/Rebol3/commit/fb662464fc2915fbde9a63eb9b266603d48185d5) Crash when using `do` on a function which returns itself
* [`d1ff72`](https://github.com/Oldes/Rebol3/commit/d1ff72d809b930b489a2a98302fa70d3cf755c7a) Crash when using `context?` on function's local word outside of its function

## 2021 June

### Features:

* [`c75649`](https://github.com/Oldes/Rebol3/commit/c756498cfe767872be880162305e53dc37080754) New `blur` native function for fast Gaussian blur of images
  Based on Ivan Kuckir's Fastest Gaussian blur implementation algorithm from article: http://blog.ivank.net/fastest-gaussian-blur.html
  
  Usage example:
  ```
  image: load %some-image.jpg
  blur image 15                 ;@@ image is modified!
  save %blured-image.png image
  
  ```
* [`77426b`](https://github.com/Oldes/Rebol3/commit/77426bf11820e4730e7ce8020128a2075ec11b4e) Munge code updated using the new built-in ZIP encoder
* [`52b269`](https://github.com/Oldes/Rebol3/commit/52b269660e5f027b3641bba0acd2815414c6110e) Added `ZIP` encoder
* [`13d7c9`](https://github.com/Oldes/Rebol3/commit/13d7c9ec53ed3424746eff95986b2c725a7475da) Munge (quickly modified to be runnable from Oldes' R3 derivative)

### Changes:

* [`9c5892`](https://github.com/Oldes/Rebol3/commit/9c58928aaabf1309880744eb1ded6f08bc084f1d) `RL_Register_Handle` from extension returns word's symbol id
* [`99a5c2`](https://github.com/Oldes/Rebol3/commit/99a5c287b3d5579f59850fb0396f4128233f0ce7) Swapped `crc` and `data` values in ZIP decoder's result

### Fixes:

* [`99b5db`](https://github.com/Oldes/Rebol3/commit/99b5db8243bd4bce2c0fedcc18d6b024a33d0ee1) Pass handle flags from external (extension) argument to the internal (host) value
* [`cf1a20`](https://github.com/Oldes/Rebol3/commit/cf1a205a29bf1b7b69f7b14b3b3fe043881804cc) Crash when using special handle's object (HOB)
* [`a03a3b`](https://github.com/Oldes/Rebol3/commit/a03a3b5c207939bc204cb826166409b04b1342b6) Munge - a little bit more optimizations
* [`7d8c10`](https://github.com/Oldes/Rebol3/commit/7d8c1084e59220db159bfc673955b76bd52b097d) Munge - make working loading excel sheet even in case when there is no info about number of columns
* [`da902d`](https://github.com/Oldes/Rebol3/commit/da902de1c79d0afa789a1c155f7c503318913179) Munge - `sheets?` function was not working
* [`73b7f3`](https://github.com/Oldes/Rebol3/commit/73b7f38ec2a2a386793747229937630a005f8bea) Munge - replaced ZIP unarchive code with R3's ZIP codec

## 2021 May

### Features:

* [`a2bcfe`](https://github.com/Oldes/Rebol3/commit/a2bcfed99cb42e3b0e2c245172752e584496aaa5) Allow conversion from `object!` to `map!`
* [`933514`](https://github.com/Oldes/Rebol3/commit/933514dafc74c307ceeca636e6b14b0bcdc5eabf) Reimplemented `mod` and `modulo` in C, `//` is now `op!` for `modulo` and `%` is now `op!` for `remainder`
* [`72a7b9`](https://github.com/Oldes/Rebol3/commit/72a7b98779256d74766af0436bdb27657f9a7e48) Allow making `op!` from `function!`
* [`c9aa62`](https://github.com/Oldes/Rebol3/commit/c9aa62861ca8e617b1b2ab0a3fb3785c52576231) Allow _slash_ words
* [`30b179`](https://github.com/Oldes/Rebol3/commit/30b179f667c4c255fe78f806a96b02870d6f92b4) Added simple test extension + handling `RXR_BAD_ARGS` value from extension command call

### Changes:

* [`6dad57`](https://github.com/Oldes/Rebol3/commit/6dad57f8d44bb19cf2426bdc4bf2948a5102122f) Don't protect `system/dialects` as it's not used anyway
* [`bc48fe`](https://github.com/Oldes/Rebol3/commit/bc48fe8db759d29620f08a2bfc12dd8dcaf2b4fc) Allow direct comparison of `time!` with `number!` types
* [`c31151`](https://github.com/Oldes/Rebol3/commit/c311519a84bca3999a24a79fa4ac0788899ed3f8) Rename MAXIMUM-OF and MINIMUM-OF to FIND-MAX and FIND-MIN
* [`c72b15`](https://github.com/Oldes/Rebol3/commit/c72b158a41acff007267e707fcfdfedd370b9489) Better sorting of specific runtime extension module values

### Fixes:

* [`e868db`](https://github.com/Oldes/Rebol3/commit/e868dba356b15915cfe0d09451f686fee40f9dab) Not truncating random time to seconds only
* [`512505`](https://github.com/Oldes/Rebol3/commit/512505109fdf0849569d6e9a12f36554ead1768f) Import of native extension (library) on Windows

## 2021 April

### Fixes:

* [`69cd34`](https://github.com/Oldes/Rebol3/commit/69cd3437c6a0c2b59f0d7d9c0e6e18bc470da132) In cloud builds it's not possible to resolve console width
* [`1075e8`](https://github.com/Oldes/Rebol3/commit/1075e8a26c168c7e1da8ddc3fb47cb52ae127376) Not changing default definition of `binary-base` from `prebol` module
* [`6384bc`](https://github.com/Oldes/Rebol3/commit/6384bc1e445d040b9864456de55017fd14f163da) Typo in ico's identify function
* [`a7adc4`](https://github.com/Oldes/Rebol3/commit/a7adc4e3740d2397c691053fb87865bf6ea65382) `%` is not an empty path anymore
* [`4167ab`](https://github.com/Oldes/Rebol3/commit/4167abe3ba151c8eda9103dcc96c630af976fcc7) Allow various variants of arrow-like words (fixed CC warning)
* [`19f86b`](https://github.com/Oldes/Rebol3/commit/19f86be34a5dc6c521e8d14ac3587fbb0c192533) Make sure that console's input buffer is not growing over 2GB
* [`cefbc9`](https://github.com/Oldes/Rebol3/commit/cefbc96afb175fcb9d3ee611c70398aa7a01b73f) Allow various variants of arrow-like words
* [`4b2eda`](https://github.com/Oldes/Rebol3/commit/4b2edac158097af5011b41d382aacc44e1bd2cfa) Allow comparison of `char!` with `integer!`

## 2021 March

### Features:

* [`e9b0bb`](https://github.com/Oldes/Rebol3/commit/e9b0bb1cae9ca725fc1c8028726444b6cfee9bf7) `debase` accepting `any-string!` for it's value
* [`5ed41d`](https://github.com/Oldes/Rebol3/commit/5ed41d3843d4ad38f8a57990e6651b455c37cd06) Allow creation of `path!` from any value
* [`46e3a2`](https://github.com/Oldes/Rebol3/commit/46e3a2ce33a4b175ac7a54d77aa6db89a93d12e3) Extended PNG codec with `chunks` function
* [`a5ba0f`](https://github.com/Oldes/Rebol3/commit/a5ba0fdb5e87802b933db7b32ffa503782636597) Added a simple codec for decoding/encoding ICO files
* [`fd67a5`](https://github.com/Oldes/Rebol3/commit/fd67a59b3bdb410145f9dd2e2d1b95960e73b8ba) Added a simple codec for decoding/encoding ICO files
* [`3f45b9`](https://github.com/Oldes/Rebol3/commit/3f45b9f832db74826182e2373b0736468c298fa3) Updated `prebol` (Rebol Preprocessor) to be a real module working with current Rebol
* [`22a6d0`](https://github.com/Oldes/Rebol3/commit/22a6d0b531a8d2620067eac1b9d45128c4062df2) Added possibility not to process default application arguments on boot

### Changes:

* [`aece05`](https://github.com/Oldes/Rebol3/commit/aece05f17345994ff4c5154a64b327f714667631) Have `%` to be a valid word and not an empty file
* [`1e71aa`](https://github.com/Oldes/Rebol3/commit/1e71aa39754d7acd5114450166b997e77cd9edd1) Removed ANSI related functions from colors mezzanine file

### Fixes:

* [`277ab5`](https://github.com/Oldes/Rebol3/commit/277ab51cf479bfb97833d45905f0ba50f96ef749) Parse rule fails when input is modified and so does not advance
* [`ea3aab`](https://github.com/Oldes/Rebol3/commit/ea3aab8bf1bad4da84bf82fa501e2b91f79e7484) Crash on boot on Windows7 (regression since 3.4.0)
* [`452893`](https://github.com/Oldes/Rebol3/commit/452893f22abeca6b3fc8b8973b28f035ca982ed1) Crash on boot on Windows7 (regression since 3.4.0)
* [`d83cf1`](https://github.com/Oldes/Rebol3/commit/d83cf1c0b2cb39a2ad0d0587d873ee3b54ed9da2) HTTP redirection does not handle correctly locations with url-encoded content

## 2021 February

### Features:

* [`552e2c`](https://github.com/Oldes/Rebol3/commit/552e2c0d0ff8943d665dc040886efc4dfe700af9) Parse string! using any-string!
* [`9249a3`](https://github.com/Oldes/Rebol3/commit/9249a364380352f498a7b91abfbc5b66d1d7b463) Exporting new library functions for registering, creation and releasing handles (so these may be also used from external native modules.
* [`c990e8`](https://github.com/Oldes/Rebol3/commit/c990e82e678d578db2429173caf989c2dead302f) Simple TCP port scanner (module)
* [`b6cbac`](https://github.com/Oldes/Rebol3/commit/b6cbac24a999355934784a604bc2c4146f0e5df5) Do some minimal extension's module protection
* [`4d44e9`](https://github.com/Oldes/Rebol3/commit/4d44e997c68951a5dc651023610ced7355ed5398) When $SHELL is not set on POSIX OS, `/bin/sh` is used as a default

### Changes:

* [`bbf89c`](https://github.com/Oldes/Rebol3/commit/bbf89ca6e7c9b810080c462a68cf3f678a502839) `DELETE` returning `false` if called on not existing file or directory
* [`6809c8`](https://github.com/Oldes/Rebol3/commit/6809c89dc01ed720401a8ce0a65f15f57c41a5df) Registered new system ID 0.4.46 as a 64bit Linux using `musl` compiler (musl.libc.org)
* [`d352af`](https://github.com/Oldes/Rebol3/commit/d352af0674aa647e942aeee82618821ff5fae7c3) Renamed module's local context from `local-lib` to `lib-local`
* [`500d3d`](https://github.com/Oldes/Rebol3/commit/500d3da6542db487cd190a48e8bb83000707b3e1) Force UTF-8 console output code page on Windows
* [`f09840`](https://github.com/Oldes/Rebol3/commit/f09840e2b6fcafc72ae27f5409181d1da2ddba4f) Using NOT_FOUND instead of 0 in handle's registration

### Fixes:

* [`a2b1a4`](https://github.com/Oldes/Rebol3/commit/a2b1a408a804cf865cf0078b8723782b27977763) Updating `PWD` system environment variable after each directory change
* [`9b5723`](https://github.com/Oldes/Rebol3/commit/9b5723a2c36d5ce1bdd97a8f707e798d51e510f4) Resolved crash in lower level (debugging) value formatter
* [`1864b6`](https://github.com/Oldes/Rebol3/commit/1864b67c28d90f06bbc3089184f7167a1c2564f8) Better results from `change-dir` function (on error and also on success).
* [`a90ee0`](https://github.com/Oldes/Rebol3/commit/a90ee0798a06a831b41d2bf42f2de0f542866794) Crash when re-opening listening port multiple times
* [`98d66e`](https://github.com/Oldes/Rebol3/commit/98d66e5f7ba5c5a44c605d49aae2651275c994ed) Using at least one image in the ICO file as a PNG encoded in RGBA32 format
* [`82f0ae`](https://github.com/Oldes/Rebol3/commit/82f0ae3f15e3133edc3e805e89adf986b61bff4a) Updated banners look
* [`be2463`](https://github.com/Oldes/Rebol3/commit/be2463a4bd0ca9837cf1b7ce6adc7015e8350af7) Memory leak in `list-env` function
* [`d1e316`](https://github.com/Oldes/Rebol3/commit/d1e31695b1f4e70465f9ff5c03d4825c43df6c18) `to word!` accepting delimiters in the input string
* [`a81b86`](https://github.com/Oldes/Rebol3/commit/a81b862ad71b5dc095c51a57a0cd1c97b7259a0d) `to word!` accepting delimiters in the input string

## 2021 January

### Features:

* [`102488`](https://github.com/Oldes/Rebol3/commit/102488d30c3a4be94ea9ada3ccb613154c38d7ca) POSIX wildcard support in `read`'s `file` argument
* [`b83660`](https://github.com/Oldes/Rebol3/commit/b836603cb640e36d5cd1f551b981965f2d721959) Replaced `dir?` mezzanine as a `native!` with optional `/check` refinement
* [`0f71dd`](https://github.com/Oldes/Rebol3/commit/0f71dd64598a3d0e2502480a5ca5e9988f7ce20b) Macro for getting c-string from a symbol
* [`65ac08`](https://github.com/Oldes/Rebol3/commit/65ac088a875ab365f196f47e0dd5288e971ad72b) Register `*.key` suffix for SSH keys
* [`15eaec`](https://github.com/Oldes/Rebol3/commit/15eaec4cf2a9741584b9e93672e4bebe8d080694) Allow to decode `*.crt` files stored in `PKIX` format
* [`88086a`](https://github.com/Oldes/Rebol3/commit/88086a5dabbd86c8249f6f7ced5dfcb645f0e5ec) Preliminary support for system managed handles
* [`2cd73c`](https://github.com/Oldes/Rebol3/commit/2cd73c6e3a637cd6c9c99afc3f911df069fc9143) `break/return` now has priority over regular return value from `remove-each`
* [`355df8`](https://github.com/Oldes/Rebol3/commit/355df845dbb1bdd2062e8609455f312618bdc6c0) Support `map!` in `remove-each`
* [`1ab00e`](https://github.com/Oldes/Rebol3/commit/1ab00e66004196b8f83deef20b356b1e780e5b64) Improved dealing with automatically collected natives from C sources
* [`775155`](https://github.com/Oldes/Rebol3/commit/77515519655a579fbef99f7e185a27a4451718d7) Replacing old checksum code with implementations from the Mbed TLS library (including optionally also SHA224, MD4 and RIPE-MD-160 checksums)
* [`40ddbf`](https://github.com/Oldes/Rebol3/commit/40ddbf368175351817f476bc906ab365bbbd8888) Introducing user defined config file for compile options

  Using INCLUDE_* instead of USE_* definition names for optional parts

### Changes:

* [`0f4d26`](https://github.com/Oldes/Rebol3/commit/0f4d26ce68a891b5ea4045f8a7a64d04d6c5ced8) `REMOVE-EACH` returns the modified series at the argument position as in Rebol2

  Added a `REMOVE-EACH/count` refinement, which toggles `REMOVE-EACH` to returning the removal count.

* [`8f30a4`](https://github.com/Oldes/Rebol3/commit/8f30a41303cd87dd5d250a642089ab55750be7ef) Making `CHECKSUM` more compatible with Red language

  * removed `/method`; now method is always used as second argument
  * removed `/tcp`; now accessible as: `checksum data 'tcp`
  * removed `/secure`; now accessible as: `checksum data 'sha1` (`sha1` is not considered as too secure anyway these days)
  * removed `/hash`; now accessible as: `checksum/with value 'hash 256`
  * renamed `/key` to `/with`; given `spec` is used as a key for HMAC and or size of hash table
  
  Available checksum methods are now listed in: `system/catalog/checksums`
  
  Note: checksum port does not support these methods: adler32, crc24, crc32 and tcp

### Fixes:

* [`5060ce`](https://github.com/Oldes/Rebol3/commit/5060cea88b8bbfa67192308dc0c600ea4c780cc6) HTTPD: allow target to be a none in decode-target
* [`040c9c`](https://github.com/Oldes/Rebol3/commit/040c9c689376dd7b898ea22e944646df7bd18377) POSIX: `query` not handling correctly file names with non ASCII chars
* [`2d5a75`](https://github.com/Oldes/Rebol3/commit/2d5a759b0b09e7a8b850c62dea25afe77cd79c53) Not possible to read url after failed lookup on posix
* [`676405`](https://github.com/Oldes/Rebol3/commit/6764052442e161abcb09437d11a846498c72f580) Disposing all handles on exit (so Valgrind doesn't report errors)
* [`e29986`](https://github.com/Oldes/Rebol3/commit/e299860a2281db57c870aaddcfc2afa7433d3412) Invalid handle's spec offset
* [`681851`](https://github.com/Oldes/Rebol3/commit/68185158ec4a1bf0c5a9033fa83c8e13deafb7bf) Updated build scripts to be compatible with the recent `CHECKSUM` change
* [`516dfe`](https://github.com/Oldes/Rebol3/commit/516dfee630e95f458bb51016a8616050635d5b5c) `reword` expecting that maps cannot accept `none` values
* [`b7118b`](https://github.com/Oldes/Rebol3/commit/b7118b71860c0ede346abe116bfad6707071f936) `foreach` not seeing `none` values in `map!`
* [`98b806`](https://github.com/Oldes/Rebol3/commit/98b806dcbf864051727b0bb5fde7330f68f6cc9b) Update MIDI device code to use the new option's configuration
* [`9723d4`](https://github.com/Oldes/Rebol3/commit/9723d43c52caf3fd70d961f6752954ce525d386e) Add missing library dependency for `request-dir` function on Windows
* [`61ab30`](https://github.com/Oldes/Rebol3/commit/61ab3023ac7bcefa6bbb42d488b70d3a71ca0e32) Avoiding compiler warnings
* [`a96958`](https://github.com/Oldes/Rebol3/commit/a969582ac6b20daee6699bd7901b0203e91ac8fa) Resolved redefinition warning

## 2020 December

### Features:

* [`379072`](https://github.com/Oldes/Rebol3/commit/3790723fb92eab6f0792f7b5b8e55269098a0354) Unified line breaks when converting map to block
* [`33e013`](https://github.com/Oldes/Rebol3/commit/33e01309018001ac22c992b4c718f3f49accbfb6) Implemented `request-dir` on Windows
* [`d5efc5`](https://github.com/Oldes/Rebol3/commit/d5efc5ee048f6593d9f23b18acd16f2bb4862c7e) Revisited `delete-dir` function
* [`6fc873`](https://github.com/Oldes/Rebol3/commit/6fc8732e5309aba4a88b00bdea1381ec49d6db0f) `wildcard` function - returns block of absolute path files filtered using * and ? wildcards.
* [`7b1ec8`](https://github.com/Oldes/Rebol3/commit/7b1ec82cdd9326ef1c033d164588e2ae42402c58) Allowed to make `block!` using `pair!`
* [`52a67d`](https://github.com/Oldes/Rebol3/commit/52a67dbb6a6d7d1b3112eadca61f0fb8a6bd08f3) Allow line-comments inside binary value specification.
* [`9ca83b`](https://github.com/Oldes/Rebol3/commit/9ca83b2c5c1d1a265bcc91492b3ccfae842a56d9) Extended system's `log` function with `/error` refinement
* [`b42fc9`](https://github.com/Oldes/Rebol3/commit/b42fc97976314261d6dd45d1e202d4cc64bee55a) Allow `none` as a `map!` value, implement `remove/key`

### Changes:

* [`0688a1`](https://github.com/Oldes/Rebol3/commit/0688a196164be4678cbbb53734b8b199b46f1a30) Have EXISTS? return the type field or none instead of true/false
* [`b71d81`](https://github.com/Oldes/Rebol3/commit/b71d8164ea57afb6b749558bf34cfe38c8c873e0) Allow `unset none` to be a no-op instead of throwing an error
* [`0c7beb`](https://github.com/Oldes/Rebol3/commit/0c7bebd9c6f4c5d83199d92f7a4785d91fbf71a8) Replace `bind?` and `bound?` with `context?`
* [`b19a10`](https://github.com/Oldes/Rebol3/commit/b19a1095db46634e1f480005fc0e249b4393e450) Simplify `TO BLOCK!` and complex construct via `MAKE BLOCK!`

  This is major change which may break some things (in a few places there may be need to replace `to block!` with `make block!`), but it's good to do it now.
  
  See the included unit tests for currently expected results!

* [`d07af9`](https://github.com/Oldes/Rebol3/commit/d07af993370be7f677826ee411fe258b4003f6ab) Don't allow `to` conversion from `logic!` to `integer!`, `money!` and `percent!`
* [`54493f`](https://github.com/Oldes/Rebol3/commit/54493f7e25aa2e58f0d4fb5d2f19ce1e2b6a5de0) Evaluating `user.reb` file if found in user's home directory.

  Home directory (`system/options/home`) can be set by using one of `REBOL_HOME`, `USERPROFILE` or `HOME` environmental variables (preference in this order). If none of these is defined, as home is used same directory as directory with used executable.

### Fixes:

* [`87c5fe`](https://github.com/Oldes/Rebol3/commit/87c5fe50aa0735a9840d85ab03c6cbc8ad99d819) MOLD NEW-LINE? behavior indents too much in BLOCK!s
* [`d2583a`](https://github.com/Oldes/Rebol3/commit/d2583a55f8201498bc49f48d960c15881ef5593e) Temporary fix for failed read of some HTTPS sites
* [`167eac`](https://github.com/Oldes/Rebol3/commit/167eac4a05af5dc019a612b76eb96b2228b6ec1a) `request-dir` not compiled on 32bit Windows
* [`70855c`](https://github.com/Oldes/Rebol3/commit/70855c502b98476537ca212dde430cfc86ee1af9) Including D2D related files
* [`dbcb3d`](https://github.com/Oldes/Rebol3/commit/dbcb3da27039a7eda6a06e079a9476d6c0aa7eed) Removed memory dispose pool related debug traces
* [`851e2f`](https://github.com/Oldes/Rebol3/commit/851e2f3afaf66b592ec394dfccd6c45efd799e86) Make old build script compatible with recent changes
* [`68beea`](https://github.com/Oldes/Rebol3/commit/68beeaf75f8d5bd9da76ac94d01851c3aa291ea6) Extra tokens at end of #endif directive
* [`2052b2`](https://github.com/Oldes/Rebol3/commit/2052b222143ccf8f9aef2c51a9a78f2a765204b7) Move `wildcard` from `base` to `mezz` + test
* [`761b44`](https://github.com/Oldes/Rebol3/commit/761b4407f5dc3fc7c278feac06fe871c298a9b1c) Comparison of blocks where one is past tail (missing file)
* [`85e592`](https://github.com/Oldes/Rebol3/commit/85e5920f363e9b68f2a7cae9dfed4a0002df6f4e) Comparison of blocks where one is past tail
* [`cedd82`](https://github.com/Oldes/Rebol3/commit/cedd82138f3e8d45ff2ba6381bcb205c9156dbd4) Report full failed expression when `assert` fails
* [`c167c2`](https://github.com/Oldes/Rebol3/commit/c167c29553689b901d18421368dc68519197a21c) Don't allow making a `module!` from `integer!` and `map!`
* [`10932d`](https://github.com/Oldes/Rebol3/commit/10932d1b1fe36b3dc5b0fec95c07512e0febf9cb) Possible fix of Valgrind's `Source and destination overlap in memcpy` error.
* [`dba14b`](https://github.com/Oldes/Rebol3/commit/dba14b77d30fe4da4428d9ae5b045cfa1c9ef325) TO PAIR! causes an error with a "long" string
* [`577b45`](https://github.com/Oldes/Rebol3/commit/577b45a7fd7985509b77283eb32167afb2f7f7dd) PARSE's INSERT used after a seek puts data at the position before the seek
* [`1752e3`](https://github.com/Oldes/Rebol3/commit/1752e3cc5dc726ddec9c2ca1216e4be7d4bb5324) Don't use ANSI color sequence in formed error

## 2020 November

### Features:

* [`030915`](https://github.com/Oldes/Rebol3/commit/0309156ad657f95e8ce674b74266cfed3e6f9338) Add possibility to set date's `/utc` and `/yearday`
* [`c49f86`](https://github.com/Oldes/Rebol3/commit/c49f86aabdfdae13a793c41555c82075f7599293) Set `local-lib` to refer to the local import library
* [`52ee40`](https://github.com/Oldes/Rebol3/commit/52ee407276ddb6e34f1592a21ea00704fcafae4d) Added `supplement` mezzanine function which appends a value into a block if not yet present
* [`2aa6c6`](https://github.com/Oldes/Rebol3/commit/2aa6c68300ffc50c163af5c608a14bca8ff8b18b) Allow direct conversion from tag! to word! (if it has valid chars)
* [`08bded`](https://github.com/Oldes/Rebol3/commit/08bdedfbbf3837578a2ea32a60d5c00ab8a5fa57) Way to combine DATE! and TIME! without a variable
* [`77a412`](https://github.com/Oldes/Rebol3/commit/77a4122b4c04a52d5b045cef40f1e8d8369ef9d5) Extend `PNG` codec with `size?` function (returning image size of the binary)
* [`5455ee`](https://github.com/Oldes/Rebol3/commit/5455ee479bb7a0bca1eb2b61cbbfc832908de1f6) Including `as-colors` module with functions, which decorates any value with bright ANSI color sequences
* [`fd1790`](https://github.com/Oldes/Rebol3/commit/fd179029ea7b6a4570fe638b660d4849b0907344) BinCode - match binary!

### Changes:

* [`6ba777`](https://github.com/Oldes/Rebol3/commit/6ba777e64bbc5a193a103e4288dc0c2cca7b20cf) Removed mention of boot in `boot-print` and `loud-print` doc-strings (these can be use also after boot)
* [`a253f5`](https://github.com/Oldes/Rebol3/commit/a253f5e8ab4b33bb2318a534d5f5c53f73d08309) A plain, explicit QUIT from nested scripts returning UNSET! instead of 0
* [`a6a07f`](https://github.com/Oldes/Rebol3/commit/a6a07f8bea642c20652dca1b446c90e9103f6eab) Updated Windows' icon file
* [`42ef0d`](https://github.com/Oldes/Rebol3/commit/42ef0d5c8720f9ab9678b3556933a94ff3e7d8f0) Updated banner and `license` output

### Fixes:

* [`04d633`](https://github.com/Oldes/Rebol3/commit/04d63353b1365b4bdb28afabcd35596be71ec90a) Removing leaked traces in posix version
* [`112c08`](https://github.com/Oldes/Rebol3/commit/112c087b84da66e904f300a688bb76d23dfae6f6) RSA related memory leak in TLS protocol
* [`f6e480`](https://github.com/Oldes/Rebol3/commit/f6e4805692c688d496c62dfc53877fa7a9a4b3ce) Typo in PNG codec extension code
* [`c7ffb5`](https://github.com/Oldes/Rebol3/commit/c7ffb5e37e180bd288f91aa45b3704c10572e600) Don't allow specific `lit-word!` and `get-word!` cases
* [`173968`](https://github.com/Oldes/Rebol3/commit/1739683623264cc51c60f13020709c48fd145a67) Reset ANSI color after `secure` help output
* [`f7e8e8`](https://github.com/Oldes/Rebol3/commit/f7e8e8fd4a6484f23a02b44bba77f8920068bfab) PROTECT and UNPROTECT bitset!
* [`7c409f`](https://github.com/Oldes/Rebol3/commit/7c409fb2b19a4e563fcf61214b37d1c5b1bad234) Conversion of bitset to binary does not count with complement bitset state
* [`ced591`](https://github.com/Oldes/Rebol3/commit/ced59161e81edd409a0b822614a114b9afdaddb9) Open does not reset the internal file pointer of a previously closed file port
* [`145ac8`](https://github.com/Oldes/Rebol3/commit/145ac8f913c3df909ea118c92e8970eb8e6a1b99) Cause an error in PARSE for invalid args to TO

## 2020 October

### Features:

* [`159b82`](https://github.com/Oldes/Rebol3/commit/159b82f2fe5ac5ce363d661f13df275e9de63a07) Bincode - added zero-based `ATz` and `INDEXz` commands
* [`08aa99`](https://github.com/Oldes/Rebol3/commit/08aa9998127b3875ba3b12eec43f9a2406c8e8d3) Support for read/part (using Range request with read/part/binary)
* [`c5e33f`](https://github.com/Oldes/Rebol3/commit/c5e33ffe4ad2f33ad44aeecc3853ef430645e55f) `sqrt` native which accepts only decimals
* [`b44fca`](https://github.com/Oldes/Rebol3/commit/b44fca36c240a8f6ae3be20d0f6c6bcd4714c560) Use default `key` event type when doing: `make event! [key: #"A"]`
* [`b53caf`](https://github.com/Oldes/Rebol3/commit/b53caf8943c4208732821c0d2903d7c96e1cff92) `find/same` and `select/same`
* [`c563d4`](https://github.com/Oldes/Rebol3/commit/c563d429f00d3a0ac215da1726007fa43fc5621b) Optionally write any (molded) value into clipboard (by default disabled)
* [`5f28c9`](https://github.com/Oldes/Rebol3/commit/5f28c9ac01c0e7f9d9b7a431144e89e7c51013e5) Improve `import/version` error message in case when module has no name

  before:
  ```
  >> import/version module [version: 1.0.0][a: 1] 2.0.0
  ** syntax error: this script needs none 2.0.0 or better to run correctly
  ```
  now:
  ```
  ** syntax error: this script needs version 2.0.0 or better to run correctly
  ```
* [`57384f`](https://github.com/Oldes/Rebol3/commit/57384f4c2fb793a437a5843f1430b22ee46385b5) Improve shortened banner
* [`eabe4a`](https://github.com/Oldes/Rebol3/commit/eabe4a66b2b9da743607316c068d1b6e47ea569e) Take a percentage of a time! value

### Fixes:

* [`bdaca6`](https://github.com/Oldes/Rebol3/commit/bdaca6afc6bc7b9a92f0fa6a1a2c18fb7da7bb54) Crash when loading specially crafted binary
* [`6121a8`](https://github.com/Oldes/Rebol3/commit/6121a84301ea138fa1dafd2853b9e6414d283079) SAVE with /header true
* [`c1594e`](https://github.com/Oldes/Rebol3/commit/c1594e2f115e3663383a89d2a0e184e8b38624e0) UNIQUE/DIFFERENCE/INTERSECT/UNION/EXCLUDE do not accept some data types
* [`85fb9b`](https://github.com/Oldes/Rebol3/commit/85fb9b6c62f4f7387989a9b7a382f21c296345e5) Shortening ANSI escape code used in error messages
* [`8b2669`](https://github.com/Oldes/Rebol3/commit/8b26692dc90a1759d07383fca90fe5dba4cef74f) Throw error when trying to make invalid errors
* [`0e9d14`](https://github.com/Oldes/Rebol3/commit/0e9d14539fe50fc241ac6adbab6c3945471a046a) Fix of redirection broken in recent HTTP scheme changes
* [`3b5f3b`](https://github.com/Oldes/Rebol3/commit/3b5f3b2916cf3914a14b81c5385f177e1262474e) Added os version into short banner and not using commit if not present any
* [`3acbfd`](https://github.com/Oldes/Rebol3/commit/3acbfdb4ec7caa339cbd680fa35016fbd65b35aa) Not using the new map syntax because old prebuild versions does not handle it yet
* [`1355fd`](https://github.com/Oldes/Rebol3/commit/1355fdbfaf2262204d63bb6cfaee80c0b0b6d65f) `log-*` functions should not return `positive number required` errors
* [`216443`](https://github.com/Oldes/Rebol3/commit/2164430b01c11bc248916a4925516cfa307bbc4c) `square-root -1` should return `1.#NaN` instead of an error
* [`6129ec`](https://github.com/Oldes/Rebol3/commit/6129eca50a658e4406fa050c88489763679e676c) `find/same` and `select/same` working with `any-word!`
* [`3c006a`](https://github.com/Oldes/Rebol3/commit/3c006a72493a50110dc80833ee4589d1707f47f3) Potentially SPA-vulnerability
* [`ceed8d`](https://github.com/Oldes/Rebol3/commit/ceed8dde6523b8e515c4cd3ddbe9c14b89ec9e99) Fix static annotation for uECC_valid_point
* [`81e9e0`](https://github.com/Oldes/Rebol3/commit/81e9e0a4e877a91295c3d7fcc337db69fcd82ff6) Quit after using `--help` startup option
* [`b440b3`](https://github.com/Oldes/Rebol3/commit/b440b351d94eecca0d7e6f1a1188bf71752d7e9e) `write/lines` regression
* [`fcc4fa`](https://github.com/Oldes/Rebol3/commit/fcc4fa11d7f1e71e9167be99e4e1fc7d8143dceb) `date!` is not listed as a `scalar!` value

## 2020 September

### Features:

* [`e82b70`](https://github.com/Oldes/Rebol3/commit/e82b7006cac4b5c172e361061402af0ff060b4ba) `? /refinement` should list function refinements

### Fixes:

* [`688ecc`](https://github.com/Oldes/Rebol3/commit/688ecc71e6a3f39fed7ac560dff57b8f13258be5) Handle use of `cd ..` in console properly
* [`289829`](https://github.com/Oldes/Rebol3/commit/289829ea6f355bba175fd9ff7c16201d4c35f223) `?` allows functions with nonsensical refinements, which can be misleading
* [`a2b61a`](https://github.com/Oldes/Rebol3/commit/a2b61a86e7fdef522500ff2cc21939ed39f32726) Limit search only to refinements!

## 2020 August

### Features:

* [`07fbf2`](https://github.com/Oldes/Rebol3/commit/07fbf25e4162ac375c78467d1e5cb0ecf55d3a9f) Simple `premultiply` function which premultiplies image's RGB channel with its ALPHA

### Fixes:

* [`0bb90c`](https://github.com/Oldes/Rebol3/commit/0bb90c5fb6496fc65355bd514a8100a7a9736845) `POKE` into `vector!` does not throw out of range error
* [`ede6fd`](https://github.com/Oldes/Rebol3/commit/ede6fdf19e73a13e74031cd4ae2e7babfc5e2cb2) `premultiply` should affect all pixels of the input image (even when input is not at head)

## 2020 July

### Features:

* [`260f59`](https://github.com/Oldes/Rebol3/commit/260f595f1c4b27c7ba714f34e8c6d378f8eaa5ec) Implement `SORT/COMPARE` on `any-string!` values
* [`aedd6b`](https://github.com/Oldes/Rebol3/commit/aedd6bbc3850e112a26c9122a988a81aa305b79e) Implemented _shutdown sequence_ to release allocated memory on exit

  That is useful with tools like Valgrind to find possible memory leaks)
* [`2854c4`](https://github.com/Oldes/Rebol3/commit/2854c46851970a7defa7ce552ff4c85100885083) Tiny code optimization
* [`c4162f`](https://github.com/Oldes/Rebol3/commit/c4162f9cacc922d62de4376caa2d76cb983f1172) Modified `ENHEX` again.

  * the `/url` refine was removed as I decided, that it should not be part of this encoding (ECMA-262 specification does not do it either)
  * added `/except unescaped [bitset!]` to be able specify, which chars must be encoded and which can be leave without encoding
  * added some predefined bitsets into new `system/catalog/bitsets` (will use them later in other places)
  * modified HTTP scheme to coop with these modifications
* [`a63bfc`](https://github.com/Oldes/Rebol3/commit/a63bfccb2c92bd32c44480ec8e6dfc8bf3cacc88) Making `ENHEX` compatible with `encodeURIComponent`, adding `/url` refine and possibility to change escape character
* [`88cd91`](https://github.com/Oldes/Rebol3/commit/88cd91b97d4a1c117cfa7826d0c27a2df333ffe1) Any `TRY` call resets `system/state/last-error` to `none`
* [`83cd93`](https://github.com/Oldes/Rebol3/commit/83cd939d87c53b48f812efa0c5b8a5ebc825511f) Added experimental Spotify module for access to Spotify's Web API using OAuth2 authorization mechanism
* [`755e8e`](https://github.com/Oldes/Rebol3/commit/755e8e92e577dae1d9e1b9d4b5451d33879894f2) HTTPD - Added possibility to stop server and return data from client (useful for OAuth2)

### Changes:

* [`3b9afc`](https://github.com/Oldes/Rebol3/commit/3b9afc00c011674af42c00c16a8adc4059446af4) Remove `found?` mezzanine
* [`d92049`](https://github.com/Oldes/Rebol3/commit/d9204910e0ba1488a2331413f10b7e9e92113548) Replacing some `context` usage with `construct` or `object`
* [`12454f`](https://github.com/Oldes/Rebol3/commit/12454ff4504c16c3cf41329dec427bdad3158632) Rename `protect/permanently` to `protect/lock` (wip)
* [`59576c`](https://github.com/Oldes/Rebol3/commit/59576cafe07993e0b7f35054c76f8c38b968d755) Renamed httpd module to have .reb extension and small improvements in its test file

### Fixes:

* [`8d2458`](https://github.com/Oldes/Rebol3/commit/8d24585df409e515d7da701b54dcdbb8760d6427) Force use of `setenv` function instead of `putenv` on Posix targets
* [`5a3cbe`](https://github.com/Oldes/Rebol3/commit/5a3cbe5075efd96fa530e9ad9dcd935f2e566d1e) `LOAD` Unicode encoded text with BOM
* [`28b47d`](https://github.com/Oldes/Rebol3/commit/28b47d3315393b5cf8934620d22139c70d02a45a) Add support for `iconv` `UTF-32` conversion on Windows
* [`7c030c`](https://github.com/Oldes/Rebol3/commit/7c030cf7a2da39f15f9a51270c6b8f2bdab2320e) Crash when composing large block
* [`6191f4`](https://github.com/Oldes/Rebol3/commit/6191f4fc5017d6284ed224edeb743b124f284aee) Allow getting empty environmental string on Posix platforms
* [`30feb0`](https://github.com/Oldes/Rebol3/commit/30feb050cf2065e4c6f309bc3a1174c8776246ad) Sorting Unicode string not working correctly on Posix
* [`0465a5`](https://github.com/Oldes/Rebol3/commit/0465a5e801968240063093d8d29c1f412e8401eb) Use case insensitivity when sorting a string by default
* [`e84c99`](https://github.com/Oldes/Rebol3/commit/e84c9962ec5ada9f5fa8f43d398317bb386330aa) Handle incomplete utf-8 sequences at tail
* [`8bd6d0`](https://github.com/Oldes/Rebol3/commit/8bd6d01bd70198bd508d8a44471e2391bfe4243d) Use case insensitivity on word comparison by default
* [`2269b5`](https://github.com/Oldes/Rebol3/commit/2269b56f1dde761e595b78929599236ee98314f6) Better error message when using bad image construction
* [`d0073c`](https://github.com/Oldes/Rebol3/commit/d0073c46ef203a90bf2b6ce8dda00b012eecc689) Throw an error on excess values in any-string construction syntax
* [`2babc5`](https://github.com/Oldes/Rebol3/commit/2babc5f81b06b35964825c933aca00c269a5d558) Throw error on invalid image! construction syntax
* [`cb7cf1`](https://github.com/Oldes/Rebol3/commit/cb7cf1e71965117c072e234919083eeced5e3841) Not freeing ROOT_BOOT if it is already NONE.. adding some memory related traces
* [`b687c2`](https://github.com/Oldes/Rebol3/commit/b687c20e186211a7661d40fe739b8964dfc51a14) Fine tune exit sequence to be nice with `-v` argument (early exit)
* [`7b39aa`](https://github.com/Oldes/Rebol3/commit/7b39aa67fd57698f6282697e3064e06bca26b4ac) Improve `delta-profile` to use computed adjustments
* [`31f922`](https://github.com/Oldes/Rebol3/commit/31f922003d7cf0d185903d8f1035087e4af8790e) Normalize `system/options/boot` so the path is always absolute
* [`359ca0`](https://github.com/Oldes/Rebol3/commit/359ca00dad9f59312c341a0cd26617d1f4e0e563) Resolving some Unicode related compiler's warnings
* [`b2fb60`](https://github.com/Oldes/Rebol3/commit/b2fb60095f81bbfe0bbea6f6c5dc8fa76471ff35) Valgrind's warning: Use of uninitialised value
* [`ed20ac`](https://github.com/Oldes/Rebol3/commit/ed20ac3c2438217b5b6d3fddce8804e09d18b23d) Modify `cd` to accept `cd /`
* [`196256`](https://github.com/Oldes/Rebol3/commit/19625652588048936a994c4a8420ec9c7c23a1d3) Minor code improvements in Spotify module
* [`7525f8`](https://github.com/Oldes/Rebol3/commit/7525f8257e2d66cee1667d1cb5413622ad793dd6) Make UNSET of unbound words cause error vs. fail silently
* [`c5b3de`](https://github.com/Oldes/Rebol3/commit/c5b3debeb32ae45236edfb685747771e4359e767) `decode-url http://host?query`
* [`7291eb`](https://github.com/Oldes/Rebol3/commit/7291eb357a6a1368b3e769359161eafbc4213c17) `print` of objects truncating strings
* [`16bebe`](https://github.com/Oldes/Rebol3/commit/16bebebdc761e7056500983f6026b867f37901a1) Spotify test file cleanup
* [`f68e85`](https://github.com/Oldes/Rebol3/commit/f68e8555a2443693e6cd28a4a0dc622db0a77e80) DO script with error should throw the error and not just pass it as a result
* [`2b1449`](https://github.com/Oldes/Rebol3/commit/2b1449bcc6c204805869acade7dfb93c5b1f4d03) `ICONV` from `UTF16` must ignore BOM
* [`64553a`](https://github.com/Oldes/Rebol3/commit/64553ac12fc76bf87502ca5494a65989c6edfbca) Small fix in HTTP scheme error reporting

## 2020 June

### Features:

* [`a7d02d`](https://github.com/Oldes/Rebol3/commit/a7d02dae51242b39af025c75d3d3c677d19ce6eb) Loading .txt files with unicode encodings
  Text files (.txt) are now loaded just as a string and not as a Rebol code. Unicode decoding is handled correctly if specified in BOM. Files (.txt) are always saved as UTF-8.
* [`afebd2`](https://github.com/Oldes/Rebol3/commit/afebd212f3d81e28a6b756b4e329ee9ddd814fb2) Added a new `ref!` datatype like: `@username`
* [`36fee1`](https://github.com/Oldes/Rebol3/commit/36fee1d2ac96c2cfaca833903118760f60ca5440) Added support for missing UTF-32 encoded binary to string conversion
* [`499a93`](https://github.com/Oldes/Rebol3/commit/499a93e4aadfa6e5df9a72c7532198e2d976ae8d) Added support for decimap pick on `pair!` and `any-string!`
* [`b6ea7f`](https://github.com/Oldes/Rebol3/commit/b6ea7fa7566f212431997e765a9efbd108d4434e) Add `enbase/part` and `debase/part`
* [`b78129`](https://github.com/Oldes/Rebol3/commit/b7812968a56e804da7ee594407972005a49319ad) `mold/part` to limit the length of the result
* [`217fce`](https://github.com/Oldes/Rebol3/commit/217fce7606dcc2c56ca6f03c6f8aa0eb9fc1fa75) Optionally execute `user.r3` file in user's home directory on startup and removing temporary help functions which now may be defined in user's file.
* [`962382`](https://github.com/Oldes/Rebol3/commit/96238234691cdfd08ab2d133e502db75e4e6f426) Added `ppk` (PuTTY Private Key) codec (so far only RSA keys)
* [`4c07b7`](https://github.com/Oldes/Rebol3/commit/4c07b7a0fee9fd0277cb2018c3878acd1850ae35) Added `ssh-key` (Secure Shell Key) codec (so far only RSA keys)
* [`90ecb3`](https://github.com/Oldes/Rebol3/commit/90ecb3dddfc980fd981f409a0c868a422d6c7a3c) Added missing `UI32BYTES` and `UI32LEBYTES` _bincode_ read commands
* [`df2c2a`](https://github.com/Oldes/Rebol3/commit/df2c2abe48ba335cd961862568a28cbfce36ca2a) Implemented proper `ask/hide` and `input/hide`

  That was not working well on Windows (with enabled virtual terminal processing), now it successfully hides user input.
  
  As a bonus this change contain a way how to turn off read-line console input and enter use the console as a key-stroke (and mouse as well) input. Simple use example is available here:
  https://gist.github.com/Oldes/face8a5790df7121a78ba606a3e150f4

### Changes:

* [`c8799b`](https://github.com/Oldes/Rebol3/commit/c8799b73fa7d4df9735b1d56e412848882ac3cf4) Using Bjoern Hoehrmann's UTF-8 decoder

  http://bjoern.hoehrmann.de/utf-8/decoder/dfa/
  It is shorter and faster.
* [`6416be`](https://github.com/Oldes/Rebol3/commit/6416be56b9bac994405da65cec54cd29e8fd2719) Migrate extensions: *.r => *.reb & *.rx => *.rebx

  This change was proposed years ago by @onetom in https://github.com/rebol/rebol/pull/140 and I agree, that the `.r` extension is lost in this century and so lets start with the new one. I'm personally still preferring `.r3` for run-able scripts which requires Rebol3.
  
* [`1e1dfa`](https://github.com/Oldes/Rebol3/commit/1e1dfa3282c0f8fd2f184bf81cbcdf358ac182b5) Rename `split/into` to `split/skip`
* [`214790`](https://github.com/Oldes/Rebol3/commit/2147901da9398b0d150ccefba0f02105a1ff1cb1) Setup `system/options/home` to default user's system home location or to one defined in `REBOL_HOME` environment variable

  For example to define custom location one can use on Windows:
  ```
  set REBOL_HOME=C:\Rebol\
  ```
  or on Linux:
  ```
  export REBOL_HOME=~/Rebol/
  ```
* [`2b2352`](https://github.com/Oldes/Rebol3/commit/2b23527852526ac9d510ce49e6d3ccd67c7053d9) Allow private RSA exponents optional (may be computed and are not in PPK files)
* [`3d1436`](https://github.com/Oldes/Rebol3/commit/3d1436ab57ae6f0e9d26a28cc5c0a9315d6f2d36) Using lowercase in codec names (it looks better)
* [`db5b44`](https://github.com/Oldes/Rebol3/commit/db5b44a6a9376a061c3d36b95de0edc2936a52cb) Replaced `mezz-crypt` module with `pkix` and `utc-time` codecs
* [`4d8866`](https://github.com/Oldes/Rebol3/commit/4d88668b47fb545dbeb7a598268f765315418d3b) Using `append` instead of `insert tail` when saving data to binary

### Fixes:

* [`6f8324`](https://github.com/Oldes/Rebol3/commit/6f8324fc38e53505fe0950ca673b296dedec7890) Avoid making a new bitset! on each `suffix?` call
* [`2d1a66`](https://github.com/Oldes/Rebol3/commit/2d1a66ba4ce37592dcf575f9cbfd9c55845f7f1c) `exists %/` must return `true` and not `none`
* [`4774ea`](https://github.com/Oldes/Rebol3/commit/4774eaf0f17c13719b4cdec6c6571e23251f2aa6) Better fix of reading list of logical drives on Windows
* [`ea39f8`](https://github.com/Oldes/Rebol3/commit/ea39f8ea976cf2225f32eea5d4389589f43adc2f) Update travis file and remove unrelated mezzanine file
* [`5e32c5`](https://github.com/Oldes/Rebol3/commit/5e32c54a850b7cb03328f5f54cb27064458a8918) `deline/lines` and `read/lines` misinterprets UTF-8 character as newline
* [`e7e72b`](https://github.com/Oldes/Rebol3/commit/e7e72b54489eab5910bf3315719525cea0d47f19) Missing file with `mold/part` native specification
* [`3f4b93`](https://github.com/Oldes/Rebol3/commit/3f4b93e55219a7560e7ffcc501d3e9010f4a888a) Wrong macro in host calls (failed standalone library compilation)
* [`1317e1`](https://github.com/Oldes/Rebol3/commit/1317e13a8ad80bade1e37a1c7224e2d3be2aa314) Path expression with path! does not work with all supported key types
* [`652045`](https://github.com/Oldes/Rebol3/commit/6520457a3bdcf4a121076c40ead3923fc94be313) Using `deline` to normalize CRLF to LF when reading text HTML content
* [`6238d2`](https://github.com/Oldes/Rebol3/commit/6238d23e53a0329a5b36c8673da926e248b35c69) Allow incomplete tag in DER codec (because it looks it can happen)
* [`9f6b34`](https://github.com/Oldes/Rebol3/commit/9f6b349a724a36bf8c5129ae9ff970db6a4248e6) Allow to `make bitset!` from `bitset!`
* [`e5442e`](https://github.com/Oldes/Rebol3/commit/e5442ebd1b8c540426ec95f3079b117c61075775) Fixed special case of `pick bitset! integer!`
* [`cb5867`](https://github.com/Oldes/Rebol3/commit/cb586701c0dd8e627e83195817d8a29a4baabb46) Using rejoin instead of reduce for compatibility with newer R3 versions (write %f [..])
* [`f09962`](https://github.com/Oldes/Rebol3/commit/f099629adfab50b31a01f6a200aacc12afad6c95) ENLINE does not convert line endings to native OS format (Posix)
* [`70c51b`](https://github.com/Oldes/Rebol3/commit/70c51ba9dff10c967b7bde3de781d1b12939d98a) Crash in `enline`


## 2020 May

### Features:

* [`55a8f9`](https://github.com/Oldes/Rebol3/commit/55a8f921935386e10028b74fd0bf502571e34fb7) Enable raw `map!` and `binary!` data in HTTP scheme's `write` action

  Using this:
  ```
  write http://localhost:8081/result/ #{0102}
  write http://localhost:8081/result/ #(a: 1)
  
  ```
  is now same like:
  ```
  write http://localhost:8081/result/ [POST #(Content-Type: "application/octet-stream") #{0102}]
  write http://localhost:8081/result/ [POST #(Content-Type: "application/json; charset=utf-8") #(a: 1)]
  
  ```
* [`01284d`](https://github.com/Oldes/Rebol3/commit/01284d6b966511902396f7837a27b6ec34425ff6) Ported Nenad's MySQL driver to be compatible with this Rebol3 version as a module/scheme

  It's just a quick rewrite, which replaces original `debug` function in the protocol with system logging and some of required modifications. There is also minnor change in error reporting. It would deserve better rewrite, but I don't have much need for this, so it's left for future or someone else.
  
  It was tested only with this so far:
  ```
  >> mysql: connect-sql mysql://root:rur@localhost/
   [MySQL] Connected: mysql://root@localhost:3306/
  >> send-sql mysql "show databases"
  == [["information_schema"] ["mysql"] ["performance_schema"] ["sys"] ["test"]]
  
  >> send-sql mysql "create database test"
  ** mysql error: [ 1007 ] "Can't create database 'test'; database exists"
  
  >> send-sql mysql "drop database test"
  == []
  
  >> send-sql/flat mysql "show databases"
  == ["information_schema" "mysql" "performance_schema" "sys"]
  
  >> close mysql
   [MySQL] Closed: mysql://root@localhost:3306/
  >>
  ```
* [`46810b`](https://github.com/Oldes/Rebol3/commit/46810b3fd487cc180b8ad7542a3d4b7afacc94a8) Allow protocols to be modules accessible from user context, if exports some functions
* [`f367ee`](https://github.com/Oldes/Rebol3/commit/f367ee5260f2af27989c470f77fd812089e76efc) It's again possible to `read dns://` to resolve a hostname

  ```
  >> read dns://
  == "Oldes-Aero"
  ```
  
  It also fixes a bug introduce in https://github.com/rebol/rebol/pull/66, that resolving domain name from ip was not possible at all. Now it's again working properly:
  ```
  >> read dns://rebol.com
  == 162.216.18.225
  >> read dns://162.216.18.225
  == "rebol.com"
  
  ```
* [`61596e`](https://github.com/Oldes/Rebol3/commit/61596e22c9975d1878f1194c3856493eb1c30246) `HTML-entities` decoder

  For decoding HTML entities into text:
  ```rebol
  >> decode 'html-entities {Test: &spades; & &#162; &lt;a&gt;&#32;and &Delta;&delta; &frac34;}
  == "Test: ♠ & ¢ <a> and Δδ ¾"
  
  ```
* [`640f91`](https://github.com/Oldes/Rebol3/commit/640f916d849be6442cdc4db296389d4e7986ac1c) Including R2's version of the `build-tag` function

  (but not enabled by default in boot-files)
* [`09d560`](https://github.com/Oldes/Rebol3/commit/09d56000dab875a4f19d534d118b6291fe050716) Added `pad` mezzanine function
* [`3f08fa`](https://github.com/Oldes/Rebol3/commit/3f08fa6894fbe6805dbcb72259a9e8eb91e69708) Updated HTTPD scheme with directory listing, logging and multipart POST processing
* [`ae88f0`](https://github.com/Oldes/Rebol3/commit/ae88f041fd2e48d2d6e94d0c4a97e278c803b69e) Enhanced `DEHEX` with `/escape` refinement for changing the escape character

  Can be used as:
  ```rebol
  >> dehex/escape "a#20b" #"#"
  == "a b"
  ```
* [`f4a1fc`](https://github.com/Oldes/Rebol3/commit/f4a1fc6683b703220eb45744804b97b9d1ce0d9a) New `resize` native function for resizing images (without any external dependency)

  The code was ported from GraphicsMagick (www.graphicsmagick.org).
  
  Basic usage example:
  ```rebol
  original-image: load %some-image.jpg
  resized-image-1: resize original-image 25%
  resized-image-2: resize original-image 100x200 ;- resized to exact size
  resized-image-3: resize original-image 100     ;- resized to width 100 (height is computed)
  resized-image-4: resize original-image 0x200   ;- resized to height 200 (width is computed)
  
  ```
  
  By default, `Lanczos` resize filter is used. It is possible to use one of filters, which are listed in `system/catalog/filters`:
  
  Here is example how to resize the original using width 250 with all filters. Results are saved as PNG files:
  
  ```rebol
  foreach filter system/catalog/filters [
      save join %image- [filter %.png] resize/filter original-image 250 filter
  ]
  ```

### Changes:

* [`8b0e6a`](https://github.com/Oldes/Rebol3/commit/8b0e6a7ce25ab0a2a6fc190753ec3882ccffa784) Using `system/platform` just as a `word!`

  Before:
  ```
  >> system/platform
  == [Windows win32-x64]
  ```
  Now:
  ```
  >> system/platform
  == Windows
  
  >> system/build/os
  == win32-x64
  
  ```
  
  Reason is, that in most cases only the `platform` name is needed so one had to use for example `switch system/platform/1 [...]`, which was a little bit cryptic. It is now also compatible with Red language.
* [`a55673`](https://github.com/Oldes/Rebol3/commit/a55673f2a159e6c561525de6901db0b262237c1a) Moved AS_DECIMAL define

  used to receive decimal value, when argument may be decimal or integer

### Fixes:

* [`7ee3e3`](https://github.com/Oldes/Rebol3/commit/7ee3e30c703abce71133c67cf6f0cff9f23650a5) Not using `system/platform` for `zero-index?` functionality
* [`9f2548`](https://github.com/Oldes/Rebol3/commit/9f2548d5fa5452e4a2ccc95df8f17fe088ecab79) Improved error handling in HTTPd (catches invalid requests)
* [`3d068a`](https://github.com/Oldes/Rebol3/commit/3d068ab1c1a274a6555e9e8bce3253af5aa1994c) `TRY/except` should store the `last-error`
* [`c4c374`](https://github.com/Oldes/Rebol3/commit/c4c3747a2d16a9fad8b810ccc011c7eca40f5ec9) Resolve incompatible pointer type compilation warning on posix targets in `browse` native function
* [`447525`](https://github.com/Oldes/Rebol3/commit/447525287f3230bb7dacc01d796d8b7961f2b8f2) Optimized (and fixed regression) when appending char on string
* [`b26700`](https://github.com/Oldes/Rebol3/commit/b267009605056f4c9ab4b87db84d674d84814ae9) Make sure that Linux ARM build is not trying to use not implemented MIDI device
* [`149aeb`](https://github.com/Oldes/Rebol3/commit/149aebe6fac5c38693a6e867f489e98a068f1ae0) Minor modifications of `MySQL` protocol error messages.
* [`3360da`](https://github.com/Oldes/Rebol3/commit/3360daccc3444b45560e1bf3876db0c7675d9979) WRITE should use MOLD instead of FORM when writing an object into a file
* [`0bb9d9`](https://github.com/Oldes/Rebol3/commit/0bb9d95d479cd11d3de437ff6a32eec457f1329c) `ICONV` from `UTF16` with BOM does not skip the BOM on Windows
* [`0020ec`](https://github.com/Oldes/Rebol3/commit/0020ec8c47225106c7be9d377f233fdbdb21c4c6) Make `case [true []]` return `unset!`
* [`eaeb6c`](https://github.com/Oldes/Rebol3/commit/eaeb6c6eaec51a8771503abb76d2b897691372b2) Using typeset should not found datatype value inside a block
* [`853cd8`](https://github.com/Oldes/Rebol3/commit/853cd8abf5e0e21c2a34579d7b47f122564870de) Optimization of CHANGE/INSERT/APPEND integer! or char!
* [`47d904`](https://github.com/Oldes/Rebol3/commit/47d9043dd9ac61218488a4a3d19eb926a356be7a) `DELTA-PROFILE` includes profiler overhead
* [`625044`](https://github.com/Oldes/Rebol3/commit/62504411f9f42a383b405ea1d84631c8fbdc6b1c) Insert/append/change binary! string!
* [`07d4e3`](https://github.com/Oldes/Rebol3/commit/07d4e37904eac5ac28b2a2955c7581510d5004c0) Small code refactoring of the `httpd` scheme
* [`b75a0e`](https://github.com/Oldes/Rebol3/commit/b75a0eb38ba25b2cb013f59d57debaec7e6c1e4a) Removed debug traces in HTTPD scheme
* [`95b454`](https://github.com/Oldes/Rebol3/commit/95b4541702397c3e5093064d72c92022e3afbee5) Include USE_IMAGE_NATIVES define in VS build
* [`b415fe`](https://github.com/Oldes/Rebol3/commit/b415fe5f80c540a2400f16d6c144831833c71d21) Catch possible evaluation errors in `in-dir` call
* [`26b19a`](https://github.com/Oldes/Rebol3/commit/26b19aaaeb32d76323b6cba3fbf83d255b5ab8bd) Catch `base code` even when comment is at file's head


## 2020 April

### Features:

* [`d4a138`](https://github.com/Oldes/Rebol3/commit/d4a1385e01b97c83df025864cfe0219b43373256) New PROTECTED? function

  Which take all of the same argument types as PROTECT, and return TRUE if that immediate argument is protected from modification.
* [`dd2ec1`](https://github.com/Oldes/Rebol3/commit/dd2ec13eae7cc888d0d7f11e771270cac3657c18) Added new `dir-tree` function which prints a directory tree

  This function is based on Toomas Vooglaid's `%dir-tree.red` script, which is available here: https://gist.github.com/toomasv/f2bcf320800ca340379457c1c4036338
  
  It was modified for use in Rebol3 and improved to support user's callback for more advanced formating of values.
* [`ba200f`](https://github.com/Oldes/Rebol3/commit/ba200f70feb234dd59c711ef40c08d952f8fbe40) Including GIT info in `system/build`

  Before this commit, `system/build` was returning just a date, when the executable was compiled.
  Now it is an object, with keys `date` and `git`. If the executable is compiled from a valid git repository, `git` contains another object with info about actual `repository`, `branch`, `commit` and it's `message`.
  
  With this info it will be easier to localize possible regressions against older executables.
* [`0b3531`](https://github.com/Oldes/Rebol3/commit/0b3531fdadbfd647a7de7321aec4a5b53eaefc8a) Added simple `mobileprovision` codec,

  which can be used to load Apple's mobile provisioning files as a string for examination (without need to use Apple's tools).
  ```rebol
  >> print load %some.mobileprovision
  <?xml version="1.0" encoding="UTF-8"?>
  <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
  <plist version="1.0">
  <dict>
          <key>AppIDName</key>
          <string>My App</string>
          <key>ApplicationIdentifierPrefix</key>
          <array>
  …
  ```
* [`2e41df`](https://github.com/Oldes/Rebol3/commit/2e41df086040bd0bfec92bf75c80ec82330d6499) Added a few more names into OID decoder and using more optimal chars instead of single byte binaries
* [`ad49c4`](https://github.com/Oldes/Rebol3/commit/ad49c4404bc83b6125d1632a986e1df43299b817) Registered `certSigningRequest` file extension with PKIX decoder
* [`1dca01`](https://github.com/Oldes/Rebol3/commit/1dca01bb65a04ad0f66ecacdb7346592d737b7fe) Added BBCode decoder for converting BBCode markup language into HTML

  For more info about BBCode you can visit: https://en.wikipedia.org/wiki/BBCode
* [`628e6b`](https://github.com/Oldes/Rebol3/commit/628e6b725b79e22a0c08efb2719f90a3e1e01b27) Added support for `find` on `map!`

  This feature was introduced in Red language so it is good to be compatible.
  Example:
  ```rebol
  >> find #("a" 1 "b" "c") "A"
  == "a"
  
  >> find #("a" 1 "b" "c") 1
  == none
  
  >> find/case #("a" 1 "b" "c") "A"
  == none
  ```
* [`a74222`](https://github.com/Oldes/Rebol3/commit/a74222aa8ce4e243b2725ecc125cb160f3f85895) Added support for `zero?` on bitsets.

  `zero?` return TRUE if no bits are set in a bitset!
  ```rebol
  >> zero? make bitset! #{0000}
  == true
  
  >> zero? make bitset! #{1000}
  == false
  ```
* [`fa8fc6`](https://github.com/Oldes/Rebol3/commit/fa8fc685f2ffeb4cac9535c6828f657a7400f24f) New native `complement?`

  Returns TRUE if the bitset is complemented. Like:
  ```rebol
  >> complement? make bitset! [not]
  == true
  
  >> complement? make bitset! []
  == false
  
  >> complement? complement charset "abc"
  == true
  ```
* [`5d5bf7`](https://github.com/Oldes/Rebol3/commit/5d5bf7ffec2c340fa31f2f88b4473ed3ab1df0cf) Implemented `query` on date values

  So one can query multiple date parts with one call, for example:
  ```rebol
  >> query/mode now [year day month time timezone]
  == [2020 8 4 12:24:54 2:00]
  ```
  If set words are used, the result is like:
  ```rebol
  >> query/mode now [year: month:]
  == [
      year: 2020
      month: 4
  ]
  ```
  Simple `query now` would return object with all possible fields.
* [`ae61eb`](https://github.com/Oldes/Rebol3/commit/ae61ebb1dc8b8b8ba614cd334c20d0dd699c908c) Implemented basic user-defined OP!

  I'm not much fan of _ops_, but now it's possible to do code like:
  ```rebol
  >> .: make op! [[a b][ join a b ]]
  >> "a"."b".(random 10)
  == "ab1"
  ```
  
  Current limitation is, that there is no runtime check of first argument's type!
* [`1e55b6`](https://github.com/Oldes/Rebol3/commit/1e55b65039d1dcee431f3465ce9ad0e6e5106a18) UNPROTECT/words object!
* [`309280`](https://github.com/Oldes/Rebol3/commit/309280675a52cf5f27613b1bcee9136faec485c5) PROTECT/words supported for an object! value

  PROTECT object! now protects the object and its words, but not their values.
  
  PRODUCT/deep object! protects the object, its words, and their values.
  
  PROTECT/words object! only protects the words of the object!, but not the object itself or the values of the words.
  
  PROTECT/words/deep object! also protects any values assigned to the words, but still not protects the object itself.

### Fixes:

* [`e128de`](https://github.com/Oldes/Rebol3/commit/e128def4f61e51b8eeaab21b415ac0a0879d74b6) Crash when used protect/words/deep with block of values which are not series
* [`f5df78`](https://github.com/Oldes/Rebol3/commit/f5df7888f7410ff2a438a4baf7a06e81f9814954) TAKE/DEEP does not copies series values within the block
* [`567f53`](https://github.com/Oldes/Rebol3/commit/567f53ffdf3bb685b3f5348296705e5d2431c411) TAKE/part on any-block! must return same type like the source is
* [`cabe48`](https://github.com/Oldes/Rebol3/commit/cabe489b03a2c28649584bb421ca77a3d730dc73) Small changes in indentation from `dir-tree` and `list-dir`
* [`cc96b3`](https://github.com/Oldes/Rebol3/commit/cc96b3b83a51d667e532641956a4f925147d9201) Rewritten `list-dir` to use recent `dir-tree` function.
* [`e7e8c6`](https://github.com/Oldes/Rebol3/commit/e7e8c6cd237124e55a7d539111f9df871542beda) Updating _system banner_
* [`2c6ac1`](https://github.com/Oldes/Rebol3/commit/2c6ac1e198c27ae9ff680f77edacf23647f33961) Get-git function used to collect information about current commit during compilation
* [`6e8233`](https://github.com/Oldes/Rebol3/commit/6e8233fde857f73fdec9dc8d9f6af460991068ee) Using old syntax for making a map (for compatibility with older prebuild executables)
* [`04cf27`](https://github.com/Oldes/Rebol3/commit/04cf27331138990c257fac063005b1fb08935941) Revisited XML codec to better handle more "XML prolog" versions (including 2 tests)
* [`f66954`](https://github.com/Oldes/Rebol3/commit/f6695479b29252d9fdc709c1eef574bc5e9dcd12) Fixed typos in the percent unit tests
* [`c9dc63`](https://github.com/Oldes/Rebol3/commit/c9dc63aa6ced899a7b7b1ea5b8ef4c85d0ce2d50) Allowing H6 tag in BBCode decoder
* [`665141`](https://github.com/Oldes/Rebol3/commit/6651418854bc23edc372931203b3556e3be757f0) Updated `put`'s doc-string
* [`0ffc8f`](https://github.com/Oldes/Rebol3/commit/0ffc8f616215052d372d51c67d40022800da5e31) Using VAL_TYPE instead of direct type code
* [`7c2a11`](https://github.com/Oldes/Rebol3/commit/7c2a11ed7a289a3515650da98e65696fbf55fd68) Disallow subtracting date from integer
* [`bb1668`](https://github.com/Oldes/Rebol3/commit/bb16682b439aa681d6fffb249d602e94cb7d4a45) Negative /SKIP with FIND causing a hang
* [`77a86d`](https://github.com/Oldes/Rebol3/commit/77a86dc8fcfe23acb8fe139c38b563fcd2c6f026) Bitset from empty string should have zero length
* [`3dfc00`](https://github.com/Oldes/Rebol3/commit/3dfc00f03ff4dcda27efff2256d4e5180af7b7c8) `zero?` on complemented bitset
* [`fbdab8`](https://github.com/Oldes/Rebol3/commit/fbdab85421c8d03d84f486d451165a730f4eee22) Fine-tune system object protection
* [`d28f95`](https://github.com/Oldes/Rebol3/commit/d28f95226c819f71c938c8e3e733b2992e812779) Fine-tune system object protection
* [`764cbe`](https://github.com/Oldes/Rebol3/commit/764cbe6cbd1edfb08702a4420e136d1a4f1d2817) Protect `lib/system` value.
* [`da722e`](https://github.com/Oldes/Rebol3/commit/da722e323cc28b42508b4cd35d61b29dda2a3e84) Unset also `native` after boot.
* [`88de50`](https://github.com/Oldes/Rebol3/commit/88de5023af6486c1ba1a03754941ac313d9789ea) Unset `action` native after boot.
* [`110e9e`](https://github.com/Oldes/Rebol3/commit/110e9eacae05f3a5e83fa03ad9d6da920baeb14d) None instead of error for an out-of-range ordinal date! selector
* [`0f4436`](https://github.com/Oldes/Rebol3/commit/0f4436c94fbc1940baa170c2ed55ab8a195910c7) Re-factored date's accessors
* [`3ace82`](https://github.com/Oldes/Rebol3/commit/3ace82acdfc4e7cd11c788d6c493906600746da6) Accessing and settings time parts of date without time
* [`cd4675`](https://github.com/Oldes/Rebol3/commit/cd46752c100c013c7748e43dad71fc7f64cd2f04) Prevent too many refinements in `now` function
* [`1087f6`](https://github.com/Oldes/Rebol3/commit/1087f6636540f2270945802bf1d641d3ff430909) Throw error also when to-hex/size is using size = 0
* [`344646`](https://github.com/Oldes/Rebol3/commit/34464615c59581a6cd27c3725ae740eeddf62c2e) Do not allow make a `task!` if not compiled with INCLUDE_TASK define
* [`44b13f`](https://github.com/Oldes/Rebol3/commit/44b13f8c9960cc76162fdf5e9e8fc1b95f20b9d2) Allow any value as a part of path!
* [`0ad97b`](https://github.com/Oldes/Rebol3/commit/0ad97baa51f786e8ec98194f3fa7ae6de0b01446) Don't allow logic as a path part when using `to-path`
* [`2f2322`](https://github.com/Oldes/Rebol3/commit/2f23220966b4caf301ca3e6bd33fa6951dc7bfba) Not using line breaks when molding/forming empty map!
* [`6e48d7`](https://github.com/Oldes/Rebol3/commit/6e48d7fe70b17629ae8e89397f6bbe5ddb93cf53) Sys-state.h redefinition
* [`700045`](https://github.com/Oldes/Rebol3/commit/700045e94ed708f06ad57a57fe8666382145e8d5) Minor Expand_Stack modification
* [`0d9dd3`](https://github.com/Oldes/Rebol3/commit/0d9dd37ea08b6dfec40aaef3d8e32621e61a9d70) Check pointer against NULL
* [`3c5e97`](https://github.com/Oldes/Rebol3/commit/3c5e97ef13336d7084ddf94ca18685c4f82ade18) Make APPLY :DO [:func …] work like DO :func …
* [`4b1148`](https://github.com/Oldes/Rebol3/commit/4b11483ad89f17294042dd40c8f7f0280a43d291) Fix: Restore Saved_State/Halt_State when a longjmp happens
* [`d2b8cd`](https://github.com/Oldes/Rebol3/commit/d2b8cd5ba50d72bc7ac5f2794d2f6b17745d9769) Ensure proper path component types during refinement processing
* [`ef6931`](https://github.com/Oldes/Rebol3/commit/ef6931fd355dd5ea0fa7937176d0f91e4257b81e) Turn off all security by default
* [`03a08b`](https://github.com/Oldes/Rebol3/commit/03a08b89b8b49245b32a54e42c16fc2361f2afb8) Fix stack expansion

### Changes:

* [`f7193a`](https://github.com/Oldes/Rebol3/commit/f7193abd89552b13f3030a03811bb70709771999) Return FALSE instead of NONE when picking a non existing bit from a bitset

  Just for compatibility with Red language. It is still possible to revert it using `PICK_BITSET_AS_NONE` define.
* [`c88857`](https://github.com/Oldes/Rebol3/commit/c888570a7fafd758282ec576644a67affffe2f41) Use partial system object protection

  At this stage it is not fully protecting system object, but it is a good start to see possible issues with it.. like https://github.com/Oldes/Rebol-issues/issues/1348
* [`127a5a`](https://github.com/Oldes/Rebol3/commit/127a5a89964c6e6518d39d15ac5867eccceff8fb) Move sys-state.h to be included by sys-core.h


## 2020 March

### Features:

* [`f913b2`](https://github.com/Oldes/Rebol3/commit/f913b2739593ead409adaa9ef366f644f84d20e3) Enable `secure` again (although not fully implemented yet)
* [`414799`](https://github.com/Oldes/Rebol3/commit/4147997d8e4983a9f9d31bcde66f7c8f76efce88) FORM on a binary! creating a string without the "decoration"

  example: `"DEADBEEF" = form #{DEADBEEF}`
* [`4e4759`](https://github.com/Oldes/Rebol3/commit/4e47595ddcb69d9a9e38aaad02f7cd9af4c93aec) `deduplicate` mezzanine (modifying `unique`)

  related to: https://github.com/Oldes/Rebol-issues/issues/1550
* [`61b39c`](https://github.com/Oldes/Rebol3/commit/61b39cafc04ffbefba496404f45d27f5597eaaec) Removing `/base` refinement from `debase` and `enbase` functions

  So eliminating ugly `??base/base` code, for example instead of:
  ```rebol
  debase/base data 16
  ```
  it is now just:
  ```rebol
  debase data 16
  ```
  
  I'm still not sure if the `base` value should not be as a first argument, but so far I will just keep it as a second one.
* [`305ed8`](https://github.com/Oldes/Rebol3/commit/305ed83f92a59d58c495a33e6ec3832a47214318) Register %.jks file extension with DER codec
* [`628234`](https://github.com/Oldes/Rebol3/commit/628234c64adffb91618dc78f4003d644527a3c1c) Support for Ascii85 (Base85) encoding

  `Ascii85` is a form of binary-to-text encoding used for example in PDF files.
  One can now use 85 as a `base` value in `debase` and `enbase` functions:
  
  ```rebol
  >> enbase/base "Lion" 85
  == "9PJE_"
  
  >> debase/base "9PJE_" 85
  == #{4C696F6E} ;== "Lion" if converted back to string
  ```
  
  This addition is optional, requires compilation with `INCLUDE_BASE85` define.
* [`9d5a6f`](https://github.com/Oldes/Rebol3/commit/9d5a6f9f9e3d96c1d9c9aa3734dbfbabf1541cd2) Added support for case-sensitivity of `map!` keys

  With this commit maps should be compatible with Red language, where creation is case sensitive, selection is by default case insensitive unless `/case` refinement is used.
* [`dbc572`](https://github.com/Oldes/Rebol3/commit/dbc572345865611ccf1196ccfc343c2b84fda994) WAV codec updated (vector support)

  Now loaded WAV sound data are as a vector instead of raw binary.
  WAV encoder was improved for vector data input.
  
  Example of simple sine WAV file generator:
  ```rebol
  samples: #[si16! 10000]
  repeat i 10000 [ samples/:i: 16000 * (sine i * 7.2) ]
  save %sine.wav samples ; by default mono with 44100Hz
  ```

### Fixes:

* [`4911e0`](https://github.com/Oldes/Rebol3/commit/4911e09e9f7c7cf3642e3b6cc148390b51a7988a) Minor code change
* [`66e47d`](https://github.com/Oldes/Rebol3/commit/66e47d5b9eae14a69e312706e75f472828383312) Do not set func word to be handle!
* [`f4b30e`](https://github.com/Oldes/Rebol3/commit/f4b30ea95d2a20df4c6f58974bfd52c8f60ceaf0) Fix: TRACE ON shows bizarre parse behavior
* [`3f9ac4`](https://github.com/Oldes/Rebol3/commit/3f9ac42c7e6766f0816db5ccd80d71d87b9f382d) Better SECURE error message with incorrectly formatted input block
* [`40083e`](https://github.com/Oldes/Rebol3/commit/40083e5bbc6f464b9ac16ed6d366b842ad2960f1) PARSE position capture combined with SET or COPY
* [`5596b6`](https://github.com/Oldes/Rebol3/commit/5596b6ec6046982961b5e889ecd2d70f2944e60d) ENTAB strips first character from line
* [`53c991`](https://github.com/Oldes/Rebol3/commit/53c991ff751afdc97451ab15115bd8e6653e4efa) Updating Eval_Count while processing parse's empty while block
* [`fcd4dc`](https://github.com/Oldes/Rebol3/commit/fcd4dcc2d4ecd61843efd8f9538cef725101b7f9) Cannot interrupt empty parse's while rule with CTRL-C
* [`2a4f79`](https://github.com/Oldes/Rebol3/commit/2a4f79892f0881408c36fb9f2b4f6b4923892b5f) Read/lines on clipboard does not return a block of strings
* [`db4537`](https://github.com/Oldes/Rebol3/commit/db4537c76c0e4c5e7c3b9c1c00921ed219543ff5) Length? of any-word!
* [`e29b73`](https://github.com/Oldes/Rebol3/commit/e29b737ebed0b42e32e051126c78118d9b8c1f1c) Replace binary! integer! integer!
* [`e60a7b`](https://github.com/Oldes/Rebol3/commit/e60a7b608a237db5935abb2e3e4dc466fecc7dd4) `change-dir %/` not working on Windows
* [`3b529e`](https://github.com/Oldes/Rebol3/commit/3b529ee74614943f335f20f2ed42214af3692a37) Minor cleanup in HTTPd script (not probing target on any request)
* [`1d0e67`](https://github.com/Oldes/Rebol3/commit/1d0e67c3255db117fd8f1581771e88ce05620346) Prevent crash when task with error is evaluated
* [`8a63f4`](https://github.com/Oldes/Rebol3/commit/8a63f423706529474234352c3a4f2f961f1a09f6) Minor cleanup in TLS protocol
* [`2dbcf3`](https://github.com/Oldes/Rebol3/commit/2dbcf33d2112d41c7e96acc4d93a78543f6678ef) Moved automatic decompression when reading HTML so it is not used when `/binary` refinement is used (to get raw, untouched data)
* [`599e94`](https://github.com/Oldes/Rebol3/commit/599e94f9705374593acf323e72e358d195729374) Enhance max decompression limit for Zlib
* [`b9723d`](https://github.com/Oldes/Rebol3/commit/b9723deaa283faac9cadb87b841d11b203e0606f) Better handling large number tuple multiplication
* [`10aa48`](https://github.com/Oldes/Rebol3/commit/10aa48c0c1ee7efe5e6920d88d5b4ef8599e93bd) Using 64bit integer in tuple math
* [`a467d3`](https://github.com/Oldes/Rebol3/commit/a467d31c2103074d401aff831d64263d763355d3) Include TASK! evaluation only with INCLUDE_TASK define
* [`663397`](https://github.com/Oldes/Rebol3/commit/663397b5211323884e6d025074ba244f85182ec5) Change on image must return position just past the change
* [`082a31`](https://github.com/Oldes/Rebol3/commit/082a316ba99f627692c6d07678184c8a45b9bb46) Returned position after INSERT action on IMAGE must be at tail not before.
* [`e81f18`](https://github.com/Oldes/Rebol3/commit/e81f18cc868b6fcc6d1e7d64e110e368504feb5f) FIND on images
* [`0d9e7d`](https://github.com/Oldes/Rebol3/commit/0d9e7d3a738684409e1cc9b484234b796586557a) Adjusting tuple parts in images
* [`427733`](https://github.com/Oldes/Rebol3/commit/427733c21f1188ff601eb9a6f834507033c8a931) Maximum uncompressed size for GZIP decompression was not large enough
* [`338a0f`](https://github.com/Oldes/Rebol3/commit/338a0f2590dcdfe7239f5e9c435d50642f5d56a0) Added 'type reflector to datatype!

## 2020 February

### Features:

* [`7c97bd`](https://github.com/Oldes/Rebol3/commit/7c97bd6cc86e55a2cc01aa195f09bed4023eb2d3) Add single quote strings to tag! syntax
* [`798b06`](https://github.com/Oldes/Rebol3/commit/798b06ddbf49e8bde1aa2c4edb2f71e66ca11d5d) Included `to-degrees` and `to-radians` native functions
* [`07379a`](https://github.com/Oldes/Rebol3/commit/07379a67862b1682da00e471a56e4c05db7f9258) Enable `read/binary` and `write/binary` in HTTP protocol

  By default, `read` automatically tries to convert text data according give specified charset in the received header info. This may be avoided using `/binary` refinement.
* [`b755c7`](https://github.com/Oldes/Rebol3/commit/b755c7a1dc20c0449536d8b629b1249614d6db28) Allowing `@name` to be a valid email! type

  For a compatibility with Red language and also because it may be useful in dialects.
  Although `@name` is not a valid real email according RFC, one can construct invalid email values in many ways and so in real use for email apps, there should be a real additional email validation anyway.
* [`f3dfa7`](https://github.com/Oldes/Rebol3/commit/f3dfa7b40b887c07d886df88d35bbd979a9c67c4) Possible auto conversion to text if found charset specification in content-type
* [`f45e86`](https://github.com/Oldes/Rebol3/commit/f45e86c24856920fc9e9b6d352411e530fc766bb) Ported JSON codec from Red language

  Main credits belongs to: Gabriele Santilli and Gregg Irwin.
  See https://github.com/giesse/red-json for more details.
* [`796f30`](https://github.com/Oldes/Rebol3/commit/796f30ea6cc093ab59bbc9ceaec1377ec691e721) Ported JSON codec from Red language

  Main credits belongs to: Gabriele Santilli and Gregg Irwin.
  See https://github.com/giesse/red-json for more details.
* [`d27e4b`](https://github.com/Oldes/Rebol3/commit/d27e4b13348ad1d634e003901d3f2ab51702442e) New AS native function for series coercion into a compatible datatype without copying it

  Example:
  ```rebol
  >> p: as path! b: [a b]
  == a/b
  
  >> append b 'c
  == [a b c]
  
  >> p
  == a/b/c
  ```
* [`14e4b9`](https://github.com/Oldes/Rebol3/commit/14e4b9e7f60435c6d0364f06ea29c7e57b2a4f7c) Implemented MAP's own syntax (compatible with Red language)

  Example of a nested maps:
  ```rebol
  >> m: #(a: 1 b: #(c: 3 d: 4))
  == #(
      a: 1
      b: #(
          c: 3
          d: 4
      )
  )
  
  >> m/b/d
  == 4
  ```
* [`a84522`](https://github.com/Oldes/Rebol3/commit/a84522c267e728eae6dabd189081cfcdd9901a17) New PUT action for replacing the value following a key (in blocks and maps)
* [`9ab1b1`](https://github.com/Oldes/Rebol3/commit/9ab1b134c1d9128f2b206b4dd698c5fe67bce7d0) Improving HELP output on datatypes.

  It also displays different results if used directly on datatype, like:
  ```rebol
  help char!
  ```
  
  or when used on value, which holds datatype, like in:
  ```rebol
  my-value: char!
  help my-value
  ```
  
  First one displays:
  ```
  CHAR! is a datatype.
  It is defined as a 8bit and 16bit character.
  It is of the general type scalar.
  
  Found these related words:
    null            char!      #"^@"
    space           char!      #" "
    sp              char!      #" "
    backspace       char!      #"^H"
    bs              char!      #"^H"
    tab             char!      #"^-"
    newline         char!      #"^/"
    newpage         char!      #"^L"
    slash           char!      #"/"
    backslash       char!      #"\"
    escape          char!      #"^["
    cr              char!      #"^M"
    lf              char!      #"^/"
  ```
  
  while the second one just:
  ```
  MY-VALUE is a datatype of value: char!
  ```
* [`1162e7`](https://github.com/Oldes/Rebol3/commit/1162e7d05541665433a1d02810164402900fc41a) Define a WHITESPACE charset

  mentioned: http://www.rebol.net/r3decide/0008.html
  
  Using just the most basic _whitespace_ chars. Anyone may extended this charset if needed.
  This one is compatible with Red language.

### Fixes:

* [`89e315`](https://github.com/Oldes/Rebol3/commit/89e3158f38d403dee220cb85b5ffb91048efa441) Use constant macros instead of suffix
* [`c39291`](https://github.com/Oldes/Rebol3/commit/c3929196932f1c3727cd2b406fac7d41235597f6) Always use 32-bit integers for CRC
* [`761fba`](https://github.com/Oldes/Rebol3/commit/761fba86294341a0f562db3cb608ab6ccf2be0c8) QUERY directory size now returns NONE instead of 0
* [`5a5ae7`](https://github.com/Oldes/Rebol3/commit/5a5ae749d2c82494e73b1d29969f114e9efc0f03) Problem with externally modified keys of map!
* [`bb4598`](https://github.com/Oldes/Rebol3/commit/bb459824939f22c03b65eb64ffb6ed5856ba83d7) PARSE modifiers do not honor PROTECT status of series
* [`242ceb`](https://github.com/Oldes/Rebol3/commit/242cebaf41b19bfa6934d6980e19bde6226b7fbf) HTTP protocol - limit input data according Content-Length
* [`55d3ae`](https://github.com/Oldes/Rebol3/commit/55d3aee15c85fa6340bb5526d3418e34ca6cfca2) Email does not handle non ascii chars correctly
* [`bcf114`](https://github.com/Oldes/Rebol3/commit/bcf11438ea65f385d03cf5320b34fd3120f54b50) WHOIS lookup throws an error when used with IP
* [`cd42c2`](https://github.com/Oldes/Rebol3/commit/cd42c252376bb7b34eb77f23f3d58e8e87c240b0) Exists? on not existing URL must return FALSE instead of error
* [`fda0e6`](https://github.com/Oldes/Rebol3/commit/fda0e649c90c170c9302ae430f75152d504a5995) Make-boot.r should wrap files marked as a `module` type into `import module` code
* [`9f6534`](https://github.com/Oldes/Rebol3/commit/9f6534108f5c532d62c47abe9598179398faf111) FOREACH works bad on image!
* [`e1b451`](https://github.com/Oldes/Rebol3/commit/e1b451274409723638a8ad82d0127b2982a0b8e8) [TLS] Error reading elyptic curve
* [`3b413f`](https://github.com/Oldes/Rebol3/commit/3b413fad1c8654fa1cae0f4ee0925f4a80d57b30) Remove the DO keyword from PARSE
* [`b2e59d`](https://github.com/Oldes/Rebol3/commit/b2e59dd28632b878866fcde966a29330f7c3ccec) JSON codec - license & rights updated
* [`e892a0`](https://github.com/Oldes/Rebol3/commit/e892a081bf300133e2bea0d612a33a7d000961eb) Allow expressions in parens as INSERT and CHANGE value argument
* [`47f7b4`](https://github.com/Oldes/Rebol3/commit/47f7b4646f3aeaa6850d31adb3bf603c1e7a75d8) READ %/ doesn't return the list of drives on Windows
* [`5ec25a`](https://github.com/Oldes/Rebol3/commit/5ec25a49523ba05761d568862f0081c02171af6d) MOLD/FLAT on MAP values does not remove line breaks
* [`5a4f0f`](https://github.com/Oldes/Rebol3/commit/5a4f0f076396c6829fe1f4f864138f6487f64dc4) Code refactoring
* [`15f6ea`](https://github.com/Oldes/Rebol3/commit/15f6ea5f40db98b54d0d75ace4f574f5eaa279c9) APPEND a word on object should set the value to NONE instead of UNSET
* [`70a331`](https://github.com/Oldes/Rebol3/commit/70a33140586b1f6826736b5ba02f794737ea0580) Overprotective PROTECT for path access to maps
* [`e81fb8`](https://github.com/Oldes/Rebol3/commit/e81fb8c61c364c155dc7ac6ad5c6ac5962fd7422) Code refactoring
* [`62af92`](https://github.com/Oldes/Rebol3/commit/62af92f5d4597e1a801a6aab4864448a99fc9c14) SAME? on VECTORs return TRUE even when vectors are not identical
* [`35815c`](https://github.com/Oldes/Rebol3/commit/35815ceaef7d9454d49ac776ae2d1b31a32255db) COPY/part on VECTOR is silently ignored
* [`735454`](https://github.com/Oldes/Rebol3/commit/735454ca325fb66ff0e5e22eac04a60b7973225a) Fix macros
* [`1d215f`](https://github.com/Oldes/Rebol3/commit/1d215f544bf9a14510b4e5bdc8e70fc5e0a457e3) CRASH when removing 32768 or more bytes from the head of series

### Changes:

* [`651219`](https://github.com/Oldes/Rebol3/commit/6512190e8cf23af76fe54eb442d71708aebef127) Renaming CRC_Table to CRC24_Table, so it has same naming as CRC32_Table.

## 2020 January

### Features:

* [`545dd9`](https://github.com/Oldes/Rebol3/commit/545dd9062a3e7ceb53eda6fb0d772689e097d1b5) VECTOR compact construction

  Example:
  ```rebol
  >> #[ui8! [1 2 3]]
  == make vector! [unsigned integer! 8 3 [1 2 3]]
  
  ```
* [`65a068`](https://github.com/Oldes/Rebol3/commit/65a0685370a413568db6e3be4cadd5a35f901b7c) MOLD on image using `flat` result if image is small enough (less than 10 pixels)
* [`09e8c4`](https://github.com/Oldes/Rebol3/commit/09e8c42c0a03511e61f522d8042b9b17700a1370) Allow `SIGNED` word as a key word in vector construction dialect as a oposite to already supported `UNSIGNED`
* [`68b197`](https://github.com/Oldes/Rebol3/commit/68b197a6765a8ff0d4b3b63c5a763cd73209b5cf) Very basic WAV encoder

### Fixes:

* [`6ff512`](https://github.com/Oldes/Rebol3/commit/6ff512b0c040e82dabc3f1029cdca17b8a69157d) Typo in a comment
* [`c8510f`](https://github.com/Oldes/Rebol3/commit/c8510f1c6dcd9a69e28e00b3dce0c938a799d668) MOLD/FLAT on image does not removes line breaks in the alpha channel
* [`181f5f`](https://github.com/Oldes/Rebol3/commit/181f5f890409f29cbd9a8e69bf0f1c01b0a733e3) MOLD/FORM on zero percent value
* [`2b1f17`](https://github.com/Oldes/Rebol3/commit/2b1f17491cdd979e895cac3a9707f8e061dafdbb) SWF codec - missing bit aligning in fill style array
* [`d076b3`](https://github.com/Oldes/Rebol3/commit/d076b3dc8d1ce0fe8cc2b3314d39bd65a9c4d967) Math error when multiplying negative integer with zero integer

## 2019 December

### Features:

* [`661779`](https://github.com/Oldes/Rebol3/commit/661779282619ae98a4d2bf047447831653cc1396) Releasing intermediate sources of modules and protocols after loading these on boot (to reduce memory use a little bit)
* [`f101d9`](https://github.com/Oldes/Rebol3/commit/f101d9fd84f1b8f0054d9a23b15198bc4af4768c) `empty?` to accept typesets

  It was already accepting bitsets, so why not to support typesets too. Added a few `typeset!` unit tests (not complete)

## 2019 November

### Features:

* [`6632c6`](https://github.com/Oldes/Rebol3/commit/6632c61a1632f44a71a953c54d431659fcf6ffa2) A little bit optimized `JOIN` mezzanine function (not using `repend` mezzanine)
* [`4ecd93`](https://github.com/Oldes/Rebol3/commit/4ecd934d51671d069e12f82784c4fb6d29ff05f7) New `tint` native for color mixtures

## 2019 October

### Features:

* [`e74831`](https://github.com/Oldes/Rebol3/commit/e74831216210d6e3c087c99810418b56ff5b4090) Added function for basic extra field data decoding into ZIP codec
* [`800115`](https://github.com/Oldes/Rebol3/commit/800115a2f268c9ca4537d0ff9035aec6cc8bb83d) Updated ZIP decoder

  * Refactored decoder so it is using central directory structure
  * Added decode/info refinement used to resolve just the info about files without decompressing
  * Added access to decompress-file function which may be used to decompress single file using info from the info records
  
  Initial ZIP decoder was just traversing the file searching for the ZIP related structures. But some ZIP files (like created on macOS) does not provide valid informations in local headers and central header must be used, as it is with this update.

### Fixes:

* [`2c152b`](https://github.com/Oldes/Rebol3/commit/2c152b83748f50d3fb651032ecfc3cc300569b79) System crash when reading any http request

## 2019 September

### Features:

* [`f38d49`](https://github.com/Oldes/Rebol3/commit/f38d499a6303c55111ae7b6a58bd371a93905740) Possibility to count average image color using `/color`

  For example having black&white image:
  ```rebol
  i: make image! 2x1 i/1: 0.0.0
  ```
  One can count average color using:
  ```rebol
  i/color ;== 127.127.127.255
  ```
  
  Using `/color:` will set RGB components of all pixels (ignoring alpha channel)
* [`4f1680`](https://github.com/Oldes/Rebol3/commit/4f1680959ea8b67e0a99858dedf19df46d4f8ebc) More getters/setters for image! type

  Could be optimized and enhanced later. For example it would be good to add also HSV, Hue, Saturation, Lightness etc. For these it would be good not to support only `binary!`, but also `vector!` as an input/output.
* [`d14793`](https://github.com/Oldes/Rebol3/commit/d14793dd005bdfab923c988112e2e36f8e08264a) Adding `codecs` as a shortcut for `system/codecs` and `keys-of` as an alias to `words-of`

  Because `keys-of codecs` looks better than `words-of system/codecs`
* [`6ca3c1`](https://github.com/Oldes/Rebol3/commit/6ca3c178eaa7228984ef3be6f359184f56a5ddfb) Added UNIXTIME codec for conversion of Rebol datetime! to Unix epoch number and back

  Example:
  ```rebol
  encode 'unixtime 18-Sep-2019/9:06:45+2:00 ;== 1568790405
  decode 'unixtime 1568790405 ;== 18-Sep-2019/9:06:45+2:00
  ```
  string! and binary! is also supported as an input/output:
  ```rebol
  encode/as 'unixtime 18-Sep-2019/9:06:45+2:00 string! ;==  "5D81D785"
  encode/as 'unixtime 18-Sep-2019/9:06:45+2:00 binary! ;== #{5D81D785}
  decode 'unixtime  "5D81D785" ;== 18-Sep-2019/9:06:45+2:00
  decode 'unixtime #{5D81D785} ;== 18-Sep-2019/9:06:45+2:00
  ```
* [`f79560`](https://github.com/Oldes/Rebol3/commit/f795601db13e68a8cdd39a76b3fafc8903a959fe) Updating Windows' prebuild R3 executable

  The previous one was over year old, so it's time to update it into up-to-date version. Especially that now it by default does not enter console on error in evaluated script.
  (https://github.com/Oldes/Rebol3/commit/61c138334d9fd93ca5760fdf685080f285251193)
  
  Note that it is not fully featured version to keep the executable small.

### Fixes:

* [`e4841c`](https://github.com/Oldes/Rebol3/commit/e4841c8319b94711cc9a501c99b787fdcdcdaa46) Function source missing indentation
* [`e03b3b`](https://github.com/Oldes/Rebol3/commit/e03b3bad59beece33caba1f656372ebe57775235) MOLD/FLAT on binary values does not remove line breaks
* [`19257d`](https://github.com/Oldes/Rebol3/commit/19257d8a6a790bc15905d7f8f5b426080bb2956b) MOLD/FLAT on image values does not remove line breaks
* [`060126`](https://github.com/Oldes/Rebol3/commit/060126a3060ed19a159f3eadf1fa1168f939c5bd) Invalid MOLD on empty image
* [`425fa5`](https://github.com/Oldes/Rebol3/commit/425fa56dd22d0b8b49206e9b1986c313d9584c43) MOLD/FLAT on block values does not remove line breaks
* [`2f5b50`](https://github.com/Oldes/Rebol3/commit/2f5b5002b4b8376b5418f7029813475cca35ce5d) MOLD/FLAT on event values does not remove line breaks
* [`a6c966`](https://github.com/Oldes/Rebol3/commit/a6c966f4a23504dde7c2ede85964da3f9f8a4821) EVENT! reports coordinates regardless of EVF_HAS_XY
* [`a3f69e`](https://github.com/Oldes/Rebol3/commit/a3f69e2094fdcf9092a10cb6301c8e387dc86790) Fixed clang warnings in 64bit build when using debug output showing various struct sizes
* [`105772`](https://github.com/Oldes/Rebol3/commit/105772a9d54982717ab00e9ce90c2874b6764be0) Small cosmetic change in `HELP` output
* [`7c4680`](https://github.com/Oldes/Rebol3/commit/7c4680e28f08f6faae5d432a73d74a1563d75075) WIC WINCODEC_ERR_BADHEADER only for JPEG images
* [`43f6d4`](https://github.com/Oldes/Rebol3/commit/43f6d4762b2758ca79c2a6ade9136a6551a0c33a) Adding missing type_traits include for casting macros with CPP11+
* [`08dc4e`](https://github.com/Oldes/Rebol3/commit/08dc4eb5b90073e4f799012bb0f120f203bb58c9) Embedded Extension Module test code included only if TEST_EXTENSIONS define is used

### Changes:

* [`19bbfe`](https://github.com/Oldes/Rebol3/commit/19bbfee7409b826a9da3c2d0a6b983e7551f399a) XML codec by default does not trim/remove strings between tags.

  Example:
  ```rebol
  third codecs/xml/decode  {<a>  <b/>  </a>} ;== [["a" none ["  " ["b" none none] "  "]]]
  third codecs/xml/decode/trim  {<a>  <b/>  </a>} ;== [["a" none [["b" none none]]]]
  ```
* [`80597e`](https://github.com/Oldes/Rebol3/commit/80597e46f22521bac0572b19cb977f7cc30c0c2f) Renamed crc32_table to CRC32_Table so it has same case as similar CRC_Table

## 2019 August

### Features:

* [`642086`](https://github.com/Oldes/Rebol3/commit/642086be8b30b072f30466a917c3366099b9219e) Improving BinCode

  * added possibility to specify number to bytes to read directly: `binary/read #{0102} 1`
  * added possibility to write FILE! and URL! types
  * added few missing UI*BYTES variants (also with little/big endian) - but that must be revisited later!
* [`1f9199`](https://github.com/Oldes/Rebol3/commit/1f9199a9122cc59f578feb5984cfa55fed24f295) Revisited original Carl's build scripts and improving these to be able create multiple _products_ without modifying it too much.

  One could spend more time in these as they were originally written to be working also with old Rebol 2 in the bootstrap phase. There is a lot of improvement possibilities, but that is not too important for now.
  
  Main change is, that I stopped using REB_CORE define and instead use REB_VIEW when I want to build Rebol with early View functionality (not much stable/useful yet).
* [`224ea4`](https://github.com/Oldes/Rebol3/commit/224ea4964ea35a89b987f96f91484ab804028226) Preliminary use of image native OS codecs on Windows (using Windows Imaging Component)

  If `USE_NATIVE_IMAGE_CODECS` is defined, than WIC is used instead of the original code, which supported only PNG, BMP and partially for JPEG and GIF).
  
  Using OS to do image de/encoding adds more possibilities, but so far, it still needs some work, especially for encoding, when user may want to specify various output options.
  
  For more details what WIC is, check: https://docs.microsoft.com/en-us/windows/win32/wic/

### Fixes:

* [`407aee`](https://github.com/Oldes/Rebol3/commit/407aeea52819929688ecfc63f970dd3347ee05ac) Revisited TLS and HTTP protocols to be more stable in some cases
* [`99a6cd`](https://github.com/Oldes/Rebol3/commit/99a6cd1daf30fc5fd9ed384946ad420c4053c2aa) Several clang warnings fixed
* [`61c138`](https://github.com/Oldes/Rebol3/commit/61c138334d9fd93ca5760fdf685080f285251193) Errors should not HALT to the console but QUIT to the system
* [`89642e`](https://github.com/Oldes/Rebol3/commit/89642e5faa66c2cf9f9e55d36ff7ffd41a2cde51) Warning: using the result of an assignment as a condition without parentheses

## 2019 July

### Features:

* [`6df2ec`](https://github.com/Oldes/Rebol3/commit/6df2ece2c4146ab32f9a43e3c8f2dc9d7b8d84d8) Added /fmt refinement to DUMP function for displaying only internal information about series value without displaying its data
* [`25720e`](https://github.com/Oldes/Rebol3/commit/25720e5d790cb1a79643d85368c189e9cc276dbe) New native `ecdsa` = Elliptic Curve Digital Signature Algorithm
* [`39957a`](https://github.com/Oldes/Rebol3/commit/39957a403c09542042831fe8bf64046b6ad75780) Updated TLS protocol to support TLS_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256 cipher suite
* [`6ec697`](https://github.com/Oldes/Rebol3/commit/6ec6976a7665b0a9c7d557ed3ba25d948d817ddf) Added possible compiler's define EXCLUDE_CHACHA20POLY1305 if the chacha/poly code is not needed in the executable
* [`a25f62`](https://github.com/Oldes/Rebol3/commit/a25f62825d93487cb981e4126c44d9c43c39df09) Modified Chacha20 and Poly1305 code to use types same like rest of the system.
* [`6a71ad`](https://github.com/Oldes/Rebol3/commit/6a71ad8d4a435ca0d08a423c9ff333a90c0935b3) Included macros for conversions from U8 to U32 and U64 (little endian) and back

### Fixes:

* [`85bc4d`](https://github.com/Oldes/Rebol3/commit/85bc4dc33a85782890a8ef907a5a8f4986b9bca2) Removed <ctype.h> dependency in PNG codec
* [`1d56a2`](https://github.com/Oldes/Rebol3/commit/1d56a21620f630a17232b5c47eb69af6b4542dc1) Several clang warnings fixed
* [`445ee8`](https://github.com/Oldes/Rebol3/commit/445ee86778aa16393000a2d5fcf9e4c925f3147f) Silence "loop has empty body" warning when used Clang
* [`3e9dce`](https://github.com/Oldes/Rebol3/commit/3e9dce6ec28a6e44572df5b94a7a896942609a13) Separate Copy_Wide_Str from Copy_OS_Str
* [`362fe7`](https://github.com/Oldes/Rebol3/commit/362fe7e1be1571d4fb10b0bf92c2407d1c13edf0) HELP was providing invalid info if value of path was of ERROR! type

## 2019 June

### Features:

* [`76e91b`](https://github.com/Oldes/Rebol3/commit/76e91b6bac19c3d50c21ca02e6508bffcd0647f0) New native `Chacha20Poly1305`
* [`0c166c`](https://github.com/Oldes/Rebol3/commit/0c166c6bc367a466ab830879b7a3368a4197b564) Added poly1305 `/finish` refinement to return computed MAC of the data.

  Note: it clears internal state so only one call to `/finish` or `/verify` will return real result!
* [`87d6fb`](https://github.com/Oldes/Rebol3/commit/87d6fbe320ca303e81e8532c237ee0cd8d2625cb) New native `poly1305` for message authentication
* [`95bb58`](https://github.com/Oldes/Rebol3/commit/95bb58ffbf3e493f175b491875468c18809156b2) New native `chacha20` for cipher stream encryption/decryption
* [`b9adfc`](https://github.com/Oldes/Rebol3/commit/b9adfc73dffdedb2ce79406d25c1b5347a46cf72) Added support for TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA and TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA cipher suites in TLS protocol.
* [`1e9339`](https://github.com/Oldes/Rebol3/commit/1e93393437508984e2b28f2e28cb8f9cf6d0ab09) New native `ecdh` for Elliptic-curve Diffie-Hellman key exchange.

  Using Kenneth MacKay's [micro-ecc project](https://github.com/kmackay/micro-ecc)
  
  For usage example see the modified `dh-test.r3` file.
* [`1bc566`](https://github.com/Oldes/Rebol3/commit/1bc566f50aaf0eeef31a20833c2123e582174f44) Added ENHEX native (string conversion with chars converted to URL-ENCODED format when needed)
* [`c57b31`](https://github.com/Oldes/Rebol3/commit/c57b31e8819525c4fca5aa3d15596a4f1c2f97b8) Adding VAL_UNI_TAIL macro for getting tail of unicode encoded series
* [`679277`](https://github.com/Oldes/Rebol3/commit/6792776838cc2705a3a4ad880292a86f75f6e030) Added possibility to set User-Agent in HTTP scheme

  One can now for example pretend that HTTP requests are coming from Chrome on Windows 10, when use this code before reading URL(s):
  ```rebol
  system/schemes/http/user-agent: "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/73.0.3683.103 Safari/537.36"
  ```
  
  This may be useful, because for example Google is serving different results, when default "REBOL" user agent is used.

### Fixes:

* [`97334b`](https://github.com/Oldes/Rebol3/commit/97334b06e36b62e8f2078e35da0ab4b1d3cafc15) In rare cases scanner was making *null holes* in strings, when the internal buffer was extended.
* [`b99224`](https://github.com/Oldes/Rebol3/commit/b99224b3dea9ffcd6dac90b484ea494dae15aeca) Adding missing dependency file for the last commit
* [`5e06fd`](https://github.com/Oldes/Rebol3/commit/5e06fd85b0ff2ae5b788e967a41011afc53c2ac9) Changing some of the log traces so they all starts with capital letter
* [`aba986`](https://github.com/Oldes/Rebol3/commit/aba9869dec107800808ae2fbccc89a3540cbec50) Removing debug trace
* [`15cb76`](https://github.com/Oldes/Rebol3/commit/15cb76aecbf2156576df21103e499b494e12d8bd) Small code cleanup
* [`cdd10e`](https://github.com/Oldes/Rebol3/commit/cdd10e7f1cabc7ec63e9fd446682a28639b92ff9) Improved TLS and HTTP protocol to correctly handle chunked data and case when TLS packed was not fully processed
* [`8215de`](https://github.com/Oldes/Rebol3/commit/8215deb642de70d1197d061f79c103dc368f588a) Using the new ENHEX function in HTTP protocol so it correctly handles requests with unicode chars
* [`295be2`](https://github.com/Oldes/Rebol3/commit/295be230529a16e01371c59a762d80aa93671106) DECODE-URL does not support UNICODE
* [`7517d0`](https://github.com/Oldes/Rebol3/commit/7517d0b5b47dd930daf6c8a267ce88d3da81ea78) ENHEX - underscore char does not have to be encoded
* [`a8b9ac`](https://github.com/Oldes/Rebol3/commit/a8b9ac9c80adc2c5850ab8fa01c187bf8ca07539) MOLD of URL containing unicode chars is invalid
* [`76c87a`](https://github.com/Oldes/Rebol3/commit/76c87a9651a0c82b894f4e02d07718a272d61b7f) Invalid ENHEX edge result
* [`aa939b`](https://github.com/Oldes/Rebol3/commit/aa939ba41fd71efb9f0a97f85993c95129c7e515) `wide` internal encoding was not used for strings containing chars in range >= 80h and <= FFh

### Changes:

* [`b774da`](https://github.com/Oldes/Rebol3/commit/b774dac68d4a64725573bd83fb44fb11d21ef45e) SWF codec - all three read-PlaceObject* functions now returns first 6 values pointing to same data [Depth move/place ClassName CharacterId Matrix Cxform], to simplify its processing, event when the values may be NONE in older PlaceObject tag versions.

## 2019 May

### Features:

* [`4e8896`](https://github.com/Oldes/Rebol3/commit/4e8896ab934fbce26cd1488b1a787f9c0a5f0d9b) SWF codec - allow none! as a valid input to decode-tag function
* [`4e3eef`](https://github.com/Oldes/Rebol3/commit/4e3eefa37125002e258d600a04db0adf46240661) When value of string type is used to be written, it is converted to UTF-8 automatically
* [`4e3eef`](https://github.com/Oldes/Rebol3/commit/4e3eefa37125002e258d600a04db0adf46240661) Added UI16LEBYTES command
* [`4a4418`](https://github.com/Oldes/Rebol3/commit/4a4418b72318da82a08b6a5b6fb2c1f82ee8f190) Bincode - value to write defined as GET-WORD! or GET-PATH!
* [`1029ea`](https://github.com/Oldes/Rebol3/commit/1029eaea7795cec1fce5638de1fe29d07b56c1b4) Bincode - added support for writing FLOAT16, FLOAT and DOUBLE values
* [`c1c1f2`](https://github.com/Oldes/Rebol3/commit/c1c1f2108f075887b65edbcf8332b34ac8ece6e0) Added support for ADLER32 checksum method
* [`935e61`](https://github.com/Oldes/Rebol3/commit/935e61b65ddaad96bbb7bdb60a3c7e97946110ee) Allow CHECKSUM again on strings
* [`dcd405`](https://github.com/Oldes/Rebol3/commit/dcd4058c26908c9c2ec07e74297a97e05b997909) Including WHOIS protocol

  Usage example:
  ```rebol
  read whois://seznam.cz ;<- will do recursive lookup
  write whois://whois.nic.cz "seznam.cz" ;<- query a given whois server directly
  ```
* [`243300`](https://github.com/Oldes/Rebol3/commit/2433000735082a34c7224d515d8cb8ce24779d8c) CHANGE: enable any value as a data type for WRITE action

### Fixes:

* [`eef497`](https://github.com/Oldes/Rebol3/commit/eef497e0b4684e927a9677d1110bb9fd17f7cfe5) Using MOLD/ONLY instead of FORM when saving data as a *.txt file
* [`88ec45`](https://github.com/Oldes/Rebol3/commit/88ec458b93824ae5f803022b2cba0e0820603d61) Valgrind reporting errors when using ENCODING? in JPEG codec
* [`f44442`](https://github.com/Oldes/Rebol3/commit/f44442149aa47b3e543c168f247ec3f4622a8c48) SAVE broken with .txt suffix
* [`7b9e65`](https://github.com/Oldes/Rebol3/commit/7b9e655d73935da66ab7eb37564e3405d530b499) Simple text (and empty text) should be identified as TEXT when using ENCODING? function.
* [`f0ab71`](https://github.com/Oldes/Rebol3/commit/f0ab71b622c8c0221925a4270c00a1beadef0e57) Bincode - read FB was not handling negative numbers correctly
* [`f65ffd`](https://github.com/Oldes/Rebol3/commit/f65ffd9589bc43a895ddf11dcb4e4d37c9257a3b) SORT/COMPARE is using reversed order when using integer as a result from the function
* [`dc46df`](https://github.com/Oldes/Rebol3/commit/dc46df8581215b4942a0cebc408057d418cb741b) Type-check call of SORT/compare handler
* [`49bcd1`](https://github.com/Oldes/Rebol3/commit/49bcd1722a80e949b5d2bddbfaefbcf61e046d11) Bincode - the output buffer position was not updated
* [`820a3a`](https://github.com/Oldes/Rebol3/commit/820a3ab269ecf5fa9b41e33ee9bf3b734d00a3f6) Warning message about duplicate specification was always shown in verbose mode (even without any duplicates)
* [`254b78`](https://github.com/Oldes/Rebol3/commit/254b78be6ca551385ca59dc05df28a97fb8879bd) Rebol cannot run a script that has UTF-8 chars in its filename or path
* [`d10845`](https://github.com/Oldes/Rebol3/commit/d10845761cd10f9b4a7785c8fd738d99751cf80d) Removed printed output in case of DNS awake

## 2019 April

### Features:

* [`89bddd`](https://github.com/Oldes/Rebol3/commit/89bdddac794160c869cb0539fdd6a0e8f45b9611) Added RL_Get_Value_Resolved as a library function which works like original RL_Get_Value, but with difference, that when resulting value is WORD, GET-WORD, PATH or GET-PATH, result is not this value, or its resolved value.

  This is useful when writing dialects as an native extension's code.
* [`bebab4`](https://github.com/Oldes/Rebol3/commit/bebab4a8dbdde5770907b13af0a36df9bffcfcd3) When used VIEW with image as an argument, open in as a centered and resize able window;
* [`ead47c`](https://github.com/Oldes/Rebol3/commit/ead47c08bc8a6ae89e575411c876ecbbf02e035b) HTTPD server now have ON-ACCEPT actor which can be used to limit connection to specified IPs only
* [`7ba3dc`](https://github.com/Oldes/Rebol3/commit/7ba3dc20932f52a950b63b78b6bad8c4170bb6aa) HTTPD server now have ON-HEADER actor which can be used for early request processing like redirection or rewrite rules (see the test file for usage example)
* [`131333`](https://github.com/Oldes/Rebol3/commit/1313331ba63bf13617254c5e778799038156eeda) Simple HTTP server scheme module

  One can use it to start a HTTP server using just:
  ```rebol
  http-server 8080
  ```
  but most probably you would like to customize it a little bit. For details see the included %test-httpd.r3 script.
* [`cf7ba1`](https://github.com/Oldes/Rebol3/commit/cf7ba1f1d763472643e0ece019b172feff93c615) Allow DEHEX to accept `binary!` value

  So one can use it to properly url-decode utf-8 sequence, like:
  ```rebol
  >> to-string dehex #{2F666F726D3F763D254335253939}
  == "/form?v=ř"
  
  >> to-string dehex to-binary "%c5%99"
  == "ř"
  
  ```

### Fixes:

* [`615e82`](https://github.com/Oldes/Rebol3/commit/615e82ba42ae219e46e1a1f20178c0ed7a0e7081) Correctly handling case where result from script evaluation is unset
* [`90a710`](https://github.com/Oldes/Rebol3/commit/90a710c298dbc5396f54e5334704a9da79dc6c78) Exclude MODULE result from evaluation to be printed in console for now (there should be first some sort of nicer formating)
* [`2eec29`](https://github.com/Oldes/Rebol3/commit/2eec29e5eafb5c036ea39cc2fe8365fc78a4c0b5) Window containing just an image was not drawn. This commit
* [`d370b4`](https://github.com/Oldes/Rebol3/commit/d370b453fca8be79fa5e70ae4ccd16e192402883) Restore original path when DO of a script fails
* [`0dae54`](https://github.com/Oldes/Rebol3/commit/0dae54bbe2b6d3c6a59edebec0ff0035b69f1a9f) Calling an action made by oneself crashes R3
* [`2d8925`](https://github.com/Oldes/Rebol3/commit/2d892516ef32d9a6e2725408677762abb0d6e8b8) HTTP protocol now handle limit of max redirection
* [`4b058f`](https://github.com/Oldes/Rebol3/commit/4b058fad2b14e52c15e2b7c68ea046b783ad8d7b) HTTP scheme redirect was not correctly dealing with port id
* [`40b9e5`](https://github.com/Oldes/Rebol3/commit/40b9e5c43cca5dbd5307736d166c6da5a02b4d9c) TO-DATA allow string date representation with day represented in one char only too

### Changes:

* [`77bd60`](https://github.com/Oldes/Rebol3/commit/77bd6065742144a14298c10ddc0711f46a6d699d) Initialize Winsock with version 2.2

## 2019 March

### Features:

* [`6b4ef6`](https://github.com/Oldes/Rebol3/commit/6b4ef6349ef39f8bf2f260eda171828138605f89) New functions TO-IDATE, TO-ITIME, enhanced TO-DATE

  For conversion to a standard Internet date/time string (as used in HTTP headers)
  Function `to-date` now can now be used to convert these strings back to date.
  It has also new refinement `/utc` for conversion to UTC date.
  
  Example:
  ```rebol
  >> to-date/utc "Thu, 28 Mar 2019 20:00:59 +0100"
  == 28-Mar-2019/19:00:59
  
  >> to-date/utc "Thu, 28 Mar 2019 20:00:59 GMT"
  == 28-Mar-2019/20:00:59
  ```
* [`8fe060`](https://github.com/Oldes/Rebol3/commit/8fe060ac34322837f994f312f5a1e0c0460021a9) Adding /TIMEZONE to DATE values which adjust the time

  `/timezome` was first introduced in Red language and it can be used to change the time zone with adjusting the time, while `/zone` keeps time unmodified.
  
  Any date can be now easily converted to UTC just setting the `timezone` to zero.
* [`3770d7`](https://github.com/Oldes/Rebol3/commit/3770d72a6557a28e3e926c09f1cee0e149507bac) Registering .epub extension into ZIP codec (so it's possible to load epub files)
* [`e08b4b`](https://github.com/Oldes/Rebol3/commit/e08b4bcc9fa35386bd9c563f8752eda26c63e25f) Using sync READ of HTTP will try to automatically convert content to text according given encoding if `content-type` of the result is specified as `text/*`

  This means, that for example:
  ```rebol
  read http://www.google.jp
  ```
  will return string properly converted to Unicode (as the result from Google is served as `Shift_JIS` charset.
* [`d71a27`](https://github.com/Oldes/Rebol3/commit/d71a27195ba83b9dec226edbef3f588356d64d33) New `ICONV` native for conversion between various code pages

  This function is on Posix wrapped around [iconv](http://man7.org/linux/man-pages/man1/iconv.1.html) function. On Windows it is emulated using Kernel functions for conversion between wide char and multi byte.
  
  So far there are two modes, conversion from binary to unicode string:
  ```rebol
  >> iconv #{9AE96D} 'cp1252
  == "šém"
  ```
  or from one binary to another with different encoding:
  ```rebol
  >> iconv/to #{9AE96D} 'cp1252 'utf-8
  == #{C5A1C3A96D}
  ```
* [`9347af`](https://github.com/Oldes/Rebol3/commit/9347afc64d81e61955dd3b2a4dbd521a4927a454) HTTP: Added support for transfer compression (gzip or deflate)

  Also now using system trace outputs. At this moment it's set by default to level 1 (info)
  Can be changed using for example:
  ```rebol
  system/options/log/http: 3 ;<- for max traces
  ```
* [`7b6264`](https://github.com/Oldes/Rebol3/commit/7b6264a5ab44e7544f7984384fd40f95e369ae49) Using system/options/log to specify system log verbosity in central place

  (this is WIP as codecs and schemes are still using not centralized traces)
* [`ebaa2c`](https://github.com/Oldes/Rebol3/commit/ebaa2c251eb09beda7ecfb20606508afc3cb100b) Decompress/deflate - decompression of data which are in raw deflate format (without envelope)

  Example:
  ```rebol
  >> decompress/deflate #{2B492D2E01000C7E7FD804}
  == #{74657374}
  
  ```
  
  Also using it in the ZIP codec.
* [`d045d4`](https://github.com/Oldes/Rebol3/commit/d045d438f623ce1856fc1b9dee32414455dc74b8) Including the TAR codec as a mezzanine
* [`af235b`](https://github.com/Oldes/Rebol3/commit/af235bc4fb19d17e913f06f9454b3e4c45d6f27c) TAR codec (decode only so far)
* [`1d2daf`](https://github.com/Oldes/Rebol3/commit/1d2daffa18079948b66e296deddc373b8b9ff189) Bincode: reading of STRING-BYTES and OCTEL-BYTES (string and octal number from fixed size bytes)
* [`94508e`](https://github.com/Oldes/Rebol3/commit/94508ee513fc5d96d51397690082151d65cd0027) Introducing system `log` function for common system trace output

  Also using it for now with ZIP codec, instead of using local log functions.
* [`e3e33b`](https://github.com/Oldes/Rebol3/commit/e3e33bb2eaf7198c13b79c2eae3dfd01870cb813) Including the ZIP codec as a mezzanine
* [`908bb3`](https://github.com/Oldes/Rebol3/commit/908bb312a87aa2ff5cc614a2934811044fcb9c87) ZIP codec (decode only so far)

  It extract ZIP data into memory.
  
  Example:
  ```rebol
  >> load %/x/test.aar
  Decode ZIP data ( 2919 bytes )
   [ZIP] Decompressing:  AndroidManifest.xml  bytes: 504 -> 832
   [ZIP] Decompressing:  R.txt  bytes: 1535 -> 7180
   [ZIP] Decompressing:  classes.jar  bytes: 120 -> 233
   [ZIP] Decompressing:  aidl/
   [ZIP] Decompressing:  assets/
   [ZIP] Decompressing:  jni/
   [ZIP] Decompressing:  res/
   [ZIP] Decompressing:  libs/
  == [
      %AndroidManifest.xml [4-Apr-2018/18:52:10 662885694 #{
  3C3F786D6C2076657273696F6E3D22312E302220656E636F64696E673D227574
  662D38223F3E0A3C212D2D0A20436F7079726967687420284329203230313420
  54686520416E64726F6964204F70656E20536F757263652050726F6A6563740A
  0A20202020204C6963656E73656420756E646572207468652041706163686520
  4C6963656E73652C2056657273696F6E20322E30202874686520224C6963656E
  736522293B0A2020202020796F75206D6179206E6F7420757365207468697320
  66696C652065786365707420696E20636F6D706C69616E636…
  
  ```
  
  It's possible to use the ZIP codec directly and extract only specified files:
  ```rebol
  >> zip-decode: :system/codecs/zip/decode
  >> zip-decode/only %/x/test.aar [%classes.jar]
  Decode ZIP data ( 2919 bytes )
   [ZIP] Decompressing:  classes.jar  bytes: 120 -> 233
  == [
      %classes.jar [4-Apr-2018/18:53:56 646121705 #{
  504B03040A000008000000002100F7D7D29907000000070000002F0015004D45
  54412D494E462F636F6D2E616E64726F69642E737570706F72745F737570706F
  72742D76342E76657273696F6E55540D0007000000000000000000000000FECA
  000032372E312E310A504B01020A000A000008000000002100F7D7D299070000
  00070000002F000D0000000000000000000000000000004D4554412D494E462F
  636F6D2E616E64726F69642E737570706F72745F737570706F72742D76342E76
  657273696F6E555405000700000000FECA0000504B050600000000010…
  
  ```
* [`ef895e`](https://github.com/Oldes/Rebol3/commit/ef895ef6caf433ebc509c91e8f14ea4dfaedb5ed) Bincode: added reading/writing for MSDOS date time formats (used for example in ZIP files)

  The new dialect words: MSDOS-DATE, MSDOS-TIME and MSDOS-DATETIME
* [`ae6795`](https://github.com/Oldes/Rebol3/commit/ae679561a425fa89505b30354d39e687d658c641) Added Server Name Indication extension into TLS scheme

  More details: https://tools.ietf.org/html/rfc6066#section-3
  
     TLS does not provide a mechanism for a client to tell a server the
     name of the server it is contacting.  It may be desirable for clients
     to provide this information to facilitate secure connections to
     servers that host multiple 'virtual' servers at a single underlying
     network address.
  
     In order to provide any of the server names, clients MAY include an
     extension of type "server_name" in the (extended) client hello.
* [`6194ff`](https://github.com/Oldes/Rebol3/commit/6194fffbb2f2e54bfc03e456e008feb6c21facac) Registering XML codec with POM file extension

  POM is a file used by Maven; contains build information, such as required software dependencies, dependency scope, and packaging information.
  
  Example:
  ```rebol
  load https://dl.google.com/dl/android/maven2/com/google/android/gms/play-services-auth-api-phone/16.0.0/play-services-auth-api-phone-16.0.0.pom
  ```
* [`4018c5`](https://github.com/Oldes/Rebol3/commit/4018c5e37d6e191e5893ffa1da6d01ec34c24e1a) Initial implementation of OpenGL widget

  Nothing special yet, just displays hard-coded triangle with random background on redraw.
  Minimal example:
  ```rebol
  view make gob! [size: 400x400 widget: 'opengl]
  ```
* [`68d210`](https://github.com/Oldes/Rebol3/commit/68d210adee11c738ec05d6e5f94e136ba37af8a7) Introducing _native GOB widgets_ for Windows OS.

  It's just an initial _proof of concept_. Using the existing `gob!` type as a holder of native GUI widgets. At this moment there are *partially* supported these native widgets: `button`, `check`, `radio`, `group-box`, `field`, `area`, `text`, `slider` and `date-time` (which so far works more like a date picker only).
  
  Known issues:
  
  * the native widgets are not converted to image, when using `to-image window-gob`.
  * it looks there is a memory leak in the compositor as opening/closing multiple windows has growing memory effect. This is probably not directly related to native widgets as I can see it with just an image too.
  * it's possible to append widgets into another widget's `pane`, but the position is not relative to the parent.
  * there is no helper for creating a native gobs tree (no `layout`).
  * there are still some output logs as this is really more just an experiment (and my learning playground).
  
  Simple example displaying field and a button with event handler:
  
  ```rebol
  handle-events [
  	name: 'gob-example
  	priority: 60
  	handler: func [event][
  		print ["view-event:" event/type event/offset event/gob]
  		if switch event/type [
  			close [true]
  			key   [event/key = escape]
  		] [
  			unhandle-events self
  			unview event/window
  			return none
  		]
  		switch event/type [
  			click
  			change [
  				print ["Field data:" mold fld/data]
  			]
  		]
  		none
  	]
  ]
  
  btn: make gob! [size: 200x29 offset: 20x20 widget: [button "hello"]]
  fld: make gob! [size: 200x29 offset: 20x50 widget: [field  "world"]]
  win: make gob! [size: 240x99 offset: 90x99 pane: [btn fld]]
  view/as-is win
  ```

### Fixes:

* [`6f7358`](https://github.com/Oldes/Rebol3/commit/6f73585f42f389668a3a2814c41b2a794bfc710c) Fixed TO-ITIME formating if time value parts are less than 10 and or time contains miliseconds
* [`8bea93`](https://github.com/Oldes/Rebol3/commit/8bea9390feaa35664c38bbdfc2dc036fe0ba792e) HTTP protocol now closes connection in case of error
* [`a059e1`](https://github.com/Oldes/Rebol3/commit/a059e1540796a611c042e3ba81b7274e77128c7c) Code cleanup
* [`679d4e`](https://github.com/Oldes/Rebol3/commit/679d4e28d1fbbd6dd53571fe60f57556d92b88ef) Adding ZONE to DATE value without TIME
* [`514e8e`](https://github.com/Oldes/Rebol3/commit/514e8e7e84a1b01d29394267d8aa75aed90eeade) Limit SET to just WORD! and LIT-WORD! instead of ANY-WORD!
* [`798cf0`](https://github.com/Oldes/Rebol3/commit/798cf0a5b39f2bac10381f30584163ccd1dd2f9c) HTTP: do code-page conversion really only when content-type is of type "text"
* [`9aa122`](https://github.com/Oldes/Rebol3/commit/9aa122be31bb31a6b097295f52705e39ceacb859) Making macOS version be compile-able again with the recent `iconv` addition.
* [`2e1a8b`](https://github.com/Oldes/Rebol3/commit/2e1a8b5f8e300e0cbf178245fcffb259f65c303e) Removing forgotten debug certificate export in TLS protocol
* [`bf1d6d`](https://github.com/Oldes/Rebol3/commit/bf1d6d1b52bf07be1723f8bf680ca3d9c42b81f2) Bincode: fixed regression in read BYTES
* [`67c8ea`](https://github.com/Oldes/Rebol3/commit/67c8ea57683fb611d101bdf57f2607069fb70c7e) Fix for the last commit ):last minute changes:(
* [`cdfe7a`](https://github.com/Oldes/Rebol3/commit/cdfe7a869bb824133575d6f30e752c5d5ccfc62b) Fixed buffer overflow in RL_Word_String()
* [`58cf7b`](https://github.com/Oldes/Rebol3/commit/58cf7b7751974d298ebbb8302cdde2495b113414) TLS: moved computation of the message-hash checksum into proper section (where possible error can be trapped)
* [`0b95ae`](https://github.com/Oldes/Rebol3/commit/0b95aebc36ab7cbf92c91ce2427189631bee94a6) TLS: using proper checksum method for message-hash (not only SHA256)
* [`d494aa`](https://github.com/Oldes/Rebol3/commit/d494aad3b9fd742b6522bd1312cd6235984880d9) Properly reseting internally used lexer's token value
* [`df959a`](https://github.com/Oldes/Rebol3/commit/df959a5892d229e2deef12f7d19899a5c8460700) Improved doc-string for `round/to` action and added a few related unit tests
* [`dd7f03`](https://github.com/Oldes/Rebol3/commit/dd7f03e1312d0612f2f43ca726d402008db60f33) Using SwapBuffers instead of wglSwapBuffers and adding opengl.lib into systems definition for not VS builds
* [`faf671`](https://github.com/Oldes/Rebol3/commit/faf6715886f9f3e5a56cd362eda457d0ece866c2) Disable GetVersionEx deprecation warning in VS
* [`9e2c7b`](https://github.com/Oldes/Rebol3/commit/9e2c7be6c341d6fb53410247377a9041683b2284) Make sure that native widgets are redraw also when window is restored from minimized state
* [`6401f7`](https://github.com/Oldes/Rebol3/commit/6401f7cebe1b0b71df3904c1696f2f00bc896f0e) Fixing simple image blit, so one can scale image gob types, like:
* [`dd1eda`](https://github.com/Oldes/Rebol3/commit/dd1edab465b292f1585280d89ba35729ac8c1f5e) Make native widgets to lose focus when user clicks with mouse anywhere outside
* [`354e5f`](https://github.com/Oldes/Rebol3/commit/354e5fed70ad276575b7fae67ebdc64c3c4180da) Cleanup of stdio traces related to view
* [`68bdaa`](https://github.com/Oldes/Rebol3/commit/68bdaa7e15be1c9a4a2754477299f5e931a0d067) Fixed memory leak in window compositor
* [`12a1ac`](https://github.com/Oldes/Rebol3/commit/12a1ac120926bc9ed245a75976917a6b6b33ae3b) Handle-events is replacing existing event handler with the same name as a new one instead of adding it multiple times, and view is not adding a default handler if, there is already any handler registered.

## 2019 February

### Features:

* [`3f182e`](https://github.com/Oldes/Rebol3/commit/3f182e925c8b9bc34fb936095808e906b8afa3c3) GOB to image conversion on Windows -> `to-image make gob! []`
* [`fd4b3f`](https://github.com/Oldes/Rebol3/commit/fd4b3f6ef99724531fdf50f2c672cc02b986b0fc) Modified Saphirion/Atronix Win32 GDI compositor to be working without AGG (for color and image gob types so far)
* [`785ea9`](https://github.com/Oldes/Rebol3/commit/785ea9f7a37e15bb08402b9e661be58aa0f3c773) FEAT: fixed crash when user dropped file into window (with `dropable` flag)

  and fixed crash when calling `show` with `none` value.
  It's now possible to receive `drop-file` event with file name in `event/data`.
  It's also possible to resolve target drop using `map-event` function.
* [`699232`](https://github.com/Oldes/Rebol3/commit/699232609f49d08d25f0467d883870587447696e) Added basic set of SWF tag decoders into SWF file codec.
* [`af1be5`](https://github.com/Oldes/Rebol3/commit/af1be5b3483ba309657989460e652abbf573f4bb) Enhanced BINCODE dialect with new features:

  * added /with refinement to provide additional value for reading single value like:
    `binary/read/with b 'BYTES 42` which returns just the binary and not a block with binary.
  * it's now possible to use zero bits without range error: `binary/read b [UB 0]` returns 0.
  * new read command `FIXED8` (16-bit 8.8 fixed-point number)
  * new read command `FIXED16` (32-bit 16.16 fixed-point number)
  * new read commands `TUPLE3` and `TUPLE4` (returns tuple type value from 3 or 4 bytes)
  * new read command `SkipBits` (allows to skip given number of bits without reading anything)
  * new read command `ALIGN` (aligns bit stream to byte boundary)
  * new read commands `FLOAT16`, `FLOAT` and `DOUBLE` (16bit, 32bit and 64bit decimal value)
  * new read command `SI16LE` (16bit signed integer using little-endian byte order)
  * new read command `FB` (Signed, fixed-point bit value)
  * fixed read command `SB`
* [`765453`](https://github.com/Oldes/Rebol3/commit/76545389164093fbbc6428a0c2ed5e6f3d22fd0a) Codec-swf: added a few tag readers and more verbose output (WIP)
* [`9ca5cc`](https://github.com/Oldes/Rebol3/commit/9ca5cc087cf7d1bdcd884215e5ad7c982de03b08) Bincode: added reading commands: BITSET8, BITSET16 and BITSET32
* [`ea0608`](https://github.com/Oldes/Rebol3/commit/ea06082a6118ce346c5e96f4d437e51eeb8920be) Added support for EncodedU32 in BINARY's read and write

  EncodedU32 is used in Adobe's SWF files. It defines an 32-bit unsigned integer as a sequence of 1 to 5 bytes. The most significant bit of a byte indicates if the next byte is also part of the value. The byte are ordered low to high.
* [`04cb59`](https://github.com/Oldes/Rebol3/commit/04cb5929931cb52cb4d74e4747b87540e4e28141) Enabled "window" host extension on Windows (without graphics) so it's possible to `view gob` or `view image` to open a system window.

  Originally the windowing was dependent on `graphics` extension. Now it is independent and the graphics (using AGG ) is not yet ready.
* [`be815c`](https://github.com/Oldes/Rebol3/commit/be815c1879683085baa0dd0ffb0f60c6661c83e6) Added code for cleaning up opened MIDI connections on system QUIT (Windows only so far)
* [`764807`](https://github.com/Oldes/Rebol3/commit/7648071ad0549d4ef5b1c790425b924bf5e6a208) Including Graham Chiu's SMTP protocol source into repository

  It is version downloaded from his repository: https://github.com/gchiu/Rebol3/blob/master/protocols/prot-smtp.r
  
  As it was not tested yet (in this branch), I'm not including it into a build. Main reason of this commit is, that I don't want to loose track of this file as it may be useful in the future (although it will need update to support TLS)
* [`1d59f8`](https://github.com/Oldes/Rebol3/commit/1d59f86e3de8181d54a4df52a94f015e2f97c781) Very minimal SWF file format codec
* [`305fc3`](https://github.com/Oldes/Rebol3/commit/305fc368dcd02a41aa60b7041a7781a490b9b4f8) Initial MIDI port scheme implementation - macOS version

  The MIDI device is now also optional and must be enabled using `USE_MIDI_DEVICE` compilation define.
* [`3591ed`](https://github.com/Oldes/Rebol3/commit/3591edf113588247aa8a954ee792dc3c9012d8bd) Initial MIDI port scheme implementation (Windows only so far)

  Here is example how to list available input/output MIDI devices:
  ```rebol
  >> query midi://
  == make object! [
      devices-in: ["Launch Control XL"]
      devices-out: ["Microsoft GS Wavetable Synth" "Launch Control XL"]
  ]
  
  ```
  This will open MIDI port in write only mode and play a chord (Middle-C, E and G):
  ```rebol
  synth: open/write midi://1
  write synth #{903C7F00 90407F00 90437F00}
  ```
  
  The scheme is low-level so supports only binary for read/write. There can be made higher level scheme which could support a dialect for preparing the data.
  
  This is example how to get MIDI input using port's `awake`:
  ```rebol
  launch: open/read midi://device:1
  launch/awake: function [event [event!]][
  	switch event/type [
  		read  [ probe read event/port     ]
  		open  [ print "MIDI port opened!" ]
  		close [ print "MIDI port closed!" ]
  	]
  	true
  ]
  ```
  If the `awake` is defined like above and you are in some sort of `wait` loop, than whenever MIDI input is received, you would see the data from it printed in console.
  
  TODO:
      * support SYSEX messages
      * at least macOS support
* [`f6b2bb`](https://github.com/Oldes/Rebol3/commit/f6b2bb089fb5f50ef364c990bd88b49c6f25bc8b) Refine standard port specifications

  Originally there was `port-spec-head` which was holding also `path` and `host`, which are not needed in not file related schemes and does not looks good when listed.
* [`4a7852`](https://github.com/Oldes/Rebol3/commit/4a7852a68d4a9a4b8a4967e88a6745244dcb3336) Refine Find_Event function to find related event not only using event type, but also per port

  This function was introduced in Atronix branch to filter GUI events, where there was only one port. Now it's possible to use it also with schemes with multiple opened ports of the same type.
* [`9f13ef`](https://github.com/Oldes/Rebol3/commit/9f13ef94c5c0f3f4fe28455786999abb27dc11fd) CHANGE: making SET native to be more compatible with Red-language version

  There is new refinement /ONLY which treat source block or object like single value (so the result is like it was in R3-alpha)
  
  The /PAD refinement was replaced with /SOME refinement. The functionality is now reversed: when no refinement is used, it works like if there was /PAD before. If /SOME is used, there is no padding and also target value is not replaced with none value from the source, if there is any.
* [`d4261f`](https://github.com/Oldes/Rebol3/commit/d4261f8b6b9c5286a06aaa5611a8b0e0147ce7dc) CHANGE: making SET native to be more compatible with Red-language version

  There is new refinement /ONLY which treat source block or object like single value (so the result is like it was in R3-alpha)
  
  The /PAD refinement was replaced with /SOME refinement. The functionality is now reversed: when no refinement is used, it works like if there was /PAD before. If /SOME is used, there is no padding and also target value is not replaced with none value from the source, if there is any.

### Fixes:

* [`6d42d3`](https://github.com/Oldes/Rebol3/commit/6d42d36c77a7cc0818b10df604242d86cc574f59) Fixed implicit declaration
* [`0915fe`](https://github.com/Oldes/Rebol3/commit/0915feb716714d7056d26098729f78608f76b46a) Validate type before inserting into Gob
* [`b86e4b`](https://github.com/Oldes/Rebol3/commit/b86e4b267ebd064f7d9ccf436c0107c5fbee3c1d) Remove comparison between unsigned and zero
* [`dc76d0`](https://github.com/Oldes/Rebol3/commit/dc76d0bec57643b2d7488129bd907453ccff8102) Shut off a coverity complain about the unintialized var
* [`58d59c`](https://github.com/Oldes/Rebol3/commit/58d59c37aff31340df0a0d743536c10cb505f21c) Renaming 2 new host-*.h files to reb-*.h files as all host-*.h files are so far not included in the git repository (all of these are generated during compilation)
* [`e7be9a`](https://github.com/Oldes/Rebol3/commit/e7be9ac671326df09063b9c1b9b27b722a353f78) Force redraw a window when restored from minimized state.
* [`4d21b2`](https://github.com/Oldes/Rebol3/commit/4d21b216aaecc818874a274487544caca951d9d2) Fixing some of compilations issues like when compiling Rebol just as a library without the host part
* [`960d50`](https://github.com/Oldes/Rebol3/commit/960d504beccb420ebf11a4b5d152d0b3609f8b61) Don't allow to `view system/view/screen-gob`
* [`7d6191`](https://github.com/Oldes/Rebol3/commit/7d61915ff3c9774e0dfb8bf9d2a74085f82db593) Win32 GDI compositor how was defined in Atronix branch
* [`709e71`](https://github.com/Oldes/Rebol3/commit/709e7169ec0435959cbdce05b38123dfee75fbb4) Adding win32 compositor template how was originally defined by Richard Smolak
* [`8a96ed`](https://github.com/Oldes/Rebol3/commit/8a96edbb68f4d6d3c307a4fcb7f531353538467b) Make sure that only functions with "OS_" at the beginning are collected
* [`5e975a`](https://github.com/Oldes/Rebol3/commit/5e975ad3943bcb051a3ab74bffacbd64764a5392) Enable better wait precision for timeouts bellow 16ms
* [`d247f8`](https://github.com/Oldes/Rebol3/commit/d247f8fb912dced1b16447acc9a8fc97b8d6a903) Enable MIDI for 32bit macOS target
* [`ae925a`](https://github.com/Oldes/Rebol3/commit/ae925a3c0cffb6ac7eb5a6eadcd0abda20b940f6) Disable use of MIDI device on 64bit Linux (not yet implemented)
* [`dbd495`](https://github.com/Oldes/Rebol3/commit/dbd49502a5b6ee85dc35a5e742b509b868b796b0) Better error when setting image's alpha value using path notation
* [`821142`](https://github.com/Oldes/Rebol3/commit/821142490c1f975df20346ebf3daf17f43cb961d) Removing traces of not yet ready scheme
* [`eb7ba2`](https://github.com/Oldes/Rebol3/commit/eb7ba292e36254239c4b9c2b0de048c71d7fb246) MOLD/FLAT on object values does not remove line breaks
* [`9e2846`](https://github.com/Oldes/Rebol3/commit/9e2846c64619803e8d71a964a9673c808d3eccd9) Removing not ready leaked files
* [`873e2a`](https://github.com/Oldes/Rebol3/commit/873e2a4c44bf12021253d212db86cb1eaf2057fd) Removing not ready leaked files

## 2019 January

### Features:

* [`0e1498`](https://github.com/Oldes/Rebol3/commit/0e1498b1881b46b6acdcdd23a3c99413b4b86c07) Set object from object keeping only existing keys of the first one
* [`c3055d`](https://github.com/Oldes/Rebol3/commit/c3055d1e5d4e52521403adff331b27fbde02654e) Initial MIDI device (Windows only) - work in progress
* [`94ed18`](https://github.com/Oldes/Rebol3/commit/94ed18f21dce04ba701c1acea2d44dbece2b7156) Added compilation define to exclude math operation with vectors: EXCLUDE_VECTOR_MATH
* [`79673b`](https://github.com/Oldes/Rebol3/commit/79673b667f20220c11ab691c9df4d05e9e2c631c) Windows: Console Virtual Terminal Sequences

  If available (since Win10) using `ENABLE_VIRTUAL_TERMINAL_PROCESSING` output mode to process ANSI Escape Sequences. On older Windows versions it keeps using the ANSI emulation like before.
  
  It's possible to disable use of `ENABLE_VIRTUAL_TERMINAL_PROCESSING` mode by defining `FORCE_ANSI_ESC_EMULATION_ON_WINDOWS` compiler definition.
  
  Related documentation: https://docs.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
  
  Known issues:
  `The VIRTUAL_TERMINAL_PROCESSING` mode is not handling escape codes for hiding input (`^[[8m` and `^[[28m`), which is currently used in ASK/HIDE function. As these sequences are also not fully supported on some POSIX systems, the hiding input should be solved using other way.
  https://github.com/Oldes/Rebol-issues/issues/476
* [`0b95e4`](https://github.com/Oldes/Rebol3/commit/0b95e4a7c78061711a3120ca467cd5af7865a38a) Enable displaying MODULE value as a result in console
* [`729705`](https://github.com/Oldes/Rebol3/commit/72970568a0284934d5aa5a313ed5d3495aded18f) Display a few values of any-block type, when using HELP on object with block values
* [`c272af`](https://github.com/Oldes/Rebol3/commit/c272af1968cb3856c3720c4e2f21fd11018fe9b5) Implemented basic ADD, SUBTRACT, DIVIDE and MULTIPLY between VECTOR and NUMBER
* [`7383e1`](https://github.com/Oldes/Rebol3/commit/7383e12105a9ff5aec90a3cd3f3d5eef729d5411) Added RL_Make_Vector into RL api, so one can make a VECTOR from extension
* [`6624a7`](https://github.com/Oldes/Rebol3/commit/6624a7e2099c2d5959604d07c8f36e2570ac8610) Added optional compilation define to turn-of mapp key normalization

  https://github.com/Oldes/Rebol-issues/issues/2353
  https://github.com/Oldes/Rebol-issues/issues/2354
* [`69f4e2`](https://github.com/Oldes/Rebol3/commit/69f4e299dcd8c881d52e6728b8d0df0da33eb3ee) CHANGE: possibility to enquire the element properties of vector! values

  This commit also changes how are stored reflectors in `system/catalog`. Previously it was just block with names, now it contains also block with datatypes, which the reflector may be used with.
  
  For `vector` values, now there are probably defined all possible values to get its properties.
  Some of them:
  ```rebol
  >> v: make vector! [unsigned integer! 32 2]
  == make vector! [unsigned integer! 32 2 [0 0]]
  
  >> v/size
  == 32
  
  >> size? v
  == 32
  
  >> reflect v 'size
  == 32
  
  >> query/mode v 'size
  == 32
  
  >> v/type
  == integer!
  
  >> query v
  == make object! [
      signed: false
      type: 'integer!
      size: 32
      length: 2
  ]
  
  >> query/mode v [size: type:]
  == [
      size: 32
      type: integer!
  ]
  
  >> query/mode v [size type signed]
  == [32 integer! false]
  
  >> spec-of v
  == [unsigned integer! 32 2]
  
  ```
* [`d08f83`](https://github.com/Oldes/Rebol3/commit/d08f835b0a760e0b66f1da7f246ed05de8b9b827) Using serialized form of vector value when displayed as a object's field in console's help
* [`ac8e31`](https://github.com/Oldes/Rebol3/commit/ac8e3166da09b22000df69a0e94c082f35fe0eba) CHANGE: possible to initialize VECTOR using raw binary data

  Now it is possible to do:
  ```rebol
  >> make vector! [integer! 16 #{0100 0200}]
  == make vector! [integer! 16 2 [1 2]]
  
  >> v: make vector! [decimal! 32 #{0000803F 000080BF}]
  == make vector! [decimal! 32 2 [1.0 -1.0]]
  
  ```
  Binary data must be in little-endian.
  
  This commit also enables limiting the input using the specified size.
  ```rebol
  >> make vector! [integer! 16 1 #{01000200}]
  == make vector! [integer! 16 1 [1]]
  
  >> make vector! [integer! 16 1 [1 2]]
  == make vector! [integer! 16 1 [1]]
  
  ```
  Originally the size was extended instead and so behaving like when there was no size specified.
    
* [`4dd601`](https://github.com/Oldes/Rebol3/commit/4dd60134fa706aacdd091dcb6d955e615b36c79b) Conversion from VECTOR to BINARY

  It provides copy of raw internal vector data (in little-endian).
* [`b71b42`](https://github.com/Oldes/Rebol3/commit/b71b4201d7138b1b400e9335a62df77412e01d1a) VECTOR can be initialized using a block with CHARs
* [`661ee3`](https://github.com/Oldes/Rebol3/commit/661ee3851e94b1aedd4db8444de487139d852ee4) Added support to encode/decode images as uncompressed BMP with alpha channel (32bit)
* [`30f9d8`](https://github.com/Oldes/Rebol3/commit/30f9d8faa14e45ba0a6e959bf63037661cabd131) New natives rgb-to-hsv and hsv-to-rgb

  These natives were in Rebol2 and so why not to have them in Rebol3 again.
  Could be enhanced in future for batch conversion and or more precise version not depending on tuple value for HSV.
* [`6ea080`](https://github.com/Oldes/Rebol3/commit/6ea0800ae0e344ca536982a424b7cdaee4a38066) Improved terminal support on posix platforms

  Now unicode input is (somehow) supported and there are new key shortcuts for clearing line (CTRL-U and CTRL-K) and navigation skipping by words (OPTION key with left or right arrow)
  
  Known issues:
  When TAB is entered, the line navigation using left/right arrows and char deletion is wrong. Any idea how to fix it without disabling TAB-ed input?
  
  Note:
  Few lines of code were taken from Brian Dickens' Ren-C

### Fixes:

* [`15ce19`](https://github.com/Oldes/Rebol3/commit/15ce19d91284198c7750c89edeb695d3e60e10b6) Temporary fix in host-window using REBXYF instead of REBPAR
* [`9a2204`](https://github.com/Oldes/Rebol3/commit/9a220409d074d089c21c8146d1de5dcca0a965c0) Crop output of block value in object field and better handling unset values
* [`7c8443`](https://github.com/Oldes/Rebol3/commit/7c8443f6061c6df35d9b407f00c63203c6913903) Increase the size of even queue as needed until 64K
* [`5493bd`](https://github.com/Oldes/Rebol3/commit/5493bd6391a9c44c4b6001683eff3b8bcfe45c4d) Add functions to update an event
* [`fc3206`](https://github.com/Oldes/Rebol3/commit/fc32069cdfeca21a87bcbbd40ccf216a3d9d2d7a) ATRONIX: Add missing va_end
* [`5be3b4`](https://github.com/Oldes/Rebol3/commit/5be3b435803749f2c5b05372f698cf1af3634f60) ATRONIX: Make sure string is null terminated
* [`3725a3`](https://github.com/Oldes/Rebol3/commit/3725a33ba087b9608eb80f7e59e3edbfb0f53478) ATRONIX: Memory from OS_Get_Locale should be free according to its doc
* [`c20872`](https://github.com/Oldes/Rebol3/commit/c20872eecce1e0979639b6b471b41146d7c7d518) TYPO b-init.c: actions -> natives
* [`067744`](https://github.com/Oldes/Rebol3/commit/067744f6e9c7f2a3ac395f4333993b5fc50c9922) Don't initialize Windows subsystem yet
* [`9d2a67`](https://github.com/Oldes/Rebol3/commit/9d2a67b8bd5a52350f9cb80f6aff4779378d3897) Setting correct color after CTRL-C on Windows
* [`900e1d`](https://github.com/Oldes/Rebol3/commit/900e1dc894b85cc7020f828108ce11c4ce50e07f) Better handling of CTRL-C in Windows console
* [`07e759`](https://github.com/Oldes/Rebol3/commit/07e759016b8cf6351fbcea21b47ba6035d3b44b3) Better handling of CTRL-C in Windows console
* [`ac07bd`](https://github.com/Oldes/Rebol3/commit/ac07bde81a67e0b69eb677a6295e5f3868a41ca5) Support async read from clipboard
* [`414863`](https://github.com/Oldes/Rebol3/commit/4148634810842603f8690e8b4d84a9ee98f36522) Make writing and reading clipboard symmetric
* [`3c7e7a`](https://github.com/Oldes/Rebol3/commit/3c7e7a8dec6598c047e3c40b4631fdb66b3b25d1) Fixing math operation on VECTOR which is not at its head position
* [`efef62`](https://github.com/Oldes/Rebol3/commit/efef62a2c1cf6b09b5bf14ff2effa1dcbf9823c0) Added missing SPEC reflector types
* [`cbc67c`](https://github.com/Oldes/Rebol3/commit/cbc67ca329dfc473e81aa4897f198c44aaaf920f) Words-of map normalizes KEY words to be always ordinary word
* [`c600d8`](https://github.com/Oldes/Rebol3/commit/c600d8a84e2d0d1b9bf5a48a3c5bf6553ec27a30) Key words of MAP normalized to SET-WORDS
* [`5973f4`](https://github.com/Oldes/Rebol3/commit/5973f48220caf732e17a9e00bdcef7c274f8b7b9) Added missing reflector's types
* [`3893ae`](https://github.com/Oldes/Rebol3/commit/3893ae8de2831e0065f64f5e4769809fcf1a62b6) Removing forgotten debug line
* [`770092`](https://github.com/Oldes/Rebol3/commit/7700920d91b8547bfcf891a17f785d53f653d0ca) Forcing DELINE to convert CRLF to LF in header file emitter scripts to be compatible with previous changes where READ does not do that now.
* [`b017f8`](https://github.com/Oldes/Rebol3/commit/b017f81ecc71f00274809d397ce86ad999d02ccd) Correctly handling file argument of BROWSE function on Windows
* [`22bd6a`](https://github.com/Oldes/Rebol3/commit/22bd6ad68df9c75e45b43dff1bcc8819d74c92ce) TO-LOCAL-FILE/FULL adding additional SLASH when used in root directory
* [`e313b9`](https://github.com/Oldes/Rebol3/commit/e313b9ef961b03e7cd84299c743c4f579d998a0e) Not using fixed values in vector indentation
* [`79724f`](https://github.com/Oldes/Rebol3/commit/79724fc1f69d429b7379ab58f628fa51699d284c) MOLD/FLAT has no effect when used on VECTOR
* [`d1c1e8`](https://github.com/Oldes/Rebol3/commit/d1c1e80a4ec4e495e39725deba61284d5cfe48d5) Crash when converting empty vector to block
* [`8c18f8`](https://github.com/Oldes/Rebol3/commit/8c18f8980d6773dbc1c8d7bb50553f39f08f07e0) Better cleanup code in image unit test file
* [`ecab38`](https://github.com/Oldes/Rebol3/commit/ecab3800638a84d56dbcdc9358e502a6d4e39f62) ATRONIX: Fix a size limit error in decoding top-down BMPs
* [`b1bc3c`](https://github.com/Oldes/Rebol3/commit/b1bc3c0bc55b24590da5824709cebe66262c2d69) ATRONIX: inverting alpha channel values and default white color
* [`365be4`](https://github.com/Oldes/Rebol3/commit/365be4af5cb5ee5ad41277989b9575a360b9a1fa) FIND/TAIL not working properly with CHAR as an argument to search
* [`5eb7fb`](https://github.com/Oldes/Rebol3/commit/5eb7fb2ef0d1c9307410f750e4d21cbe211a1c2a) Remove CRLF on Windows and LF on Posix platforms from tail of INPUT line
* [`9d3d85`](https://github.com/Oldes/Rebol3/commit/9d3d85be70ee6c22aa82b7cce40083c0b2e35ee5) Removing trailing CRLF from formed error message on Windows
* [`911097`](https://github.com/Oldes/Rebol3/commit/9110974bfadd79d587a5d7c4ab366caf46861527) Using same return values from OS_Browse on Windows like on Posix
* [`c4e2b7`](https://github.com/Oldes/Rebol3/commit/c4e2b7dd252329f3d1d6abdd3809f9455f6fec1f) Removing forgotten debug trace line
* [`501a39`](https://github.com/Oldes/Rebol3/commit/501a3924a67b25cc839927ecb8c317f1111ad59e) Resolving last error id on posix platforms, if provided value to Make_OS-Error is zero (on Windows there is already used GetLastError in such a situation)
* [`f249af`](https://github.com/Oldes/Rebol3/commit/f249af5c14b0a2e2d40dda72a08b1509826967aa) Preferring (void) instead of () for functions without any arguments
* [`75c3c6`](https://github.com/Oldes/Rebol3/commit/75c3c69d121ce14f2b66b5139bfa8425d4bf05b4) Using LZMA unit tests only if LZMA is available in the build
* [`7f5355`](https://github.com/Oldes/Rebol3/commit/7f5355e527f1e23b7e8aad555466870a2002cb80) Moved setting system/options/home directory shortcut (~) to final phase
* [`d273f4`](https://github.com/Oldes/Rebol3/commit/d273f4a3d303ee8d2007911086f32b9378b87df3) Fixing more compiler warnings

### Changes:

* [`f59f31`](https://github.com/Oldes/Rebol3/commit/f59f31e9f37ef9b81919704e02e26abe223d48e6) Moved some of VECTOR related definitions to sys-value.h so they can be shared with extensions.
* [`da8353`](https://github.com/Oldes/Rebol3/commit/da8353a165fca9a7fd3e7f708b56d7e23bb9ac62) Reorganizing system value definitions so they may be shared when making extensions

## 2018 December

### Features:

* [`af0eb9`](https://github.com/Oldes/Rebol3/commit/af0eb92688ef5a4f1186a581383dc1b4f5e5d6cc) WRITE action if used on FILE is returning the same file name instead of (closed) port!

  It enables chaining actions… for example:
  ```rebol
  >> query write %xxx "a"
  == make object! [
      name: %/X/GIT/Rebol/release-x64/xxx
      size: 1
      date: 19-Dec-2018/0:17:51.338+1:00
      type: 'file
  ]
  ```
    

  Note: `save` function is now also returning file!
* [`26b2f5`](https://github.com/Oldes/Rebol3/commit/26b2f5671c2afefc426454ccf920144665003d67) Not throwing error when QUERY on INPUT port is run in a script not running in terminal (Travis)
* [`32732d`](https://github.com/Oldes/Rebol3/commit/32732d4f76797a8291c1ebbb18166e97184ac7c1) Throwing error if QUERY on INPUT port fails from some reason
* [`542d8d`](https://github.com/Oldes/Rebol3/commit/542d8dd6c2fc07f91769b8ae0ebc6b9f263100db) Letting MOLD/ALL on datetime value to produce ISO8601 valid result

  https://github.com/Oldes/Rebol-issues/issues/438
  https://github.com/Oldes/Rebol-issues/issues/2089
  https://github.com/Oldes/Rebol-issues/issues/2092
* [`1ebbbc`](https://github.com/Oldes/Rebol3/commit/1ebbbc3d4607b84a004df5de330b0e5d1120c777) Enabling `T` as a delimiter between date and time and `Z` as a zero timezone, so subset of ISO8601 datetime values are loadable from Rebol.

  Example:
  ```rebol
  >> 2013-11-08T17:01
  == 8-Nov-2013/17:01
  >> 2013-11-08T17:01+0100
  == 8-Nov-2013/17:01+1:00
  >> 2013-11-08T17:01Z
  == 8-Nov-2013/17:01
  ```
* [`035df2`](https://github.com/Oldes/Rebol3/commit/035df2a2f72578c514b26ebd7843d4cc7cc51a1d) New BINCODE's read functions: SB (signed bits) and ALIGN (aligns bit stream to byte)
* [`4f6c2e`](https://github.com/Oldes/Rebol3/commit/4f6c2eb6bb056dc6286ec7e60addf5736429ccb5) Exported new library functions for better UTF8 text conversions from Rebol extension code

  New functions:
  ```C  
  RL_Get_UTF8_String
  RL_Encode_UTF8
  RL_Encode_UTF8_String
  RL_Decode_UTF_String
  ```
* [`823158`](https://github.com/Oldes/Rebol3/commit/823158057ab8c3f786523e3c05956988771d5917) Introduced new `command` error
* [`dbd6f6`](https://github.com/Oldes/Rebol3/commit/dbd6f6513c0da088c96a5ce6068f93607d974623) Improved support for dealing with HANDLE types in extensions

  Originally, HANDLE was just pointer, but I enhanced it recently to also being able define its type, so one can on native side test, type of provided handle and not just blindly throw some pointers as an arguments. In this commit it is reflected also when dealing with native extensions.
* [`71442f`](https://github.com/Oldes/Rebol3/commit/71442f06b678b6ee05c9f4cbfa02aae6e5692a92) Implemented block argument for QUERY/MODE for FILE, DIR, INPUT (console) and HTTP ports
* [`ebb25c`](https://github.com/Oldes/Rebol3/commit/ebb25c7360caf310059d901f05b535355880ab67) Enhanced descriptions for P64 and L64 compile-flags
* [`c02753`](https://github.com/Oldes/Rebol3/commit/c02753a66a82039dd4d36f2100c0fc15492b0f66) Implemented QUERY/MODE for FILE, DIR, INPUT (console) and HTTP ports
* [`25255d`](https://github.com/Oldes/Rebol3/commit/25255d1fb21e021a1d92e7a3cc14eb5f53bbb442) Enable WITH function to accept also MODULE! as a context
* [`ca7bb3`](https://github.com/Oldes/Rebol3/commit/ca7bb39ebb49437683710d5469072ca17f8765ec) On Windows emulating ANSI escape codes for hiding cursor in console

  ```rebol
  print "^[[?25l" ;<-- hides cursor
  print "^[[?25h" ;<-- restores cursor visibility
  ```
  
  Here is how it is done in other languages (mostly using ANSI too):
  https://rosettacode.org/wiki/Terminal_control/Hiding_the_cursor
  
  Note: this only hides the cursor.. the user input is still visible.
  To hide user input one can use:
  ```rebol
  print "^[[8m"  ;<-- hides user input
  print "^[[28m" ;<-- restores the input visibility
  ```
  Or just using: `ask/hide`
  Related commit: ad681030ac488a146cca3c57f41136f99725f27d
* [`ad6810`](https://github.com/Oldes/Rebol3/commit/ad681030ac488a146cca3c57f41136f99725f27d) Implemented ASK/HIDE (which prevents echoing user input)

  It is implemented using standard ANSI escape sequences CONCEAL/REVEAL, which are now emulated on Windows using `SetConsoleMode`.

### Fixes:

* [`ff5175`](https://github.com/Oldes/Rebol3/commit/ff5175328d51a8d1977208152412563b9518e626) Fixing some "This function declaration is not a prototype" warnings
* [`e652dc`](https://github.com/Oldes/Rebol3/commit/e652dc9099959b3b524991d72f447f6c2057f88b) Not using Dev_Checksum on host side as it is handled only in core.
* [`a87941`](https://github.com/Oldes/Rebol3/commit/a8794126acfdcedcac79958a0d24002898e4ed5d) Using isfinite instead of finite function on macOS as finite is reported to be deprecated
* [`548f5f`](https://github.com/Oldes/Rebol3/commit/548f5f135351138e6e59653216ee0c474450b38f) Fixing some unused variables and variables never read (reported by xcode's analyzer)
* [`d111b5`](https://github.com/Oldes/Rebol3/commit/d111b5367f9004be7f190bef61fbb1b8daab38b3) Adding missing last argument (recently added) in one location used on POSIX
* [`7eb820`](https://github.com/Oldes/Rebol3/commit/7eb8209a0d1a0c3846b670ea0d3651d12476a329) WRITE/LINES of STRING! doesn't enforce terminal newline
* [`f9a10b`](https://github.com/Oldes/Rebol3/commit/f9a10bf817070c4e6418c9c7ba4f21fe1bab34da) Fine tunning CRLF/CR conversion to LF - now the conversion is used with READ/STRING,
* [`d09fff`](https://github.com/Oldes/Rebol3/commit/d09fffa924bd0674824894c66e37942e3426fb7a) Turned off CRLF/CR conversion to LF when converting binary to string
* [`9190f1`](https://github.com/Oldes/Rebol3/commit/9190f1ca96214e530f0d1c22fa390d08a78fb746) READ/string crashes for UCS4 ("UTF-32") LE/BE files with a BOM
* [`c82176`](https://github.com/Oldes/Rebol3/commit/c821763de49b7f9fc676ec13cbf7b5a80118eb27) When argument for ?? function was not word or block, it was throwing an error
* [`6749aa`](https://github.com/Oldes/Rebol3/commit/6749aa1b02a81814cba1e3c18e639a176389c5b6) Commented out unused (as probably unfinished) Encode_String function
* [`3e0312`](https://github.com/Oldes/Rebol3/commit/3e03129f9da6d6f70dffda56e5d497136da065cf) Fix of the previous fix..
* [`6267d9`](https://github.com/Oldes/Rebol3/commit/6267d9a4c3588917e6e2b1173cf1ececbc1bbb6a) Letting FORALL to reset position, when make it through normally, but not on BREAK/THROW/ERROR
* [`0612b7`](https://github.com/Oldes/Rebol3/commit/0612b72afbbc6ec48d3af6229a95f5afd1049902) Making FORALL behavior compatible with Rebol2 and Red
* [`a0f697`](https://github.com/Oldes/Rebol3/commit/a0f697ce72c0873292de5759692e54719382092c) REWORD with multi-character delimiter bug
* [`1beb0d`](https://github.com/Oldes/Rebol3/commit/1beb0db1c2093305b1d84d96d6c923ad9eb1924c) Not using temp variable in ASK function

## 2018 November

### Features:

* [`998952`](https://github.com/Oldes/Rebol3/commit/9989527c899df8ec3415dbe61288356ce507c9dc) Pass indexes to ARRAY/initial function

  When calling ARRAY/initial with a function used to generate the values,
  pass the current index at the position the function is being called for,
  with each index in a block of sizes provided to create a multidimensional
  array being provided as a separate parameter to the function in the order
  specified in the sizes block. Uses APPLY so the function can optionally
  not take the parameters. Uses an internal function option for efficiency.
  
  See https://github.com/Oldes/Rebol-issues/issues/2193 for details.
* [`e977b2`](https://github.com/Oldes/Rebol3/commit/e977b2e591049d61860dba08dcd9bb38423e1df7) Re-factored reb-config.h a little bit by moving __LP64__ definitions into system.r script and reverting definition of HAS_LL_CONSTS (now enabled by default and undefined where needed (old MSVC compiler only?)
* [`38f5ab`](https://github.com/Oldes/Rebol3/commit/38f5ab3c054372d6262c49a08fee42527ffda77c) Allow one RETURN: […] specification in function's spec-block, so one could use (some) Red functions without code modifications

  So far it is ignored during function evaluation, but when used, it is visible as a new section in function's help.
* [`ed7a99`](https://github.com/Oldes/Rebol3/commit/ed7a99d5199c9770e09ae747490ad592e9d1ee79) Indent function's help from following prompt line
* [`1eb672`](https://github.com/Oldes/Rebol3/commit/1eb672a5b5f066d6bb9015a3fd60288dd47654ab) Support for wildcards in string searches (refinements /ANY and /WITH in FIND and SELECT)
* [`9daa54`](https://github.com/Oldes/Rebol3/commit/9daa54593a71d72c62c72b4e54bda188d5f6dfe4) Modernized HELP (now as a module)
* [`93aa81`](https://github.com/Oldes/Rebol3/commit/93aa81b27c1eeca6a7cfc4b6e8046b96a8c1076d) Using more robust posix implementation of the QUERY on console port
* [`b1f770`](https://github.com/Oldes/Rebol3/commit/b1f770a80edb326840ed4dbdd5d456bbacc11278) Added unit test for QUERY on INPUT port
* [`e33bb6`](https://github.com/Oldes/Rebol3/commit/e33bb61ad7f4101d153fd9c3ef4eb3c608fe77aa) Implemented QUERY action on INPUT port to get information about console's size
* [`fba3b1`](https://github.com/Oldes/Rebol3/commit/fba3b101081642cef02a6282bf8fa031a47b9a2b) Added colored function names in the WHAT function output and added max and min function name indentation
* [`02a397`](https://github.com/Oldes/Rebol3/commit/02a397926ee999c6c3099b554cba13682c806256) A few optimizations after quick review of the original parse-xml code
* [`6ff6d7`](https://github.com/Oldes/Rebol3/commit/6ff6d7d9ac9588475c4cddc4b61f29723f0f45b3) Using Gavin F. MacKenzie's parse-xml+ script as a XML codec

  It's based on script downloaded from http://www.rebol.org/view-script.r?script=xml-parse.r
  I was using this script for years in my Rebol2 environment, so I think I can include it in R3 too.. so far I never had time to review it, but I believe, it could be improved and also there are some TODO notes inside the original script.
* [`a9597c`](https://github.com/Oldes/Rebol3/commit/a9597ce41bef60ce64ffeaad12a78e0dfb02c919) Added infix shift operators: <<  and >>
* [`1c5924`](https://github.com/Oldes/Rebol3/commit/1c5924ffd50ecc00010882e1df359aea48540b1f) Added "bincode" tests for recently added key-words (STRING and UNIXTIME-NOW-LE)
* [`140a5d`](https://github.com/Oldes/Rebol3/commit/140a5d7d777e5dcfbc74107fa1f8cabc0502357e) Added delta time counter for unit tests
* [`096f25`](https://github.com/Oldes/Rebol3/commit/096f254c230ad41feabac666ba038b1c6b25bdcc) Simple GZIP codec
* [`849b0a`](https://github.com/Oldes/Rebol3/commit/849b0aa0c46c8485faeecda037766dddce7d2657) Bincode: added STRING reading and UNIXTIME-NOW-LE writing key-words
* [`3a2262`](https://github.com/Oldes/Rebol3/commit/3a22625355fb3357e95c18c07d1a156046ed2e40) Updated ZLIB to version 1.2.11. Enhanced COMPRESS and DECOMPRESS natives.

### Fixes:

* [`6531fb`](https://github.com/Oldes/Rebol3/commit/6531fbbe1ce3172051415272e294a083812ade34) ATRONIX: Close the socket before raising an error
* [`4918d8`](https://github.com/Oldes/Rebol3/commit/4918d8a1e9e9468b5580158ba7359a7f6376dba8) ATRONIX: Always send a Error event in case of error
* [`16ae58`](https://github.com/Oldes/Rebol3/commit/16ae5889948c59ae1f4f7ecd9d8729a714aa2937) ATRONIX: Prevent SIGPIPE when writing
* [`c905a8`](https://github.com/Oldes/Rebol3/commit/c905a894532e3c243d8ce53181ae5c5793f30989) Check return value from fcntl
* [`b36f2e`](https://github.com/Oldes/Rebol3/commit/b36f2ea8ea96f703b195acb297b7eb2d3a3dca58) Added missing line break in HELP on objects output
* [`44ad70`](https://github.com/Oldes/Rebol3/commit/44ad7012ef795e241c9b1428618d1c530af1b50c) Providing DUMP-OBJ function again as it is used not just in the help's context
* [`334a0b`](https://github.com/Oldes/Rebol3/commit/334a0b2bf87e055563b42249f2a7ee7582c8caf0) Missing #endif in the previous commit
* [`d443cb`](https://github.com/Oldes/Rebol3/commit/d443cb1ecee178eea0538029e6efbb06f380546c) Added missing X (columns) information of the console's `window-size on POSIX
* [`b0e31e`](https://github.com/Oldes/Rebol3/commit/b0e31e9fecddc2ab3ad2be057fae61f32e6cc216) Adding missing integer unit test file
* [`d4e023`](https://github.com/Oldes/Rebol3/commit/d4e023c5fa8e7ce3fc1f9ed06975975eca157b42) Skip system dependent header part in GZIP test
* [`57cc08`](https://github.com/Oldes/Rebol3/commit/57cc0857441bace073655aa723eed0c468d16704) Include UNUSED(x) macro
* [`458e0d`](https://github.com/Oldes/Rebol3/commit/458e0d664a7e40f197389c95edd6d2b3a6387494) Enable LZMA compression in Visual Studio make script
* [`00b1c9`](https://github.com/Oldes/Rebol3/commit/00b1c9aed29747669616ded266c1507c756aa08a) Trying to fix some of lzma related issues reported by Travis
* [`902973`](https://github.com/Oldes/Rebol3/commit/90297372773e7bfca128bd8f2ebc26c668e3ff6b) Trying to fix some of lzma related issues reported by Travis

## 2018 October

### Features:

* [`e46eff`](https://github.com/Oldes/Rebol3/commit/e46effee5ec8e029918890f84f39a4d6e9763712) Initial support for (optional) LZMA compression in COMPRESS and DECOMPRESS functions

  So far it is implemented like the original COMPRESS, which is appending size of uncompressed data at tail of compressed data.
  
  To include the LZMA, there must be used USE_LZMA compiler's definition while compiling Rebol.
  
  LZMA source is from LZMA SDK version 18.05 from https://www.7-zip.org/sdk.html slightly modified for use with Rebol source.
* [`5ec778`](https://github.com/Oldes/Rebol3/commit/5ec778ee15c88053a0d6425951099e4a5f4c3f39) Experimentally trying to protect system/standard object from modification
* [`30e89a`](https://github.com/Oldes/Rebol3/commit/30e89acd0bb4c102be0883c813196a4e8e71d779) Rewritten TLS protocol to support TLSv1.2 (may be still unstable)

  It's possible to turn on various traces using:
  
  ```rebol
  system/schemes/tls/set-verbose 3
  ```
* [`25bbca`](https://github.com/Oldes/Rebol3/commit/25bbca8b7552bbf286a08695d414eae4062747cb) Initial version of CRT codec

  Converts certificate raw data into readable tree structure. It is most probably not complete. But is usable in basic form for retrieving RSA key from a certificate.
* [`26dd2c`](https://github.com/Oldes/Rebol3/commit/26dd2cda62be98e5f422f1736971fc485e358c29) Allowing ANY-STRING! for start-* functions in quick-test-module

  (so one can use for example URL as a test title)
* [`2b9b90`](https://github.com/Oldes/Rebol3/commit/2b9b90dd27bf80bb0d7ba2ec4495242668390b6f) Revisited REB_HANDLE implementation and usage

  Initially the HANDLE value was just holding a pointer to function. But I think it is also useful to have HANDLE hold a series value, which can hold any internal data. In most cases GC can free this series once handle is not referenced. There will be also needed some sort of callback to dispose function for some more specific HANDLEs in future. That is not yet implemented.
  
  I also reviewed a little bit the CRYPT natives as HANDLEs are used in these functions to hold context. What I can say so far, HANDLE was also used as a CODEC entry (where I added test for the proper handle type) and extension dispatcher.
* [`3d347f`](https://github.com/Oldes/Rebol3/commit/3d347f1ee85053fec04f27d0417443a2ab7e91f7) Initial version of DER (Distinguished Encoding Rules) decoder (codec)
* [`3bf6c7`](https://github.com/Oldes/Rebol3/commit/3bf6c7c8e79e181a28ce1029f06ce1462a54be26) Basic WAV codec (decoder) written directly in Rebol
* [`69015f`](https://github.com/Oldes/Rebol3/commit/69015fea0a45389298795fe22a03d5c780939ce7) Forcing recycle at end of tests and showing memory stats
* [`3e9a71`](https://github.com/Oldes/Rebol3/commit/3e9a71d4ec8d631d34a9b942101bbd9b375b3b98) Added counter of asserts in current test (so failed asserts are easier to find)
* [`cec598`](https://github.com/Oldes/Rebol3/commit/cec598ba44dfd6fd78b38dea80b207c66d65b7a3) Initial version of BinCode => Binary DSL
* [`850d69`](https://github.com/Oldes/Rebol3/commit/850d697be3db394198a5f11c0cf6f4544fb95170) Improved FORMAT dialect so one can specify escape color codes (using money! value)

  Simple usage example:
  
  ```rebol
  print format [$31.47 "some text" $0] ""
  ```
  
  would print red text on gray background (depending on color palette of the terminal)
  
  The string returned by format function would look like: `"^[[32;47msome text^[[0m"`
* [`23f22a`](https://github.com/Oldes/Rebol3/commit/23f22a2c08a4ead607fa60f0928359e5adaf91bc) Added a few test vectors for AES128 and AES256 cipher suite
* [`902547`](https://github.com/Oldes/Rebol3/commit/902547531daef1ff50c0b7ec7d15753e71c8bb25) Added recommendation note not to use RC4 as it is prohibited by https://tools.ietf.org/html/rfc7465
* [`cf740c`](https://github.com/Oldes/Rebol3/commit/cf740c4cd0670271ca7265e4c418890421059e1f) Making UNSET! as a conditionally "TRUTHY"

  It reverts changes discussed in above issues as I consider this behavior to be more useful and it is compatible with Rebol2 and Red.
  
  My main reason is to be able use PRINT inside ALL blocks without need to worry if these would affect the evaluation.
  
  Like in this case:
  ```rebol
  	if all [
  		not headers
  		any [
  			all [
  				d1: find conn/data crlfbin
  				d2: find/tail d1 crlf2bin
  				print "server using standard content separator of #{0D0A0D0A}"
  			]
  			all [
  				d1: find conn/data #{0A}
  				d2: find/tail d1 #{0A0A}
  				print "server using malformed line separator of #{0A0A}"
  			]
  		]
  	][ … ]
  
  ```
* [`c5cfb6`](https://github.com/Oldes/Rebol3/commit/c5cfb69cdda891118e2027fefcb76855e7fcff4c) Adding function ENUM for making standard enumeration objects from given specification

  See the included test file for usage examples.
* [`7c07c7`](https://github.com/Oldes/Rebol3/commit/7c07c7b1bea2d9eb4c24a09e6bc664eb82b1ee8b) Including WITH control function

  (evaluates a given block binded to specified context)
* [`7c2341`](https://github.com/Oldes/Rebol3/commit/7c23410230829b61b9b01c16b67dfd99c615a704) Small change in output of ?? debug function

  (so it is easier to copy the output for immediate use in console again)

### Fixes:

* [`116bb8`](https://github.com/Oldes/Rebol3/commit/116bb867554a352337c23d6181095642906c53f6) DELINE / ENLINE does not check series' protection
* [`dc2795`](https://github.com/Oldes/Rebol3/commit/dc27950a48bffb9d8ac1a12af74286ca06416e1c) Using <sys/malloc.h> instead of <malloc.h> when compiling to 64bit macOS version
* [`8efcf4`](https://github.com/Oldes/Rebol3/commit/8efcf4153cc7d034080e31b15c5500a3e5732662) Using new handle for extensions with additional check for handle type when importing extension commands.
* [`c86393`](https://github.com/Oldes/Rebol3/commit/c86393143a7df4c6c6ea476561f9d2e11e85b71d) Don't try to convert unknown OID to word in CRT codec
* [`65b664`](https://github.com/Oldes/Rebol3/commit/65b664831302a4029571b5bd8179e34809c28984) Linking the new CRT codec file into mezzanines.
* [`6e4b8f`](https://github.com/Oldes/Rebol3/commit/6e4b8f9d2284117457e8a842033a9ca5279d29e7) Handle situation when internal connection of HTTP port is closed unexpectedly
* [`5819e6`](https://github.com/Oldes/Rebol3/commit/5819e6d0744daff502469bb8dc6651abfabf9fcd) Disabling verbose mode by default in DER codec
* [`dc9a65`](https://github.com/Oldes/Rebol3/commit/dc9a6521a5a66dec0f267bff078f47830ffa6ccf) Correct formating in the DER codec warning
* [`df7822`](https://github.com/Oldes/Rebol3/commit/df782250e273e9726f8b2bf6861f80742ecfaf34) ROUND/EVEN/TO differences from Rebol2
* [`88a9d1`](https://github.com/Oldes/Rebol3/commit/88a9d147741c677c75cd159bc65dcb84ff1a1d5b) FIND/PART not working after skipping the series
* [`77e201`](https://github.com/Oldes/Rebol3/commit/77e201442d8aa8b8e97ea7a739b848109d9c1a65) Adding missing RC4 test file
* [`f3fb0a`](https://github.com/Oldes/Rebol3/commit/f3fb0a915c5f72c0e4b00087892b3aa41571c511) DER codec errors in some cases in verbose mode
* [`2bb8c4`](https://github.com/Oldes/Rebol3/commit/2bb8c43c320c9a82b008861f7fe499c2567db791) Including DER codec as a mezzanine
* [`dac96b`](https://github.com/Oldes/Rebol3/commit/dac96b2141043344614eece2adf65137e97e27cd) Few additional castings to prevent compilation warnings
* [`253769`](https://github.com/Oldes/Rebol3/commit/253769f0794e5d72f15155c5bf4958b7f905ecb3) Disabling the original WAV codec written in C and enabling the new one written in Rebol directly
* [`4a2ee0`](https://github.com/Oldes/Rebol3/commit/4a2ee0fb7095d500f0c79d7166f83ead66986e4e) More fixes trying to lower number of Clang warnings
* [`f3d35c`](https://github.com/Oldes/Rebol3/commit/f3d35c5a28c594f74b9b1378ed7526578a6db9e6) Fix the comparison between unsigned and negative
* [`f67b8a`](https://github.com/Oldes/Rebol3/commit/f67b8a8594e5bde06bc9b3bb823eee58465b904f) More fixes trying to lower number of Clang warnings
* [`7aa784`](https://github.com/Oldes/Rebol3/commit/7aa78405e1fc05eacdc029527f680464b4e549c2) Warning: comparison of unsigned expression >= 0 is always true
* [`ef2c89`](https://github.com/Oldes/Rebol3/commit/ef2c892144dbf9166b5efdc50d3da3df6433655d) More fixes trying to lower number of Clang warnings
* [`d3a9cd`](https://github.com/Oldes/Rebol3/commit/d3a9cdf2f492ff80006684446105693a54b1e5d6) Trying to get rid of pointer-sign warnings when using Clang
* [`664a01`](https://github.com/Oldes/Rebol3/commit/664a0118009ba9cc184beb2ece3a1e14d43e7e79) Add some asserts to shut off coverity
* [`9448d9`](https://github.com/Oldes/Rebol3/commit/9448d9619f1deef66714413768f38073249ddc1b) Integer overflow when making TIME
* [`bc24f6`](https://github.com/Oldes/Rebol3/commit/bc24f6d2d2c55ba158f1f77d78297ea34689d1a1) Getting rid of "shifting a negative signed value is undefined" warnings using Clang
* [`ab75f8`](https://github.com/Oldes/Rebol3/commit/ab75f8f4c9822e9db80e16e6cedbcd37cdcbda13) Fixes based on some warnings from Clang
* [`46398c`](https://github.com/Oldes/Rebol3/commit/46398cb3cf8ab1da9bad5072a1e727d83ccae35b) Removed initial declaration inside `for` loop
* [`7b7166`](https://github.com/Oldes/Rebol3/commit/7b7166713b26bef0f0845f515b4047c416baffa6) Invalid value pointer for Expand_series in checksum's READ action
* [`aa183b`](https://github.com/Oldes/Rebol3/commit/aa183b9ccb20017f9abb9e6c727ca54c120944ff) RANDOM can alter the sequence of series after PROTECT
* [`e880ce`](https://github.com/Oldes/Rebol3/commit/e880ce423ef796fc9027de4e1bbe984aaa8ab04b) RANDOM can alter the sequence of series after PROTECT
* [`4ff49a`](https://github.com/Oldes/Rebol3/commit/4ff49a8d9d86ebac982fd04aafccf91ec70f4be2) Removing ANSI escape char from White_Chars table
* [`470761`](https://github.com/Oldes/Rebol3/commit/47076113417331780c9634b330fd57507abbbe7a) Using reversed order of codecs in ENCODING? function, so user defined codecs has higher chance to be recognized
* [`1e36d1`](https://github.com/Oldes/Rebol3/commit/1e36d12ac7b7d3c2f4a3405b7bc739f8ab996489) Revisited color attributes in ANSI escape codes on Windows so it is more consistent with POSIX terminals now.
* [`2f8ae5`](https://github.com/Oldes/Rebol3/commit/2f8ae52f7ccaa9b9a452c299c90645fe7344b6e9) Correcting typos in headers of some test files
* [`f21d51`](https://github.com/Oldes/Rebol3/commit/f21d5164e01d5713270d42c3c46a1d5ed19cb0c0) ENCLOAK and DECLOAK don't honor PROTECT status
* [`badbba`](https://github.com/Oldes/Rebol3/commit/badbbac40173f632caa8ab08918d144d73e77e7f) Fixed crash with CHECKSUM/key with /method using SHA384 and SHA512
* [`e2d866`](https://github.com/Oldes/Rebol3/commit/e2d866ba510f8fe15c5b3568e92aeee25868871c) Keeping spec/ref value in CHECKSUM scheme specification as some HELP functions are looking for it.

## 2018 September

### Features:

* [`fb113c`](https://github.com/Oldes/Rebol3/commit/fb113c63760ce9a9606e45988d574f5265e65d68) Added support for SHA384 and SHA512 checksums

  Replaced Brad Conte's SHA256 only implementation with Aaron D. Gifford's version which supports the new methods too.
* [`5149e4`](https://github.com/Oldes/Rebol3/commit/5149e475698cecfd21e2f7db4f6698938c527f70) Added functional /PART and /SEEK refinements for WRITE action on CHECKSUM port

  Possible usage:
  ```rebol
  data: #{0bad}
  port: open checksum://
  sum1: read write port bin
  sum2: read write/part open port tail bin -2 ;port was restarted using OPEN
  sum3: read write/seek/part open port bin 2 -2
  all [sum1 = sum2 sum1 = sum3] ;== true
  ```
* [`78cc32`](https://github.com/Oldes/Rebol3/commit/78cc325c8309eefe4262284d449cece3b4980968) CHECKSUM implemented also as a scheme (so one can do incremental computation)

  Basic usage:
  ```rebol
  port: open checksum://sha256
  write port #{0bad}
  write port #{cafe}
  read port ;== #{66B0326285B97FAC1ECFC9436374B8F079ED565A18BF7368079BB39A6703B383}
  close port
  ```
* [`7d9a21`](https://github.com/Oldes/Rebol3/commit/7d9a21ded96e9e67ea2a2ade0313f1533ee4b736) Added new system function Random_Bytes, which fills destination buffer with given number of random bytes in OS independent way.
* [`263cb0`](https://github.com/Oldes/Rebol3/commit/263cb02bdedffa0a52daf9be6914bf812fad7289) Improved comment explaining the DELTA value in Expand_Series function
* [`740aae`](https://github.com/Oldes/Rebol3/commit/740aaed98ce5579d0856a3e8e3d425e6f31f3d45) Diffie-Hellman algorithm

  Using original Cameron Rich bigint implementation. Rewritten Richard Smolaks module from Atronix fork as a native functions.
  
  See DH-test.r3 file for usage example.
* [`5ce9c6`](https://github.com/Oldes/Rebol3/commit/5ce9c64d1981d1ac007288e5021fdc417e2c9a2d) DER/BER UTC-time decoder as a codec
* [`f3e243`](https://github.com/Oldes/Rebol3/commit/f3e243be4760ddb867e53ad92d2d740ee621a670) Allow block! as an argument for REGISTER-CODEC function
* [`efd3c7`](https://github.com/Oldes/Rebol3/commit/efd3c744abed7cd737c4b528d85d5d47bc278d5c) Added PKIX as a basic codec
* [`d79f41`](https://github.com/Oldes/Rebol3/commit/d79f41c53f557be9d445a6f50e45c92c4afc70f2) Added a way how to create not native codecs (keeping the original codecs infrastructure)
* [`54097f`](https://github.com/Oldes/Rebol3/commit/54097fe913c6b0c7cbb9449f7edd7631bdceed53) Added DS_PUSH_END macro which pushes REB_END value at top of stack
* [`c2dfd0`](https://github.com/Oldes/Rebol3/commit/c2dfd09ee9ec87c0c0d3d5eab1de1a2f2405dd17) Letting Travis to run the quick tests.
* [`7ea6ef`](https://github.com/Oldes/Rebol3/commit/7ea6efd69484379c8ca0fd27e8c7c15e2fdf3181) Modularized Quick-test and added a few more test cases
* [`1f6335`](https://github.com/Oldes/Rebol3/commit/1f633593ea27956b185d3b06c9918346dbbdc390) Load-PKIX function (as a part of the new mezz-crypt module)
* [`8d5bdd`](https://github.com/Oldes/Rebol3/commit/8d5bdd4c51062eb8b2f5e1539d9358d6fc888b3f) Including ENBASE/DEBASE unit tests
* [`55031b`](https://github.com/Oldes/Rebol3/commit/55031b7ec6dbefdd3da57487cb44ea886a4eab17) Initial RSA crypt integration

  The main RSA/Bigint code (Copyright (c) 2007, Cameron Rich) is taken from the latest Atronix version, but the Rebol native part was rewritten as a native while original Cyphre's version was as an extension. I also quite didn't like his RSA key as a simple Rebol object, modified the command names and removing padding setting (which was just confusing for me and source of possible problems)
  
  In this commit is also included a test file (tests/units/rsa-test.r3) as it really deserve some testing and it may be used as a hint how to use it.
  
  Know issues:
  
  - there is no way to release or re-use the RSA key handles.
  - when running from Visual Studio, the random number generator calls some assertion in cryptsp.dll, so it would be good to fix it or use own random generator.
  
  TODO:
  
  - possibility to import the key from some standard text files
  - it would be nice to have in-build RSA key generator
* [`b54e1b`](https://github.com/Oldes/Rebol3/commit/b54e1b865c9311871f10537667c6206f09a766fa) Improving DEBASE so it can decode URL safe Base64 variant input even without using a /url refinement

  Previously this would throw an error:
  
  ```rebol
  >> debase "qL8R4QIcQ_ZsRqOAbeRfcZhilN_MksRtDaErMA=="
  ** Script error: data not in correct format: "qL8R4QIcQ_ZsRqOAbeRfcZhilN_MksRtDaErMA=="
  ```
  
  The reason is that the input is using URL safe alphabet. With this commit when such a situation is detected, the decoder restarts itself automatically as if it would be used with /url refinement.
  
  NOTE: if you are sure that input should be in URL safe format, use the refinement to avoid unnecessary computations.
  
  Possible test:
  ```rebol
  key: "qL8R4QIcQ_ZsRqOAbeRfcZhilN_MksRtDaErMA=="
  equal? (debase key) (debase/url key) ;== should be TRUE
  ```
* [`e5e2a8`](https://github.com/Oldes/Rebol3/commit/e5e2a8e7b3bc8c583cb6728ce8dac5a60df06af0) Base64URL encoding/decoding in DEBASE and ENBASE functions

  improves: https://github.com/Oldes/Rebol-issues/issues/2318
  
  Possible test:
  ```rebol
  key1: "qL8R4QIcQ_ZsRqOAbeRfcZhilN_MksRtDaErMA=="
  probe bin: debase/url key1
  probe key1 = enbase/url bin ;== should be true
  
  ;debase is working also when input is missing the padding
  key2: "qL8R4QIcQ_ZsRqOAbeRfcZhilN_MksRtDaErMA"
  probe bin = debase/url key2 ;== should be true
  
  ```
  
  For more info see: https://tools.ietf.org/html/rfc4648#section-5

### Fixes:

* [`f9f2ae`](https://github.com/Oldes/Rebol3/commit/f9f2aeb843ec3533761a316382cc7e5d6d78523f) Fixes for the previous CHECKSUM port related commit
* [`e8a882`](https://github.com/Oldes/Rebol3/commit/e8a8824636775cc180c97faca606e414c77c460d) CHECKSUM/KEY argument doesn't allow binary! keys
* [`767fd0`](https://github.com/Oldes/Rebol3/commit/767fd06dabf122a4a36bec4b44cd2611233193f4) Enbase is not working correctly with series which are not at head
* [`fb7292`](https://github.com/Oldes/Rebol3/commit/fb72927d97b06228787b6a53d6b92819200e0e61) Removed declarations inside FOR loop in code from previous commit
* [`73496e`](https://github.com/Oldes/Rebol3/commit/73496e9e473d79e7242da28aa99508882ad3011b) Missing file change for commit: https://github.com/Oldes/Rebol3/commit/69edc861a9543664adf249d9ef58151791252f77
* [`3b1dc9`](https://github.com/Oldes/Rebol3/commit/3b1dc98837c09ec5d893d1e2dbb86cc625f1cd8e) Add a /dump-series refinement to STATS
* [`56a8a6`](https://github.com/Oldes/Rebol3/commit/56a8a67307b3f1d0e76d8eab6df9437273c5920a) Avoid undefined behavior caused by malloc(0)
* [`b9eadc`](https://github.com/Oldes/Rebol3/commit/b9eadc183df07115be13fdef9bc467d44925f7f1) Decrease the Series_Memory when the series is freed
* [`dae8ad`](https://github.com/Oldes/Rebol3/commit/dae8adf69cda0e5ef65918eaa692860d5fab0a00) Correct the allocated memory size in stats
* [`aa648e`](https://github.com/Oldes/Rebol3/commit/aa648e5052b1a7f9b2b44151ed87175497ffe150) SER_EXT only marks data in external memory
* [`24b748`](https://github.com/Oldes/Rebol3/commit/24b74824985227ebf58d9997e3c57e5c93e2c474) Do not crash if the series size doesn't match segment width
* [`69edc8`](https://github.com/Oldes/Rebol3/commit/69edc861a9543664adf249d9ef58151791252f77) Copy the whole union in the series
* [`289480`](https://github.com/Oldes/Rebol3/commit/28948005182581acb81fa29b42342198e1385bcb) Add a comment to eliminate warning from clang
* [`74622d`](https://github.com/Oldes/Rebol3/commit/74622dbbbf4f6cf464a7c2532ccf78787ec2c090) Fix parameter to Remove_Series
* [`e25a5d`](https://github.com/Oldes/Rebol3/commit/e25a5d9ea4e10f4321eed9163168fc369b916d22) Fix Remove_Series with big len
* [`b8e0a9`](https://github.com/Oldes/Rebol3/commit/b8e0a91e606a3cf42bb282d20a06b8e173e7caed) Fix bias overflow in Remove_Series
* [`f3f06b`](https://github.com/Oldes/Rebol3/commit/f3f06b3564b606524fd7dd8507fea8b9b5317157) Use macros for overflow detection
* [`7eca4c`](https://github.com/Oldes/Rebol3/commit/7eca4c3ae1cc755b7a18a52c145b74fcb46d69e8) Check overflow of size calculation in series expansion
* [`afb328`](https://github.com/Oldes/Rebol3/commit/afb328263290e2323bec977a7497def299ed70c0) Adding original Smolak's TLSv1.0 protocol as used in Atronix fork
* [`0c54e7`](https://github.com/Oldes/Rebol3/commit/0c54e7d99053dbb9ef9fc9e3293406e6f41e709a) Resolving some compiler's warnings
* [`b5378c`](https://github.com/Oldes/Rebol3/commit/b5378c5d28a7be7b5dc80b922aa768accb599d7e) Diffie-Hellman algorithm (missing files)
* [`2967a2`](https://github.com/Oldes/Rebol3/commit/2967a2cbc013422a279867b493519f6de66b62bf) Don't allow function! as a path argument of the CD function
* [`db842b`](https://github.com/Oldes/Rebol3/commit/db842b503136841eeda1229afae85f4381935fed) Avoid heap-buffer-overflow
* [`d85fda`](https://github.com/Oldes/Rebol3/commit/d85fda226d526448addea7e562c21be13c210095) EMPTY? is not working with directory port as it was in Rebol2
* [`1a4c3a`](https://github.com/Oldes/Rebol3/commit/1a4c3a61e62ff3aad9f6160bfa437a8aaca3514d) Issue #2319 - LEXER: sign before POUND char like [-#"a"] is not recognized as a word and is misinterpreted
* [`c711c7`](https://github.com/Oldes/Rebol3/commit/c711c73a81eb359a10f8626924538b234ebf8d52) Typo fixed
* [`e754d1`](https://github.com/Oldes/Rebol3/commit/e754d12417eeeed220853a6f2670b98e3a094e9f) Using rsa/verify instead of rsa/validate
* [`d3393a`](https://github.com/Oldes/Rebol3/commit/d3393a8eb6c9e17071c447316b0ce11a5a6c34df) Avoiding some of conversion warnings in VS under 64 bit build
* [`a1ce4b`](https://github.com/Oldes/Rebol3/commit/a1ce4bdd9d775186a9a53fbd87e39e862b2cd839) Better handling missing padding of the DEBASE (64) input
* [`a01c68`](https://github.com/Oldes/Rebol3/commit/a01c6836aa1fc3039e70ad14a63814c64a64c5fb) Additional fix for previous feature
* [`8ca681`](https://github.com/Oldes/Rebol3/commit/8ca68199884915728ffb9d7296c1c3469214392e) DEBASE is not able to decode valid input in some cases

## 2018 August

### Fixes:

* [`dde29e`](https://github.com/Oldes/Rebol3/commit/dde29ee2bcda0da61e38320236bf7c6131fcd630) Added missing copyrights for implementation of SHA256, RC4 and AES algorithms

## 2018 July

### Features:

* [`64fbcc`](https://github.com/Oldes/Rebol3/commit/64fbcc695a8ac8ff969fd259232653ab527de57c) Just a very basic WAV codec (decoder)

  To be honest, I'm not sure how useful it is as one gets just binary,
  but no aditional info. It should return a sound object instead!
  So far it can be considered just as a minimal codec example.

### Fixes:

* [`fc5b8a`](https://github.com/Oldes/Rebol3/commit/fc5b8a46f12d3bbd30239c13a02875efa3f789b9) DELECT Bypasses PROTECT
* [`942470`](https://github.com/Oldes/Rebol3/commit/942470a847915c63a67834402b0fe9d15a3a4d0d) Better handling ANSI escape sequences (on Windows)

## 2018 June

### Features:

* [`2231a1`](https://github.com/Oldes/Rebol3/commit/2231a1db082cbb80649c53bb20c3f2476e4d949f) Add PAIR! support for REPEAT

  This change allows PAIR! values as value argument of REPEAT. This
  enables convenient traversal over a 2D space:
  ```rebol
  >> repeat p 3x3 [print p]
  1x1
  2x1
  3x1
  1x2
  2x2
  3x2
  1x3
  2x3
  3x3
  ```
  Similar to REPEAT over decimals, REPEAT over pairs implies a start of
  1.0x1.0 and a default bump of 1.0 for each component. The first pair
  component ("X") is varied first, which is in line with pair!-based
  indexing into image!s.
  
  For the sake of completeness, PAIR! support is also enabled in the
  legacy C-style FOR:
  ```rebol
  >> for p 10x10 15x20 5x5 [print p]
  10x10
  15x10
  10x15
  15x15
  10x20
  15x20
  
  >> for p 3x1 1x2 -1x1 [print p]
  3x1
  2x1
  1x1
  3x2
  2x2
  1x2
  ```
* [`6c399b`](https://github.com/Oldes/Rebol3/commit/6c399babc309aecc1efe373113d8321bdb11ea4a) Updating macOS' prebuild R3 executable
* [`a35168`](https://github.com/Oldes/Rebol3/commit/a35168b18b5b4430eced8e60294dbf471ea8041d) New native: NAN?

  Returns TRUE if the number is Not-a-Number.
  Example:
  ```rebol
  >> nan? 1
  == false
  
  >> nan? 1.#NaN
  == true
  
  >> nan? 1.0 / 0
  == false
  
  >> nan? sin 2.0
  == false
  
  >> nan? asin 2.0
  == true
  
  ```
* [`a13b33`](https://github.com/Oldes/Rebol3/commit/a13b3367fbd6f450e192907f530a2e94743e6de5) Updating Windows' prebuild R3 executable
* [`cd09db`](https://github.com/Oldes/Rebol3/commit/cd09db2bfc270bd74e2b9750c4c470fba4bdde8b) New natives ARCTANGENT2 and ATAN2

  ATAN2 is direct mapping to libc's ATAN2 function without any conversion so does not accept integer types as an input.
  
  ARCTANGENT2 is more user friendly (but little bit slower) - it uses pair! type as an input of the coordinates and result is in degrees by default (may be changed using /radians refinement)
  
  Example output:
  ```rebol
  >> arctangent2 10x-10
  == -45.0
  
  >> arctangent2/radians 10x-10
  == -0.7853981633974483
  
  >> atan2 -10.0 10.0
  == -0.7853981633974483
  
  ```
  
  NOTE: notice that ATAN2 is using the input in classic order (Y and X)
* [`72d815`](https://github.com/Oldes/Rebol3/commit/72d81552cc965a8d6fd00f2b4e088f5cb6a2184c) Faster trigonometric functions (without conversions and bounds checks)

  While the original functions SINE, COSINE, TANGENT and ARCTANGENT works also with degrees and do better output formating, these new functions: SIN, COS, TAN and ATAN expects only value in radians and are directly mapped to libc trigonometric functions. They will be faster, but one must expect different output in some cases!
  
  Difference example:
  ```rebol
  >> tangent/radians (pi / 2)
  == 1.#INF
  
  >> tan (pi / 2)
  == 1.633123935319537e16
  ```
  NOTE: these functions are available only with INFINITY support (needs 1.#NaN value)
* [`669f86`](https://github.com/Oldes/Rebol3/commit/669f86dd66b629347d330158775006afb53c15a3) Modified the structure of the Rebol native code specification in C files

  Now it looks more like the default Rebol source code header banner.
  This is full example of the native function including it's Rebol specification:
  
  ```C
  /***********************************************************************
  **
  */	REBNATIVE(sin)
  /*
  //	sin: native [
  //		{Returns the trigonometric sine.}
  //		value [decimal!] "In radians"
  //	]
  ***********************************************************************/
  {
  	SET_DECIMAL(D_RET, sin(VAL_DECIMAL(D_ARG(1))));
  	return R_RET;
  }
  ```
  
  NOTE: these definitions are parsed in tools/make-headers.r script
  At this moment the parser don't work with C preprocessing - so it would collect also code from `#ifndef` blocks -  this must be improved!
* [`d7fd2e`](https://github.com/Oldes/Rebol3/commit/d7fd2ec11af0f250dbb9e19335cc5a952524f405) Added support for loading NaN (1.#NaN) and Infinite (1.#INF) values and using them in decimal computations

  This feature is optional and can be turned of by using USE_NO_INFINITY compilation define.
* [`d5a978`](https://github.com/Oldes/Rebol3/commit/d5a978bf725f3ef650accbdd63e595b7cb5b46d5) Define ~ as a shortcut for system/options/home

  So it is like in *nix shells and one can use it like:
  ```rebol
  >> cd ~
  == %/X/Rebol/
  
  >> cd ~/..
  == %/X/
  
  ```
* [`5fa2db`](https://github.com/Oldes/Rebol3/commit/5fa2db14bd77b1b71a626d2ab5cbc9b137c767e7) Added manifest file into Windows resources

  and renamed ico file
* [`c011a2`](https://github.com/Oldes/Rebol3/commit/c011a259e2bf626a96c4111566596aa07d10d9a1) Unifying return and error handling from the original `access-os` function

  Instead of `0` it returns `true` as a result of successful `set` operation
  Instead of returning none, it throws `not supported on your system` error when return value is of type OS_ENA
  It throws error when any other error happen.
  
  So for example on Windows it looks like:
  
  ```rebol
  >> access-os/set 'pid 33488 ;<-- terminates process
  == true
  
  >> access-os/set 'pid 33488
  ** Access error: process not found: 33488
  ** Where: access-os
  ** Near: access-os/set 'pid 33488
  
  >> access-os 'euid
  ** Internal error: euid not supported on your system
  ** Where: access-os
  ** Near: access-os 'euid
  
  ```
* [`64b724`](https://github.com/Oldes/Rebol3/commit/64b724c3b5d493f4667ebdf17e4b6761c74b5f6d) Making at least basic SIGTERM and SIGKILL signals be mapped to Os_Kill on Windows to be compatible with posix version…

  So `access-os/set 'pid [32464 15]` will terminate process on all systems.
* [`f7ed5e`](https://github.com/Oldes/Rebol3/commit/f7ed5ef37fd608150a3c1528b44b142e0afc8d3f) Add a native 'access-os'

  Based on commit: https://github.com/zsx/r3/commit/275c8672810f2beb760ecfeebcf82769da76b78e
  But slightly modified - the OS_E* definitions were moved to reb-defs.h file
  
  Original commit message:
  
  > It's almost like what's in R2, with the addition of 'euid and 'egid, and
  > additional signal number to kill the process:
  >
  > access-os/set 'pid [2123 9] ;send a signal 9 (SIGKILL) to process 2123
* [`5c5241`](https://github.com/Oldes/Rebol3/commit/5c5241c96b49a6c37023491c8e26db337bf3dd30) Added block with define flags into make-settings so one could use these in the compilation using generated makefile
* [`aa10d1`](https://github.com/Oldes/Rebol3/commit/aa10d1221ed6e3ac075be453d3f9e23bc31cd89b) Image codecs are now included only when requested using compilation defines

  To enable these codecs one must use these defines:
  ```C
  USE_PNG_CODEC
  USE_BMP_CODEC
  USE_JPG_CODEC
  USE_GIF_CODEC
  ```
* [`a7b056`](https://github.com/Oldes/Rebol3/commit/a7b05627a3d31621732add94e8f76f9307e8bffb) Automatically collecting base code from C sources

  If such a comment is used in C file:
  ```C
  /**********************************************************************
  **  Base-code:
  
  	if find system/codecs 'wav [
  		system/codecs/wav/suffixes: [%.wav %.wave]
  		system/codecs/wav/type: 'binary!
  		append append system/options/file-types system/codecs/wav/suffixes 'wav
  	]
  
  ***********************************************************************/
  ```
  The Rebol code from it is extracted and included just after content of the src/mezz/base-defs.r
  
  Note that at this stage there is just a minimal system available, so one must be careful, what code is used.
* [`582e43`](https://github.com/Oldes/Rebol3/commit/582e43d309b30602e579917b0bd148945bb5911a) Picking the right PICK

  In Rebol2, picking value with index 0 was always returning null. In R3-alpha it was changed that zero index was working like index -1… more at this blog http://www.rebol.net/cgi-bin/r3blog.r?view=0173
  
  I decided to revert this change and let it working as before (and so be compatible with Red too)
  
  So now it works for example like:
  ```rebol
  >> pick tail "123" -1
  == #"3"
  >> pick tail "123" 0
  == none
  ```
  and it is also consistent with AT positioning:
  ```rebol
  >> at tail "123" -1
  == "3"
  ```
* [`690dff`](https://github.com/Oldes/Rebol3/commit/690dffaa0474075f4b327e2a93a80c2b3bb24858) REDUCE/NO-SET working also with set-path!

  ```rebol
  >>  reduce/no-set [x/1: now/date]
  == [x/1: 9-Jun-2018]
  ```
* [`338518`](https://github.com/Oldes/Rebol3/commit/33851857ade29ccdbeea9d2c5c3b5d074809c30c) Rename SPLIT's /INTO refinement
* [`cfe455`](https://github.com/Oldes/Rebol3/commit/cfe4555e963c55c20a5db354ac239a9a745feef0) Improved CD shortcut command

  fixes:
  https://github.com/Oldes/Rebol-issues/issues/272
  https://github.com/Oldes/Rebol-issues/issues/2309
* [`7af09d`](https://github.com/Oldes/Rebol3/commit/7af09dccd24a112c3d045f7230aab2efb17616ac) Some improvements in HELP output
* [`829bd7`](https://github.com/Oldes/Rebol3/commit/829bd7842a53a52770e978be36882589789e1576) Added missing updated file for previous protect/permanently feature
* [`053846`](https://github.com/Oldes/Rebol3/commit/053846baa6119ce087f033450c57f314c244a804) Added /permanently refinement to protect function

  By default, values protected using PROTECT function could be unprotected by user using UNPROTECT function.
  When /PERMANENTLY flag is used in the PROTECT call, the value will be stay locked until end of its time.

### Fixes:

* [`fdc05f`](https://github.com/Oldes/Rebol3/commit/fdc05fd0bd5d0a5dc7b8cbafd93e0d0308b85299) Bitset not allowed in to/thru block
* [`b940cb`](https://github.com/Oldes/Rebol3/commit/b940cbec774f87d2a18c06b53baa607126df5186) PARSE string using tag! doesn't consider the < and > when used with to block!
* [`b95ebf`](https://github.com/Oldes/Rebol3/commit/b95ebfedd69164822cef8ed5a10fb4493516390f) Removed recently added NAN? native and replaced NUMBER? function with native equivalent which properly checks if decimal value is not a NAN
* [`89a055`](https://github.com/Oldes/Rebol3/commit/89a055d1f045854d6e1131e866a4b4fd01212173) Using `FREE_MEM` instead of `free` directly
* [`f27e71`](https://github.com/Oldes/Rebol3/commit/f27e71964a4cf9b153b25d919b90fd35f291560a) Failure when system/script/args > 1022 bytes
* [`9adf4b`](https://github.com/Oldes/Rebol3/commit/9adf4b3c91ea0124665c99a83171e4226781c4f0) One more fix of the appveyor url
* [`50734e`](https://github.com/Oldes/Rebol3/commit/50734ecbbdb40f64ccef0724b2de9e13ebebc407) Changing file name of the icon used in MSVC project generator
* [`d1bf53`](https://github.com/Oldes/Rebol3/commit/d1bf53279066f331121396b4b4f0f05bec611b42) Updated links in appveyor and travis badges to follow rename of the repository from R3 to Rebol3
* [`4f5c88`](https://github.com/Oldes/Rebol3/commit/4f5c88d94fd554adc7ae76709341535408e826e8) Updating Linux's prebuild R3 and temporary forcing TO-WORD conversion in the native name test in make-headers.r file (because old R3 was keeping : char when converting set-word to string using FORM)
* [`8feac4`](https://github.com/Oldes/Rebol3/commit/8feac4236e1b9113140e6184ab32254e682ffec5) Using ATAN instead of ARCTAN
* [`722a9a`](https://github.com/Oldes/Rebol3/commit/722a9a960621b6fb6c518b96443c68fff2080372) Warning: conversion from 'REBU64' to 'REBYTE', possible loss of data
* [`97e421`](https://github.com/Oldes/Rebol3/commit/97e421638f5e6b7ca95f8a5dabc60262cd61f4b9) Warning: conversion from 'size_t' to 'REBYTE', possible loss of data
* [`973a8d`](https://github.com/Oldes/Rebol3/commit/973a8d305067dccec65f28314ebc77fa58392f1f) Warning: 'N_access_os': not all control paths return value
* [`0427be`](https://github.com/Oldes/Rebol3/commit/0427be402cff153f9f1b1d219d76059f97781a86) Warning: initialization discards 'const' qualifier from pointer target type
* [`879314`](https://github.com/Oldes/Rebol3/commit/879314da78758c3478b80664bd915b32f16baaf6) Warning: implicit declaration of function 'reb_qsort'
* [`cee4ce`](https://github.com/Oldes/Rebol3/commit/cee4ce4d225d524303dedd86d54493a07c109af0) Added casting to prevent "signed/unsigned mismatch" warning in Visual Studio
* [`6cfc9d`](https://github.com/Oldes/Rebol3/commit/6cfc9d913df3c8b77794d44759e21dedda9bb02e) Add an assert in Loop_Each to help coverity
* [`9a9b19`](https://github.com/Oldes/Rebol3/commit/9a9b1955bcb5b2f62e805110244cc1cdfa010f99) Fix break from remove-each
* [`e41a25`](https://github.com/Oldes/Rebol3/commit/e41a254d935ef45dabe708e16fdcb681e3d5b037) Fix foreach with set-word
* [`4966f0`](https://github.com/Oldes/Rebol3/commit/4966f0b532d895dd2991d943e860d50a627e1bed) Check overflow in Loop_Integer
* [`f2d3cd`](https://github.com/Oldes/Rebol3/commit/f2d3cdae74344a310e1643410834ef28530713f0) Let TANGENT 90 return an infinity value instead of throwing overflow error
* [`956119`](https://github.com/Oldes/Rebol3/commit/956119c6a7a05d3ee2008d06413fed1933b3e6bf) Removing POWER related unused code
* [`045fff`](https://github.com/Oldes/Rebol3/commit/045fff82845d2be561b4f636d4f0c70dba904b51) Zero to the power of zero is 1 instead of 0
* [`8ea1b1`](https://github.com/Oldes/Rebol3/commit/8ea1b17e10da540ddcfa63ba28ad7a0b55fe2c53) PORT! state binary appears empty when it is not
* [`3b0f7a`](https://github.com/Oldes/Rebol3/commit/3b0f7ae1a00e7d0e399fa035f083d8dd6ae03f18) Check REB_TIME for timeout parameter
* [`8ec6e5`](https://github.com/Oldes/Rebol3/commit/8ec6e591ada833934f02ae79baa4a776ade3f99c) Rename OS_Wait_Process to OS_Reap_Process (and add it to os/posix)
* [`918b61`](https://github.com/Oldes/Rebol3/commit/918b610853c2c49cc27de3683c0594e6dcdc02d2) Fix the poor performance in network writing
* [`02c203`](https://github.com/Oldes/Rebol3/commit/02c203c61832cea2d9156edf85a86fd1c6993418) Correctly close IO device
* [`75a5de`](https://github.com/Oldes/Rebol3/commit/75a5deb945df5034b71d0d19febd424d1df8bf0d) Correctly dealing with defines for clipboard device in the devices table
* [`3263da`](https://github.com/Oldes/Rebol3/commit/3263dab99c1e14251f6985026cccb677c28f0bab) Updating system version to be 3.*.* and not 2.*.* anymore
* [`1c86e0`](https://github.com/Oldes/Rebol3/commit/1c86e0100bbd55d99df679245eb0fcb0748472e0) Embedded test extension not setting size of the created image
* [`6202a8`](https://github.com/Oldes/Rebol3/commit/6202a850c610bd2596cbb22d1fda21ab9dddbb23) DECIMAL! picking acting as SELECT, though it PICKed in Rebol2
* [`59f9cc`](https://github.com/Oldes/Rebol3/commit/59f9cc3fff654668d843f8be9b104d35697a17ab) Check integer overflow before arithmetic operation
* [`2abd4c`](https://github.com/Oldes/Rebol3/commit/2abd4c224f737cd2370f79bad704d4e345573c50) Trap evaluation attempts of ET_INVALID values
* [`27278b`](https://github.com/Oldes/Rebol3/commit/27278b6c5f2fd08e12db5da752da3caecf0b329e) COMPOSE/into of non-block doesn't insert into target
* [`6d14ee`](https://github.com/Oldes/Rebol3/commit/6d14eea4f4d390b46e47dfaf51c0079748e52fc9) REDUCE/into of non-block doesn't insert into target
* [`f4c48b`](https://github.com/Oldes/Rebol3/commit/f4c48b7781e0bcbfd9f73cbab7fd33c9819c4c8e) Removing code which is no more needed (there is no RETURN/REDO anymore)
* [`7e61a4`](https://github.com/Oldes/Rebol3/commit/7e61a4aa219f50a7015096a82336c205b73b0e13) Type-check call of TRY/except handler
* [`bf34bb`](https://github.com/Oldes/Rebol3/commit/bf34bb5fb74a6ab6f8eb5bc226c19e54574bd98e) Rename variables to improve clarity
* [`8e918f`](https://github.com/Oldes/Rebol3/commit/8e918f782d56cfda4d7c25b2d725e494a4c9d537) Removed printed output if CD command is used without any argument
* [`71a1fa`](https://github.com/Oldes/Rebol3/commit/71a1faec816e693a7bde0d0264e085b5f112c569) Removing mentioned functions DOCS and WHY? in HELP output (as these were commented out in previous commit with other outdated help functions)
* [`eb3b7c`](https://github.com/Oldes/Rebol3/commit/eb3b7c14e4c2da904bbf9a10a8d3b8660602b099) HELP was providing invalid info if value of path was of ERROR! type
* [`eea737`](https://github.com/Oldes/Rebol3/commit/eea7372a1001d3d719f34151ff036421466cf20c) PROTECT-ed series are not protected against REMOVE-EACH

### Changes:

* [`82f4c8`](https://github.com/Oldes/Rebol3/commit/82f4c816626f319222c284d520819ff2b424c89d) Renamed TAN color to TANNED as TAN is now used as a trigonometric function
* [`a54c69`](https://github.com/Oldes/Rebol3/commit/a54c6971097d415006a20f91dac01fcb8f679676) ARCSINE and ARCCOSINE returns 1.#NaN where they were throwing error before (if supported).

  For example:
  ```rebol
  >> arccosine/radians 2.0
  == 1.#NaN
  ```
* [`c87742`](https://github.com/Oldes/Rebol3/commit/c87742be41377edcfd72a6e146578948e79ba873) Lone hash sign is not none! anymore (if not defined using USE_EMPTY_HASH_AS_NONE define) to be compatible with Red language

## 2018 May

### Features:

* [`ecee1a`](https://github.com/Oldes/Rebol3/commit/ecee1af0f8b4e593cddf077757b7e0e8195adff5) Adding different output for form/mold functions used on word types (as it was in Rebol2)

  So it is again:
  ```rebol
  >> mold #foo
  == "#foo"
  >> form #foo
  == "foo"
  ```
  NOTE: it also affects conversion using TO string! (and string variants like file!)
  
  Related ancient blog article: http://www.rebol.net/cgi-bin/r3blog.r?view=0304
* [`d60466`](https://github.com/Oldes/Rebol3/commit/d6046697956cabefd1f1ca9ca1d34804ff98543a) Added some comments and improved output in the host test extension
* [`c1857f`](https://github.com/Oldes/Rebol3/commit/c1857ff6dcc9dac48f10cb4b4f32a5511d4f84a1) Automatically collecting symbols (like SYM_PLAY) so one can use them in C code without adding new words into boot/words.r file (which is still in use)
* [`38df5c`](https://github.com/Oldes/Rebol3/commit/38df5c9d0a83a9ccc1cc6ba2df4b087ce4adb229) Added support for HTTP redirection
* [`b03dd6`](https://github.com/Oldes/Rebol3/commit/b03dd686157dc5439c55948c26f5b63aaec55f03) Added AppVeyor automated building

### Fixes:

* [`00453d`](https://github.com/Oldes/Rebol3/commit/00453d3d71c8de2e8c9bf1ad580aa5a22c1b4ec4) FIX: Mold of an Image with length of multiple of 10
* [`2bcdbc`](https://github.com/Oldes/Rebol3/commit/2bcdbc4e826ac5cb3fbd8f64727c21ee0e41175d) FIX: Use 32-bit for image pixel
* [`4bbec0`](https://github.com/Oldes/Rebol3/commit/4bbec09679c3cd49ebb68cbfefedff9591449191) Modified host test extension so it is possible to compile it again
* [`fa0c04`](https://github.com/Oldes/Rebol3/commit/fa0c04609c8e4c90c049d7d8b3a3b68affaf7c0d) Additional test for existence of the event custom type value in the http scheme
* [`2a9b7b`](https://github.com/Oldes/Rebol3/commit/2a9b7b0ce0ab7f56f1d060393088c8cb5acac91e) Do not guard port/state
* [`f9fed0`](https://github.com/Oldes/Rebol3/commit/f9fed0e4c80e8020a5fb9382cdf446c968e5d2f5) Treat unhandled events as no activity in Awake_System
* [`fbd300`](https://github.com/Oldes/Rebol3/commit/fbd300d65b9b1f18de015320d3928804056e6972) Dynamically change resolution passed to OS_WAIT
* [`c27cbc`](https://github.com/Oldes/Rebol3/commit/c27cbcac3251fb30fe7a8a4f0da4d368ee77ac8d) Add a /only refinement to wait
* [`84ed8b`](https://github.com/Oldes/Rebol3/commit/84ed8b555303e8f985f2017e64583a95147ba81e) Fix invalid memory access
* [`931dcf`](https://github.com/Oldes/Rebol3/commit/931dcf140bf5b1e148cf65c9d1becd6b7cac7052) Fix the missing port returned from wait/all
* [`c7e259`](https://github.com/Oldes/Rebol3/commit/c7e25945f104e89ec365d3ad08ec44f3d937f524) Fix infinite loop in wait
* [`fb1ca8`](https://github.com/Oldes/Rebol3/commit/fb1ca88052e63748382a229e3d823d956cb98c83) Moved recycle call out of the timing so it does not affect its measurement
* [`d2d0aa`](https://github.com/Oldes/Rebol3/commit/d2d0aaac50a2a0b283e92c4f707c975a42099d89) Patch to prot-http.r to not fail on tiny webserver (http://www.rebol.net/cookbook/recipes/0057.html)
* [`d2a06d`](https://github.com/Oldes/Rebol3/commit/d2a06d858601f7aa85958413ffdc9c72ae62a19c) Updated prot-http script header
* [`fed3fb`](https://github.com/Oldes/Rebol3/commit/fed3fb35b7ffa6903142ef09e33c102ff7d81fc1) Improving previous commit (https://github.com/Oldes/r3/commit/38df5c9d0a83a9ccc1cc6ba2df4b087ce4adb229) so urls with not standard ports are handled too.
* [`3b89ed`](https://github.com/Oldes/Rebol3/commit/3b89ed9ed40a5297900118582d4c5e1e0c844f36) Query on directory was returning garbage size info.
* [`1c737e`](https://github.com/Oldes/Rebol3/commit/1c737e174437a7b0b7968af0a1b2d4191faffb98) Better `date` handling… (to date! is now working on idate string)
* [`15f8e4`](https://github.com/Oldes/Rebol3/commit/15f8e47c0760a2ec853b4ae71075163e159d42a5) Query on URL was returning just none
* [`ae93f7`](https://github.com/Oldes/Rebol3/commit/ae93f766aba07ab29421aa071d208fd22dbcbeba) Updated HTTP scheme to version 0.1.4 (Richard Smalak changes from Atronix fork)
* [`3c124a`](https://github.com/Oldes/Rebol3/commit/3c124a0d03c3135ccd087e378e566a2b5edf2707) Preparing target dirs where is copied resulted exe if Visual Studio project is used (so building does not fails in automated process)
* [`bc24cd`](https://github.com/Oldes/Rebol3/commit/bc24cd1d5a46411c694cb24cc45897f405bc153e) Removing user input if making VS project from build script (so it can be used in automated builds)

### Changes:

* [`8eda0e`](https://github.com/Oldes/Rebol3/commit/8eda0ef7493e42cf0ca38a1974a10fe825b8a2c5) Removing `info?` function (as it duplicates `query`), and having `exists?` returning logic value.

## 2018 April

### Fixes:

* [`2458be`](https://github.com/Oldes/Rebol3/commit/2458be18aa59ef4dd1cdd2d67a7259d3b0ebfebe) Reverting previous try to fix VC warning as MingW does not like it at all.
* [`472b2c`](https://github.com/Oldes/Rebol3/commit/472b2c73b97907bdbf4e3fa4646e416cc2bfff31) Better fix for previous (warning) fix (as Clang requires char ** for second parameter of main function)
* [`44d5e3`](https://github.com/Oldes/Rebol3/commit/44d5e3a6a99d1b7d15a81f1f371ca8374da3d35b) Preventing signed/unsigned mismatch warnings in Visual Studio
* [`b69272`](https://github.com/Oldes/Rebol3/commit/b6927292f672c6a843e5c631765e2611a2798989) Removing unreferenced local variables
* [`1870b5`](https://github.com/Oldes/Rebol3/commit/1870b5d8931e1645a94ceac8ec07b7880fc16690) Preventing assignment from incompatible pointer type warning
* [`4c1fba`](https://github.com/Oldes/Rebol3/commit/4c1fba8799503043fc69ff1faab9b89ea3f5efe3) Forcing deletion of b-init.o file so compilation may be used without cleaning everything
* [`5613da`](https://github.com/Oldes/Rebol3/commit/5613dabea37cbb0d04d21177bd02586a5d5c8755) REBNATIVE(browse) should trap on OS_BROWSE failure, not success.
* [`008708`](https://github.com/Oldes/Rebol3/commit/008708abb1c3e9475b26eb362693875fffe056a5) Add check for 64-bit macOS in Try_Browser.

## 2018 March

### Features:

* [`70e92e`](https://github.com/Oldes/Rebol3/commit/70e92e2ac2cc1d48e056844940647fa0d4215fc9) Windows: updated prebuild r3 exe and changed path to mingw32 folder
* [`fe0886`](https://github.com/Oldes/Rebol3/commit/fe0886f1576f4bb90a56ce49c94a6381bc732a2e) Added support for copy/deep and copy/types on map! datatype

### Fixes:

* [`259b25`](https://github.com/Oldes/Rebol3/commit/259b2525578a0634acc2a07ac0f3abe4d4935452) MT_Map must have only 3 args

## 2018 February

### Fixes:

* [`d7eb95`](https://github.com/Oldes/Rebol3/commit/d7eb95cb890361297d0c12d2e9418e2bc534e632) ATRONIX: Avoid overwriting the variable in testing
* [`fa10bd`](https://github.com/Oldes/Rebol3/commit/fa10bd75ecd86c19093032c7c307afc8acc982ab) ATRONIX: Initialize remote_addr unconditionally
* [`4b990d`](https://github.com/Oldes/Rebol3/commit/4b990d0c8fb2fd819d77199baeb1457f0f3784f2) ATRONIX: Put accepted connection in non-block mode
* [`982eb0`](https://github.com/Oldes/Rebol3/commit/982eb027bc39b53a0d247780e98bdfc594517645) ATRONIX: Do not oversend data
* [`04e7a1`](https://github.com/Oldes/Rebol3/commit/04e7a1444f56f35a12a1fb6849b595e36b9034af) ATRONIX: Check for error in port CLOSE
* [`755395`](https://github.com/Oldes/Rebol3/commit/7553959f543954f36fa07af1c0a4451701552e48) ATRONIX: Do not free port/state

## 2018 January

### Features:

* [`8c9b3a`](https://github.com/Oldes/Rebol3/commit/8c9b3ab5ed4d4162a4f4db556a1e6da0bcd6cbe6) Cleaning project files directly from Rebol instead of using `makefile`

### Fixes:

* [`7091c9`](https://github.com/Oldes/Rebol3/commit/7091c94935297d63a4f670241cf62201d4addb76) Not using linear search for maps of small size as it was causing crashes in rare cases.
* [`7ce369`](https://github.com/Oldes/Rebol3/commit/7ce36996bc895a0802cf658702998824a422dab6) [Linux] Save signal mask when jumpping
* [`e06825`](https://github.com/Oldes/Rebol3/commit/e0682549574d95221d23ffa59d679b0c76be3476) Use {STDIN,STDOUT}_FILENO instead of 0, 1

## 2017 November

### Features:

* [`e5638c`](https://github.com/Oldes/Rebol3/commit/e5638c00c615af1f8a04ca4949c7b2609dbe3e7e) Does not CLEAN on each build command (for faster re-compilation)
* [`ca2054`](https://github.com/Oldes/Rebol3/commit/ca2054507e800f311f6f3ec8edb87e1f2de01142) Added SHA256 method into CHECKSUM native function.
* [`834a11`](https://github.com/Oldes/Rebol3/commit/834a11f7ba706070b7a7ed80afb047b8ba79c0e6) AES encryption native function
* [`102a5d`](https://github.com/Oldes/Rebol3/commit/102a5d6969e97b8c68fdd294985ce57e279f3d88) RC4 encryption native function
* [`acdabd`](https://github.com/Oldes/Rebol3/commit/acdabdc8200eae390680eb6ed180532b9f2f4e9a) Handle! can contain its name which may be used for handle type check and can be recognized by GC to free its linked resources
* [`8cdfa2`](https://github.com/Oldes/Rebol3/commit/8cdfa2fb9d771bbc1ebd21627852eb5c9288f806) Let handle! be visible as a result in console
* [`3a2902`](https://github.com/Oldes/Rebol3/commit/3a2902e0ce1026deadab234b7b2cecb7c7ca7755) Updated travis script not to depend on r3 from rebol.com
* [`e489fe`](https://github.com/Oldes/Rebol3/commit/e489fef30413df9a9c9e3383da917ced30990697) Native's specification may be defined directly in C code using special header structure

  This is based on Hostile Fork's work on Ren-c, just without using his INCLUDE_PARAMS_OF_* macros.
  It is still possible to use the old way definitions, so the original code does not have to be completely modified.
  
  For example:
  ```C
  /**********************************************************************/
  //
  //  test-native: native [
  //	  {This is just a native function for test purpose.}
  //	  'value [any-type!] "This can be value of any type"
  //  ]
  //
  REBNATIVE(test_native)
  {
  	REBVAL *val = D_ARG(1);
  	printf("Value has type ID: %i\n", VAL_TYPE(val));
  	return R_ARG1; //return the same value
  }
  ```
* [`e86d9b`](https://github.com/Oldes/Rebol3/commit/e86d9bcae31e5b91f5239c6402d8e35faa30e159) Updated pre-compiled r3-make-osx
* [`f68dbe`](https://github.com/Oldes/Rebol3/commit/f68dbe065b3559366302a519be3781645387faed) Updated pre-compiled r3-make-linux
* [`cfa54b`](https://github.com/Oldes/Rebol3/commit/cfa54b272bc92f743c99685fc42474ec09087a56) Adding line break after [escape] message so prompt starts on a new line
* [`0395c7`](https://github.com/Oldes/Rebol3/commit/0395c7703ccc85580ba561396423d5d4e0bf289b) Multi-line input support for console

  Rewritten version of commit 10070b44f6565dccc3c13f7769b630b60cf31ccc
  which was prepared by Andreas Bolka, based on the code proposed by @sqlab
  via AltME.
  
  There is also improved part of handling Windows subsystem and missing title resource text for build using mingw on Windows.
* [`f61b6a`](https://github.com/Oldes/Rebol3/commit/f61b6a9349c0a5008071991b85b105251b09128b) Errors printed in purple color
* [`2f91cf`](https://github.com/Oldes/Rebol3/commit/2f91cf76bd73e35d8255fe05255bc7abfb5f4e94) Windows - removed hardcoded console title and setting the title using string defined in resources file
* [`51b357`](https://github.com/Oldes/Rebol3/commit/51b357892136a65c4f5ff18819f9de15d89ed8f0) Visual Studio project using Windows console subsystem by default
* [`17c7e7`](https://github.com/Oldes/Rebol3/commit/17c7e755d1f5b869e8615269ff386dca5bf8ca4a) Script for making Visual Studio project for building Rebol3 on Windows

### Fixes:

* [`7153c2`](https://github.com/Oldes/Rebol3/commit/7153c2a8c8241f17e553c8470c43e8b76249bbc5) Using common function header formating for GetConsoleHwnd
* [`0e8884`](https://github.com/Oldes/Rebol3/commit/0e888438a10b8956625ce1e93ada77dc2433e49a) Input from console on Windows was not working correctly with unicode non latin chars as the console was allocated twice.
* [`7596f2`](https://github.com/Oldes/Rebol3/commit/7596f2c3931a037ba88ab7e3bec91416f7d49870) Modified TODO text as it does not make sense to create random IV in the AES encryption;
* [`3d5f32`](https://github.com/Oldes/Rebol3/commit/3d5f32fc21f57791cbfbcd3c1d0ac58cc9f9cc47) Aes native was not defined because of invalid definition in the header
* [`2ad2f1`](https://github.com/Oldes/Rebol3/commit/2ad2f1d2e3663c689133ee34b215105e2036fba2) Correctly handling multi-line input from PASTE into console in Posix version (tested only on Linux so far)
* [`07831f`](https://github.com/Oldes/Rebol3/commit/07831f527357ebe5f96e92216088c0c0143713d9) Posix stdio related changes as were made by Andreas Bolka and Hostile Fork
* [`b1f243`](https://github.com/Oldes/Rebol3/commit/b1f243185ea0cbf44bfb8af4aaea51f38a57efec) By Hostile Fork - Quietly tolerate EINTR in POSIX %dev-event.c
* [`a0b5e3`](https://github.com/Oldes/Rebol3/commit/a0b5e353363307c2eb6ff9ab15ade280658c19f6) By Shixin Zeng - Pass correct length to Deline_*
* [`626105`](https://github.com/Oldes/Rebol3/commit/62610532a90f125653bdedb7a94e0ca16d01eb23) By Shixin Zeng - Remove free'd serieses from GC_Infants
* [`9e2f7a`](https://github.com/Oldes/Rebol3/commit/9e2f7ad015f9a1347c69b315cb49fc3dfb5bd3a8) By Shixin Zeng - Do not increase tail before extension
* [`679215`](https://github.com/Oldes/Rebol3/commit/679215ee27902f909660a0c542dd5c15beec703c) By Shixin Zeng - Fix a crash caused by integer underflow
* [`b46f6e`](https://github.com/Oldes/Rebol3/commit/b46f6e9ab78b2d7d87ff2e9168e051209a03bfdc) By Shixin Zeng - Fix a heap-buffer-overflow found by AddressSanitizer
* [`0455c0`](https://github.com/Oldes/Rebol3/commit/0455c09df45aa6794a8c881f5bea9778704d8c28) Redme text improved
* [`eb70ff`](https://github.com/Oldes/Rebol3/commit/eb70ffd919c9108cec9e9339e7e2981709f08173) Cosmetic change in the readme file.
* [`dce47d`](https://github.com/Oldes/Rebol3/commit/dce47d68b521f4bf771947634aca333e5012d91a) Cosmetic change in the readme file.
* [`e5e419`](https://github.com/Oldes/Rebol3/commit/e5e4190aff0951964e82a8b3fa906f57c5223f36) Prevent warning C4005: 'min': macro redefinition
* [`1a0569`](https://github.com/Oldes/Rebol3/commit/1a05697a5a9c00817750e5a1d945ef2475461c66) Added initialization of local pointers to prevent Visual Studio errors C4703: potentially uninitialized local pointer variable
* [`ff41a1`](https://github.com/Oldes/Rebol3/commit/ff41a13441d9ec96329b5d551b07f2a68c1674b8) Locally disabling warning C4146: unary minus operator applied to unsigned type, result still unsigned, as it blocks compilation in Visual Studio
* [`d90604`](https://github.com/Oldes/Rebol3/commit/d90604a348f738d6e5d4d0bc143c4904e2a9f8f2) Do not use the system qsort

## 2017 October

### Features:

* [`820dff`](https://github.com/Oldes/Rebol3/commit/820dffc64ad437dfe95861113b55d482e9bbfe36) Prebuild r3 binary (version 0.2.40) for making OSX versions on macOS using the interactive script
* [`f642f2`](https://github.com/Oldes/Rebol3/commit/f642f268d75bc4e4b7100dc2015b09dc93a9c677) Posibility to use pipe() where pipe2() is not available
* [`470451`](https://github.com/Oldes/Rebol3/commit/470451017bd6217a52e2ad1b6e7bdf6173dc584c) Prebuild r3 binary (version 0.2.40) for making OSX versions on macOS using the interactive script
* [`6a8303`](https://github.com/Oldes/Rebol3/commit/6a8303ce220f2113a276631a6b995577da77b921) Posibility to use pipe() where pipe2() is not available
* [`b1529a`](https://github.com/Oldes/Rebol3/commit/b1529a0e78943e03050d330bb51ae8208d7c4644) Improved interactive build script to work on Linux (prebuild r3 binary included - version libc-2.3)
* [`459138`](https://github.com/Oldes/Rebol3/commit/459138e31486ac28c1d29e646ba8dad6b0d2be0a) Updated boot banner
* [`d8486d`](https://github.com/Oldes/Rebol3/commit/d8486daeb4cf99849c00394627d2727b6660c9d3) Adding interactive building script with prebuild Windows r3 binary
* [`1aa73f`](https://github.com/Oldes/Rebol3/commit/1aa73f49a86d78617a300d4f6a0440d1c8320dda) Added support for building 64-Bit Windows version (0.3.40)

  + moved some defines from reb.config.h into makefile (using the make-make.r script)
* [`f73266`](https://github.com/Oldes/Rebol3/commit/f73266ff0debeb31769b2252712e05d951ec3581) Added builded libraries and host.exe into gitignore
* [`15fc76`](https://github.com/Oldes/Rebol3/commit/15fc761d5afd21221f2e6a2b29c91fc45f343e67) Added support for building using makefile on Windows OS
* [`a819d6`](https://github.com/Oldes/Rebol3/commit/a819d6858143f19ebd62720e73f5ef77444362a3) Added icon and resources file for Windows application builds
* [`2c22bd`](https://github.com/Oldes/Rebol3/commit/2c22bddb61514b4939bad0a303c5a994f4ff726f) Additional comments for the previous commit
* [`bdeb75`](https://github.com/Oldes/Rebol3/commit/bdeb751858cf868431d066ef60f5d5bdeb0e8ca7) Support for ANSI escape sequences on Windows also if compiled with Console subsystem
* [`592f66`](https://github.com/Oldes/Rebol3/commit/592f66fc0929af7ab3734952e46c0d5c91095f47) Added support for Windows build with VS2017 and VS2015

### Fixes:

* [`26aa05`](https://github.com/Oldes/Rebol3/commit/26aa0547df111b085bea7a082faae27851b0be11) Clipboard is not yet ready on posix platforms
* [`aa4c06`](https://github.com/Oldes/Rebol3/commit/aa4c060880a4f82247908e28ebf0249741d18563) Make sure that home directory is absolute on all platforms
* [`3c25db`](https://github.com/Oldes/Rebol3/commit/3c25db2235d3d3ab15871c8c1705707dfd947d69) Warning: implicit declaration of function ‘write’ and 'read'
* [`4b59d5`](https://github.com/Oldes/Rebol3/commit/4b59d59dbc24d0183505d7c3b8307d78c3d134b0) Warning: implicit declaration of function ‘pipe2’
* [`563f23`](https://github.com/Oldes/Rebol3/commit/563f23fec393cf00b5384f0cedc96395dd440578) Warning: implicit declaration of function ‘close’
* [`d1f0b0`](https://github.com/Oldes/Rebol3/commit/d1f0b05ff177e13f77bc01fb07bd0dc1ee5c5479) Dylib was defined on wrong OS
* [`b95ae6`](https://github.com/Oldes/Rebol3/commit/b95ae696d52a2213f29cf6a811cafb7299177238) Using correct library extension on OSX
* [`d78caf`](https://github.com/Oldes/Rebol3/commit/d78caf619a5824fe734d39325a4ea1c9d320f6ec) Using pre-prepared makefile for Linux
* [`f06bda`](https://github.com/Oldes/Rebol3/commit/f06bdab79036cddbcd613eccda0491e813aa1fef) Correcting make settings file extension
* [`923ca9`](https://github.com/Oldes/Rebol3/commit/923ca9e800a2bd4b3ded053b2f455803c04e17b5) Adding missing posix headers needed for the CALL enhancement
* [`d7f26c`](https://github.com/Oldes/Rebol3/commit/d7f26c1432ee67ce6bdb7fbbbc1798eac16e93fb) Extended CALL native command
* [`1f3e2e`](https://github.com/Oldes/Rebol3/commit/1f3e2ece3f280e22778900cd76f66b57cf58c29a) Using correct argument type to avoid compiler warnings
* [`d962f6`](https://github.com/Oldes/Rebol3/commit/d962f60c3dc6c195b683693d1d2f62165324b8ef) Move integer arithmetic operations into their own functions
* [`2e9881`](https://github.com/Oldes/Rebol3/commit/2e98817d44002813fba0b35e8cd07188a4709144) Removing use of INLINE in some functions to avoid compilers warnings of type:
* [`db51a2`](https://github.com/Oldes/Rebol3/commit/db51a20d8f86a3b83295b47fa8a78abfaa6e9a78) Adding correct decoration around Emit_Decimal function so it is recognized by the make-headers.r script and so avoiding warning during compilation.
* [`04c8c2`](https://github.com/Oldes/Rebol3/commit/04c8c281198465840cec41c8396d6e4d64f2db4d) Added casting to prevent compiler warnings
* [`65200d`](https://github.com/Oldes/Rebol3/commit/65200ddfe4c79b75d095e719e44f991191ab1861) Correct path to Visual Studio 2017 dev command
* [`973d39`](https://github.com/Oldes/Rebol3/commit/973d3943197d65a6fe7a95a3de1978bad9f0b885) ANSI escape sequence handling was not working if compiled with Windows subsystem

## 2014 July

### Features:

* [`9b724d`](https://github.com/Oldes/Rebol3/commit/9b724df313aa94419207ae6958a4d12bd3467f2c) Adding some colors to list of help commands
* [`9eb7dd`](https://github.com/Oldes/Rebol3/commit/9eb7dd0286afa807ec957e8b8bb21c4c09359789) Adding color to prompt and result value.
* [`f228be`](https://github.com/Oldes/Rebol3/commit/f228be5b74d9fa5e0fc421011653924bb33dd75d) Reversed lookup order in the vcbuild so the latest version of VS is used if found.
* [`31899a`](https://github.com/Oldes/Rebol3/commit/31899ae261dfedcbdcc4dd4d0ecb6cfa86da80b0) ANSI escape sequence emulation under Windows.

### Fixes:

* [`f76852`](https://github.com/Oldes/Rebol3/commit/f76852c9249613a49b48e2705c94a7b27af3e0aa) ANSI colors are used, where are not expected.
* [`97a0f4`](https://github.com/Oldes/Rebol3/commit/97a0f4627862a8586e7d1347ba9ae47a4b2976a9) Changing order of values for setting cursor position (Esc[Line;ColumnH)
* [`c6b17c`](https://github.com/Oldes/Rebol3/commit/c6b17ca8cf4275ced94341832afc69035e47503f) Keeping the OBJS list sorted alphabetically.
* [`81016e`](https://github.com/Oldes/Rebol3/commit/81016ec67415b8df66a75b8ef9de5e34fceef356) Adding missing objs/f-dtoa.obj into makefile.vc
* [`7b2489`](https://github.com/Oldes/Rebol3/commit/7b24896e9419836483df9b82c973de87bf79cf5f) Correcting path to VS12's vsvars32.bat
* [`d02267`](https://github.com/Oldes/Rebol3/commit/d0226717a4c3ad95975193177016f3db336fca2d) Correcting OS_ID version for Windows
