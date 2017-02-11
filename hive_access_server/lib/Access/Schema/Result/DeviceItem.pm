use utf8;
package Access::Schema::Result::DeviceItem;

use strict;
use warnings;

use base 'DBIx::Class::Core';

__PACKAGE__->load_components('InflateColumn::DateTime');
__PACKAGE__->table('device_item');

__PACKAGE__->add_columns(
  'device_id',
  { data_type => 'uuid', is_foreign_key => 1, is_nullable => 0, size => 16 },
  'item_id',
  { data_type => 'uuid', is_foreign_key => 1, is_nullable => 0, size => 16 },
);

__PACKAGE__->set_primary_key('device_id', 'item_id');

__PACKAGE__->belongs_to(
  'device',
  'Access::Schema::Result::Device',
  { device_id => 'device_id' },
);
__PACKAGE__->belongs_to(
  'item',
  'Access::Schema::Result::Item',
  { item_id => 'item_id' },
);

1;
