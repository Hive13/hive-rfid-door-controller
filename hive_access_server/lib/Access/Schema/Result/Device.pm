use utf8;
package Access::Schema::Result::Device;

use strict;
use warnings;

use base 'DBIx::Class::Core';

__PACKAGE__->load_components('InflateColumn::DateTime');
__PACKAGE__->table('device');


__PACKAGE__->add_columns(
  'device_id',
  { data_type => 'uuid', is_nullable => 0, size => 16 },
  'key',
  { data_type => 'bytea', is_nullable => 0 },
  'name',
  { data_type => 'varchar', is_nullable => 0, size => 64 },
);

__PACKAGE__->has_many(
  'device_items',
  'Access::Schema::Result::DeviceItem',
  { 'foreign.device_id' => 'self.device_id' },
  { cascade_copy => 0, cascade_delete => 0 },
);

__PACKAGE__->set_primary_key('device_id');
__PACKAGE__->many_to_many('items', 'device_items', 'item');

1;
