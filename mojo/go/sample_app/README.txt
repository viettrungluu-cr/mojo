Sample Go application that can be loaded into a mojo shell running on Android.
The application exports a MojoMain entry point for the shell and then makes
a GetTimeTicksNow system call.

Setup instructions

1) Generate the NDK toolchain for the android platform you plan to use.

$ cd mojo/src/third_party/android_tools/ndk
$ NDK_ROOT=$HOME/android/ndk-toolchain
$ ./build/tools/make-standalone-toolchain.sh --platform=android-14 --install-dir=$NDK_ROOT
$ NDK_CC=$NDK_ROOT/bin/arm-linux-androideabi-gcc

2) Download/Install the Go compiler.

$ unset GOBIN GOPATH GOROOT
$ hg clone https://code.google.com/p/go
$ export GOROOT=`pwd`/go
$ cd go/src
$ CC_FOR_TARGET=$NDK_CC GOOS=android GOARCH=arm GOARM=7 ./make.bash
$ ls $GOROOT/bin/go

3) Now, we switch to the Mojo workspace and build the sample application.

$ cd mojo/src
$ gn args <output_directory>

Set the following arguments
mojo_use_go=true
go_build_tool="<path_to_go_binary>"
os="android"

$ gn gen <output_directory>
$ ninja -C <output_directory> go_sample_app
