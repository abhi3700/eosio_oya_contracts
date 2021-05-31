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


	// check that the collection is neither listed in sale nor auction table
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

	// add the asset into nftownership table
	// nftownership_index nftownership_table(get_self(), author_id);
	// auto nftownership_it = nftownership_table.find(collection_name.value);

	// if (nftownership_it == nftownership_table.end()) {
	// 	nftownership_table.emplace(get_self(), [&](auto &row){
	// 		row.collection_name = collection_name;
	// 		row.asset_id = asset_id;
	// 		row.is_author = author;
	// 	});
	// }

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

	// check that the asset is neither listed in sale nor auction table
	// 1. Sale
	sale_index sale_table(get_self(), get_self().value);
	auto asset_sale_idx = sale_table.get_index<"byasset"_n>();
	auto asset_sale_it = asset_sale_idx.find(asset_id);

	check(asset_sale_it == asset_sale_idx.end(), "The asset is listed in sale with id, so can\'t be deleted.");

	// 2. Auction
	auction_index auction_table(get_self(), get_self().value);
	auto asset_auction_idx = auction_table.get_index<"byasset"_n>();
	auto asset_auction_it = asset_auction_idx.find(asset_id);

	check(asset_auction_it == asset_auction_idx.end(), "The asset is listed in auction, so can\'t be deleted.");

	// remove the asset from nftownership table


	// Instantiate the asset table
	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	check(asset_it != asset_table.end(), "The asset id is not present.");

	asset_table.erase(asset_it);



}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::delitem(
				const name& collection_name,
				uint64_t item_id,
				uint64_t author_id
			)
{
	require_auth(get_self());

	// check whether valid collection_name
	collection_index collection_table(get_self(), author_id);
	auto collection_it = collection_table.find(collection_name.value);

	check(collection_it != collection_table.end(), "The collection is not present.");

	// extract the asset_id from item_id
	// uint64_t asset_id = str_to_uint64t(std::to_string(item_id).substr(0, 14));

	// check that the item is neither listed in sale nor auction table
	// 1. Sale
	sale_index sale_table(get_self(), get_self().value);
	auto item_sale_idx = sale_table.get_index<"byitem"_n>();
	auto item_sale_it = item_sale_idx.find(item_id);

	check(item_sale_it == item_sale_idx.end(), "The item is listed in sale, so can\'t be deleted.");

	// 2. Auction
	auction_index auction_table(get_self(), get_self().value);
	auto item_auction_idx = auction_table.get_index<"byitem"_n>();
	auto item_auction_it = item_auction_idx.find(item_id);

	check(item_auction_it == item_auction_idx.end(), "The asset is listed in auction, so can\'t be deleted.");

	// todo: reduce by 1 from both `asset_copies_qty_total` & 'asset_copies_qty_used' in asset table
	

	// todo: remove the item_id from oyanonauthor table 

}



void oyanftmarket::additmother(
				uint64_t nonauthor_id,
				const name& collection_name,
				uint64_t item_id
			)
{
	require_auth(get_self());

	// extract the asset_id from item_id
	uint64_t asset_id = str_to_uint64t(std::to_string(item_id).substr(0, 14));

	oyanonauthor_index oyanonauthor_table(get_self(), nonauthor_id);
	oyanonauthor_it = oyanonauthor_table.find(collection_name.value);

	if (oyanonauthor_it == oyanonauthor_table.end()) {
		oyanonauthor_table.emplace(get_self(), [&](auto &row){
			row.collection_name = collection_name;
			row.asset_id = asset_id;
			// todo: add item_id
		});
	} else {
		oyanonauthor_table.modify(oyanonauthor_it, get_self(), [&](auto &row){
			// todo: add item_id
		});
	}
}

void oyanftmarket::rmitmother(
				uint64_t nonauthor_id,
				uint64_t item_id
			)
{
	require_auth(get_self());

	// extract the asset_id from item_id
	uint64_t asset_id = str_to_uint64t(std::to_string(item_id).substr(0, 14));

	oyanonauthor_index oyanonauthor_table(get_self(), nonauthor_id);
	asset_oyanonauthor_idx = oyanonauthor_table.get_index<"byasset">();
	asset_oyanonauthor_it = asset_oyanonauthor_idx.find(asset_id);

	check(asset_oyanonauthor_it != asset_oyanonauthor_idx.end(), "the asset containing this item doesn\'t exist.");

	// todo: check if item is present in `asset_oyanonauthor_it->item_ids` list

	// todo: delete the asset id if there is no item inside after removing parsed item_id
	// if( ( asset_oyanonauthor_it->item_ids.size() == 1 ) && (has_item(asset_oyanonauthor_it->item_ids, item_id)) )
	// asset_oyanonauthor_idx.erase(asset_oyanonauthor_it);

}