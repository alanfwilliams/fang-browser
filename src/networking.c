#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

#include <Python.h>
#include <structmember.h>
#include <moduleobject.h>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

EXPORT PyObject * net_request(PyObject *self, PyObject *args)
{
    const char *host;
    int port;
    const char *command;
    int sock;
    int sts;

    if (!PyArg_ParseTuple(args, "si", &host, &port, &command))
        return NULL;
    
    Py_BEGIN_ALLOW_THREADS
    // initialize the network API on Windows
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif

    // create a socket on Windows
    // taken from https://learn.microsoft.com/en-us/windows/win32/winsock/creating-a-socket-for-the-client
    #ifdef _WIN32
    int iResult;
    struct addrinfo *result = NULL,
                *ptr = NULL,
                hints;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    #define DEFAULT_PORT "80"

    // Resolve the server address and port
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);
    iResult = getaddrinfo(host, port_str, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        PyErr_SetString(PyExc_RuntimeError, "getaddrinfo failed");
        return NULL;
    }

    SOCKET ConnectSocket = INVALID_SOCKET;

    // Attempt to connect to the first address returned by
    // the call to getaddrinfo
    ptr=result;

    // Create a SOCKET for connecting to server
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
        ptr->ai_protocol);
    
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        PyErr_SetString(PyExc_RuntimeError, "socket creation failed");
        return NULL;
    }
    // Connect to server.
    iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET; 
    }

    // Should really try the next address returned by getaddrinfo
    // if the connect call failed
    // But for this simple example we just free the resources
    // returned by getaddrinfo and print an error message

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        PyErr_SetString(PyExc_RuntimeError, "Unable to connect to server");
        return NULL;
    }

    #define DEFAULT_BUFLEN 512

    int recvbuflen = DEFAULT_BUFLEN;

    const char *sendbuf = command;
    char recvbuf[DEFAULT_BUFLEN];

    // Send an initial buffer
    iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        PyErr_SetString(PyExc_RuntimeError, "send failed");
        return NULL;
    }

    printf("Bytes Sent: %d\n", iResult);

    // shutdown the connection for sending since no more data will be sent
    // the client can still use the ConnectSocket for receiving data
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        PyErr_SetString(PyExc_RuntimeError, "shutdown failed");
        return NULL;
    }

    // Receive data until the server closes the connection
    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
            printf("Bytes received: %d\n", iResult);
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed: %d\n", WSAGetLastError());
    } while (iResult > 0);
    // shutdown the send half of the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        PyErr_SetString(PyExc_RuntimeError, "shutdown failed");
        return NULL;
    }
    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
    #else

    // create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return NULL;
    }
    // connect to the server
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(host);
    sts = connect(sock, (struct sockaddr *)&server, sizeof(server));
    if (sts < 0) {
        return NULL;
    }
    #endif
    Py_END_ALLOW_THREADS
    return PyLong_FromLong(sts);
}

// Array of methods exposed by the module
static PyMethodDef NetworkingMethods[] = {
    {"net_request", net_request, METH_VARARGS, "Make a network request."},
    {NULL, NULL, 0, NULL} // Sentinel
};

// Module definition structure
static struct PyModuleDef networkingmodule = {
    PyModuleDef_HEAD_INIT,
    "networking",             // Name of the module
    "A module for making network requests", // Module documentation
    -1,                       // Size of per-interpreter state of the module,
                              // or -1 if the module keeps state in global variables.
    NetworkingMethods         // The methods defined in the module
};

// Module initialization function
PyMODINIT_FUNC PyInit_networking(void) {
    return PyModule_Create(&networkingmodule);
}