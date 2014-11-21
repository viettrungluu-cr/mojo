Indirect Service Demo
=====================

This demo is intended to highlight the difference between requesting a service
and providing one. The demo is based on two services: IntegerService and
IndirectIntegerService.

interface IntegerService {
  Increment() => (int32 value);
};

This trival interface just manages a single internal integer that's initialized
to 0. The Increment() method returns a new value.

interface IndirectIntegerService {
    Set(IntegerService? service);
    Get(IntegerService&? service);
};

This service delegates to the one IntegerService provided by the Set() method.
Clients use Get() to request a connection to an IntegerService that targets the
delegate. This is roughly an IntegerService "pointer".

The demo creates a set of threads all of which get their own connection to the
shared IntegerService via the IndirectIntegerService. The threads all access
the IntegerService at the same time and then display a little table of the
results.
