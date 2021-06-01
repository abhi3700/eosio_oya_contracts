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

	check(collection_it != collection_table.end(), "The collection is not present for this author.");


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
			row.asset_copies_qty_listed_sale = 0;
			row.asset_copies_qty_listed_auct = 0;
			row.asset_copies_qty_total = asset_copies_qty_total;
			row.asset_royaltyfee = asset_royaltyfee;
			row.asset_artist = asset_artist;
		});
	} else {
		asset_table.modify(asset_it, get_self(), [&](auto &row){
			if (current_owner_id != 0 && current_owner_id != author_id) row.current_owner_id = current_owner_id;
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



// --------------------------------------------------------------------------------------------------------------------
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
	oyanonauthor_it = oyanonauthor_table.find(asset_id);

	if (oyanonauthor_it == oyanonauthor_table.end()) {
		oyanonauthor_table.emplace(get_self(), [&](auto &row){
			row.asset_id = asset_id;
			row.collection_name = collection_name;
			row.item_ids = vector<uint64_t>{item_id};
		});
	} 
	else {
		// add the item if does't exist already in the list
		if (!has_item_in_vector(oyanonauthor_it->item_ids, item_id)) {
			oyanonauthor_table.modify(oyanonauthor_it, get_self(), [&](auto &row){
				row.item_ids.emplace_back(item_id);
			});
		}
	}
}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::rmitmother(
				uint64_t nonauthor_id,
				uint64_t item_id
			)
{
	require_auth(get_self());

	// extract the asset_id from item_id
	uint64_t asset_id = str_to_uint64t(std::to_string(item_id).substr(0, 14));

	oyanonauthor_index oyanonauthor_table(get_self(), nonauthor_id);
	oyanonauthor_it = oyanonauthor_table.find(asset_id);

	check(oyanonauthor_it != oyanonauthor_table.end(), "the asset containing this item doesn\'t exist.");

	// check if item is present in `oyanonauthor_it->item_ids` list
	check(has_item_in_vector(oyanonauthor->item_ids, item_id), ("Sorry!, the item id doesn\'t exist in the items list for the asset id \'").append(std::to_string(asset_id)).append("\'") );

	auto item_id_it = std::find(oyanonauthor_it->item_ids.begin(), oyanonauthor_it->item_ids.end(), item_id);
	oyanonauthor_table.modify(oyanonauthor_it, get_self(), [&](auto &row){
		if (item_id_it != oyanonauthor_it->item_ids.end())		// if item found
			row.item_ids.erase(item_id_it);
	});

	// lastly, delete the asset id row, if there is no item inside list, after removing parsed item_id. This is to restore the contract's RAM.
	if( oyanonauthor_it->item_ids.size() == 1  )
		asset_oyanonauthor_idx.erase(asset_oyanonauthor_it);

}


void oyanftmarket::listitemsale(
				vector<uint64_t> item_ids,
				const name& collection_name,
				const name& seller_id,
				const asset& listing_price_crypto,
				float listing_price_fiat,
			)
{
	require_auth(get_self());

	check( item_ids.size() > 0, "there is no item id parsed." );

	// extract the asset_id from item_id
	uint64_t asset_id = str_to_uint64t(std::to_string(item_ids[0]).substr(0, 14));

	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for the parsed collection name");

	check( (asset_it->asset_copies_qty_total - asset_it->asset_copies_qty_listed_sale - asset_it->asset_copies_qty_listed_auct) >= item_ids.size(), 
		"sorry, the no. of items pending for listed is less than the parsed list size." ):

	// create unique sale id i.e. 3700<current_time><last_3_digit_tg_id>
	uint64_t sale_id = create_saleauc_id(3700, asset_it->author_id);

	// check for valid collection name
	collection_index collection_table(get_self(), asset_it->author_id);
	auto collection_it = collection_table.find(collection_name.value);

	check(collection_it != collection_table.end(), "The collection is not present for this author.");

	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	check(asset_it != asset_table.end(), "The asset id is not available for the parsed collection");

    check( listing_price_crypto.is_valid(), "invalid price in crypto");
    check( listing_price_crypto.amount > 0, "crypto qty must be positive");

    sale_index sale_table(get_self(), get_self().value);
    auto sale_it = sale_table.find(sale_id);

    check(sale_it == sale_table.end(), "the sale already exist.");

    sale_table.emplace(get_self(), [&](auto &row){
		row.sale_id = sale_id;
		row.item_ids = item_ids;
		row.asset_id = asset_id;
		row.seller_id = seller_id;		// todo
		row.listing_price_crypto = listing_price_crypto;
		row.listing_price_fiat_usd = listing_price_fiat_usd;
		row.collection_name = collection_name;
		row.royalty_fee = asset_id->asset_royaltyfee;
	});

    /*
    TODO:
    =====
	Currently, everytime this ACTION executes, a new sale id is created => by default it will take to creating
	a new row i.e. sale.

	Now, the problem is for an existing listed item, the sale might be created again.

    */


    // increase the 'asset_copies_qty_listed' by item_ids.size() in assets table
    asset_table.modify(asset_it, get_self(), [&](auto &row){
		row.asset_copies_qty_listed += item_ids.size();
	});


}