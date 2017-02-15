use utf8;
package Access::Schema::Result::Member;

use strict;
use warnings;

use base 'DBIx::Class::Core';

__PACKAGE__->load_components(qw/ InflateColumn::DateTime UUIDColumns Core /);
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
  { data_type => 'varchar', default_value => '', is_nullable => 0, size => 255 },
  'accesscard',
  { data_type => 'varchar', is_nullable => 1, size => 255 },
  'is_lockedout',
  { data_type => 'boolean', is_nullable => 1, is_boolean => 1 },
);

__PACKAGE__->uuid_columns('member_id');
__PACKAGE__->add_unique_constraint('index_members_on_email', ['email']);
__PACKAGE__->add_unique_constraint('members_member_id_key', ['member_id']);

__PACKAGE__->has_many(
  'access_logs',
  'Access::Schema::Result::AccessLog',
  { 'foreign.member_id' => 'self.member_id' },
  { cascade_copy => 0, cascade_delete => 0 },
);

__PACKAGE__->has_many(
  'badges',
  'Access::Schema::Result::Badge',
  { 'foreign.member_id' => 'self.member_id' },
  { cascade_copy => 0, cascade_delete => 0 },
);

__PACKAGE__->has_many(
  'member_mgroups',
  'Access::Schema::Result::MemberMgroup',
  { 'foreign.member_id' => 'self.member_id' },
  { cascade_copy => 0, cascade_delete => 0 },
);

__PACKAGE__->many_to_many('mgroups', 'member_mgroups', 'mgroup');

1;
