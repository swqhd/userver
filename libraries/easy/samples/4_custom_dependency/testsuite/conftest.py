import pytest

from testsuite.databases.pgsql import discover

pytest_plugins = ['pytest_userver.plugins.postgresql']

USERVER_CONFIG_HOOKS = ['userver_actions_service']


@pytest.fixture(scope='session')
def userver_actions_service(mockserver_info):
    def do_patch(config_yaml, config_vars):
        components = config_yaml['components_manager']['components']
        components['action-client']['service-url'] = mockserver_info.url(
            '/v1/action',
        )

    return do_patch


@pytest.fixture(scope='session')
def pgsql_local(db_dump_schema_path, pgsql_local_create):
    databases = discover.find_schemas('admin', [db_dump_schema_path])
    return pgsql_local_create(list(databases.values()))
