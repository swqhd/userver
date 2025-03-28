# gRPC middleware

## Before you start

Make sure that you can compile and run core tests and read a basic example
@ref scripts/docs/en/userver/tutorial/hello_service.md.

## Step by step guide

In this example, you will write an authentication middleware for both
'GreeterService' and 'GreeterClient' of the basic grpc_service. 
See @ref scripts/docs/en/userver/tutorial/grpc_service.md

### Installation

Generate and link with necessary libraries:

@snippet samples/grpc_middleware_service/CMakeLists.txt  gRPC middleware sample - CMake

### The client middleware

Client middleware will add metadata to every `GreeterClient` call.

Derive `Middleware` and `MiddlewareFactory` from the respective base class and declare a middleware-factory:

@snippet samples/grpc_middleware_service/src/middlewares/client/auth.hpp Middleware declaration

`PreStartCall` method of `Middleware` does the actual work:

@snippet samples/grpc_middleware_service/src/middlewares/client/auth.cpp Middleware implementation

Lastly, add this component to the static config:

```
# yaml
components_manager:
    components:
        grpc-auth-client:
```

And connect it with `ClientFactory`:

@snippet samples/grpc_middleware_service/static_config.yaml gRPC middleware sample - static config client middleware


### The server middleware

Server middleware, in its turn, will validate metadata that comes with an rpc.

Everything is the same as it is for client middleware, except there is no
factory and the component stores the middleware itself:

@snippet samples/grpc_middleware_service/src/middlewares/server/auth.hpp Middleware declaration

Handle method of `Middleware` does the actual work:

@snippet samples/grpc_middleware_service/src/middlewares/server/auth.cpp gRPC Middleware implementation

Respective component:

@snippet samples/grpc_middleware_service/src/middlewares/server/auth.hpp Middleware component declaration

Lastly, add this component to the static config:

```
# yaml
components_manager:
    components:
        grpc-auth-server:
```

And connect it with `Service`:

@snippet samples/grpc_middleware_service/static_config.yaml gRPC middleware sample - static config server middleware


### int main()

Finally, register components and start the server.

@snippet samples/grpc_middleware_service/main.cpp gRPC middleware sample - components registration


### Build and Run

To build the sample, execute the following build steps at the userver root
directory:

```
mkdir build_release
cd build_release
cmake -DCMAKE_BUILD_TYPE=Release ..
make userver-samples-grpc_middleware_service
```

The sample could be started by running
`make start-userver-samples-grpc_middleware_service`. The command would invoke
@ref scripts/docs/en/userver/functional_testing.md "testsuite start target" that sets proper
paths in the configuration files and starts the service.

To start the service manually run
`./samples/grpc_middleware_service/userver-samples-grpc_middleware_service -c </path/to/static_config.yaml>`.

The service is available locally at port 8091 (as per project `static_config.yaml`).


### Functional testing
To implement @ref scripts/docs/en/userver/functional_testing.md "Functional tests" for the
service some preparational steps should be done.


#### Preparations
First of all, import the required modules and add the required
pytest_userver.plugins.grpc pytest plugin:

@snippet samples/grpc_middleware_service/tests/conftest.py  Prepare modules


#### gRPC server mock

To mock the gRPC server provide a hook for the static config to change
the endpoint:

@snippet samples/grpc_middleware_service/tests/conftest.py  Prepare configs

Alternatively, use `$grpc_mockserver`
@ref pytest_userver.plugins.config.userver_config_substitutions "substitution var"
in `config_vars.testsuite.yaml`:

@code{.yaml}
greeter-client-endpoint: $grpc_mockserver
@endcode

Write the mocking fixtures using @ref pytest_userver.plugins.grpc.mockserver.grpc_mockserver "grpc_mockserver":

@snippet samples/grpc_middleware_service/tests/conftest.py  Prepare server mock


#### gRPC client

To do the gRPC requests write a client fixture using
@ref pytest_userver.plugins.grpc.client.grpc_channel "grpc_channel":

@snippet samples/grpc_middleware_service/tests/conftest.py  grpc client

Use it to do gRPC requests to the service:

@snippet samples/grpc_middleware_service/tests/test_middlewares.py  grpc authentication tests


## Full sources

See the full example at:

* @ref samples/grpc_middleware_service/src/middlewares/client/auth.hpp
* @ref samples/grpc_middleware_service/src/middlewares/client/auth.cpp

* @ref samples/grpc_middleware_service/src/middlewares/server/auth.hpp
* @ref samples/grpc_middleware_service/src/middlewares/server/auth.cpp

* @ref samples/grpc_middleware_service/src/middlewares/auth.hpp
* @ref samples/grpc_middleware_service/src/middlewares/auth.cpp

* @ref samples/grpc_middleware_service/main.cpp
* @ref samples/grpc_middleware_service/proto/samples/greeter.proto
* @ref samples/grpc_middleware_service/static_config.yaml
* @ref samples/grpc_middleware_service/tests/conftest.py
* @ref samples/grpc_middleware_service/tests/test_middlewares.py
* @ref samples/grpc_middleware_service/CMakeLists.txt

----------

@htmlonly <div class="bottom-nav"> @endhtmlonly
⇦ @ref scripts/docs/en/userver/tutorial/grpc_service.md | @ref scripts/docs/en/userver/tutorial/postgres_service.md ⇨
@htmlonly </div> @endhtmlonly

@example samples/grpc_middleware_service/src/middlewares/client/auth.hpp
@example samples/grpc_middleware_service/src/middlewares/client/auth.cpp
@example samples/grpc_middleware_service/src/middlewares/server/auth.hpp
@example samples/grpc_middleware_service/src/middlewares/server/auth.cpp
@example samples/grpc_middleware_service/src/middlewares/auth.hpp
@example samples/grpc_middleware_service/src/middlewares/auth.cpp
@example samples/grpc_middleware_service/main.cpp
@example samples/grpc_middleware_service/proto/samples/greeter.proto
@example samples/grpc_middleware_service/static_config.yaml
@example samples/grpc_middleware_service/tests/conftest.py
@example samples/grpc_middleware_service/tests/test_middlewares.py
@example samples/grpc_middleware_service/CMakeLists.txt
