Sample Go application that can be loaded into a mojo shell running on Android.
The application exports a MojoMain entry point for the shell and then makes
a GetTimeTicksNow system call.

Build instructions
gn args <output_directory>

Set the following arguments
mojo_use_go=true
go_build_tool=<go_binary_location>
os="android"

gn gen <output_directory>
ninja -C <output_directory> go_sample_app

You can now following instructions from the below link to run the app.
http://dev.chromium.org/developers/how-tos/run-mojo-shell

Start the Go application.
./build/android/adb_run_mojo_shell http://127.0.0.1:4444/go_sample_app
