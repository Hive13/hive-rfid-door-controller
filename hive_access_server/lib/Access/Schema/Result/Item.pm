use utf8;
package Access::Schema::Result::Item;

use strict;
use warnings;

use base 'DBIx::Class::Core';

__PACKAGE__->load_components('InflateColumn::DateTime');

__PACKAGE__->table('item');

__PACKAGE__->add_columns(
  'item_id',
  { data_type => 'uuid', is_nullable => 0, size => 16 },
  'name',
  { data_type => 'char', is_nullable => 0, size => 32 },
);

__PACKAGE__->set_primary_key('item_id');

__PACKAGE__->has_many(
  'access_logs',
  'Access::Schema::Result::AccessLog',
  { 'foreign.item_id' => 'self.item_id' },
  { cascade_copy => 0, cascade_delete => 0 },
);

__PACKAGE__->has_many(
  'item_mgroups',
  'Access::Schema::Result::ItemMgroup',
  { 'foreign.item_id' => 'self.item_id' },
  { cascade_copy => 0, cascade_delete => 0 },
);

__PACKAGE__->has_many(
  'device_items',
  'Access::Schema::Result::DeviceItem',
  { 'foreign.item_id' => 'self.item_id' },
  { cascade_copy => 0, cascade_delete => 0 },
);

__PACKAGE__->many_to_many('mgroups', 'item_mgroups', 'mgroup');
__PACKAGE__->many_to_many('devices', 'device_items', 'device');

1;
