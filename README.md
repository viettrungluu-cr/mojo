Mojo
====

This repo uses gclient to manage dependencies, so to build things from this
repo you have to first download depot_tools and make sure it is in your path:

http://www.chromium.org/developers/how-tos/install-depot-tools

Then, create a directory somewhere for your checkout, cd into it,
and run the following commands:

```
$ fetch mojo # use --target_os=android if you want an Android build.
$ cd src
$ ./build/install-build-deps.sh
```

This creates a directory called 'src' under your checkout directory, clones
the repository and its dependencies, and installs any packages needed to build.

You can then build Mojo by running:

```
$ gn gen out/Debug
$ ninja -C out/Debug root
```

If you're a Googler and want to use Goma:
```
$ gn args out/Debug
```

and add
```
use_goma = true
```

Assuming Goma is installed in the default location (~/goma).


Run a demo:
```
mojo/tools/mojo_demo.sh --browser
```



Run the tests:
```
mojo/tools/mojob.sh test
```


To land a patch:
```
$ git cl land
```

Don't break build! Waterfall is here: http://build.chromium.org/p/client.mojo/waterfall .
