use utf8;
package Access::Schema::Result::Member;

use strict;
use warnings;

use base 'DBIx::Class::Core';

__PACKAGE__->load_components('InflateColumn::DateTime');
__PACKAGE__->table_class('DBIx::Class::ResultSource::View');
__PACKAGE__->table('member_access_view');

__PACKAGE__->add_columns(
  'member_id',
  { data_type => 'uuid', is_nullable => 1, size => 16 },
  'fname',
  { data_type => 'varchar', is_nullable => 1, size => 255 },
  'lname',
  { data_type => 'varchar', is_nullable => 1, size => 255 },
  'email',
  { data_type => 'varchar', is_nullable => 1, size => 255 },
  'accesscard',
  { data_type => 'varchar', is_nullable => 1, size => 255 },
	'is_lockedout',
	{ data_type => 'boolean', is_nullable => 1, is_boolean => 1 },
);

1;
