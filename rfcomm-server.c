#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

sdp_session_t *register_service()
{
  uint32_t service_uuid_int[] = { 0, 0, 0, 0xABCD};
  uint8_t rfcomm_channel = 11;
  const char *service_name = "Roto-Rooter Data Router";
  const char *service_dsc = "An experimental plumbing router";
  const char *service_prov = "Roto-Rooter";

  uuid_t root_uuid, l2cap_uuid, rfcomm_uuid, svc_uuid;
  sdp_list_t *l2cap_list = 0,
  *rfcomm_list = 0,
  *root_list = 0,
  *proto_list = 0,
  *access_proto_list = 0;
  sdp_data_t *channel = 0, *psm = 0;

  sdp_record_t *record = sdp_record_alloc();

  // set the general service ID
  sdp_uuid128_create( &svc_uuid, &service_uuid_int );
  sdp_set_service_id( record, svc_uuid );

  sdp_list_t service_class = {NULL, &svc_uuid};
  sdp_set_service_classes(record, &service_class);

  // make the service record publicly browsable
  sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
  root_list = sdp_list_append(0, &root_uuid);
  sdp_set_browse_groups( record, root_list );

  // set l2cap information
  sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
  l2cap_list = sdp_list_append( 0, &l2cap_uuid );
  proto_list = sdp_list_append( 0, l2cap_list );

  // set rfcomm information
  sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
  channel = sdp_data_alloc(SDP_UINT8, &rfcomm_channel);
  rfcomm_list = sdp_list_append( 0, &rfcomm_uuid );
  sdp_list_append( rfcomm_list, channel );
  sdp_list_append( proto_list, rfcomm_list );

  // attach protocol information to service record
  access_proto_list = sdp_list_append( 0, proto_list );
  sdp_set_access_protos( record, access_proto_list );

  // set the name, provider, and description
  sdp_set_info_attr(record, service_name, service_prov, service_dsc);
  int err = 0;
  sdp_session_t *session = 0;

  // connect to the local SDP server, register the service record, and
  // disconnect
  session = sdp_connect( BDADDR_ANY, BDADDR_LOCAL, SDP_RETRY_IF_BUSY );
  err = sdp_record_register(session, record, 0);
  // cleanup
  sdp_data_free( channel );
  sdp_list_free( l2cap_list, 0 );
  sdp_list_free( rfcomm_list, 0 );
  sdp_list_free( root_list, 0 );
  sdp_list_free( access_proto_list, 0 );

  return session;
}
int main(int argc, char **argv)
{
  struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
  char buf[1024] = { 0 };
  int s, client, bytes_read;
  socklen_t opt = sizeof(rem_addr);
  sdp_session_t *sss;

  // allocate socket
  s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  // bind socket to port 1 of the first available
  // local bluetooth adapter
  loc_addr.rc_family = AF_BLUETOOTH;
  loc_addr.rc_bdaddr = *BDADDR_ANY;
  loc_addr.rc_channel = (uint8_t) 0;
  bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));

  // put socket into listening mode
  listen(s, 1);
  sss = register_service();
  puts("done");
  // accept one connection
  client = accept(s, (struct sockaddr *)&rem_addr, &opt);

  ba2str( &rem_addr.rc_bdaddr, buf );
  memset(buf, 0, sizeof(buf));

  // read data from the client
  while(1) {
    bytes_read = read(client, buf, sizeof(buf));
    if( bytes_read > 0 ) {
      printf("%s\n", buf);
      fflush(stdout);
    }
  }

  // close connection
  close(client);
  sdp_close(sss);
  close(s);
  return 0;
}
