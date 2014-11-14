Mojo
====

Mojo is an effort to extract a common platform out of Chrome's renderer and plugin processes that can support multiple types of sandboxed content, such as HTML, Pepper, or NaCl.

## Set up your environment

The instructions below only need to be done once. Note that a simple "git clone" command is not sufficient to build the source code because this repo uses the gclient command from depot_tools to manage most third party dependencies.

1. Download depot_tools and make sure it is in your path:<br>http://www.chromium.org/developers/how-tos/install-depot-tools<br>

2. Create a directory somewhere for your checkout, cd into it, and run the following commands:


```
$ fetch mojo # use --target_os=android if you want an Android build.
$ cd src
$ ./build/install-build-deps.sh
$ gn gen out/Debug
```

The "fetch mojo" command does the following:
- creates a directory called 'src' under your checkout directory
- clones the repository using git clone
- clones dependencies with gclient sync

Finally, "install-build-deps.sh" installs any packages needed to build.

If the fetch command fails, you will need to delete the src directory and start over.

## Build Mojo

Build Mojo by running:

```
$ ninja -C out/Debug -j 10 root
```

The "root" parameter specifies the target to build, it's not a special keyword. You can find the "root" target in src/BUILD.gn.

(If you are a Googler, see the section at the end of this document for faster builds.)

You can also use the mojob.py script for building. This script automatically calls ninja and sets -j to an appropriate value based on whether Goma is present. You cannot specify a target name with this script.
```
mojo/tools/mojob.py build
```

Run a demo:
```
mojo/tools/mojo_demo.sh --browser
```

Run the tests:
```
mojo/tools/mojob.py test
```

## Update your repo

You can update your repo like this:
```
$ gclient sync
$ git pull --rebase
```

You do not need to rerun "gn gen out/Debug". Ninja will do so automatically as needed.

## Contribute

With git you should make all your changes in a local branch. Once your change is committed, you can delete this branch.

Create a local branch named "mywork" and make changes to it.
```
  cd src
  git new-branch mywork
  vi ...
```
Commit your change locally (this doesn't commit your change to the SVN or Git server)

```
  git commit -a
```
If you added new files, you should tell git so by running `git add <files>` before committing.

Upload your change for review

```
  git cl upload
```

Respond to review comments
See <a href="http://www.chromium.org/developers/contributing-code">Contributing code</a> for more detailed git instructions, including how to update your CL when you get review comments. There's a short tutorial that might be helpful to try before your first change: <a href="http://dev.chromium.org/developers/cpp-in-chromium-101-codelab">C++ in Chromium 101</a>.

To land a change after receiving LGTM:
```
$ git cl land
```

Don't break the build! Waterfall is here: http://build.chromium.org/p/client.mojo/waterfall

## Googlers

If you're a Googler, you can use Goma, a distributed compiler service for open-source projects such as Chrome and Android. The instructions below assume that Goma is installed in the default location (~/goma).

To enable Goma, update your "args.gn" file. Open the file in your editor with this command:
```
$ gn args out/Debug
```

Add this line to the end of the file:
```
use_goma = true
```

After you close the editor, the "gn args" command will automatically run "gn gen out/Debug" again.

Now you can dramatically increase the number of parallel tasks:
```
$ ninja -C out/Debug -j 1000 root
```
