import networking  # type: ignore

networking.net_request("example.com", 80, "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n")
