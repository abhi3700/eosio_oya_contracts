#include "oyanftmarket.hpp"

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::addmodcol(
				uint64_t creator_id,
				const name& collection_name,
				const string& collection_displayname,
				const string& collection_desc,
				const string& collection_url,
			)
{
	require_auth(get_self());

	collection_index collection_table(get_self(), creator_id);
	auto collection_it = collection_table.find(collection_name.value);

	if (collection_it == collection_table.end()) {
		collection_table.emplace(get_self(), [&](auto &row){
			row.collection_name = collection_name;
			row.collection_displayname = collection_displayname;
			row.collection_desc = collection_desc;
			row.collection_url = collection_url;
		});
	} else {
		collection_table.modify(collection_it, get_self(), [&](auto &row){
			if (!collection_displayname.empty()) row.collection_displayname = collection_displayname;
			if (!collection_desc.empty()) row.collection_desc = collection_desc;
			if (!collection_url.empty()) row.collection_url = collection_url;
		});
	}

}


// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::delcol(
				uint64_t creator_id,
				const name& collection_name
			)
{
	require_auth(get_self());

	// check the collection is not present for this creator.
	collection_index collection_table(get_self(), creator_id);
	auto collection_it = collection_table.find(collection_name.value);

	check(collection_it != collection_table.end(), "The collection is not present for this creator.");


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
				uint64_t creator_id,
				uint64_t current_owner_id,
				const string& asset_name,
				const string& asset_desc,
				const checksum256& asset_img_hash,
				const checksum256& asset_vid_hash,
				const checksum256& asset_gif_hash,
				const checksum256& asset_file_hash,
				uint64_t asset_copies_qty_total,
				float asset_royaltyfee,
				const string& asset_artist
			)
{
	require_auth(get_self());

	// check for valid collection name
	collection_index collection_table(get_self(), creator_id);
	auto collection_it = collection_table.find(collection_name.value);

	check(collection_it != collection_table.end(), "The collection is not present.");

	// check for total asset copies as <= 99999
	check(asset_copies_qty_total <= 99999, "The total asset copies can\'t be greater than 99999.")

	// check positive royaltyfee
	check(asset_royaltyfee >= 0, "royalty fee can't be negative");

	// create unique asset id i.e. 9210<current_time><last_3_digit_tg_id>
	uint64_t asset_id = create_astsaleauc_id(9210, creator_id);

	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	if (asset_it == asset_table.end()) {
		asset_table.emplace(get_self(), [&](auto &row){
			row.asset_id = asset_id;
			row.creator_id = creator_id;
			row.current_owner_id = creator_id;
			row.asset_name = asset_name;
			row.asset_desc = asset_desc;
			row.asset_img_hash = asset_img_hash;
			row.asset_vid_hash = asset_vid_hash;
			row.asset_gif_hash = asset_gif_hash;
			row.asset_file_hash = asset_file_hash;
			row.asset_item_ids_listed_sale = {};
			row.asset_item_ids_listed_auct = {};
			row.asset_copies_qty_total = asset_copies_qty_total;
			row.asset_royaltyfee = asset_royaltyfee;
			row.asset_artist = asset_artist;
		});
	} else {
		asset_table.modify(asset_it, get_self(), [&](auto &row){
			if (current_owner_id != 0 && current_owner_id != creator_id) row.current_owner_id = current_owner_id;
			if (!asset_name.empty()) row.asset_name = asset_name;
			if (!asset_desc.empty()) row.asset_desc = asset_desc;
			if (asset_copies_qty_total != 0) row.asset_copies_qty_total = asset_copies_qty_total;
			if (asset_img_hash != checksum256()	/*not empty*/) row.asset_img_hash = asset_img_hash;
			if (asset_vid_hash != checksum256()	/*not empty*/) row.asset_vid_hash = asset_vid_hash;
			if (asset_gif_hash != checksum256()	/*not empty*/) row.asset_gif_hash = asset_gif_hash;
			if (asset_file_hash != checksum256()	/*not empty*/) row.asset_file_hash = asset_file_hash;
			if (asset_royaltyfee >= 0) row.asset_royaltyfee = asset_royaltyfee;
			if (!asset_artist.empty()) row.asset_artist = asset_artist;
		});
	}

}


// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::delasset(
				const name& collection_name,
				uint64_t asset_id,
				uint64_t creator_id
			)
{
	require_auth(get_self());

	// check for valid collection name
	collection_index collection_table(get_self(), creator_id);
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

	// Instantiate the asset table
	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	check(asset_it != asset_table.end(), "The asset id is not present.");

	asset_table.erase(asset_it);

}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::delitem(
				const name& collection_name,
				uint64_t asset_id,
				uint64_t creator_id,
				uint64_t item_qty,
			)
{
	require_auth(get_self());

	// check whether valid collection_name
	collection_index collection_table(get_self(), creator_id);
	auto collection_it = collection_table.find(collection_name.value);

	check(collection_it != collection_table.end(), "The collection is not present for this creator.");

	// check for valid item id by checking asset id existence 
	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	check(asset_it != asset_table.end(), "the item id doesn\'t belong to any existing asset for this creator.");

	check(item_qty > 0, "item quantity must be positive and at least 1 as value.");

	// reduce by item_qty from remaining qty i.e. (asset_copies_qty_total - listed_sale_qty - listed_auction_qty) in asset table
	check( (asset_it->asset_copies_qty_total - asset_it->asset_copies_qty_listed_sale.size() - asset_it->asset_copies_qty_listed_auct.size()) >= item_qty, 
		"sorry, the no. of items not listed is less than the item qty to be deleted." ):

	asset_table.modify(asset_it, get_self(), [&](auto &row){
		row.asset_copies_qty_total -= item_qty;
	});

}



// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::additemnctor(
				uint64_t noncreator_id,
				const name& collection_name,
				uint64_t item_id
			)
{
	require_auth(get_self());

	// extract the asset_id from item_id
	uint64_t asset_id = str_to_uint64t(std::to_string(item_id).substr(0, 14));

	oyanocreator_index oyanocreator_table(get_self(), noncreator_id);
	oyanocreator_it = oyanocreator_table.find(asset_id);

	if (oyanocreator_it == oyanocreator_table.end()) {
		oyanocreator_table.emplace(get_self(), [&](auto &row){
			row.asset_id = asset_id;
			row.collection_name = collection_name;
			row.item_ids = vector<uint64_t>{item_id};
		});
	} 
	else {
		// add the item if does't exist already in the list
		if (!has_item_in_vector(oyanocreator_it->item_ids, item_id)) {
			oyanocreator_table.modify(oyanocreator_it, get_self(), [&](auto &row){
				row.item_ids.emplace_back(item_id);
			});
		}
	}
}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::rmitemnctor(
				uint64_t noncreator_id,
				uint64_t item_id
			)
{
	require_auth(get_self());

	// extract the asset_id from item_id
	uint64_t asset_id = str_to_uint64t(std::to_string(item_id).substr(0, 14));

	oyanocreator_index oyanocreator_table(get_self(), noncreator_id);
	oyanocreator_it = oyanocreator_table.find(asset_id);

	check(oyanocreator_it != oyanocreator_table.end(), "the asset containing this item doesn\'t exist for this non-creator.");

	// check if item is present in `oyanocreator_it->item_ids` list
	check(has_item_in_vector(oyanocreator->item_ids, item_id), ("Sorry!, the item id doesn\'t exist in the items list for the asset id \'").append(std::to_string(asset_id)).append("\' for this non-creator.") );

	// find out the position if item_id exist
	auto item_id_it = std::find(oyanocreator_it->item_ids.begin(), oyanocreator_it->item_ids.end(), item_id);
	oyanocreator_table.modify(oyanocreator_it, get_self(), [&](auto &row){
		if (item_id_it != oyanocreator_it->item_ids.end())		// if item found
			row.item_ids.erase(item_id_it);
	});

	// lastly, delete the asset id row, if there is no item inside list, after removing parsed item_id. This is to restore the contract's RAM.
	if( oyanocreator_it->item_ids.size() == 1  )
		asset_oyanocreator_idx.erase(asset_oyanocreator_it);

}


// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::listitemsale(
				const name& seller_id,
				const name& collection_name,
				const vector<uint64_t> item_ids,
				const asset& listing_price_crypto,
				float listing_price_fiat,
			)
{
	require_auth(get_self());

	check( item_ids.size() > 0, "there is no item id parsed." );

	// extract the asset_id from item_id
	uint64_t asset_id = str_to_uint64t(std::to_string(item_ids[0]).substr(0, 14));

	// check for valid collection name & asset_id
	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for the parsed collection name");

	check( (asset_it->asset_copies_qty_total - asset_it->asset_item_ids_listed_sale.size() - asset_it->asset_item_ids_listed_auct.size()) >= item_ids.size(), 
		"the no. of items remaining for listed is less than the parsed list size." ):

	// check item_ids must not be in listed sale
	for(auto&& item_id : item_ids) {
		check(!has_item_in_vector(asset_it->asset_item_ids_listed_sale, item_id), 
			"the item id: \'".append(item_id).append("\' is already listed in sale"));
	}

    check( listing_price_crypto.is_valid(), "invalid price in crypto");
    check( listing_price_crypto.amount > 0, "crypto qty must be positive");

	// find out if seller is a creator or not from assets table
	bool is_creator = false;
	if (asset_it->creator_id == seller_id) is_creator = true;

    // prove the ownership for seller_id
    if (!is_creator) {			// if non-creator
		oyanocreator_index oyanocreator_table(get_self(), seller_id);
		oyanocreator_it = oyanocreator_table.find(asset_id);

		check(oyanocreator_it != oyanocreator_table.end(), "the asset containing this item doesn\'t exist for this non-creator.");
    } 

	// create unique sale id i.e. 3700<current_time><last_3_digit_tg_id>
	uint64_t sale_id = create_astsaleauc_id(3700, asset_it->creator_id);

    sale_index sale_table(get_self(), get_self().value);
    auto sale_it = sale_table.find(sale_id);

    check(sale_it == sale_table.end(), "the sale already exist.");

    sale_table.emplace(get_self(), [&](auto &row){
		row.sale_id = sale_id;
		row.item_ids = item_ids;
		row.asset_id = asset_id;
		row.seller_id = seller_id;
		row.listing_price_crypto = listing_price_crypto;
		row.listing_price_fiat_usd = listing_price_fiat_usd;
		row.collection_name = collection_name;
		row.royalty_fee = asset_id->asset_royaltyfee;
	});

    // add the item_ids into the asset_item_ids_listed_sale in asset table
	asset_table.modify(asset_it, get_self(), [&](auto &row){
		row.asset_item_ids_listed_sale.insert(
			asset_it->asset_item_ids_listed_sale.begin(),
			item_ids.begin(),
			item_ids.end()
		);
	});

}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::ulistitemsale(
				uint64_t sale_id,
				uint64_t seller_id
			)
{
	require_auth(get_self());

    sale_index sale_table(get_self(), get_self().value);
    auto sale_it = sale_table.find(sale_id);

    check(sale_it != sale_table.end(), "the sale id doesn\'t exist.");

    // checked the seller is original
    check(sale_it->seller_id == seller_id, "the parsed seller_id doesn\'t match with the actual one for this sale.");

    // remove the item_ids from `asset_item_ids_listed_sale` in asset table
	asset_index asset_table(get_self(), sale_it->collection_name.value);
	auto asset_it = asset_table.find(sale_it->asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for this sale\'s collection name");

	// remove the item_id(s) also from asset_item_ids_listed_sale in asset table
	for(auto&& item_id : sale_it->item_ids) {
		auto item_id_it = std::find(asset_it->asset_item_ids_listed_sale.begin(), asset_it->asset_item_ids_listed_sale.end(), item_id);
		if(item_id_it != asset_it->asset_item_ids_listed_sale.end()) {		// if item_id found
			asset_table.modify(asset_it, get_self(), [&](auto &row){
				row.asset_item_ids_listed_sale.erase(item_id_it);
			});
		}	
	}

    sale_table.erase(sale_it);

}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::additemsale(
				uint64_t sale_id, 
				uint64_t seller_id,
				const vector<uint64_t> item_ids
			)
{
	require_auth(get_self());

    sale_index sale_table(get_self(), get_self().value);
    auto sale_it = sale_table.find(sale_id);

    check(sale_it != sale_table.end(), "the sale id doesn\'t exist.");

    // checked the seller is original
    check(sale_it->seller_id == seller_id, "the parsed seller_id doesn\'t match with the actual one for this sale.");

	asset_index asset_table(get_self(), sale_it->collection_name.value);
	auto asset_it = asset_table.find(sale_it->asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for the sale\'s collection name");

	check( (asset_it->asset_copies_qty_total - asset_it->asset_item_ids_listed_sale.size() - asset_it->asset_item_ids_listed_auct.size()) >= item_ids.size(), 
		"the no. of items remaining for listed is less than the parsed list size." ):

	// check item_ids must not be in listed sale
	for(auto&& item_id : item_ids) {
		check(!has_item_in_vector(asset_it->asset_item_ids_listed_sale, item_id), 
			"the item id: \'".append(item_id).append("\' is already listed in sale"));
	}

    // add the item_ids into the `item_ids` in sales table
	sale_table.modify(sale_it, get_self(), [&](auto &row){
		row.item_ids.insert(
			sale_it->item_ids.begin(),
			item_ids.begin(),
			item_ids.end()
		);
	});

    // add the item_ids into the `asset_item_ids_listed_sale` in asset table
	asset_table.modify(asset_it, get_self(), [&](auto &row){
		row.asset_item_ids_listed_sale.insert(
			asset_it->asset_item_ids_listed_sale.begin(),
			item_ids.begin(),
			item_ids.end()
		);
	});

}

