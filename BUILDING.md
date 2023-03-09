Building
========

Building on a Linux-based system (including, probably, WSL although I haven't
tried it personally) should be fairly simple.

You will need development tools (a C compiler), CMake and SDL development 
libraries; on Ubuntu or any other Debian-based system, this should be as simple
as

`apt update && apt install build-essentials cmake libsdl2-dev`

Once you're set up, clone and build the repo in the usual way

```
git clone https://github.com/ahnlak/tessalatrix.git
cd tessalatrix
mkdir build
cd build
cmake ..
make
```

If you want to bundle up all the relavent files to put them somewhere else,
`make package` will generate a zip / tar file with everything assembled for you.

In *theory*, all of the above should apply to a properly configured Windows
development system too - that's essentially how the automatic builds in GitHub
works - but as I don't have access to such a system, I don't know the precise
steps!
