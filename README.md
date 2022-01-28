[![N-Gage SDK.](https://raw.githubusercontent.com/ngagesdk/ngage-toolchain/master/media/ngagesdk-readme-header.png)](https://raw.githubusercontent.com/ngagesdk/ngage-toolchain/master/media/ngagesdk-header-2x-white.png?raw=true "N-Gage SDK.")

# SDL 1.2

Even though SDL 1.2 is outdated and officially no longer supported, this
is an attempt to get it running on Symbian S60v1 devices such as the
Nokia N-Gage.

This project is part of a larger project aiming to create a functional
ecosystem for developing (or porting existing) games for the Nokia
N-Gage using modern and up-to-date tools such as Visual Studio 2019/2022
and CMake.

You can find the toolchain here:
[ngage-toolchain](https://github.com/ngagesdk/ngage-toolchain)

If you are interested in the Nokia N-Gage in general, you are cordially
invited to visit our small online community. You can find us on
[Discord](https://discord.gg/dbUzqJ26vs),
[Telegram](https://t.me/nokia_ngage) and in #ngage on
[EFnet](http://www.efnet.org/).

## How-to

Clone this project into the projects sub-directory of the toolchain:

```bash
git clone https://github.com/ngagesdk/ngage-toolchain.git
cd ngage-toolchain
cd projects
git clone https://github.com/ngagesdk/SDL-1.2.git
```

Open the cloned directory in Visual Studio and wait until the CMake
solution has finished generating. You can then compile the library and
the included demo application.
