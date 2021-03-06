#include "server.h"
#include "serverOptions.h"
#include "serverLogs.h"
#include "crypto/crypto.h"

/// <global instances>
SOAP_SOCKET g_soapSocket;
struct soap * g_soap = NULL;
/// </global instances>

/// <summary>
/// Prepare the "g_soap", to work with users.
/// </summary>
/// <param name = "port"> The server port </param>
void initServer()
{
  printf("Started initing the server.\n");
  g_soap = soap_new1(SOAP_C_UTFSTRING | SOAP_XML_INDENT | SOAP_ENC_SSL | SOAP_XML_TREE);//create the soap object
  soap_ssl_init(); 
  if (CRYPTO_thread_setup()) // OpenSSL
  {
    fprintf(stderr, "Cannot setup thread mutex\n");
    writeLogLine(error, "Cannot setup thread mutex");
    exit(1);
  } 

  soap_init(g_soap);

  if(soap_ssl_server_context(g_soap, SOAP_SSL_NO_AUTHENTICATION, NULL, NULL, NULL, NULL, NULL, NULL, NULL))//init SSL with no AUTHENTICATION
  {
    writeLogSoapErr(g_soap);
    printf("Cannot create SSL encryption.");
    exit(2);//exit because without ssl, cannot connect to HTTPS
  }

  g_soap->accept_timeout = g_serverOptions.u_acceptTimeout;//setting timeout
  g_soap->send_timeout = g_serverOptions.u_sendTimeout;//max socket inactivity time
  g_soap->recv_timeout = g_serverOptions.u_recvTimeout;//max socket inactivity time
  g_soapSocket = soap_bind(g_soap, NULL, g_serverOptions.u_port, 100);//bind the port

  printf("Server started on port: %ld.\n", g_serverOptions.u_port);
  writeLogLine(info, "Server started.");
}

/// <summary>
/// Shutdown server.
/// </summary> 
void closeServer()
{
  soap_done(g_soap);
  printf("Server closing.\n");
  writeLogLine(info, "Server closing.");
  soap_destroy(g_soap);//delete the GSOAP objects
  soap_end(g_soap);//release memory
  soap_free(g_soap);//release object from memory
  printf("Server closed.\n");
  writeLogLine(info, "Server closed.");
  CRYPTO_thread_cleanup(); // OpenSSL 
}

/// <summary>
/// Works in thread function.
/// </summary>
/// <param name = "argc"> The argument </param>
void * requestClient(void * argc)
{
  struct soap * m_soap = (struct soap *)argc;

  printf("User connected on %d port.\n", (m_soap->port));

  soap_serve(m_soap);//delete the GSOAP objects
  soap_destroy(m_soap);//release memory
  soap_end(m_soap);//release object from memory
  soap_free(m_soap);
  
  return NULL;
}


