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
			row.asset_item_ids_transferred = {};
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
				const name& price_mode,
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
		"the no. of items remaining to be listed is less than the parsed items size." ):

	check( (price_mode == "crypto"_n) || (price_mode == "fiat"_n), "invalid price mode.");

	// check item_ids must not be in listed sale
	for(auto&& item_id : item_ids) {
		check(!has_item_in_vector(asset_it->asset_item_ids_listed_sale, item_id), 
			"the item id: \'".append(item_id).append("\' is already listed on sale in asset table"));
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
		
		// check the seller (non-creator) owns the item_ids 
		for(auto&& item_id : item_ids) {
			check(has_item_in_vector(oyanocreator_it->item_ids, item_id), 
				"the item id: \'".append(item_id).append("\' is not owned by the non-creator seller."));
		}

	} 

	// create unique sale id i.e. 3700<current_time><last_3_digit_tg_id>
	uint64_t sale_id = create_astsaleauc_id(3700, seller_id);

	sale_index sale_table(get_self(), get_self().value);
	auto sale_it = sale_table.find(sale_id);

	check(sale_it == sale_table.end(), "the sale already exist.");

	sale_table.emplace(get_self(), [&](auto &row){
		row.sale_id = sale_id;
		row.item_ids = item_ids;
		row.asset_id = asset_id;
		row.seller_id = seller_id;
		if(price_mode == "crypto"_n) {
			row.listing_price_crypto = listing_price_crypto;
			row.listing_price_fiat_usd = 0;
		}
		else if(price_mode == "fiat"_n){
			row.listing_price_fiat_usd = listing_price_fiat_usd;
			row.listing_price_crypto.symbol = cryptopay_token_symbol;
		}
		row.collection_name = collection_name;
		row.royalty_fee = asset_id->asset_royaltyfee;
	});

	// add the item_ids into the asset_item_ids_listed_sale in asset table
	asset_table.modify(asset_it, get_self(), [&](auto &row){
		row.asset_item_ids_listed_sale.insert(
			asset_it->asset_item_ids_listed_sale.end(),
			item_ids.begin(),
			item_ids.end()
		);
	});

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

	// check item_ids must not be in listed sale in sale table
	for(auto&& item_id : item_ids) {
		check(!has_item_in_vector(sale_it->item_ids, item_id), 
			"the item id: \'".append(item_id).append("\' is already listed on sale in sale table."));
	}

	// check item_ids must not be in listed sale in asset table
	for(auto&& item_id : item_ids) {
		check(!has_item_in_vector(asset_it->asset_item_ids_listed_sale, item_id), 
			"the item id: \'".append(item_id).append("\' is already listed on sale in asset table."));
	}

	// add the item_ids into the `item_ids` in sales table
	sale_table.modify(sale_it, get_self(), [&](auto &row){
		row.item_ids.insert(
			sale_it->item_ids.end(),
			item_ids.begin(),
			item_ids.end()
		);
	});

	// add the item_ids into the `asset_item_ids_listed_sale` in asset table
	asset_table.modify(asset_it, get_self(), [&](auto &row){
		row.asset_item_ids_listed_sale.insert(
			asset_it->asset_item_ids_listed_sale.end(),
			item_ids.begin(),
			item_ids.end()
		);
	});

}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::rmitemsale(
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

	// check item_ids must be in listed sale in sale table
	for(auto&& item_id : item_ids) {
		check(has_item_in_vector(sale_it->item_ids, item_id), 
			"the item id: \'".append(item_id).append("\' is not listed on sale in sale table"));
	}

	// check item_ids must be in listed sale in 
	for(auto&& item_id : item_ids) {
		check(has_item_in_vector(asset_it->asset_item_ids_listed_sale, item_id), 
			"the item id: \'".append(item_id).append("\' is not listed on sale in asset table"));
	}


	// remove the item_id(s) also from asset_item_ids_listed_sale in asset table
	for(auto&& item_id : item_ids) {
		auto item_id_ast_it = std::find(asset_it->asset_item_ids_listed_sale.begin(), asset_it->asset_item_ids_listed_sale.end(), item_id);
		if(item_id_ast_it != asset_it->asset_item_ids_listed_sale.end()) {		// if item_id found
			asset_table.modify(asset_it, get_self(), [&](auto &row){
				row.asset_item_ids_listed_sale.erase(item_id_ast_it);
			});
		}	
	}

	// remove item_ids from the sale table
	for(auto&& item_id : item_ids) {
		auto item_id_sale_it = std::find(sale_it->item_ids.begin(), sale_it->item_ids.end(), item_id);
		if(item_id_sale_it != sale_it->item_ids.end()) {		// if item_id found
			sale_table.modify(sale_it, get_self(), [&](auto &row){
				row.item_ids.erase(item_id_sale_it);
			});
		}	
	}

}


// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::setpricesale(
				uint64_t sale_id,
				uint64_t seller_id,
				const name& price_mode,
				const asset& listing_price_crypto,
				float listing_price_fiat_usd
			)
{
	require_auth(get_self());

	sale_index sale_table(get_self(), get_self().value);
	auto sale_it = sale_table.find(sale_id);

	check(sale_it != sale_table.end(), "the sale id doesn\'t exist.");

	// checked the seller is original
	check(sale_it->seller_id == seller_id, "the parsed seller_id doesn\'t match with the actual one for this sale.");

	check( (price_mode == "crypto"_n) || (price_mode == "fiat"_n), "invalid price mode.");

	if (price_mode == "crypto"_n) {
		check( listing_price_crypto.amount > 0, "the listing price amount in crypto must be positive.");

		check(has_item_in_vector(crypto_token_symbol_list, listing_price_crypto.symbol), "Only chosen tokens are accepted as crypto for trading asset.");

		sale_table.modify(sale_it, get_self(), [&](auto &row){;
			row.listing_price_crypto = listing_price_crypto;
		});

	} else if (price_mode == "fiat"_n)  {
		check(listing_price_fiat_usd > 0.0, "the listing price amount in USD amount must be positive.")

		sale_table.modify(sale_it, get_self(), [&](auto &row){;
			row.listing_price_fiat_usd = listing_price_fiat_usd;
		});
	}
}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::buyitemsale(
				uint64_t sale_id,
				uint64_t buyer_id,
				const name& pay_mode
			)
{
	require_auth(get_self());

	sale_index sale_table(get_self(), get_self().value);
	auto sale_it = sale_table.find(sale_id);

	check(sale_it != sale_table.end(), "the sale id doesn\'t exist.");

	// instantiate the asset table
	asset_index asset_table(get_self(), sale_it->collection_name.value);
	auto asset_it = asset_table.find(sale_it->asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for the sale\'s collection name");

	check( (pay_mode == "crypto"_n) || (pay_mode == "fiat"_n), "invalid price mode.");

	// ************************************
	// Payment (crypto)
	if (pay_mode == "crypto"_n) {
		// create asset for seller
		auto qty_seller = asset(0, sale_it->listing_price_crypto.symbol);
		qty_seller.amount = sale_it->listing_price_crypto.amount * (1 - sale_it->royalty_fee - platform_commission_rate); 

		// create asset for creator
		auto qty_creator = asset(0, sale_it->listing_price_crypto.symbol);
		qty_creator.amount = sale_it->listing_price_crypto.amount * sale_it->royalty_fee; 

		sub_balance(buyer_id, sale_it->listing_price_crypto);			// from buyer
		add_balance(sale_it->seller_id, qty_seller, get_self());		// to seller
		add_balance(asset_it->creator_id, qty_creator, get_self());		// to creator as royalty_fee

	}

	// ************************************
	// Seller
	bool seller_is_creator = false;
	// check if seller is an creator or not from asset table corresponding to asset_id
	if(asset_it->creator_id == sale_it->seller_id) seller_is_creator = true;

	if(!seller_is_creator) {
		oyanocreator_index oyanocreator_seller_table(get_self(), sale_it->seller_id);
		oyanocreator_seller_it = oyanocreator_seller_table.find(sale_it->asset_id);

		check(oyanocreator_seller_it != oyanocreator_seller_table.end(), "the asset containing this item doesn\'t exist for this non-creator seller.");

		for(auto&& item_id : sale_it->item_ids) {
			// check the items exist in `item_ids` in oyanocreator table
			check(has_item_in_vector(oyanocreator_seller_it->item_ids, item_id), 
				"the item id: \'".append(item_id).append("\' is not owned by the seller in oyanocreator table."));

			// remove the items from `item_ids` in oyanocreator table
			auto item_id_it = std::find(oyanocreator_seller_it->item_ids.begin(), oyanocreator_seller_it->item_ids.end(), item_id);
			if(item_id_it != oyanocreator_seller_it->item_ids.end()) {		// if item_id found
				oyanocreator_seller_table.modify(oyanocreator_seller_it, get_self(), [&](auto &row){
					row.item_ids.erase(item_id_it);
				});
			}	
		}

	} else {
		// check that the items does not exist into `asset_item_ids_transferred` in asset table
		for(auto&& item_id : sale_it->item_ids) {
			check(!has_item_in_vector(asset_it->asset_item_ids_transferred, item_id), "the item id: \'".append(item_id).append("\' already exist as transferred item in asset table"));
		}

		// add the items into `asset_item_ids_transferred` in asset table
		asset_table.modify(asset_it, get_self(), [&](auto &row){
			row.asset_item_ids_transferred.insert(asset_it->asset_item_ids_transferred.end(),sale_it->item_ids.begin(), sale_it->item_ids.end());
		});

	}

	for (auto&& item_id : sale_it->item_ids) {
		// check the item_ids in sale table exist in `asset_item_ids_listed_sale` in asset table
		check(has_item_in_vector(asset_it->asset_item_ids_listed_sale, item_id), 
			"the item id: \'".append(item_id).append("\' is not listed on sale in asset table"));

		// remove items from the `asset_item_ids_listed_sale` in asset table
		auto item_id_it = std::find(asset_it->asset_item_ids_listed_sale.begin(), asset_it->asset_item_ids_listed_sale.end(), item_id);
		if(item_id_it != asset_it->asset_item_ids_listed_sale.end()) {		// if item_id found
			asset_table.modify(asset_it, get_self(), [&](auto &row){
				row.asset_item_ids_listed_sale.erase(item_id_it);
			});
		}

	}


	// ************************************
	// Buyer
	// NOTE: The buyer could be creator (where, try to buy the item_id back) or non-creator
	bool buyer_is_creator = false;
	// check if buyer is an creator or not from asset table corresponding to asset_id
	if(asset_it->creator_id == buyer_id) buyer_is_creator = true;

	if(!buyer_is_creator) {
		oyanocreator_index oyanocreator_buyer_table(get_self(), buyer_id);
		oyanocreator_buyer_it = oyanocreator_buyer_table.find(sale_it->asset_id);

		check(oyanocreator_buyer_it != oyanocreator_buyer_table.end(), "the asset containing this item doesn\'t exist for this non-creator buyer.");

		for(auto&& item_id : sale_it->item_ids) {
			// check that the buyer doesn't already own these items in oyanocreator table
			check(!has_item_in_vector(oyanocreator->item_ids, item_id), "the item id: \'".append(item_id).append("\' already owned by the buyer in oyanocreator table"));
		}

		// add the items into `item_ids` in oyanocreator table
		oyanocreator_buyer_table.modify(oyanocreator_buyer_it, get_self(), [&](auto &row){
			row.item_ids.insert(oyanocreator_buyer_it->item_ids.end(), sale_it->item_ids.begin(), sale_it->item_ids.end());
		});

	} else {
		for (auto&& item_id : sale_it->item_ids) {
			// check the item_ids exist in `asset_item_ids_transferred` in asset table
			check(has_item_in_vector(asset_id->asset_item_ids_transferred, item_id), "the item id: \'".append(item_id).append("\' doesn\'t exist as transferred item in asset table."));
	
			// restore by removing the items from `asset_item_ids_transferred` in asset table
			auto item_id_it = std::find(asset_it->asset_item_ids_transferred.begin(), asset_it->asset_item_ids_transferred.end(), item_id);
			if(item_id_it != asset_it->asset_item_ids_transferred.end()) {		// if item_id found
				asset_table.modify(asset_it, get_self(), [&](auto &row){
					row.asset_item_ids_transferred.erase(item_id_it);
				});
			}

		}


	}

	// ************************************
	// add buyer in sale table
	sale_table.modify(sale_it, get_self(), [&](auto &row){;
		row.buyer_id = buyer_id;
	});

}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::sub_balance( uint64_t owner_id, const asset& qty ) {
	cryptobal_index from_cryptobal_table(get_self(), get_self().value);
	auto from_cryptobal_it = from_cryptobal_table.find(owner_id);

	check(from_cryptobal_it != from_cryptobal_table.end(), "user not found in cryptobal table.")
	// const auto& from_cryptobal_it = from_cryptobal_table.get( qty.symbol.raw(), "no crypto balance object found" );
	check(from_cryptobal_it->balance.symbol.raw() == qty.symbol.raw(), "no crypto balance object found for buyer");
	check( from_cryptobal_it->balance.amount >= qty.amount, "overdrawn crypto balance" );

	from_cryptobal_table.modify( from_cryptobal_it, get_self(), [&]( auto& row ) {
		 row.balance -= qty;
	});
}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::add_balance( uint64_t owner_id, const asset& qty, const name& ram_payer )
{
	cryptobal_index to_cryptobal_table(get_self(), get_self().value);
	auto to_cryptobal_it = to_cryptobal_table.find(owner_id);
	// auto to_cryptobal_it = to_cryptobal_table.find(qty.symbol.raw());
    // auto to = to_acnts.find( value.symbol.code().raw() );
	
	if( to_cryptobal_it == to_cryptobal_table.end() ) {
	  to_cryptobal_table.emplace( ram_payer, [&]( auto& row ){
	  	row.user_id = owner_id;
		row.balance = qty;
	  });
	} else {
		check(from_cryptobal_it->balance.symbol.raw() == qty.symbol.raw(), "no crypto balance object found for seller");
		
		to_cryptobal_table.modify( to_cryptobal_it, same_payer, [&]( auto& row ) {
		row.balance += qty;
		});
	}
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
// mainly executed after the successful purchase automatically i.e. ACTION - buyitemsalel by system/platform
void oyanftmarket::delsale(
				uint64_t sale_id
			)
{
	require_auth(get_self());

	sale_index sale_table(get_self(), get_self().value);
	auto sale_it = sale_table.find(sale_id);

	check(sale_it != sale_table.end(), "the sale id doesn\'t exist.");

	// remove the item_id(s) also from `asset_item_ids_listed_sale` in asset table
	for(auto&& item_id : sale_it->item_ids) {
		auto item_id_it = std::find(asset_it->asset_item_ids_listed_sale.begin(), asset_it->asset_item_ids_listed_sale.end(), item_id);
		if(item_id_it != asset_it->asset_item_ids_listed_sale.end()) {		// if item_id found
			asset_table.modify(asset_it, get_self(), [&](auto &row){
				row.asset_item_ids_listed_sale.erase(item_id_it);
			});
		}	
	}

	// delete the sale after successful purchase
	sale_table.erase(sale_it);
}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::listitemauct(
				const name& seller_id,
				const name& collection_name,
				const vector<uint64_t> item_ids,
				const name& price_mode,
				const asset& current_bid_crypto,
				float current_bid_fiat_usd,
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
		"the no. of items remaining to be listed is less than the parsed items size." ):

	check( (price_mode == "crypto"_n) || (price_mode == "fiat"_n), "invalid price mode.");

	// check item_ids must not be in listed auction
	for(auto&& item_id : item_ids) {
		check(!has_item_in_vector(asset_it->asset_item_ids_listed_auct, item_id), 
			"the item id: \'".append(item_id).append("\' is already listed on auction in asset table"));
	}

	check( current_bid_crypto.is_valid(), "invalid price in crypto");
	check( current_bid_crypto.amount > 0, "crypto qty must be positive");

	// find out if seller is a creator or not from assets table
	bool is_creator = false;
	if (asset_it->creator_id == seller_id) is_creator = true;

	// prove the ownership for seller_id
	if (!is_creator) {			// if non-creator
		oyanocreator_index oyanocreator_table(get_self(), seller_id);
		oyanocreator_it = oyanocreator_table.find(asset_id);

		check(oyanocreator_it != oyanocreator_table.end(), "the asset containing this item doesn\'t exist for this non-creator.");
		
		// check the seller (non-creator) owns the item_ids 
		for(auto&& item_id : item_ids) {
			check(has_item_in_vector(oyanocreator_it->item_ids, item_id), 
				"the item id: \'".append(item_id).append("\' is not owned by the non-creator seller."));
		}

	} 

	// create unique auction id i.e. 3701<current_time><last_3_digit_tg_id>
	uint64_t auction_id = create_astsaleauc_id(3701, seller_id);

	auction_index auction_table(get_self(), get_self().value);
	auto auction_it = auction_table.find(auction_id);

	check(auction_it == auction_table.end(), "the auction already exist.");

	auction_table.emplace(get_self(), [&](auto &row){
		row.auction_id = auction_id;
		row.item_ids = item_ids;
		row.asset_id = asset_id;
		row.seller_id = seller_id;
		row.start_time = now();
		row.end_time = 0;
		if(price_mode == "crypto"_n) {
			row.current_bid_crypto = current_bid_crypto;
			row.current_bid_fiat_usd = 0;
		}
		else if(price_mode == "fiat"_n){
			row.current_bid_fiat_usd = current_bid_fiat_usd;
			row.current_bid_crypto.symbol = cryptopay_token_symbol;
		}
		row.claimed_by_seller = 0;
		row.claimed_by_buyer = 0;
		row.collection_name = collection_name;
		row.royalty_fee = asset_id->asset_royaltyfee;
	});

	// add the item_ids into the asset_item_ids_listed_auct in asset table
	asset_table.modify(asset_it, get_self(), [&](auto &row){
		row.asset_item_ids_listed_auct.insert(
			asset_it->asset_item_ids_listed_auct.end(),
			item_ids.begin(),
			item_ids.end()
		);
	});

}

// --------------------------------------------------------------------------------------------------------------------
// void oyanftmarket::additemauct(
// 				uint64_t sale_id, 
// 				uint64_t seller_id,
// 				const vector<uint64_t> item_ids
// 			)
// {
// 	require_auth(get_self());

// 	sale_index sale_table(get_self(), get_self().value);
// 	auto sale_it = sale_table.find(sale_id);

// 	check(sale_it != sale_table.end(), "the sale id doesn\'t exist.");

// 	// checked the seller is original
// 	check(sale_it->seller_id == seller_id, "the parsed seller_id doesn\'t match with the actual one for this sale.");

// 	asset_index asset_table(get_self(), sale_it->collection_name.value);
// 	auto asset_it = asset_table.find(sale_it->asset_id);

// 	check(asset_it != asset_table.end(), "there is no such asset id for the sale\'s collection name");

// 	check( (asset_it->asset_copies_qty_total - asset_it->asset_item_ids_listed_sale.size() - asset_it->asset_item_ids_listed_auct.size()) >= item_ids.size(), 
// 		"the no. of items remaining for listed is less than the parsed list size." ):

// 	// check item_ids must not be in listed sale in sale table
// 	for(auto&& item_id : item_ids) {
// 		check(!has_item_in_vector(sale_it->item_ids, item_id), 
// 			"the item id: \'".append(item_id).append("\' is already listed on sale in sale table."));
// 	}

// 	// check item_ids must not be in listed sale in asset table
// 	for(auto&& item_id : item_ids) {
// 		check(!has_item_in_vector(asset_it->asset_item_ids_listed_sale, item_id), 
// 			"the item id: \'".append(item_id).append("\' is already listed on sale in asset table."));
// 	}

// 	// add the item_ids into the `item_ids` in sales table
// 	sale_table.modify(sale_it, get_self(), [&](auto &row){
// 		row.item_ids.insert(
// 			sale_it->item_ids.end(),
// 			item_ids.begin(),
// 			item_ids.end()
// 		);
// 	});

// 	// add the item_ids into the `asset_item_ids_listed_sale` in asset table
// 	asset_table.modify(asset_it, get_self(), [&](auto &row){
// 		row.asset_item_ids_listed_sale.insert(
// 			asset_it->asset_item_ids_listed_sale.end(),
// 			item_ids.begin(),
// 			item_ids.end()
// 		);
// 	});

// }

// // --------------------------------------------------------------------------------------------------------------------
// void oyanftmarket::rmitemauct(
// 				uint64_t sale_id, 
// 				uint64_t seller_id,
// 				const vector<uint64_t> item_ids
// 			)
// {
// 	require_auth(get_self());

// 	sale_index sale_table(get_self(), get_self().value);
// 	auto sale_it = sale_table.find(sale_id);

// 	check(sale_it != sale_table.end(), "the sale id doesn\'t exist.");

// 	// checked the seller is original
// 	check(sale_it->seller_id == seller_id, "the parsed seller_id doesn\'t match with the actual one for this sale.");

// 	asset_index asset_table(get_self(), sale_it->collection_name.value);
// 	auto asset_it = asset_table.find(sale_it->asset_id);

// 	check(asset_it != asset_table.end(), "there is no such asset id for the sale\'s collection name");

// 	check( (asset_it->asset_copies_qty_total - asset_it->asset_item_ids_listed_sale.size() - asset_it->asset_item_ids_listed_auct.size()) >= item_ids.size(), 
// 		"the no. of items remaining for listed is less than the parsed list size." ):

// 	// check item_ids must be in listed sale in sale table
// 	for(auto&& item_id : item_ids) {
// 		check(has_item_in_vector(sale_it->item_ids, item_id), 
// 			"the item id: \'".append(item_id).append("\' is not listed on sale in sale table"));
// 	}

// 	// check item_ids must be in listed sale in 
// 	for(auto&& item_id : item_ids) {
// 		check(has_item_in_vector(asset_it->asset_item_ids_listed_sale, item_id), 
// 			"the item id: \'".append(item_id).append("\' is not listed on sale in asset table"));
// 	}


// 	// remove the item_id(s) also from asset_item_ids_listed_sale in asset table
// 	for(auto&& item_id : item_ids) {
// 		auto item_id_ast_it = std::find(asset_it->asset_item_ids_listed_sale.begin(), asset_it->asset_item_ids_listed_sale.end(), item_id);
// 		if(item_id_ast_it != asset_it->asset_item_ids_listed_sale.end()) {		// if item_id found
// 			asset_table.modify(asset_it, get_self(), [&](auto &row){
// 				row.asset_item_ids_listed_sale.erase(item_id_ast_it);
// 			});
// 		}	
// 	}

// 	// remove item_ids from the sale table
// 	for(auto&& item_id : item_ids) {
// 		auto item_id_sale_it = std::find(sale_it->item_ids.begin(), sale_it->item_ids.end(), item_id);
// 		if(item_id_sale_it != sale_it->item_ids.end()) {		// if item_id found
// 			sale_table.modify(sale_it, get_self(), [&](auto &row){
// 				row.item_ids.erase(item_id_sale_it);
// 			});
// 		}	
// 	}

// }


// // --------------------------------------------------------------------------------------------------------------------
// void oyanftmarket::setpriceauct(
// 				uint64_t sale_id,
// 				uint64_t seller_id,
// 				const name& price_mode,
// 				const asset& listing_price_crypto,
// 				float listing_price_fiat_usd
// 			)
// {
// 	require_auth(get_self());

// 	sale_index sale_table(get_self(), get_self().value);
// 	auto sale_it = sale_table.find(sale_id);

// 	check(sale_it != sale_table.end(), "the sale id doesn\'t exist.");

// 	// checked the seller is original
// 	check(sale_it->seller_id == seller_id, "the parsed seller_id doesn\'t match with the actual one for this sale.");

// 	check( (price_mode == "crypto"_n) || (price_mode == "fiat"_n), "invalid price mode.");

// 	if (price_mode == "crypto"_n) {
// 		check( listing_price_crypto.amount > 0, "the listing price amount in crypto must be positive.");

// 		check(has_item_in_vector(crypto_token_symbol_list, listing_price_crypto.symbol), "Only chosen tokens are accepted as crypto for trading asset.");

// 		sale_table.modify(sale_it, get_self(), [&](auto &row){;
// 			row.listing_price_crypto = listing_price_crypto;
// 		});

// 	} else if (price_mode == "fiat"_n)  {
// 		check(listing_price_fiat_usd > 0.0, "the listing price amount in USD amount must be positive.")

// 		sale_table.modify(sale_it, get_self(), [&](auto &row){;
// 			row.listing_price_fiat_usd = listing_price_fiat_usd;
// 		});
// 	}
// }

// // --------------------------------------------------------------------------------------------------------------------
// void oyanftmarket::buyitemauct(
// 				uint64_t sale_id,
// 				uint64_t buyer_id,
// 				const name& pay_mode
// 			)
// {
// 	require_auth(get_self());

// 	sale_index sale_table(get_self(), get_self().value);
// 	auto sale_it = sale_table.find(sale_id);

// 	check(sale_it != sale_table.end(), "the sale id doesn\'t exist.");

// 	// instantiate the asset table
// 	asset_index asset_table(get_self(), sale_it->collection_name.value);
// 	auto asset_it = asset_table.find(sale_it->asset_id);

// 	check(asset_it != asset_table.end(), "there is no such asset id for the sale\'s collection name");

// 	check( (pay_mode == "crypto"_n) || (pay_mode == "fiat"_n), "invalid price mode.");

// 	// ************************************
// 	// Payment (crypto)
// 	if (pay_mode == "crypto"_n) {
// 		// create asset for seller
// 		auto qty_seller = asset(0, sale_it->listing_price_crypto.symbol);
// 		qty_seller.amount = sale_it->listing_price_crypto.amount * (1 - sale_it->royalty_fee - platform_commission_rate); 

// 		// create asset for creator
// 		auto qty_creator = asset(0, sale_it->listing_price_crypto.symbol);
// 		qty_creator.amount = sale_it->listing_price_crypto.amount * sale_it->royalty_fee; 

// 		sub_balance(buyer_id, sale_it->listing_price_crypto);			// from buyer
// 		add_balance(sale_it->seller_id, qty_seller, get_self());		// to seller
// 		add_balance(asset_it->creator_id, qty_creator, get_self());		// to creator as royalty_fee

// 	}

// 	// ************************************
// 	// Seller
// 	bool seller_is_creator = false;
// 	// check if seller is an creator or not from asset table corresponding to asset_id
// 	if(asset_it->creator_id == sale_it->seller_id) seller_is_creator = true;

// 	if(!seller_is_creator) {
// 		oyanocreator_index oyanocreator_seller_table(get_self(), sale_it->seller_id);
// 		oyanocreator_seller_it = oyanocreator_seller_table.find(sale_it->asset_id);

// 		check(oyanocreator_seller_it != oyanocreator_seller_table.end(), "the asset containing this item doesn\'t exist for this non-creator seller.");

// 		for(auto&& item_id : sale_it->item_ids) {
// 			// check the items exist in `item_ids` in oyanocreator table
// 			check(has_item_in_vector(oyanocreator_seller_it->item_ids, item_id), 
// 				"the item id: \'".append(item_id).append("\' is not owned by the seller in oyanocreator table."));

// 			// remove the items from `item_ids` in oyanocreator table
// 			auto item_id_it = std::find(oyanocreator_seller_it->item_ids.begin(), oyanocreator_seller_it->item_ids.end(), item_id);
// 			if(item_id_it != oyanocreator_seller_it->item_ids.end()) {		// if item_id found
// 				oyanocreator_seller_table.modify(oyanocreator_seller_it, get_self(), [&](auto &row){
// 					row.item_ids.erase(item_id_it);
// 				});
// 			}	
// 		}

// 	} else {
// 		// check that the items does not exist into `asset_item_ids_transferred` in asset table
// 		for(auto&& item_id : sale_it->item_ids) {
// 			check(!has_item_in_vector(asset_it->asset_item_ids_transferred, item_id), "the item id: \'".append(item_id).append("\' already exist as transferred item in asset table"));
// 		}

// 		// add the items into `asset_item_ids_transferred` in asset table
// 		asset_table.modify(asset_it, get_self(), [&](auto &row){
// 			row.asset_item_ids_transferred.insert(asset_it->asset_item_ids_transferred.end(),sale_it->item_ids.begin(), sale_it->item_ids.end());
// 		});

// 	}

// 	for (auto&& item_id : sale_it->item_ids) {
// 		// check the item_ids in sale table exist in `asset_item_ids_listed_sale` in asset table
// 		check(has_item_in_vector(asset_it->asset_item_ids_listed_sale, item_id), 
// 			"the item id: \'".append(item_id).append("\' is not listed on sale in asset table"));

// 		// remove items from the `asset_item_ids_listed_sale` in asset table
// 		auto item_id_it = std::find(asset_it->asset_item_ids_listed_sale.begin(), asset_it->asset_item_ids_listed_sale.end(), item_id);
// 		if(item_id_it != asset_it->asset_item_ids_listed_sale.end()) {		// if item_id found
// 			asset_table.modify(asset_it, get_self(), [&](auto &row){
// 				row.asset_item_ids_listed_sale.erase(item_id_it);
// 			});
// 		}

// 	}


// 	// ************************************
// 	// Buyer
// 	// NOTE: The buyer could be creator (where, try to buy the item_id back) or non-creator
// 	bool buyer_is_creator = false;
// 	// check if buyer is an creator or not from asset table corresponding to asset_id
// 	if(asset_it->creator_id == buyer_id) buyer_is_creator = true;

// 	if(!buyer_is_creator) {
// 		oyanocreator_index oyanocreator_buyer_table(get_self(), buyer_id);
// 		oyanocreator_buyer_it = oyanocreator_buyer_table.find(sale_it->asset_id);

// 		check(oyanocreator_buyer_it != oyanocreator_buyer_table.end(), "the asset containing this item doesn\'t exist for this non-creator buyer.");

// 		for(auto&& item_id : sale_it->item_ids) {
// 			// check that the buyer doesn't already own these items in oyanocreator table
// 			check(!has_item_in_vector(oyanocreator->item_ids, item_id), "the item id: \'".append(item_id).append("\' already owned by the buyer in oyanocreator table"));
// 		}

// 		// add the items into `item_ids` in oyanocreator table
// 		oyanocreator_buyer_table.modify(oyanocreator_buyer_it, get_self(), [&](auto &row){
// 			row.item_ids.insert(oyanocreator_buyer_it->item_ids.end(), sale_it->item_ids.begin(), sale_it->item_ids.end());
// 		});

// 	} else {
// 		for (auto&& item_id : sale_it->item_ids) {
// 			// check the item_ids exist in `asset_item_ids_transferred` in asset table
// 			check(has_item_in_vector(asset_id->asset_item_ids_transferred, item_id), "the item id: \'".append(item_id).append("\' doesn\'t exist as transferred item in asset table."));
	
// 			// restore by removing the items from `asset_item_ids_transferred` in asset table
// 			auto item_id_it = std::find(asset_it->asset_item_ids_transferred.begin(), asset_it->asset_item_ids_transferred.end(), item_id);
// 			if(item_id_it != asset_it->asset_item_ids_transferred.end()) {		// if item_id found
// 				asset_table.modify(asset_it, get_self(), [&](auto &row){
// 					row.asset_item_ids_transferred.erase(item_id_it);
// 				});
// 			}

// 		}


// 	}

// 	// ************************************
// 	// add buyer in sale table
// 	sale_table.modify(sale_it, get_self(), [&](auto &row){;
// 		row.buyer_id = buyer_id;
// 	});

// }

// // --------------------------------------------------------------------------------------------------------------------
// void oyanftmarket::ulistitemauct(
// 				uint64_t sale_id,
// 				uint64_t seller_id
// 			)
// {
// 	require_auth(get_self());

// 	sale_index sale_table(get_self(), get_self().value);
// 	auto sale_it = sale_table.find(sale_id);

// 	check(sale_it != sale_table.end(), "the sale id doesn\'t exist.");

// 	// checked the seller is original
// 	check(sale_it->seller_id == seller_id, "the parsed seller_id doesn\'t match with the actual one for this sale.");

// 	// remove the item_ids from `asset_item_ids_listed_sale` in asset table
// 	asset_index asset_table(get_self(), sale_it->collection_name.value);
// 	auto asset_it = asset_table.find(sale_it->asset_id);

// 	check(asset_it != asset_table.end(), "there is no such asset id for this sale\'s collection name");

// 	// remove the item_id(s) also from asset_item_ids_listed_sale in asset table
// 	for(auto&& item_id : sale_it->item_ids) {
// 		auto item_id_it = std::find(asset_it->asset_item_ids_listed_sale.begin(), asset_it->asset_item_ids_listed_sale.end(), item_id);
// 		if(item_id_it != asset_it->asset_item_ids_listed_sale.end()) {		// if item_id found
// 			asset_table.modify(asset_it, get_self(), [&](auto &row){
// 				row.asset_item_ids_listed_sale.erase(item_id_it);
// 			});
// 		}	
// 	}

// 	sale_table.erase(sale_it);

// }


// // --------------------------------------------------------------------------------------------------------------------
// // mainly executed after the successful purchase automatically i.e. ACTION - buyitemsalel by system/platform
// void oyanftmarket::delauct(
// 				uint64_t sale_id
// 			)
// {
// 	require_auth(get_self());

// 	sale_index sale_table(get_self(), get_self().value);
// 	auto sale_it = sale_table.find(sale_id);

// 	check(sale_it != sale_table.end(), "the sale id doesn\'t exist.");

// 	// remove the item_id(s) also from `asset_item_ids_listed_sale` in asset table
// 	for(auto&& item_id : sale_it->item_ids) {
// 		auto item_id_it = std::find(asset_it->asset_item_ids_listed_sale.begin(), asset_it->asset_item_ids_listed_sale.end(), item_id);
// 		if(item_id_it != asset_it->asset_item_ids_listed_sale.end()) {		// if item_id found
// 			asset_table.modify(asset_it, get_self(), [&](auto &row){
// 				row.asset_item_ids_listed_sale.erase(item_id_it);
// 			});
// 		}	
// 	}

// 	// delete the sale after successful purchase
// 	sale_table.erase(sale_it);
// }
