#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#endif

#include <Python.h>
#include <structmember.h>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

EXPORT PyObject * net_request(PyObject *self, PyObject *args)
{
    PyGILState_STATE gstate = PyGILState_Ensure();  // Ensure the thread has a valid state

    const char *host;
    int port;
    const char *command;
    int sts = -1;
    int error_flag = 0;
    const char *error_msg = NULL;

    if (!PyArg_ParseTuple(args, "sis", &host, &port, &command)) {
        PyGILState_Release(gstate);
        return NULL;
    }
    
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        error_flag = 1;
        error_msg = "WSAStartup failed";
        sts = -1;
        goto cleanup;
    }
    
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);
    int iResult = getaddrinfo(host, port_str, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        error_flag = 1;
        error_msg = "getaddrinfo failed";
        sts = -1;
        goto cleanup;
    }
    
    SOCKET ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        error_flag = 1;
        error_msg = "socket creation failed";
        sts = -1;
        goto cleanup;
    }
    
    ptr = result;
    iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    freeaddrinfo(result);
    if (iResult == SOCKET_ERROR || ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        if (ConnectSocket != INVALID_SOCKET)
            closesocket(ConnectSocket);
        WSACleanup();
        error_flag = 1;
        error_msg = "Unable to connect to server";
        sts = -1;
        goto cleanup;
    }
    
    #define DEFAULT_BUFLEN 512
    int recvbuflen = DEFAULT_BUFLEN;
    const char *sendbuf = command;
    char recvbuf[DEFAULT_BUFLEN];
    
    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        error_flag = 1;
        error_msg = "send failed";
        sts = -1;
        goto cleanup;
    }
    printf("Bytes Sent: %d\n", iResult);
    
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        error_flag = 1;
        error_msg = "shutdown failed";
        sts = -1;
        goto cleanup;
    }
    
    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);
            printf("Received: %s\n", recvbuf); 
        } else if (iResult == 0) {
            printf("Connection closed\n");
        } else {
            printf("recv failed: %d\n", WSAGetLastError());
        }
    } while (iResult > 0);
    
    // (Optionally, you might check for recv errors here)
    
    closesocket(ConnectSocket);
    WSACleanup();
    sts = 0;  // success

#endif  // _WIN32

cleanup:
    if (error_flag) {
        PyErr_SetString(PyExc_RuntimeError, error_msg);
    }
    PyGILState_Release(gstate);
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