use utf8;
package Access::Schema::Result::Device;

# Created by DBIx::Class::Schema::Loader
# DO NOT MODIFY THE FIRST PART OF THIS FILE

=head1 NAME

Access::Schema::Result::Device

=cut

use strict;
use warnings;

use base 'DBIx::Class::Core';

=head1 COMPONENTS LOADED

=over 4

=item * L<DBIx::Class::InflateColumn::DateTime>

=back

=cut

__PACKAGE__->load_components("InflateColumn::DateTime");

=head1 TABLE: C<device>

=cut

__PACKAGE__->table("device");

=head1 ACCESSORS

=head2 device_id

  data_type: 'uuid'
  is_nullable: 0
  size: 16

=head2 key

  data_type: 'bytea'
  is_nullable: 0

=head2 name

  data_type: 'varchar'
  is_nullable: 0
  size: 64

=cut

__PACKAGE__->add_columns(
  "device_id",
  { data_type => "uuid", is_nullable => 0, size => 16 },
  "key",
  { data_type => "bytea", is_nullable => 0 },
  "name",
  { data_type => "varchar", is_nullable => 0, size => 64 },
);

=head1 PRIMARY KEY

=over 4

=item * L</device_id>

=back

=cut

__PACKAGE__->set_primary_key("device_id");


# Created by DBIx::Class::Schema::Loader v0.07042 @ 2017-02-11 14:17:13
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:YohxotZ7q8d3eGkvB3PbkA


# You can replace this text with custom code or comments, and it will be preserved on regeneration
1;
