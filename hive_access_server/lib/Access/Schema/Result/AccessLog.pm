use utf8;
package Access::Schema::Result::AccessLog;

use strict;
use warnings;

use base 'DBIx::Class::Core';

__PACKAGE__->load_components(qw/InflateColumn::DateTime UUIDColumns/);
__PACKAGE__->table("access_log");

__PACKAGE__->add_columns(
  "access_id",
  { data_type => "uuid", is_nullable => 0, size => 16 },
  "access_time",
  { data_type => "timestamp with time zone", is_nullable => 0 },
  "item_id",
  { data_type => "uuid", is_foreign_key => 1, is_nullable => 0, size => 16 },
  "member_id",
  { data_type => "uuid", is_foreign_key => 1, is_nullable => 0, size => 16 },
	"granted",
	{ data_type => "boolean", is_boolean => 1 },
);

__PACKAGE__->uuid_columns("access_id");
__PACKAGE__->set_primary_key("access_id");
__PACKAGE__->belongs_to(
  "item",
  "Access::Schema::Result::Item",
  { item_id => "item_id" },
  { is_deferrable => 0, on_delete => "RESTRICT", on_update => "RESTRICT" },
);
__PACKAGE__->belongs_to(
  "member",
  "Access::Schema::Result::Member",
  { member_id => "member_id" },
  { is_deferrable => 0, on_delete => "RESTRICT", on_update => "RESTRICT" },
);

1;
