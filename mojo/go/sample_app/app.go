package main

import (
	"code.google.com/p/go.mobile/app"
	"log"
	"mojo/public/go/system"
)

//#include "mojo/public/c/system/types.h"
import "C"

//export MojoMain
func MojoMain(handle C.MojoHandle) C.MojoResult {
	if core := system.GetCore(); core != nil {
		log.Printf("GetTimeTicksNow:%v", core.GetTimeTicksNow())
	}
	return C.MOJO_RESULT_OK
}

func main() {
	app.Run(app.Callbacks{})
}
