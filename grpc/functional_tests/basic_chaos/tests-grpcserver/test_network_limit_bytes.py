import pytest
import requests_server


@pytest.mark.parametrize('case', requests_server.ALL_CASES)
async def test_network_limit_bytes(grpc_client, gate, case):
    for i in (50, 100, 150):
        gate.to_client_limit_bytes(i)
        gate.to_server_limit_bytes(i)
        await requests_server.check_unavailable_for(case, grpc_client, gate)

    gate.to_client_pass()
    gate.to_server_pass()
    await requests_server.check_ok_for(case, grpc_client, gate)
