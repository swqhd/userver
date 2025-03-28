import json

import pytest

pytest_plugins = ['pytest_userver.plugins.kafka']


@pytest.fixture(scope='session')
def kafka_local(_patched_bootstrap_servers_internal):
    return _patched_bootstrap_servers_internal


@pytest.fixture(scope='session')
def kafka_components():
    return ['kafka-consumer', 'kafka-producer']


@pytest.fixture(scope='session')
def service_env(kafka_secdist):
    return {'SECDIST_CONFIG': json.dumps(kafka_secdist)}
