Mojo
====

This repo uses gclient to manage dependencies, so to build things from this
repo you have to first download depot_tools and make sure it is in your path

http://www.chromium.org/developers/how-tos/install-depot-tools

then create a file called '.gclient' that has the following contents:

```
solutions = [
  { "name"        : "src",
    "url"         : "https://github.com/domokit/mojo.git",
    "deps_file"   : "DEPS",
    "managed"     : False,
    "safesync_url": "",
  },
]
# Include the following line only if you're interested in building for android.
target_os = ['android']
```

Then run 'gclient sync' from the directory containing the .gclient file.
This will clone this repository into a subdirectory called 'src'.  From that
directory, you can then ensure you have all the dependencies installed by
running:

```
$ ./build/install-build-deps.sh
```

You can then build by running:

```
$ gn gen out/Debug
$ ninja -C out/Debug mojo
```
