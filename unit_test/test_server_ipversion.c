#include "test_framework.h"
#include "server.h"
#include "tls.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#define d_ipversion_test_cert_file (TLS_TEST_CERTS_DIR "/server.crt")
#define d_ipversion_test_ca_file (TLS_TEST_CERTS_DIR "/ca.crt")
/* f_socket_create_server only speaks IPv4 (struct sockaddr_in), so an IPv6-only listener for these
 * tests has to be built by hand here, with plain socket()/bind()/listen(). No accept() thread is
 * needed anywhere in this file: once listen() has run, a connect() against that port completes at
 * the TCP level purely from the kernel's backlog queue, whether or not anyone ever calls accept(). */
static unsigned short start_ipv6_only_listener(int *out_descriptor) {
  unsigned short result = 0;
  int descriptor = socket(AF_INET6, SOCK_STREAM, 0);
  if (descriptor >= 0) {
    struct sockaddr_in6 address;
    memset(&address, 0, sizeof(address));
    address.sin6_family = AF_INET6;
    address.sin6_port = 0;
    inet_pton(AF_INET6, "::1", &address.sin6_addr);
    if ((bind(descriptor, (struct sockaddr *) &address, sizeof(address)) == 0) && (listen(descriptor, 8) == 0)) {
      struct sockaddr_in6 actual;
      socklen_t actual_length = sizeof(actual);
      if (getsockname(descriptor, (struct sockaddr *) &actual, &actual_length) == 0)
        result = ntohs(actual.sin6_port);
    }
    if (result == 0) {
      close(descriptor);
      descriptor = -1;
    }
  }
  *out_descriptor = descriptor;
  return result;
}
static unsigned short start_ipv4_only_listener(int *out_descriptor) {
  struct sockaddr_in configuration;
  if (f_socket_create_server(0, "127.0.0.1", 8, out_descriptor, &configuration) != NOICE)
    return 0;
  struct sockaddr_in actual;
  socklen_t actual_length = sizeof(actual);
  getsockname(*out_descriptor, (struct sockaddr *) &actual, &actual_length);
  return ntohs(actual.sin_port);
}
void run_test_server_ipversion_suite(void) {
  SUITE_BEGIN("server -- f_socket_create_client IPv4/IPv6 family selection");

  /* ---- the headline use case: "localhost" is a genuinely dual-stack hostname on this machine */
  /* (/etc/hosts has both a 127.0.0.1 and an ::1 entry) -- forcing IPv4 (IPv4=true, IPv6=false) must */
  /* make getaddrinfo return ONLY the 127.0.0.1 record, so a client asking for "localhost" with that */
  /* flag combination reaches an IPv4-only listener and is refused by an IPv6-only one, even though */
  /* both are reachable under the unrestricted default. ---- */
  {
    int ipv4_descriptor, ipv6_descriptor;
    unsigned short ipv4_port = start_ipv4_only_listener(&ipv4_descriptor);
    unsigned short ipv6_port = start_ipv6_only_listener(&ipv6_descriptor);
    CHECK((ipv4_port != 0) && (ipv6_port != 0), "test setup: both an IPv4-only and an IPv6-only loopback listener start correctly");
    int client_descriptor = -1;
    coremio_result rc = f_socket_create_client(ipv4_port, "localhost", &client_descriptor, 2000, true, false);
    CHECK(rc == NOICE, "forcing IPv4 (IPv4=true, IPv6=false) against \"localhost\" successfully reaches the IPv4-only listener");
    if (client_descriptor >= 0)
      f_socket_close(&client_descriptor);
    client_descriptor = -1;
    rc = f_socket_create_client(ipv6_port, "localhost", &client_descriptor, 2000, true, false);
    CHECK(rc != NOICE, "forcing IPv4 against \"localhost\" correctly FAILS to reach the IPv6-only listener's port -- it never tries the ::1 record "
        "at all, even though \"localhost\" does resolve to it");
    if (client_descriptor >= 0)
      f_socket_close(&client_descriptor);
    close(ipv4_descriptor);
    close(ipv6_descriptor);
  }

  /* ---- the mirror image: forcing IPv6 (IPv4=false, IPv6=true) must reach the IPv6-only listener */
  /* and be refused by the IPv4-only one. ---- */
  {
    int ipv4_descriptor, ipv6_descriptor;
    unsigned short ipv4_port = start_ipv4_only_listener(&ipv4_descriptor);
    unsigned short ipv6_port = start_ipv6_only_listener(&ipv6_descriptor);
    int client_descriptor = -1;
    coremio_result rc = f_socket_create_client(ipv6_port, "localhost", &client_descriptor, 2000, false, true);
    CHECK(rc == NOICE, "forcing IPv6 (IPv4=false, IPv6=true) against \"localhost\" successfully reaches the IPv6-only listener");
    if (client_descriptor >= 0)
      f_socket_close(&client_descriptor);
    client_descriptor = -1;
    rc = f_socket_create_client(ipv4_port, "localhost", &client_descriptor, 2000, false, true);
    CHECK(rc != NOICE, "forcing IPv6 against \"localhost\" correctly FAILS to reach the IPv4-only listener's port -- it never tries the "
        "127.0.0.1 record at all");
    if (client_descriptor >= 0)
      f_socket_close(&client_descriptor);
    close(ipv4_descriptor);
    close(ipv6_descriptor);
  }

  /* ---- regression: leaving both flags unrestricted (true, true -- or the equivalent false, false) */
  /* must keep the ORIGINAL, pre-flag behavior: try every family getaddrinfo returns, so a dual-stack */
  /* hostname reaches either an IPv4-only or an IPv6-only listener exactly as before this feature was */
  /* added. ---- */
  {
    int ipv4_descriptor, ipv6_descriptor;
    unsigned short ipv4_port = start_ipv4_only_listener(&ipv4_descriptor);
    unsigned short ipv6_port = start_ipv6_only_listener(&ipv6_descriptor);
    int client_descriptor = -1;
    coremio_result rc = f_socket_create_client(ipv4_port, "localhost", &client_descriptor, 2000, true, true);
    CHECK(rc == NOICE, "REGRESSION: with both flags true (unrestricted), \"localhost\" still reaches an IPv4-only listener, exactly like before "
        "this feature existed");
    if (client_descriptor >= 0)
      f_socket_close(&client_descriptor);
    client_descriptor = -1;
    rc = f_socket_create_client(ipv6_port, "localhost", &client_descriptor, 2000, true, true);
    CHECK(rc == NOICE, "REGRESSION: with both flags true (unrestricted), \"localhost\" still reaches an IPv6-only listener too");
    if (client_descriptor >= 0)
      f_socket_close(&client_descriptor);
    client_descriptor = -1;
    rc = f_socket_create_client(ipv4_port, "localhost", &client_descriptor, 2000, false, false);
    CHECK(rc == NOICE, "with both flags false, the (!IPv4 && IPv6) / (IPv4 && !IPv6) checks both fall through to AF_UNSPEC too, so this behaves "
        "the same as both-true -- also unrestricted, not a lockout");
    if (client_descriptor >= 0)
      f_socket_close(&client_descriptor);
    close(ipv4_descriptor);
    close(ipv6_descriptor);
  }

  /* ---- literal-address edge case, asymmetric by design of the underlying resolver (not a coremio */
  /* bug): forcing IPv4 against a literal IPv6 address ("::1") fails cleanly at getaddrinfo itself */
  /* (EAI_NONAME -- there is no IPv4 representation to synthesize), surfacing as */
  /* SHIT_SOCKET_GET_ATTRIBUTES before any socket is even created. ---- */
  {
    int client_descriptor = -1;
    coremio_result rc = f_socket_create_client(80, "::1", &client_descriptor, 500, true, false);
    CHECK(rc == SHIT_SOCKET_GET_ATTRIBUTES, "forcing IPv4 against the literal IPv6 address \"::1\" fails cleanly at the resolver step "
        "(SHIT_SOCKET_GET_ATTRIBUTES), since there is no IPv4 form of that address to fall back to");
    CHECK(client_descriptor == -1, "and no descriptor is left allocated on that failure");
  }

  /* ---- NOTE, not a failure: forcing IPv6 against a literal IPv4 address ("127.0.0.1") does NOT */
  /* fail the same way -- macOS's resolver synthesizes an IPv4-mapped IPv6 address */
  /* (::ffff:127.0.0.1) for an AF_INET6 hint, so getaddrinfo succeeds. Recorded here so the asymmetry */
  /* is visible and intentional-looking, not a surprise discovered later. Whether the resulting */
  /* v4-mapped connection attempt itself then succeeds depends on the destination; this check only */
  /* confirms the resolution step itself does not reject it the way the reverse direction does. ---- */
  {
    struct addrinfo hints, *resolved = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    int rc = getaddrinfo("127.0.0.1", "80", &hints, &resolved);
    CHECK((rc == 0) && (resolved) && (resolved->ai_family == AF_INET6),
        "ASYMMETRY NOTE (resolver-level, not a coremio bug): unlike the ::1 + force-IPv4 case above, a literal IPv4 address with an IPv6 family "
        "hint resolves successfully via IPv4-mapped-address synthesis (::ffff:127.0.0.1), rather than failing -- so \"force IPv6\" does not "
        "strictly guarantee a literal IPv4 input gets rejected the way \"force IPv4\" rejects a literal IPv6 input");
    if (resolved)
      freeaddrinfo(resolved);
  }

  /* ---- the TLS wrapper forwards IPv4/IPv6 exactly the same way: forcing IPv4 still reaches a real */
  /* TLS server bound to 127.0.0.1, and forcing IPv6 against that same literal address fails at the */
  /* underlying f_socket_create_client call before any TLS handshake is attempted. ---- */
  {
    s_client_socket_SSL client;
    coremio_result rc = f_socket_create_client_ssl(1, "127.0.0.1", &client, d_ipversion_test_ca_file, 500, false, true);
    CHECK(rc == SHIT_SOCKET_CONNECT, "f_socket_create_client_ssl correctly forwards IPv4=false/IPv6=true down to f_socket_create_client -- "
        "forcing IPv6 against the literal address \"127.0.0.1\" with nothing listening fails at the plain socket layer "
        "(SHIT_SOCKET_CONNECT via IPv4-mapped synthesis), never reaching the TLS handshake");
    CHECK((client.context == NULL) && (client.session == NULL), "and no TLS state is allocated on that early failure");
  }

  SUITE_END();
}
