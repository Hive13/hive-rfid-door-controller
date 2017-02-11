use utf8;
package Access::Schema::Result::Member;

# Created by DBIx::Class::Schema::Loader
# DO NOT MODIFY THE FIRST PART OF THIS FILE

=head1 NAME

Access::Schema::Result::MemberAccessView

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
__PACKAGE__->table_class("DBIx::Class::ResultSource::View");

=head1 TABLE: C<member_access_view>

=cut

__PACKAGE__->table("member_access_view");

=head1 ACCESSORS

=head2 member_id

  data_type: 'uuid'
  is_nullable: 1
  size: 16

=head2 fname

  data_type: 'varchar'
  is_nullable: 1
  size: 255

=head2 lname

  data_type: 'varchar'
  is_nullable: 1
  size: 255

=head2 email

  data_type: 'varchar'
  is_nullable: 1
  size: 255

=head2 accesscard

  data_type: 'varchar'
  is_nullable: 1
  size: 255

=cut

__PACKAGE__->add_columns(
  "member_id",
  { data_type => "uuid", is_nullable => 1, size => 16 },
  "fname",
  { data_type => "varchar", is_nullable => 1, size => 255 },
  "lname",
  { data_type => "varchar", is_nullable => 1, size => 255 },
  "email",
  { data_type => "varchar", is_nullable => 1, size => 255 },
  "accesscard",
  { data_type => "varchar", is_nullable => 1, size => 255 },
);


# Created by DBIx::Class::Schema::Loader v0.07042 @ 2017-02-10 20:44:52
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:gUV88ashyM/49Ip+PrMtVg


# You can replace this text with custom code or comments, and it will be preserved on regeneration
1;
