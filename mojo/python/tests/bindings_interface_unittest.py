# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import gc
import weakref

# pylint: disable=F0401,E0611
import mojo.system as system

import mojo_unittest
import mojo.bindings.promise as promise
import sample_factory_mojom
import sample_service_mojom


def _BuildProxy(impl):
  pipe = system.MessagePipe()
  impl.__class__.manager.Bind(impl, pipe.handle0)
  return impl.__class__.manager.Proxy(pipe.handle1)


def _ExtractValue(p):
  container = []
  def GetValue(value):
    container.append(value)
  p.Then(GetValue)
  assert len(container)
  return container[0]


class EmptyServiceImpl(sample_service_mojom.Service):

  def __init__(self):
    pass


class ServiceImpl(sample_service_mojom.Service):

  def __init__(self):
    pass

  # pylint: disable=C0102,W0613
  def Frobinate(self, foo, baz, port):
    if self.client:
      self.client.DidFrobinate(baz)


class ServiceClientImpl(sample_service_mojom.ServiceClient):

  def __init__(self):
    self.last_res = None

  def DidFrobinate(self, result):
    self.last_res = result


class NamedObjectImpl(sample_factory_mojom.NamedObject):

  def __init__(self):
    self.name = 'name'

  def SetName(self, name):
    self.name = name

  def GetName(self):
    return self.name


class DelegatingNamedObject(sample_factory_mojom.NamedObject):

  def __init__(self):
    self.proxy = _BuildProxy(NamedObjectImpl())

  def SetName(self, name):
    self.proxy.SetName(name)

  def GetName(self):
    return self.proxy.GetName()

class InterfaceTest(mojo_unittest.MojoTestCase):

  def testBaseInterface(self):
    service = sample_service_mojom.Service()
    with self.assertRaises(AttributeError):
      service.NotExisting()
    with self.assertRaises(NotImplementedError):
      service.Frobinate()

  def testEmpty(self):
    service = EmptyServiceImpl()
    with self.assertRaises(NotImplementedError):
      service.Frobinate()

  def testCallingImplementation(self):
    service_impl = ServiceImpl()
    service_client_impl = ServiceClientImpl()
    # pylint: disable=W0201
    service_impl.client = service_client_impl
    service_impl.Frobinate(None, 42, None)
    self.assertEquals(service_client_impl.last_res, 42)

  def testCallingImplementationThroughPipes(self):
    service_impl = ServiceImpl()
    # Only keep a weak reference on the service implementation.
    r = weakref.ref(service_impl)
    proxy = _BuildProxy(service_impl)
    del service_impl

    # Check that the service is kept alive by the handle.
    gc.collect()
    self.assertIsNotNone(r())

    service_client_impl = ServiceClientImpl()
    proxy.client = service_client_impl
    response = proxy.Frobinate(None, 42, None)

    self.assertEquals(response.state, promise.Promise.STATE_FULLFILLED)
    self.loop.RunUntilIdle()
    self.assertEquals(service_client_impl.last_res, 42)

    # Check that closing the proxy release the service.
    proxy = None
    self.loop.RunUntilIdle()
    gc.collect()
    self.assertIsNone(r())

  def testServiceWithReturnValue(self):
    proxy = _BuildProxy(DelegatingNamedObject())
    p1 = proxy.GetName()

    self.assertEquals(p1.state, promise.Promise.STATE_PENDING)
    self.loop.RunUntilIdle()
    self.assertEquals(p1.state, promise.Promise.STATE_FULLFILLED)
    name = _ExtractValue(p1)
    self.assertEquals(name, 'name')

    proxy.SetName('hello')
    p2 = proxy.GetName()

    self.assertEquals(p2.state, promise.Promise.STATE_PENDING)
    self.loop.RunUntilIdle()
    self.assertEquals(p2.state, promise.Promise.STATE_FULLFILLED)
    name = _ExtractValue(p2)
    self.assertEquals(name, 'hello')

  def testCloseImplementation(self):
    service_impl = ServiceImpl()
    proxy = _BuildProxy(service_impl)
    service_client_impl = ServiceClientImpl()
    proxy.client = service_client_impl
    response = proxy.Frobinate(None, 42, None)
    service_impl.manager.Close()

    self.assertEquals(response.state, promise.Promise.STATE_FULLFILLED)
    self.loop.RunUntilIdle()

    self.assertIsNone(service_client_impl.last_res)

  def testCloseProxy(self):
    service_impl = ServiceImpl()
    proxy = _BuildProxy(service_impl)
    service_client_impl = ServiceClientImpl()
    proxy.client = service_client_impl
    response = proxy.Frobinate(None, 42, None)
    proxy.manager.Close()

    self.assertEquals(response.state, promise.Promise.STATE_FULLFILLED)
    self.loop.RunUntilIdle()

    self.assertIsNone(service_client_impl.last_res)

  def testCloseImplementationWithResponse(self):
    impl = DelegatingNamedObject()
    proxy = _BuildProxy(impl)
    p1 = proxy.GetName()

    self.assertEquals(p1.state, promise.Promise.STATE_PENDING)

    impl.manager.Close()
    self.loop.RunUntilIdle()

    self.assertEquals(p1.state, promise.Promise.STATE_REJECTED)
