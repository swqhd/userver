## MongoDB service


## Before you start

Make sure that you can compile and run core tests and read a basic example
@ref scripts/docs/en/userver/tutorial/hello_service.md.


## Step by step guide

In this tutorial we will write a service that stores history of translation
changes and returns the most recent translations. MongoDB would be used as a
database. The service would have the following Rest API:

* HTTP PATCH by path '/v1/translations' with query parameters
  'key', 'lang' and 'value' updates a translation.
* HTTP GET by path '/v1/translations' with query parameter
  'last_update' returns unique translations that were added after the 'last_update'.


### HTTP handler component

Like in @ref scripts/docs/en/userver/tutorial/hello_service.md we create a component for
handling HTTP requests:

@snippet samples/mongo_service/main.cpp  Mongo service sample - component

Note that the component holds a storages::mongo::PoolPtr - a client to the Mongo.
That client is thread safe, you can use it concurrently from different threads
and tasks.


### Translations::InsertNew

In the `Translations::InsertNew` function we get the request arguments and form
a BSON document for insertion.

@snippet samples/mongo_service/main.cpp  Mongo service sample - InsertNew

There are different ways to form a document, see @ref scripts/docs/en/userver/formats.md.


### Translations::ReturnDiff

MongoDB queries are just BSON documents. Each mongo document has an implicit
`_id` field that stores the document creation time. Knowing that, we can use
formats::bson::Oid::MakeMinimalFor() to find all the documents that were added
after `update_time`. Query sorts the documents by modification times (by `_id`),
so when the results are written into formats::json::ValueBuilder latter writes
rewrite previous data for the same key.

@snippet samples/mongo_service/main.cpp  Mongo service sample - ReturnDiff

See @ref scripts/docs/en/userver/mongodb.md for MongoDB hints and more usage samples.


### Static config

Static configuration of service is quite close to the configuration from
@ref scripts/docs/en/userver/tutorial/hello_service.md except for the handler and DB:

@snippet samples/mongo_service/static_config.yaml  Mongo service sample - static config

There are more static options for the MongoDB component configuration, all of
them are described at components::Mongo.


### int main()

Finally, we add our component to the
components::MinimalServerComponentList(),
and start the server with static configuration `kStaticConfig`.

@snippet samples/mongo_service/main.cpp  Mongo service sample - main


### Build and Run

To build the sample, execute the following build steps at the userver root directory:
```
mkdir build_release
cd build_release
cmake -DCMAKE_BUILD_TYPE=Release ..
make userver-samples-mongo_service
```

The sample could be started by running
`make start-userver-samples-mongo_service`. The command would invoke
@ref scripts/docs/en/userver/functional_testing.md "testsuite start target" that sets proper
paths in the configuration files, prepares and starts the DB, and starts the
service.

To start the service manually start the DB server and run
`./samples/mongo_service/userver-samples-mongo_service -c </path/to/static_config.yaml>`.

Now you can send a request to your service from another terminal:
```
bash
$ curl -X PATCH 'http://localhost:8090/v1/translations?key=hello&lang=ru&value=Привки'
$ curl -X PATCH 'http://localhost:8090/v1/translations?key=hello&lang=ru&value=Дратути'
$ curl -X PATCH 'http://localhost:8090/v1/translations?key=hello&lang=ru&value=Здрасьте'
$ curl -s http://localhost:8090/v1/translations?last_update=2021-11-01T12:00:00Z | jq
{
  "content": {
    "hello": {
      "ru": "Дратути"
    },
    "wellcome": {
      "ru": "Здрасьте"
    }
  },
  "update_time": "2021-12-20T10:17:37.249767773+00:00"
}
```

### Unit tests
@ref scripts/docs/en/userver/testing.md "Unit tests" for the service could be
implemented with one of UTEST macros in the following way:

@snippet samples/mongo_service/unittests/mongo_test.cpp  Unit test

### Functional testing
@ref scripts/docs/en/userver/functional_testing.md "Functional tests" for the service could be
implemented using the testsuite. To do that you have to:

* Turn on the pytest_userver.plugins.mongo plugin and provide Mongo settings
  info for the testsuite:
  @snippet samples/mongo_service/testsuite/conftest.py mongodb settings
  The pytest_userver.plugins.service.auto_client_deps() fixture
  already known about the mongodb fixture, so there's no need to override the
  extra_client_deps() fixture.

* Write the test:
  @snippet samples/mongo_service/testsuite/test_mongo.py  Functional test

## Full sources

See the full example:
* @ref samples/mongo_service/main.cpp
* @ref samples/mongo_service/static_config.yaml
* @ref samples/mongo_service/CMakeLists.txt
* @ref samples/mongo_service/testsuite/conftest.py
* @ref samples/mongo_service/testsuite/test_mongo.py

----------

@htmlonly <div class="bottom-nav"> @endhtmlonly
⇦ @ref scripts/docs/en/userver/tutorial/postgres_service.md | @ref scripts/docs/en/userver/tutorial/redis_service.md ⇨
@htmlonly </div> @endhtmlonly


@example samples/mongo_service/main.cpp
@example samples/mongo_service/static_config.yaml
@example samples/mongo_service/CMakeLists.txt
@example samples/mongo_service/testsuite/conftest.py
@example samples/mongo_service/testsuite/test_mongo.py
