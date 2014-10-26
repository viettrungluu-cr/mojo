// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package tests

import (
	"bytes"
	"mojo/go/system/embedder"
	"mojo/public/go/system"
	m "mojo/public/go/system/impl"
	"testing"
)

var core system.Core

func init() {
	embedder.InitializeMojoEmbedder()
	core = m.GetCore()
}

func TestGetTimeTicksNow(t *testing.T) {
	x := core.GetTimeTicksNow()
	if x < 10 {
		t.Error("Invalid GetTimeTicksNow return value")
	}
}

func TestMessagePipe(t *testing.T) {
	var h0, h1 m.MojoHandle
	var r m.MojoResult

	if r, h0, h1 = core.CreateMessagePipe(nil); r != m.MOJO_RESULT_OK {
		t.Fatalf("CreateMessagePipe failed:%v", r)
	}
	if h0 == m.MOJO_HANDLE_INVALID || h1 == m.MOJO_HANDLE_INVALID {
		t.Fatalf("CreateMessagePipe returned invalid handles h0:%v h1:%v", h0, h1)
	}
	if r = core.Wait(h0, m.MOJO_HANDLE_SIGNAL_READABLE, 0); r != m.MOJO_RESULT_DEADLINE_EXCEEDED {
		t.Fatalf("h0 should not be readable:%v", r)
	}
	if r = core.Wait(h0, m.MOJO_HANDLE_SIGNAL_WRITABLE, 0); r != m.MOJO_RESULT_OK {
		t.Fatalf("h0 should be writeable:%v", r)
	}
	if r, _, _, _, _ = core.ReadMessage(h0, m.MOJO_READ_MESSAGE_FLAG_NONE); r != m.MOJO_RESULT_SHOULD_WAIT {
		t.Fatalf("Read on h0 did not return wait:%v", r)
	}
	kHello := []byte("hello")
	if r = core.WriteMessage(h1, kHello, nil, m.MOJO_WRITE_MESSAGE_FLAG_NONE); r != m.MOJO_RESULT_OK {
		t.Fatalf("Failed WriteMessage on h1:%v", r)
	}
	if r = core.Wait(h0, m.MOJO_HANDLE_SIGNAL_READABLE, m.MOJO_DEADLINE_INDEFINITE); r != m.MOJO_RESULT_OK {
		t.Fatalf("h0 should be readable after WriteMessage to h1:%v", r)
	}
	r, msg, _, _, _ := core.ReadMessage(h0, m.MOJO_READ_MESSAGE_FLAG_NONE)
	if r != m.MOJO_RESULT_OK {
		t.Fatalf("Failed ReadMessage on h0:%v", r)
	}
	if !bytes.Equal(msg, kHello) {
		t.Fatalf("Invalid message expected:%s, got:%s", kHello, msg)
	}
	if r = core.WaitMany([]m.MojoHandle{h0}, []m.MojoHandleSignals{m.MOJO_HANDLE_SIGNAL_READABLE}, 10); r != m.MOJO_RESULT_DEADLINE_EXCEEDED {
		t.Fatalf("h0 should not be readable after reading message:%v", r)
	}
	if r = core.Close(h0); r != m.MOJO_RESULT_OK {
		t.Fatalf("Close on h0 failed:%v", r)
	}
	if r = core.Wait(h1, m.MOJO_HANDLE_SIGNAL_READABLE|m.MOJO_HANDLE_SIGNAL_WRITABLE, 100); r != m.MOJO_RESULT_FAILED_PRECONDITION {
		t.Fatalf("h1 should not be readable/writeable after h0 has been closed:%v", r)
	}
	if r = core.Close(h1); r != m.MOJO_RESULT_OK {
		t.Fatalf("Close on h1 failed:%v", r)
	}
}

func TestDataPipe(t *testing.T) {
	var hp, hc m.MojoHandle
	var r m.MojoResult

	if r, hp, hc = core.CreateDataPipe(nil); r != m.MOJO_RESULT_OK {
		t.Fatalf("CreateDataPipe failed:%v", r)
	}
	if hp == m.MOJO_HANDLE_INVALID || hc == m.MOJO_HANDLE_INVALID {
		t.Fatalf("CreateDataPipe returned invalid handles hp:%v hc:%v", hp, hc)
	}
	if r = core.Wait(hc, m.MOJO_HANDLE_SIGNAL_READABLE, 0); r != m.MOJO_RESULT_DEADLINE_EXCEEDED {
		t.Fatalf("hc should not be readable:%v", r)
	}
	if r = core.Wait(hp, m.MOJO_HANDLE_SIGNAL_WRITABLE, 0); r != m.MOJO_RESULT_OK {
		t.Fatalf("hp should be writeable:%v", r)
	}
	kHello := []byte("hello")
	r, numBytes := core.WriteData(hp, kHello, m.MOJO_WRITE_DATA_FLAG_NONE)
	if r != m.MOJO_RESULT_OK || numBytes != (uint32)(len(kHello)) {
		t.Fatalf("Failed WriteData on hp:%v numBytes:%d", r, numBytes)
	}
	if r = core.Wait(hc, m.MOJO_HANDLE_SIGNAL_READABLE, 1000); r != m.MOJO_RESULT_OK {
		t.Fatalf("hc should be readable after WriteData on hp:%v", r)
	}
	r, data := core.ReadData(hc, m.MOJO_READ_DATA_FLAG_NONE)
	if r != m.MOJO_RESULT_OK {
		t.Fatalf("Failed ReadData on hc:%v", r)
	}
	if !bytes.Equal(data, kHello) {
		t.Fatalf("Invalid data expected:%s, got:%s", kHello, data)
	}
	if r = core.Close(hp); r != m.MOJO_RESULT_OK {
		t.Fatalf("Close on hp failed:%v", r)
	}
	if r = core.Wait(hc, m.MOJO_HANDLE_SIGNAL_READABLE, 100); r != m.MOJO_RESULT_FAILED_PRECONDITION {
		t.Fatalf("hc should not be readable after hp closed:%v", r)
	}
	if r = core.Close(hc); r != m.MOJO_RESULT_OK {
		t.Fatalf("Close on hc failed:%v", r)
	}
}
