#include "logserver.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define PACKET_TYPE_LOG 0x01
#define PACKET_END_SYMBOL 0x11

struct logserver_client {
  int sockfd;
  struct sockaddr addr;
  socklen_t addrlen;
  struct logserver_client *next;
  struct logserver_client *prev;
};


struct logserver_client *logclients = NULL;
pthread_mutex_t logclients_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_t logserver_client_accepter_thread;

void cleanup_client_accepter(void *args) {
#ifdef DEBUG
  printf("Closing socket...\n");
#endif
  int serverfd = *((int*)args);
  close(serverfd);
  free(args);
}

void *logserver_client_accepter(void *args) {
  struct sockaddr_in client_addr;
  socklen_t addrlen = sizeof(client_addr);
  int serverfd, clientfd;
  /* Setup cleanup func (will be called on cancel) */
  pthread_cleanup_push(cleanup_client_accepter, args);
  serverfd = *((int*)args);
#ifdef DEBUG
  printf("Waiting for client...\n");
#endif
  
  while((clientfd = accept(serverfd, (struct sockaddr*)&client_addr, &addrlen)) != -1) {
#ifdef DEBUG
    printf("Accepted client\n");
#endif
    struct logserver_client *client = malloc(sizeof(struct logserver_client));
    if(client != NULL) {
      if(pthread_mutex_lock(&logclients_mtx) == 0) {
        client->sockfd = clientfd;
        memcpy(&client->addr, &client_addr, sizeof(struct sockaddr));
        client->addrlen = addrlen;
        client->next = logclients;
        if(client->next != NULL) {
          client->next->prev = client;
        }
        client->prev = NULL;
        logclients = client;
        pthread_mutex_unlock(&logclients_mtx);
      }
    }
  }
  fprintf(stderr, "ERROR\n");
  fprintf(stderr, "Error: %s\n", strerror(errno));
  /* Execute cleanup func */
  pthread_cleanup_pop(1);
  return NULL;
}

int logserver_start(void) {
  int sockfd, n;
  struct addrinfo hints, *res, *ressave;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  if((n = getaddrinfo(NULL, LOGSERVER_PORT, &hints, &res)) != 0) {
    fprintf(stderr, "Error getting addrinfo: %s\n", gai_strerror(n));
    goto err_getaddrinfo;
  }
  ressave = res;
  
  do {
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    
    if(sockfd == -1) {
      continue; /* Try next addr */
    }

    if(bind(sockfd, res->ai_addr, res->ai_addrlen) != -1) {
      break; /* Success */
    }

    close(sockfd);
  } while((res = res->ai_next) != NULL);
  
  if(res == NULL) {
    /* No addr succeeded */
    fprintf(stderr, "Could not create listener socket\n");
    freeaddrinfo(ressave);
    goto err_nosocket;
  }
  freeaddrinfo(ressave);
  
  /* Start listener thread */
  if(listen(sockfd, 8) == -1) {
    fprintf(stderr, "Could not listen on socket: %s\n", strerror(errno));
    goto err_listen;
  }
  int *sockfd_val = malloc(sizeof(int));
  if(sockfd_val == NULL) {
    fprintf(stderr, "Error allocating sockfd copy\n");
    goto err_sockfd_alloc;
  }
  *sockfd_val = sockfd;
  if(pthread_create(&logserver_client_accepter_thread, NULL, logserver_client_accepter, sockfd_val) != 0) {
    fprintf(stderr, "Error creating logserver accepter thread\n");
    goto err_accepter_thread;
  }
  return 0;
  
 err_accepter_thread:
  free(sockfd_val);
 err_sockfd_alloc:
 err_listen:
  close(sockfd);
 err_nosocket:
 err_getaddrinfo:
  return -1;
}

void logserver_cleanup(void) {
  if(pthread_mutex_lock(&logclients_mtx) == 0) {
    /* Release */
    struct logserver_client *next, *client = logclients;
    while(client != NULL) {
      next = client->next;
      close(client->sockfd);
      free(client);
      client = next;
    }
    pthread_mutex_unlock(&logclients_mtx);
  }
  pthread_cancel(logserver_client_accepter_thread);
}


static void broadcast_msg(const char *packet, int len) {
  if(pthread_mutex_lock(&logclients_mtx) == 0) {
    struct logserver_client *next, *client = logclients;
    while(client != NULL) {
      next = client->next;
      if(send(client->sockfd, packet, len, MSG_NOSIGNAL) == -1) {
        fprintf(stderr, "Could not send log to client: %s\n", strerror(errno));
        close(client->sockfd);
        /* Remove client from list */
        if(client->next != NULL) {
          client->next->prev = client->prev;
        }
        if(client->prev != NULL) {
          client->prev->next = client->next;
        } else {
          /* First element */
          logclients = client->next;
        }
        free(client);
      }
      client = next;
    }
    pthread_mutex_unlock(&logclients_mtx);
  }
}

void broadcast_log(struct log_entry *entry) {
  static char packet[50];

  char *packet_ptr = packet;
  *packet_ptr = (char)PACKET_TYPE_LOG;
  packet_ptr++;
  *((uint32_t*)packet_ptr) = htonl((uint32_t)(entry->temp*100.0));
  packet_ptr += 4;
  *((uint32_t*)packet_ptr) = htonl((uint32_t)(entry->knop_val*100.0));
  packet_ptr += 4;
  *((uint32_t*)packet_ptr) = htonl(entry->time_ms);
  packet_ptr += 4;
  *packet_ptr = (char)entry->pop;
  packet_ptr++;
  *packet_ptr = (char)PACKET_END_SYMBOL;

  broadcast_msg(packet, 15);
}
