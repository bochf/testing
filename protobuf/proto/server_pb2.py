# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: server.proto

import sys
_b=sys.version_info[0]<3 and (lambda x:x) or (lambda x:x.encode('latin1'))
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from google.protobuf import reflection as _reflection
from google.protobuf import symbol_database as _symbol_database
from google.protobuf import descriptor_pb2
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor.FileDescriptor(
  name='server.proto',
  package='ClioServerStatus',
  syntax='proto2',
  serialized_pb=_b('\n\x0cserver.proto\x12\x10\x43lioServerStatus\"\x99\x01\n\x0cServerStatus\x12\x11\n\thost_name\x18\x01 \x02(\t\x12\x12\n\nip_address\x18\x02 \x02(\t\x12\x37\n\x06status\x18\x03 \x02(\x0e\x32\'.ClioServerStatus.ServerStatus.E_STATUS\")\n\x08\x45_STATUS\x12\x06\n\x02UP\x10\x00\x12\x08\n\x04\x44OWN\x10\x01\x12\x0b\n\x07SUSPEND\x10\x02')
)
_sym_db.RegisterFileDescriptor(DESCRIPTOR)



_SERVERSTATUS_E_STATUS = _descriptor.EnumDescriptor(
  name='E_STATUS',
  full_name='ClioServerStatus.ServerStatus.E_STATUS',
  filename=None,
  file=DESCRIPTOR,
  values=[
    _descriptor.EnumValueDescriptor(
      name='UP', index=0, number=0,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='DOWN', index=1, number=1,
      options=None,
      type=None),
    _descriptor.EnumValueDescriptor(
      name='SUSPEND', index=2, number=2,
      options=None,
      type=None),
  ],
  containing_type=None,
  options=None,
  serialized_start=147,
  serialized_end=188,
)
_sym_db.RegisterEnumDescriptor(_SERVERSTATUS_E_STATUS)


_SERVERSTATUS = _descriptor.Descriptor(
  name='ServerStatus',
  full_name='ClioServerStatus.ServerStatus',
  filename=None,
  file=DESCRIPTOR,
  containing_type=None,
  fields=[
    _descriptor.FieldDescriptor(
      name='host_name', full_name='ClioServerStatus.ServerStatus.host_name', index=0,
      number=1, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='ip_address', full_name='ClioServerStatus.ServerStatus.ip_address', index=1,
      number=2, type=9, cpp_type=9, label=2,
      has_default_value=False, default_value=_b("").decode('utf-8'),
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
    _descriptor.FieldDescriptor(
      name='status', full_name='ClioServerStatus.ServerStatus.status', index=2,
      number=3, type=14, cpp_type=8, label=2,
      has_default_value=False, default_value=0,
      message_type=None, enum_type=None, containing_type=None,
      is_extension=False, extension_scope=None,
      options=None),
  ],
  extensions=[
  ],
  nested_types=[],
  enum_types=[
    _SERVERSTATUS_E_STATUS,
  ],
  options=None,
  is_extendable=False,
  syntax='proto2',
  extension_ranges=[],
  oneofs=[
  ],
  serialized_start=35,
  serialized_end=188,
)

_SERVERSTATUS.fields_by_name['status'].enum_type = _SERVERSTATUS_E_STATUS
_SERVERSTATUS_E_STATUS.containing_type = _SERVERSTATUS
DESCRIPTOR.message_types_by_name['ServerStatus'] = _SERVERSTATUS

ServerStatus = _reflection.GeneratedProtocolMessageType('ServerStatus', (_message.Message,), dict(
  DESCRIPTOR = _SERVERSTATUS,
  __module__ = 'server_pb2'
  # @@protoc_insertion_point(class_scope:ClioServerStatus.ServerStatus)
  ))
_sym_db.RegisterMessage(ServerStatus)


# @@protoc_insertion_point(module_scope)
