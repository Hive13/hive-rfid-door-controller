#!/usr/bin/perl

use warnings;
use strict;
use lib 'lib';
use JSON::PP;
use Digest::SHA qw(sha512_hex);
use Data::Dumper;
use Try::Tiny;

use Access::Schema;

my $db_user = $ENV{DB_USER};
my $db_pass = $ENV{DB_PASS};
my $schema  = Access::Schema->connect("DBI:Pg:dbname=door;host=door.at.hive13.org", $db_user, $db_pass) || die $!;
my $sorter  = sub { $JSON::PP::a cmp $JSON::PP::b; };
my $js      = JSON::PP->new();
my $in;

	{
	local $/ = undef;
	$in = <STDIN>;
	}

sub make_hash
	{
	my $json     = shift;
	my $key      = shift;
	my $hash_str = $js->sort_by($sorter)->encode($json);

	return uc(sha512_hex($key . $hash_str));
	}

sub hash_it
	{
	my $in_json  = shift;
	my $key      = shift;
	my $out_json = {};

	$out_json->{data} = $in_json;
	if (defined($key))
		{
		my $random = [];
		for (my $i = 0; $i < 16; $i++)
			{
			push(@{$random}, int(rand(256)));
			}
		$in_json->{random}    = $random;
		$out_json->{checksum} = make_hash($in_json, $key);
		}

	return $out_json;
	}

sub access
	{
	my $badge_no = shift;
	my $iname    = shift;
	my $item     = $schema->resultset('Item')->find( { name => $iname } );
	my $badge    = $schema->resultset('Badge')->find( { badge_number => $badge_no } );
	my $member;

	if (defined($badge))
		{
		$member = $badge->member();
		}
	else
		{
		$member = $schema->resultset('Member')->find( { accesscard => $badge_no } );
		}
	return "Invalid badge"
		if (!defined($member));
	return "Invalid item"
		if (!defined($item));
	
	return "Locked out"
		if ($member->is_lockedout());

	# Does the member have access to the item through any groups
	my $access = $item
		->search_related('item_mgroups')
		->search_related('mgroup')
		->search_related('member_mgroups', { member_id => $member->member_id() })
		->count();
	
	# Log the access
	$schema->resultset('AccessLog')->create(
		{
		member_id => $member->member_id(),
		item_id   => $item->item_id(),
		granted   => ($access > 0) ? 1 : 0,
		});
	return $access > 0 ? undef : "Access denied";
	}

my $json = {};
my $key;
try
	{
	$json = decode_json($in);
	};
my $out_obj = {};
my $data    = $json->{data} // {};
my $device  = $schema->resultset('Device')->find({ name => ($json->{device} // '') });

if (defined($device))
	{
	my $shasum  = make_hash($data, $device->key());
	my $matches = $shasum eq uc($json->{checksum} // "");

	$out_obj->{response} = $matches ? JSON::PP->true() : JSON::PP->false();
	$key = $device->key();

	if ($matches || 1)
		{
		my $operation = lc($data->{operation} // "access");
		if ($operation eq "access")
			{
			my $badge  = $data->{badge};
			my $item   = $data->{location} // $data->{item};
			my $access = access($badge, $item);
			my $d_i    = $device
				->search_related('device_items')
				->search_related('item', { name => $item } );

			if ($d_i->count() < 1)
				{
				$out_obj->{access} = JSON::PP->false();
				$out_obj->{error} = "Device not authorized for " . $item;
				}
			elsif (defined($access))
				{
				$out_obj->{access} = JSON::PP->false();
				$out_obj->{error} = $access;
				}
			else
				{
				$out_obj->{access} = JSON::PP->true();
				}
			}
		}
	}
else
	{
	$out_obj->{response} = JSON::PP->false();
	$out_obj->{error} = 'Cannot find device.';
	}

my $out_str = $js->sort_by($sorter)->encode(hash_it($out_obj, $key));
printf("Content-type: text/json\n\n");
printf("%s\n", $out_str);