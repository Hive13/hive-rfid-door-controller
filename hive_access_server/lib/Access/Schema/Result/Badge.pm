use utf8;
package Access::Schema::Result::Badge;

use strict;
use warnings;

use base 'DBIx::Class::Core';

__PACKAGE__->load_components(qw/ InflateColumn::DateTime UUIDColumns Core /);
__PACKAGE__->table('badge');
__PACKAGE__->add_columns(
  'badge_id',
  { data_type => 'uuid', is_nullable => 0, size => 16 },
  'badge_number',
  { data_type => 'integer', is_nullable => 0 },
  'member_id',
  { data_type => 'uuid', is_foreign_key => 1, is_nullable => 0, size => 16 },
);

__PACKAGE__->uuid_columns(qw/badge_id member_id/);
__PACKAGE__->set_primary_key('badge_id');
__PACKAGE__->belongs_to(
  'member',
  'Access::Schema::Result::Member',
  { member_id => 'member_id' },
  { is_deferrable => 0, on_delete => 'RESTRICT', on_update => 'RESTRICT' },
);

1;
