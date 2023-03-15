#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <uuid/uuid.h>

#include <pthread.h>

#define PROBE_MESSAGE_BUFFER_SIZE 2048
#define USHRT_MAX 65536

typedef struct _receive_thread_parameter
{
    int* socket_for_probe;
    bool* b_loop;
}receive_thread_parameter;

void* receive_thread(void* p_param);

int main(int argc, char* argv[])
{
    int result;

    int socket_for_probe = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (0 >= socket_for_probe)
    {
        printf("socket error, code: %s", strerror(errno));
        exit(0);
    }

    struct timeval time_out = { 0 };
    time_out.tv_sec = 5;
    time_out.tv_usec = 0;
    result = setsockopt(socket_for_probe, SOL_SOCKET, SO_RCVTIMEO, (char*)&time_out, sizeof(struct timeval));
    if (0 != result)
    {
        printf("setsockopt error, code: %s", strerror(errno));
        exit(0);
    }

    struct sockaddr_in sockaddr_client;
    memset(&sockaddr_client, 0x0, sizeof(struct sockaddr_in));
    sockaddr_client.sin_addr.s_addr = htonl(INADDR_ANY);
    sockaddr_client.sin_family = AF_INET;
    sockaddr_client.sin_port = htons(0);

    result = bind(socket_for_probe, (struct sockaddr*)&sockaddr_client, sizeof(struct sockaddr_in));
    if (0 != result)
    {
        printf("bind error, code: %s", strerror(errno));
        exit(0);
    }

    uuid_t uuid;
    if (0 != uuid_generate_time_safe(uuid))
    {
        printf("uuid_generate_time_safe() error\n");
        exit(EXIT_SUCCESS);
    }
    char str_uuid[10] = { 0 };
    uuid_unparse_upper(uuid, str_uuid);


    char* p_probe_message = (char*)malloc(PROBE_MESSAGE_BUFFER_SIZE);
    if (NULL == p_probe_message)
    {
        printf("malloc() error");
        exit(0);
    }
    result = snprintf(
        p_probe_message,
        PROBE_MESSAGE_BUFFER_SIZE,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
        "<SOAP-ENV:Envelope "
        "xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
        "xmlns:SOAP-ENC=\"http://www.w3.org/2003/05/soap-encoding\" "
        "xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
        "xmlns:wsdd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\">"
        "<SOAP-ENV:Header>"
        "<wsa:MessageID>urn:uuid:%s</wsa:MessageID>"
        "<wsa:To SOAP-ENV:mustUnderstand=\"true\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To>"
        "<wsa:Action SOAP-ENV:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</wsa:Action>"
        "</SOAP-ENV:Header>"
        "<SOAP-ENV:Body>"
        "<wsdd:Probe></wsdd:Probe>"
        "</SOAP-ENV:Body>"
        "</SOAP-ENV:Envelope>", str_uuid);
    if (-1 == result)
    {
        printf("snprintf error: %s", strerror(errno));
        exit(0);
    }

    // result = snprintf(
    //     p_probe_message,
    //     PROBE_MESSAGE_BUFFER_SIZE,
    //     "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
    //     "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" "
    //     "xmlns:SOAP-ENC=\"http://www.w3.org/2003/05/soap-encoding\" "
    //     "xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
    //     "xmlns:wsdd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\">"
    //     "<SOAP-ENV:Header>"
    //     "<wsa:MessageID>urn:uuid:bc9fb550-1dd1-11b2-807c-c056e3fb5481</wsa:MessageID>"
    //     "<wsa:To SOAP-ENV:mustUnderstand=\"true\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To>"
    //     "<wsa:Action SOAP-ENV:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</wsa:Action>"
    //     "</SOAP-ENV:Header>"
    //     "<SOAP-ENV:Body>"
    //     "<wsdd:Probe><wsdd:Types>Device</wsdd:Types><wsdd:Scopes></wsdd:Scopes></wsdd:Probe>"
    //     "</SOAP-ENV:Body>"
    //     "</SOAP-ENV:Envelope>");
    // if (-1 == result)
    // {
    //     printf("snprintf error: %s", strerror(errno));
    //     exit(0);
    // }

    receive_thread_parameter parameter;
    bool loop = true;
    pthread_t thread_id;
    parameter.socket_for_probe = &socket_for_probe;
    parameter.b_loop = &loop;

    result = pthread_create(&thread_id, NULL, receive_thread, &parameter);
    if (0 != result)
    {
        printf("pthread_create error %s", strerror(errno));
        exit(0);
    }


    struct sockaddr_in sockaddr_multicast_addr_for_onvif;
    memset(&sockaddr_multicast_addr_for_onvif, 0x0, sizeof(struct sockaddr_in));
    inet_pton(AF_INET, "239.255.255.250", &sockaddr_multicast_addr_for_onvif.sin_addr.s_addr);
    sockaddr_multicast_addr_for_onvif.sin_family = AF_INET;
    sockaddr_multicast_addr_for_onvif.sin_port = htons(3702);

    result = sendto(socket_for_probe, p_probe_message, result, 0, (struct sockaddr*)&sockaddr_multicast_addr_for_onvif, sizeof(struct sockaddr_in));
    if (0 > result)
    {
        printf("sendto error %s", strerror(errno));
        exit(0);
    }

    sleep(5);

    loop = false;

    pthread_join(thread_id, NULL);

    free(p_probe_message);
    p_probe_message = NULL;

    result = close(socket_for_probe);
    if (0 != result)
    {
        printf("close error: %s", strerror(errno));
        exit(0);
    }

    return 0;
}

void* receive_thread(void* p_param)
{
    receive_thread_parameter* parameter = (receive_thread_parameter*)p_param;

    struct sockaddr_in received_from;
    int fromlen = sizeof(struct sockaddr_in);
    ssize_t bytesReceived;

    char* buffer = (char*)malloc(USHRT_MAX);

    while (*parameter->b_loop)
    {
        memset(buffer, 0x0, USHRT_MAX);

        bytesReceived = recvfrom((*parameter->socket_for_probe), buffer, USHRT_MAX, 0, (struct sockaddr*)&received_from, &fromlen);
        if (0 >= bytesReceived)
        {
            printf("recvfrom error: %s\n", strerror(errno));
        }
        else
        {
            printf("%s\n\n\n", buffer);
        }
    }

    free(buffer);
    buffer = NULL;

    return 0;
}
