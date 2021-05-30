#include "oyanftmarket.hpp"

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::addmodcol(
			uint64_t author_id,
			const name& collection_name,
			const string& collection_desc,
			const string& collection_url,
		)
{
	require_auth(get_self());

	collection_index collection_table(get_self(), author_id);
	auto collection_it = collection_table.find(collection_name.value);

	if (collection_it == collection_table.end()) {
		collection_table.emplace(get_self(), [&](auto &row){
			row.collection_name = collection_name;
			row.collection_desc = collection_desc;
			row.collection_url = collection_url;
		});
	} else {
		collection_table.modify(collection_it, get_self(), [&](auto &row){
			if (collection_name != ""_n) row.collection_name = collection_name;
			if (!collection_name.empty()) row.collection_desc = collection_desc;
			if (!collection_url.empty()) row.collection_url = collection_url;
		});
	}

}


// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::delcol(
				const name& collection_name,
				uint64_t asset_id,
				uint64_t author_id
			)
{
	require_auth(get_self());

	// check the collection is not present for this author.
	collection_index collection_table(get_self(), author_id);
	auto collection_it = collection_table.find(collection_name.value);

	check(collection_it != collection_table.end(), "The collection is not present.");


	// check that the collection is neither listed in sale or auction table
	// 1. Sale
	sale_index sale_table(get_self(), get_self().value);
	auto collection_sale_idx = sale_table.get_index<"bycollection"_n>();
	auto collection_sale_it = collection_sale_idx.find(collection_name.value);

	check(collection_sale_it == collection_sale_idx.end(), "The collection is listed in sale, so can\'t be deleted.");

	// 2. Auction
	auction_index auction_table(get_self(), get_self().value);
	auto collection_auction_idx = auction_table.get_index<"bycollection"_n>();
	auto collection_auction_it = collection_auction_idx.find(collection_name.value);

	check(collection_auction_it == collection_auction_idx.end(), "The collection is listed in auction, so can\'t be deleted.");


	// erase the collection
	collection_table.erase(collection_it);

}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::addmodasset(
				const name& collection_name,
				uint64_t asset_id,
				uint64_t author_id,
				uint64_t current_owner_id,
				const string& asset_name,
				const string& asset_desc,
				const checksum256& asset_img_hash,
				const checksum256& asset_vid_hash,
				const checksum256& asset_gif_hash,
				uint64_t asset_copies_qty_used,
				uint64_t asset_copies_qty_total,
				float asset_royaltyfee,
				const string& asset_artist
			)
{
	require_auth(get_self());

	// check for valid collection name
	collection_index collection_table(get_self(), author_id);
	auto collection_it = collection_table.find(collection_name.value);

	check(collection_it != collection_table.end(), "The collection is not present.");

	// check for total asset copies as <= 99999
	check(asset_copies_qty_total <= 99999, "The total asset copies can\'t be greater than 99999.")

	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	if (asset_it == asset_table.end()) {
		asset_table.emplace(get_self(), [&](auto &row){
			row.asset_id = asset_id;
			row.author_id = author_id;
			row.current_owner_id = author_id;
			row.asset_name = asset_name;
			row.asset_desc = asset_desc;
			row.asset_img_hash = asset_img_hash;
			row.asset_vid_hash = asset_vid_hash;
			row.asset_gif_hash = asset_gif_hash;
			row.asset_copies_qty_used = 0;
			row.asset_copies_qty_total = asset_copies_qty_total;
			row.asset_royaltyfee = asset_royaltyfee;
			row.asset_artist = asset_artist;
		});
	} else {
		asset_table.modify(asset_it, get_self(), [&](auto &row){
			if (current_owner_id != 0 && current_owner_id != author_id) row.current_owner_id = current_owner_id;
			if (asset_copies_qty_used != 0) row.asset_copies_qty_used += asset_copies_qty_used;
			if (asset_copies_qty_total != 0) row.asset_copies_qty_total = asset_copies_qty_total;
			row.asset_royaltyfee = asset_royaltyfee;
			row.asset_artist = asset_artist;
		});
	}

}


// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::delasset(
				const name& collection_name,
				uint64_t asset_id,
				uint64_t author_id
			)
{
	require_auth(get_self());

	// check for valid collection name
	collection_index collection_table(get_self(), author_id);
	auto collection_it = collection_table.find(collection_name.value);

	check(collection_it != collection_table.end(), "The collection is not present.");

	// check for total asset copies as <= 99999
	check(asset_copies_qty_total <= 99999, "The total asset copies can\'t be greater than 99999.")

	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	if (asset_it == asset_table.end()) {
		asset_table.emplace(get_self(), [&](auto &row){
			row.asset_id = asset_id;
			row.author_id = author_id;
			row.current_owner_id = author_id;
			row.asset_name = asset_name;
			row.asset_desc = asset_desc;
			row.asset_img_hash = asset_img_hash;
			row.asset_vid_hash = asset_vid_hash;
			row.asset_gif_hash = asset_gif_hash;
			row.asset_copies_qty_used = 0;
			row.asset_copies_qty_total = asset_copies_qty_total;
			row.asset_royaltyfee = asset_royaltyfee;
			row.asset_artist = asset_artist;
		});
	} else {
		asset_table.modify(asset_it, get_self(), [&](auto &row){
			if (current_owner_id != 0 && current_owner_id != author_id) row.current_owner_id = current_owner_id;
			if (asset_copies_qty_used != 0) row.asset_copies_qty_used += asset_copies_qty_used;
			if (asset_copies_qty_total != 0) row.asset_copies_qty_total = asset_copies_qty_total;
			row.asset_royaltyfee = asset_royaltyfee;
			row.asset_artist = asset_artist;
		});
	}

}
