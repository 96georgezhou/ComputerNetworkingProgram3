#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

const int BUFFSIZE = 1500;
const int NUM_CONNECTIONS = 5;

enum machineType
{
    CLIENT,
    SERVER

} machine;

bool verbose = false;
bool stayAlive = false;

/**
 * main function
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[])
{
  if (argc == 2)
  {
    machine = SERVER;
    if (verbose)
    {
      cout << "Running as server." << endl;
    }
  }
  else if (argc == 3)
  {
    machine = CLIENT;
    if (verbose)
    {
      cout << "Running as client." << endl;
    }
  }
  else if (argc < 2 || argc > 3)
  {
    cerr << "usage:" << endl;
    cerr << "server invocation: ./hw3 ipPort" << endl;
    cerr << "client invocation: ./hw3 ipPort ipName" << endl;
    return -1;
  }
  // check if  the port number is valid
  if (atoi(argv[1]) < 1024 || atoi(argv[1]) > 65536)
  {
    cerr << "Cannot bind the local address to the server socket." << endl;
    return -1;
  }

  int serverPort = atoi(argv[1]);
  const char *server = argv[2];

  char databuf[BUFFSIZE];
  bzero(databuf, BUFFSIZE);

  // Client side
  if (machine == CLIENT)
  {
    struct hostent *host = gethostbyname(server);
    if (host == NULL)
    {
      cerr << "Cannot find hostname." << endl;
      return -1;
    }

    sockaddr_in sendSocketAddress;
    bzero((char *)&sendSocketAddress, sizeof(sendSocketAddress));
    sendSocketAddress.sin_family = AF_INET;     // Address Family: Internet
    sendSocketAddress.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*) (*host->h_addr_list)));
    sendSocketAddress.sin_port = htons(serverPort);

    // Tcp socket
    int clientSD = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSD < 0)
    {
      cerr << "Could not open socket." << endl;
      close(clientSD);
      return -1;
    }
    // connect to server
    int returnCode = connect(clientSD, (sockaddr *)&sendSocketAddress, sizeof(sendSocketAddress));
    if (returnCode < 0) // If the returnCode is negative, this will indicates failure
    {
      cerr << "Connection failed." << endl;
      close(clientSD);
      return -1;
    }
    // send 10 bytes and receive acknowledgement
    int bytesWritten = write(clientSD, databuf, 10);
    if (verbose)
    {
      cout << "bytesWritten: " << bytesWritten << endl;
    }
    // receive 10 bytes and send ACK
    int bytesRead = read(clientSD, databuf, BUFFSIZE);
    if (verbose)
    {
      cout << "bytesRead: " << bytesRead << endl;
    }
    // send 1450 and receive ACK
    bytesWritten = write(clientSD, databuf, 1450);
    if (verbose)
    {
      cout << "bytesWritten: " << bytesWritten << endl;
    }
    // send FIN and tear down connection
    shutdown(clientSD, SHUT_RDWR);
    close(clientSD);
  }
  // Server side
  if (machine == SERVER)
  {
    // receiving sockets
    sockaddr_in acceptSocketAddress;
    bzero((char *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    acceptSocketAddress.sin_family = AF_INET;   // Address Family:  Internet
    acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    acceptSocketAddress.sin_port = htons(serverPort);
    // open tcp
    int serverSD = socket(AF_INET, SOCK_STREAM, 0);
    const int on = 1;
    setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
    // tcp socket
    int returnCode = bind(serverSD, (sockaddr *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    if (returnCode < 0) // failed
    {
      cerr << "Bind failed." << endl;
      close(serverSD);
      return -1;
    }
    // listen
    listen(serverSD, NUM_CONNECTIONS);

    do
    {
      // wait for client connection
      sockaddr_in newSockAddr;
      socklen_t newSockAddrSize = sizeof(newSockAddr);
      int newSD = accept(serverSD, (sockaddr *) &newSockAddr, &newSockAddrSize);
      // start new multithread copy
      if (fork() == 0)
      {
        close(serverSD);
        // receive 10 bytes and send ACK
        read(newSD, databuf, BUFFSIZE);
        // send 10 bytes and receive ACK
        write(newSD, databuf, 10);
        // send FIN however still are allowed to read from connection
        shutdown(newSD, SHUT_WR);
        // receive 1450 bytes and send ACK
        read(newSD, databuf, BUFFSIZE);
        // tera down tcp connection
        close(newSD);
        exit(0);
      }
      else
      {
        close(newSD);
      }
    } while (stayAlive);

    close(serverSD);
  }
  return 0;
}