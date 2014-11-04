Setup instructions to run system_test

1) Download/Install the Go compiler

$ unset GOBIN GOPATH GOROOT
$ hg clone https://code.google.com/p/go
$ cd go/src
$ ./all.bash
$ ls ../bin

3) Now, we switch to the Mojo workspace and build system_test

$ cd mojo/src
$ gn args <output_directory>

mojo_use_go=true
go_build_tool="<path_to_go_binary>"

$ gn gen <output_directory>
$ ninja -C <output_directory> system_test
$ <output_directory>/obj/mojo/go/system_test -test.v
