#include "oyanftmarket.hpp"

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::deposit( const name& from_ac, 
							const name& contract_ac, 
							const asset& quantity,
							const string& memo ) 
{
	// check for conditions if either or both of these are true for `from_ac` & `to`
	// M-1: here, the msg will be printed on the console, provided print --to-console is activated on the used nodeos url
	if(contract_ac != get_self() ||  from_ac == get_self()) {		// atleast one of the condition is true
		print("Either money is not sent to the contract or contract itself is the buyer.");
		return;
	}
	
/*	// M-2: here, the contract ac can't be payer for RAM, CPU, NET then it will throw the error as eosio.token::transfer notifies this ACTION
	check( (contract_ac == get_self()) && (from_ac != get_self()), "Either money is not sent to the contract or contract itself is the sender.");
*/
	// Although this is checked in "eosio.token::transfer" action, but fund_token_symbol check is pending. 
	// So, in addition the entire asset check is done using static func defined in "dciico.hpp" file.
	// check quantity is valid for all conditions as 'asset'
	// check_quantity(quantity, fund_token_symbol, native_token_symbol);

	// if there is no alphabets, just the telegram_id in memo.
	if(count_alpha(memo) == 0) {
		auto tg_id = str_to_uint64t(memo);		// capture the telegram_id by converting from string to uint64_t

		// instantiate the `account` table
		account_index account_table(get_self(), get_self().value);
		auto account_it = account_table.find(tg_id);

		// update (emplace/modify) the deposit_qty
		if(account_it == account_table.end()) {
			account_table.emplace(get_self(), [&](auto& row) {
				row.owner = tg_id;
				row.balances = map<extended_symbol, uint64_t>{
					make_pair(extended_symbol(quantity.symbol, get_first_receiver()), quantity.amount)
				};
			});
		} 
		else {
			account_table.modify(account_it, get_self(), [&](auto& row) {
				creatify_balances_map(row.balances, quantity, 1);		// 1 for add balance
/*
				// ----------------------------------------------------------------------------
				// code snippet for modifying the value in place of creatify_balances_map() func
				// ----------------------------------------------------------------------------
				bool arithmetic_op = 1;		// 1 for add balance
				auto s_it = std::find_if(row.balances.begin(), row.balances.end(), 
									[&](auto& ms) {return ms.first.get_symbol() == quantity.symbol;});
				
				if(s_it != row.balances.end()) {		// key found
					if (arithmetic_op == 1)
						s_it->second += quantity.amount;
					else if (arithmetic_op == 0)
						s_it->second -= quantity.amount;
				}
				else {						// key NOT found
					row.balances.insert( make_pair(extended_symbol(quantity.symbol, get_first_receiver()), quantity.amount) );
				}
				// ----------------------------------------------------------------------------
				// ----------------------------------------------------------------------------
*/			});
		}

	}

}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::withdraw( uint64_t from_id,
							 const string& from_username,
							 const name& to_ac,
							 const asset& quantity,
							 const string& memo 
							)
{
	require_auth(get_self());

	check( quantity.is_valid(), "invalid quantity" );
	check( quantity.amount > 0, "must withdraw positive quantity" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

	// instantiate the `account` table
	account_index account_table(get_self(), get_self().value);
	auto frm_account_it = account_table.find(from_id);

	check(frm_account_it != account_table.end(), "there is no account available for the given from_id.");

	// check the amount present in balances map's value
	check_amount_in_map( frm_account_it->balances, quantity );

    // transfer quantity from oyanftmarket contract to to_ac
    action(
		permission_level{get_self(), "active"_n},
		capture_contract_in_map(frm_account_it->balances, quantity),
		"transfer"_n,
		std::make_tuple(get_self(), to_ac, quantity, memo)
	).send();

	// update (substract) the balances' value in from_id accounts table
	account_table.modify(frm_account_it, get_self(), [&](auto& row) {
		creatify_balances_map(row.balances, quantity, 0);		// 0 for sub balance
/*
		// ----------------------------------------------------------------------------
		// code snippet for modifying the value in place of creatify_balances_map() func
		// ----------------------------------------------------------------------------
		bool arithmetic_op = 0;		// 0 for sub balance
		auto s_it = std::find_if(row.balances.begin(), row.balances.end(), 
							[&](auto& ms) {return ms.first.get_symbol() == quantity.symbol;});
		
		if(s_it != row.balances.end()) {		// key found
			if (arithmetic_op == 1)
				s_it->second += quantity.amount;
			else if (arithmetic_op == 0)
				s_it->second -= quantity.amount;
		}
		else {						// key NOT found
			row.balances.insert( make_pair(extended_symbol(quantity.symbol, get_first_receiver()), quantity.amount) );
		}
		// ----------------------------------------------------------------------------
		// ----------------------------------------------------------------------------
*/	});

	// erase the from_id in accounts table due to zero balances' value
	// this is to save the contract's RAM space
	auto s_it = std::find_if(frm_account_it->balances.begin(), frm_account_it->balances.end(), 
							[&](auto& ms) {return ms.first.get_symbol() == quantity.symbol;});
		
	if(s_it != frm_account_it->balances.end()) {		// key found
		if (s_it->second == 0)			// if val == 0
			account_table.erase(frm_account_it);
	}	
}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::tip(
						uint64_t from_id,
						uint64_t to_id,
						const string& from_username,
						const string& to_username,
						const asset& quantity,
						const string& memo )
{
	require_auth(get_self());

	check(from_id != to_id, "from_id & to_id can\'t be same.");
	check(from_username != to_username, "from_username & to_username can\'t be same.");
	
	check( quantity.is_valid(), "invalid quantity" );
	check( quantity.amount > 0, "must transfer positive quantity" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

	// instantiate the `account` table
	account_index account_table(get_self(), get_self().value);
	auto frm_account_it = account_table.find(from_id);
	auto to_account_it = account_table.find(to_id);

	check(frm_account_it != account_table.end(), "there is no account available for the given from_id.");

	// check the amount present in balances map's value
	check_amount_in_map( frm_account_it->balances, quantity );

	// -------------------------------------------------------------------------
	// update (substract) the `balances' value in from_id accounts table
	account_table.modify(frm_account_it, get_self(), [&](auto& row) {
			creatify_balances_map(row.balances, quantity, 0);		// 0 for sub balance
/*
		// ----------------------------------------------------------------------------
		// code snippet for modifying the value in place of creatify_balances_map() func
		// ----------------------------------------------------------------------------
		bool arithmetic_op = 0;		// 0 for sub balance
		auto s_it = std::find_if(row.balances.begin(), row.balances.end(), 
							[&](auto& ms) {return ms.first.get_symbol() == quantity.symbol;});
		
		if(s_it != row.balances.end()) {		// key found
			if (arithmetic_op == 1)
				s_it->second += quantity.amount;
			else if (arithmetic_op == 0)
				s_it->second -= quantity.amount;
		}
		else {						// key NOT found
			row.balances.insert( make_pair(extended_symbol(quantity.symbol, get_first_receiver()), quantity.amount) );
		}
		// ----------------------------------------------------------------------------
		// ----------------------------------------------------------------------------
*/	});

	// -------------------------------------------------------------------------
	// update (add) the balances' value in to_id accounts table
	if(to_account_it == account_table.end()) {						// table for to_ac doesn't exist
		account_table.emplace(get_self(), [&](auto& row) {
			row.owner = to_id;
			row.balances = map<extended_symbol, uint64_t>{
				make_pair(extended_symbol(quantity.symbol, capture_contract_in_map( frm_account_it->balances, quantity )), quantity.amount)
			};
		});
	} else {														// table for to_ac exist
		account_table.modify(to_account_it, get_self(), [&](auto& row) {
			creatify_balances_map(row.balances, quantity, 1);	// 1 for add balance

/*			// ----------------------------------------------------------------------------
			// code snippet for modifying the value in place of creatify_balances_map() func
			// ----------------------------------------------------------------------------
			bool arithmetic_op = 1;		// 1 for add balance
			auto s_it = std::find_if(row.balances.begin(), row.balances.end(), 
								[&](auto& ms) {return ms.first.get_symbol() == quantity.symbol;});
			
			if(s_it != row.balances.end()) {		// key found
				if (arithmetic_op == 1)
					s_it->second += quantity.amount;
				else if (arithmetic_op == 0)
					s_it->second -= quantity.amount;
			}
			else {						// key NOT found
				row.balances.insert( make_pair(extended_symbol(quantity.symbol, get_first_receiver()), quantity.amount) );
			}
			// ----------------------------------------------------------------------------
			// ----------------------------------------------------------------------------
*/
		});
	}

	// -------------------------------------------------------------------------
	// erase the from_id in accounts table due to zero balances' value
	// this is to save the contract's RAM space
	auto s_it = std::find_if(frm_account_it->balances.begin(), frm_account_it->balances.end(), 
							[&](auto& ms) {return ms.first.get_symbol() == quantity.symbol;});
		
	if(s_it != frm_account_it->balances.end()) {		// key found
		if (s_it->second == 0)			// if val == 0
			account_table.erase(frm_account_it);
	}

	// if memo contains 'sponsor <id>', then add the from_id to the creator's asset table
	
}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::addmodcol(
				uint64_t creator_id,
				const name& collection_name,
				const string& collection_displayname,
				const string& collection_desc,
				const string& collection_url
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
	check(asset_copies_qty_total <= 99999, "The total asset copies can\'t be greater than 99999.");

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

	// Instantiate the asset table
	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	check(asset_it != asset_table.end(), "The asset id is not present.");

	// check for total asset copies as <= 99999
	check(asset_it->asset_copies_qty_total <= 99999, "The total asset copies can\'t be greater than 99999.");

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

	asset_table.erase(asset_it);
}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::delitem(
				const name& collection_name,
				uint64_t asset_id,
				uint64_t creator_id,
				uint64_t item_qty
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
	check( (asset_it->asset_copies_qty_total - asset_it->asset_item_ids_listed_sale.size() - asset_it->asset_item_ids_listed_auct.size()) >= item_qty, 
		"sorry, the no. of items not listed is less than the item qty to be deleted." );

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
	auto oyanocreator_it = oyanocreator_table.find(asset_id);

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
	auto oyanocreator_it = oyanocreator_table.find(asset_id);

	check(oyanocreator_it != oyanocreator_table.end(), "the asset containing this item doesn\'t exist for this non-creator.");

	// check if item is present in `oyanocreator_it->item_ids` list
	check(has_item_in_vector(oyanocreator_it->item_ids, item_id), std::to_string("Sorry!, the item id doesn\'t exist in the items list for the asset id \'").append(std::to_string(asset_id)).append("\' for this non-creator.") );

	// find out the position if item_id exist
	auto item_id_it = std::find(oyanocreator_it->item_ids.begin(), oyanocreator_it->item_ids.end(), item_id);
	oyanocreator_table.modify(oyanocreator_it, get_self(), [&](auto &row){
		if (item_id_it != oyanocreator_it->item_ids.end())		// if item found
			row.item_ids.erase(item_id_it);
	});

	// lastly, delete the asset id row, if there is no item inside list, after removing parsed item_id. This is to restore the contract's RAM.
	if( oyanocreator_it->item_ids.size() == 1  )
		oyanocreator_table.erase(oyanocreator_it);

}


// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::listitemsale(
				uint64_t seller_id,
				const name& collection_name,
				const vector<uint64_t> item_ids,
				const name& price_mode,
				const asset& listing_price_crypto,
				float listing_price_fiat_usd
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
		"the no. of items remaining to be listed is less than the parsed items size." );

	check( (price_mode == "crypto"_n) || (price_mode == "fiat"_n), "invalid price mode.");

	// check item_ids must not be in listed sale
	for(auto&& item_id : item_ids) {
		check(!has_item_in_vector(asset_it->asset_item_ids_listed_sale, item_id), 
			"the item id: \'".append(item_id).append("\' is already listed on sale in asset table"));
	}

	// find out if seller is a creator or not from assets table
	bool is_creator = false;
	if (asset_it->creator_id == seller_id) is_creator = true;

	// prove the ownership for seller_id
	if (!is_creator) {			// if non-creator
		oyanocreator_index oyanocreator_table(get_self(), seller_id);
		auto oyanocreator_it = oyanocreator_table.find(asset_id);

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
			check( listing_price_crypto.is_valid(), "invalid price in crypto");
			check( listing_price_crypto.amount > 0, "crypto qty must be positive");

			row.listing_price_crypto = listing_price_crypto;
			row.listing_price_fiat_usd = 0;
		}
		else if(price_mode == "fiat"_n){
			check( listing_price_fiat_usd > 0, "fiat usd price must be positive");

			row.listing_price_fiat_usd = listing_price_fiat_usd;
			// row.listing_price_crypto.symbol = cryptopay_token_symbol;
		}
		row.collection_name = collection_name;
		row.royalty_fee = asset_it->asset_royaltyfee;
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

	// check the sale is still ongoing
	check(sale_it->buyer_id == 0, "the sale is closed now, so items can\'t be added.");

	asset_index asset_table(get_self(), sale_it->collection_name.value);
	auto asset_it = asset_table.find(sale_it->asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for the sale\'s collection name");

	check( (asset_it->asset_copies_qty_total - asset_it->asset_item_ids_listed_sale.size() - asset_it->asset_item_ids_listed_auct.size()) >= item_ids.size(), 
		"the no. of items remaining for listed is less than the parsed list size." );

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

	// check the sale is still ongoing
	check(sale_it->buyer_id == 0, "the sale is closed now, so items can\'t be removed.");

	asset_index asset_table(get_self(), sale_it->collection_name.value);
	auto asset_it = asset_table.find(sale_it->asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for the sale\'s collection name");

	check( (asset_it->asset_copies_qty_total - asset_it->asset_item_ids_listed_sale.size() - asset_it->asset_item_ids_listed_auct.size()) >= item_ids.size(), 
		"the no. of items remaining for listed is less than the parsed list size." );

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

	// check the sale is still ongoing
	check(sale_it->buyer_id == 0, "the sale is closed now, so price can\'t be set.");

	check( (price_mode == "crypto"_n) || (price_mode == "fiat"_n), "invalid price mode.");

	if (price_mode == "crypto"_n) {
		check( listing_price_crypto.amount > 0, "the listing price amount in crypto must be positive.");

		check(has_item_in_vector(crypto_token_symbol_list, listing_price_crypto.symbol), "Only chosen tokens are accepted as crypto for trading asset.");

		sale_table.modify(sale_it, get_self(), [&](auto &row){;
			row.listing_price_crypto = listing_price_crypto;
		});

	} else if (price_mode == "fiat"_n)  {
		check(listing_price_fiat_usd > 0, "the listing price amount in USD amount must be positive.");

		sale_table.modify(sale_it, get_self(), [&](auto &row){;
			row.listing_price_fiat_usd = listing_price_fiat_usd;
		});
	}
}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::buysale(
				uint64_t sale_id,
				uint64_t buyer_id,
				const name& pay_mode
			)
{
	require_auth(get_self());

	sale_index sale_table(get_self(), get_self().value);
	auto sale_it = sale_table.find(sale_id);

	check(sale_it != sale_table.end(), "the sale id doesn\'t exist.");

	// check the sale is still ongoing
	check(sale_it->buyer_id == 0, "the sale is closed now, so can\'t be purchased.");

	// instantiate the asset table
	asset_index asset_table(get_self(), sale_it->collection_name.value);
	auto asset_it = asset_table.find(sale_it->asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for the sale\'s collection name");

	check( (pay_mode == "crypto"_n) || (pay_mode == "fiat"_n), "invalid pay mode.");

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
		add_balance(buyer_id, sale_it->seller_id, qty_seller, get_self());		// to seller
		add_balance(buyer_id, asset_it->creator_id, qty_creator, get_self());		// to creator as royalty_fee
		// TODO: also send the creator's share into it's investors
	}

	// ************************************
	// Seller
	bool seller_is_creator = false;
	// check if seller is an creator or not from asset table corresponding to asset_id
	if(asset_it->creator_id == sale_it->seller_id) seller_is_creator = true;

	if(!seller_is_creator) {
		oyanocreator_index oyanocreator_seller_table(get_self(), sale_it->seller_id);
		auto oyanocreator_seller_it = oyanocreator_seller_table.find(sale_it->asset_id);

		check(oyanocreator_seller_it != oyanocreator_seller_table.end(), "the asset containing item(s) doesn\'t exist for this non-creator seller.");

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
		auto oyanocreator_buyer_it = oyanocreator_buyer_table.find(sale_it->asset_id);

		check(oyanocreator_buyer_it != oyanocreator_buyer_table.end(), "the asset containing this item doesn\'t exist for this non-creator buyer.");

		for(auto&& item_id : sale_it->item_ids) {
			// check that the buyer doesn't already own these items in oyanocreator table
			check(!has_item_in_vector(oyanocreator_buyer_it->item_ids, item_id), "the item id: \'".append(item_id).append("\' already owned by the buyer in oyanocreator table"));
		}

		// add the items into `item_ids` in oyanocreator table
		oyanocreator_buyer_table.modify(oyanocreator_buyer_it, get_self(), [&](auto &row){
			row.item_ids.insert(oyanocreator_buyer_it->item_ids.end(), sale_it->item_ids.begin(), sale_it->item_ids.end());
		});

	} else {
		for (auto&& item_id : sale_it->item_ids) {
			// check the item_ids exist in `asset_item_ids_transferred` in asset table
			check(has_item_in_vector(asset_it->asset_item_ids_transferred, item_id), "the item id: \'".append(item_id).append("\' doesn\'t exist as transferred item in asset table."));
	
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
void oyanftmarket::unlistsale(
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

	// check the sale is still ongoing
	check(sale_it->buyer_id == 0, "the sale is closed now, so can\'t be unlisted.");

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
// mainly executed after the successful purchase automatically i.e. ACTION - `buysale` by system/platform
void oyanftmarket::delsale(
				uint64_t sale_id
			)
{
	require_auth(get_self());

	sale_index sale_table(get_self(), get_self().value);
	auto sale_it = sale_table.find(sale_id);

	check(sale_it != sale_table.end(), "the sale id doesn\'t exist.");

	check(sale_it->buyer_id != 0, "the sale is still ongoing, so can\'t be deleted now.");

	// instantiate the asset table
	asset_index asset_table(get_self(), sale_it->collection_name.value);
	auto asset_it = asset_table.find(sale_it->asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for the sale\'s collection name");

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
				uint64_t seller_id,
				const name& collection_name,
				const vector<uint64_t> item_ids,
				uint32_t end_time,
				const name& price_mode,
				const asset& current_price_crypto,
				float current_price_fiat_usd
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
		"the no. of items remaining to be listed is less than the parsed items size." );

	check( (price_mode == "crypto"_n) || (price_mode == "fiat"_n), "invalid price mode.");

	// check item_ids must not be in listed auction
	for(auto&& item_id : item_ids) {
		check(!has_item_in_vector(asset_it->asset_item_ids_listed_auct, item_id), 
			"the item id: \'".append(item_id).append("\' is already listed on auction in asset table"));
	}

	// check end_time can't be now()
	check(end_time != now(), "end_time can\'t be set as current time");

	check( current_price_crypto.is_valid(), "invalid price in crypto");
	check( current_price_crypto.amount > 0, "crypto qty must be positive");

	// find out if seller is a creator or not from assets table
	bool is_creator = false;
	if (asset_it->creator_id == seller_id) is_creator = true;

	// prove the ownership for seller_id
	if (!is_creator) {			// if non-creator
		oyanocreator_index oyanocreator_table(get_self(), seller_id);
		auto oyanocreator_it = oyanocreator_table.find(asset_id);

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
		row.end_time = end_time;
		if(price_mode == "crypto"_n) {
			row.current_price_crypto = current_price_crypto;
			row.current_price_fiat_usd = 0;
		}
		else if(price_mode == "fiat"_n){
			row.current_price_fiat_usd = current_price_fiat_usd;
			row.current_price_crypto.symbol = cryptopay_token_symbol;
		}
		row.claimed_by_seller = 0;
		row.collection_name = collection_name;
		row.royalty_fee = asset_it->asset_royaltyfee;
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
void oyanftmarket::additemauct(
				uint64_t auction_id, 
				uint64_t seller_id,
				const vector<uint64_t> item_ids
			)
{
	require_auth(get_self());

	auction_index auction_table(get_self(), get_self().value);
	auto auction_it = auction_table.find(auction_id);

	check(auction_it != auction_table.end(), "the auction id doesn\'t exist.");

	// checked the seller is original
	check(auction_it->seller_id == seller_id, "the parsed seller_id doesn\'t match with the actual one for this auction.");

	// check the auction is still ongoing
	check( (!auction_it->claimed_by_seller && (auction_it->end_time >= now()) ), "the auction is closed now, so items can\'t be added.");

	asset_index asset_table(get_self(), auction_it->collection_name.value);
	auto asset_it = asset_table.find(auction_it->asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for the auction\'s collection name");

	check( (asset_it->asset_copies_qty_total - asset_it->asset_item_ids_listed_sale.size() - asset_it->asset_item_ids_listed_auct.size()) >= item_ids.size(), 
		"the no. of items remaining for listed is less than the parsed items size." );

	// check item_ids must not be in listed auction in auction table
	for(auto&& item_id : item_ids) {
		check(!has_item_in_vector(auction_it->item_ids, item_id), 
			"the item id: \'".append(item_id).append("\' is already listed on auction in auction table."));
	}

	// check item_ids must not be in listed auction in asset table
	for(auto&& item_id : item_ids) {
		check(!has_item_in_vector(asset_it->asset_item_ids_listed_auct, item_id), 
			"the item id: \'".append(item_id).append("\' is already listed on auction in asset table."));
	}

	// add the item_ids into the `item_ids` in auction table
	auction_table.modify(auction_it, get_self(), [&](auto &row){
		row.item_ids.insert(
			auction_it->item_ids.end(),
			item_ids.begin(),
			item_ids.end()
		);
	});

	// add the item_ids into the `asset_item_ids_listed_auct` in asset table
	asset_table.modify(asset_it, get_self(), [&](auto &row){
		row.asset_item_ids_listed_auct.insert(
			asset_it->asset_item_ids_listed_auct.end(),
			item_ids.begin(),
			item_ids.end()
		);
	});

}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::rmitemauct(
				uint64_t auction_id, 
				uint64_t seller_id,
				const vector<uint64_t> item_ids
			)
{
	require_auth(get_self());

	auction_index auction_table(get_self(), get_self().value);
	auto auction_it = auction_table.find(auction_id);

	check(auction_it != auction_table.end(), "the auction id doesn\'t exist.");

	// checked the seller is original
	check(auction_it->seller_id == seller_id, "the parsed seller_id doesn\'t match with the actual one for this auction.");

	// check the auction is still ongoing
	check( (!auction_it->claimed_by_seller && (auction_it->end_time >= now()) ), "the auction is closed now, so items can\'t be removed.");

	asset_index asset_table(get_self(), auction_it->collection_name.value);
	auto asset_it = asset_table.find(auction_it->asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for the auction\'s collection name");

	check( (asset_it->asset_copies_qty_total - asset_it->asset_item_ids_listed_sale.size() - asset_it->asset_item_ids_listed_auct.size()) >= item_ids.size(), 
		"the no. of items remaining for listed is less than the parsed list size." );

	// check item_ids must be in listed auction in auction table
	for(auto&& item_id : item_ids) {
		check(has_item_in_vector(auction_it->item_ids, item_id), 
			"the item id: \'".append(item_id).append("\' is not listed on auction in auction table"));
	}

	// check item_ids must be in listed auction in 
	for(auto&& item_id : item_ids) {
		check(has_item_in_vector(asset_it->asset_item_ids_listed_auct, item_id), 
			"the item id: \'".append(item_id).append("\' is not listed on auction in asset table"));
	}


	// remove the item_id(s) also from asset_item_ids_listed_auct in asset table
	for(auto&& item_id : item_ids) {
		auto item_id_ast_it = std::find(asset_it->asset_item_ids_listed_auct.begin(), asset_it->asset_item_ids_listed_auct.end(), item_id);
		if(item_id_ast_it != asset_it->asset_item_ids_listed_auct.end()) {		// if item_id found
			asset_table.modify(asset_it, get_self(), [&](auto &row){
				row.asset_item_ids_listed_auct.erase(item_id_ast_it);
			});
		}	
	}

	// remove item_ids from the auction table
	for(auto&& item_id : item_ids) {
		auto item_id_sale_it = std::find(auction_it->item_ids.begin(), auction_it->item_ids.end(), item_id);
		if(item_id_sale_it != auction_it->item_ids.end()) {		// if item_id found
			auction_table.modify(auction_it, get_self(), [&](auto &row){
				row.item_ids.erase(item_id_sale_it);
			});
		}	
	}

}


// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::setpriceauct(
				uint64_t auction_id,
				uint64_t seller_id,
				const name& price_mode,
				const asset& current_price_crypto,
				float current_price_fiat_usd
			)
{
	require_auth(get_self());

	auction_index auction_table(get_self(), get_self().value);
	auto auction_it = auction_table.find(auction_id);

	check(auction_it != auction_table.end(), "the auction id doesn\'t exist.");

	// checked the seller is original
	check(auction_it->seller_id == seller_id, "the parsed seller_id doesn\'t match with the actual one for this auction.");

	// check the auction is still ongoing
	check( (!auction_it->claimed_by_seller && (auction_it->end_time >= now()) ), "the auction is closed now, so items can\'t be added.");

	check( (price_mode == "crypto"_n) || (price_mode == "fiat"_n), "invalid price mode.");

	if (price_mode == "crypto"_n) {
		check( current_price_crypto.amount > 0, "the listing price amount in crypto must be positive.");

		check(has_item_in_vector(crypto_token_symbol_list, current_price_crypto.symbol), "Only chosen tokens are accepted as crypto for trading asset.");

		auction_table.modify(auction_it, get_self(), [&](auto &row){;
			row.current_price_crypto = current_price_crypto;
		});

	} else if (price_mode == "fiat"_n)  {
		check(current_price_fiat_usd > 0, "the listing price amount in USD amount must be positive.");

		auction_table.modify(auction_it, get_self(), [&](auto &row){;
			row.current_price_fiat_usd = current_price_fiat_usd;
		});
	}
}


// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::bidforauct(
				uint64_t auction_id,
				uint64_t bidder_id,
				const name& pay_mode,
				const asset& bid_price_crypto,
				float bid_price_fiat_usd
			)
{
	require_auth(get_self());

	auction_index auction_table(get_self(), get_self().value);
	auto auction_it = auction_table.find(auction_id);

	check(auction_it != auction_table.end(), "the auction id doesn\'t exist.");

	// check the auction is still ongoing
	check( (!auction_it->claimed_by_seller && (auction_it->end_time >= now()) ), "the auction is closed now, so bidding can\'t be done.");

	check( (pay_mode == "crypto"_n) || (pay_mode == "fiat"_n), "invalid pay mode.");

	// Payment (crypto)
	if (pay_mode == "crypto"_n) {
		auction_table.modify(auction_it, get_self(), [&](auto &row){
			bid_t b1{};
			b1.claimed_by_bidder = 0;
			b1.bid_crypto_price = bid_price_crypto;
			creatify_bidder_map(row.map_bidderid_info, bidder_id, b1, 'c');		// by default during initialization, claimed_by_bidder is set to '0'
/*			creatify_map(row.map_bidderid_claimedbybidder, bidder_id, 0);
			creatify_map(row.map_bidderid_cprice, bidder_id, bid_price_crypto);
*/		});
	}
	// Payment (fiat)
	else if (pay_mode == "fiat"_n) {
		auction_table.modify(auction_it, get_self(), [&](auto &row){
			bid_t b1{};
			b1.claimed_by_bidder = 0;
			b1.bid_fiat_price_usd = bid_price_fiat_usd;
			creatify_bidder_map(row.map_bidderid_info, bidder_id, b1, 'f');		// by default during initialization, claimed_by_bidder is set to '0'
/*			creatify_map(row.map_bidderid_claimedbybidder, bidder_id, 0);
			creatify_map(row.map_bidderid_fprice, bidder_id, bid_price_fiat_usd);
*/		});
	}

}
// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::sclaimauct(
				uint64_t auction_id,
				uint64_t bidder_id
			)
{
	require_auth(get_self());

	auction_index auction_table(get_self(), get_self().value);
	auto auction_it = auction_table.find(auction_id);

	check(auction_it != auction_table.end(), "the auction id doesn\'t exist.");

	// check the auction is still ongoing
	check( (!auction_it->claimed_by_seller && (auction_it->end_time >= now()) ), "the auction is closed now, so seller can\'t claim.");

	// check whether this bidder is found in the map - `map_bidderid_claimedbybidder`
	check( key_found_in_map(auction_it->map_bidderid_info, bidder_id), "This bidder is not found in the bidding list.");
	// check( key_found_in_map(auction_it->map_bidderid_claimedbybidder, bidder_id), "This bidder is not found in the bidding list.");

	auction_table.modify(auction_it, get_self(), [&](auto &row){
		row.claimed_by_seller = 1;
		row.confirmed_bidder_id_by_seller = bidder_id;
	});
}
// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::bclaimauct(
				uint64_t auction_id,
				uint64_t bidder_id,
				const name& pay_mode
			)
{
	require_auth(get_self());

	auction_index auction_table(get_self(), get_self().value);
	auto auction_it = auction_table.find(auction_id);

	check(auction_it != auction_table.end(), "the auction id doesn\'t exist.");

	// check that the seller has claimed this auction
	check( auction_it->claimed_by_seller, "The seller has not yet confirmed this auction.");

	// check the auction is still ongoing
	check( auction_it->end_time >= now(), "the auction is closed now, so bidder can\'t claim.");

	// check the bidder claiming matches with the confirmed bidder by seller
	check( auction_it->confirmed_bidder_id_by_seller == bidder_id, "This bidder has not been confirmed by the seller.");

	// instantiate the asset table
	asset_index asset_table(get_self(), auction_it->collection_name.value);
	auto asset_it = asset_table.find(auction_it->asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for the auction\'s collection name");

	check( (pay_mode == "crypto"_n) || (pay_mode == "fiat"_n), "invalid pay mode.");

	// ************************************
	// Payment (crypto)
	if (pay_mode == "crypto"_n) {
		check( crypto_found_in_map(auction_it->map_bidderid_info, bidder_id), "bidder had not chosen crypto pay mode for auction purchase." );
/*		check( key_found_in_map(auction_it->map_bidderid_cprice, bidder_id), "bidder had not chosen crypto pay mode for auction purchase." );
		auto bidder_price = auction_it->map_bidderid_cprice[bidder_id];
*/		auto bidder_price = auction_it->map_bidderid_info[bidder_id].bid_crypto_price;

		// create asset for seller
		auto qty_seller = asset(0, bidder_price.symbol);
		qty_seller.amount = bidder_price.amount * (1 - auction_it->royalty_fee - platform_commission_rate); 

		// create asset for creator
		auto qty_creator = asset(0, bidder_price.symbol);
		qty_creator.amount = bidder_price.amount * auction_it->royalty_fee; 

		sub_balance(bidder_id, bidder_price);			// from buyer
		add_balance(buyer_id, auction_it->seller_id, qty_seller, get_self());		// to seller
		add_balance(buyer_id, asset_it->creator_id, qty_creator, get_self());		// to creator as royalty_fee
		// TODO: also send the creator's share into it's investors
	}

	// ************************************
	// Seller
	bool seller_is_creator = false;
	// check if seller is an creator or not from asset table corresponding to asset_id
	if(asset_it->creator_id == auction_it->seller_id) seller_is_creator = true;

	if(!seller_is_creator) {
		oyanocreator_index oyanocreator_seller_table(get_self(), auction_it->seller_id);
		oyanocreator_seller_it = oyanocreator_seller_table.find(auction_it->asset_id);

		check(oyanocreator_seller_it != oyanocreator_seller_table.end(), "the asset containing item(s) doesn\'t exist for this non-creator seller.");

		for(auto&& item_id : auction_it->item_ids) {
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
		for(auto&& item_id : auction_it->item_ids) {
			check(!has_item_in_vector(asset_it->asset_item_ids_transferred, item_id), "the item id: \'".append(item_id).append("\' already exist as transferred item in asset table"));
		}

		// add the items into `asset_item_ids_transferred` in asset table
		asset_table.modify(asset_it, get_self(), [&](auto &row){
			row.asset_item_ids_transferred.insert(asset_it->asset_item_ids_transferred.end(),auction_it->item_ids.begin(), auction_it->item_ids.end());
		});

	}

	for (auto&& item_id : auction_it->item_ids) {
		// check the item_ids in auction table exist in `asset_item_ids_listed_auct` in asset table
		check(has_item_in_vector(asset_it->asset_item_ids_listed_auct, item_id), 
			"the item id: \'".append(item_id).append("\' is not listed on auction in asset table"));

		// remove items from the `asset_item_ids_listed_auct` in asset table
		auto item_id_it = std::find(asset_it->asset_item_ids_listed_auct.begin(), asset_it->asset_item_ids_listed_sale.end(), item_id);
		if(item_id_it != asset_it->asset_item_ids_listed_auct.end()) {		// if item_id found
			asset_table.modify(asset_it, get_self(), [&](auto &row){
				row.asset_item_ids_listed_auct.erase(item_id_it);
			});
		}

	}


	// ************************************
	// Bidder
	// NOTE: The bidder could be creator (where, try to buy the item_id back) or non-creator
	bool bidder_is_creator = false;
	// check if bidder is an creator or not from asset table corresponding to asset_id
	if(asset_it->creator_id == bidder_id) bidder_is_creator = true;

	if(!bidder_is_creator) {
		oyanocreator_index oyanocreator_bidder_table(get_self(), bidder_id);
		oyanocreator_bidder_it = oyanocreator_bidder_table.find(auction_it->asset_id);

		check(oyanocreator_bidder_it != oyanocreator_bidder_table.end(), "the asset containing this item doesn\'t exist for this non-creator bidder.");

		for(auto&& item_id : auction_it->item_ids) {
			// check that the bidder doesn't already own these items in oyanocreator table
			check(!has_item_in_vector(oyanocreator_bidder_it->item_ids, item_id), "the item id: \'".append(item_id).append("\' already owned by the bidder in oyanocreator table"));
		}

		// add the items into `item_ids` in oyanocreator table
		oyanocreator_bidder_table.modify(oyanocreator_bidder_it, get_self(), [&](auto &row){
			row.item_ids.insert(oyanocreator_bidder_it->item_ids.end(), auction_it->item_ids.begin(), auction_it->item_ids.end());
		});

	} else {
		for (auto&& item_id : auction_it->item_ids) {
			// check the item_ids exist in `asset_item_ids_transferred` in asset table
			check(has_item_in_vector(asset_it->asset_item_ids_transferred, item_id), "the item id: \'".append(item_id).append("\' doesn\'t exist as transferred item in asset table."));
	
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
	// bidder claim by setting '1' in auction table
	auction_table.modify(auction_it, get_self(), [&](auto &row){
		bid_t b1{};
		b1.claimed_by_bidder = 1;
		creatify_map(row.map_bidderid_info, bidder_id, b1, 'b');
		// creatify_map(row.map_bidderid_claimedbybidder, bidder_id, 1);
		row.status = 1;
	});

}

// // --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::ulistauction(
				uint64_t auction_id,
				uint64_t seller_id
			)
{
	require_auth(get_self());

	auction_index auction_table(get_self(), get_self().value);
	auto auction_it = auction_table.find(auction_id);

	check(auction_it != auction_table.end(), "the auction id doesn\'t exist.");

	// checked the seller is original
	check(auction_it->seller_id == seller_id, "the parsed seller_id doesn\'t match with the actual one for this auction.");

	// check the auction is still ongoing
	check( (!auction_it->claimed_by_seller && (auction_it->end_time >= now()) ), "the auction is closed now, so can\'t be unlisted.");

	// remove the item_ids from `asset_item_ids_listed_auct` in asset table
	asset_index asset_table(get_self(), auction_it->collection_name.value);
	auto asset_it = asset_table.find(auction_it->asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for this auction\'s collection name");

	// remove the item_id(s) also from asset_item_ids_listed_auct in asset table
	for(auto&& item_id : auction_it->item_ids) {
		auto item_id_it = std::find(asset_it->asset_item_ids_listed_auct.begin(), asset_it->asset_item_ids_listed_auct.end(), item_id);
		if(item_id_it != asset_it->asset_item_ids_listed_auct.end()) {		// if item_id found
			asset_table.modify(asset_it, get_self(), [&](auto &row){
				row.asset_item_ids_listed_auct.erase(item_id_it);
			});
		}	
	}

	auction_table.erase(auction_it);

}


// --------------------------------------------------------------------------------------------------------------------
// mainly executed after the successful purchase automatically i.e. ACTION - `bclaimauct` by system/platform
void oyanftmarket::delauction(
				uint64_t auction_id
			)
{
	require_auth(get_self());

	auction_index auction_table(get_self(), get_self().value);
	auto auction_it = auction_table.find(auction_id);

	check(auction_it != auction_table.end(), "the auction id doesn\'t exist.");

	// check the auction is closed only if the status is marked as '1'(when `bclaimauct` is done) no matter time is left or auction is claimed by seller
	// NOTE: this param - 'status' is introduced only for this ACTION. Otherwise, we have to loop in the map. The prob. is the map length is too long, then
	// it might take a long time to do this ACTION
	check( auction_it->status, "the auction is still open, so can\'t be deleted.");

	// remove the item_id(s) also from `asset_item_ids_listed_auction` in asset table
	for(auto&& item_id : auction_it->item_ids) {
		auto item_id_it = std::find(asset_it->asset_item_ids_listed_auct.begin(), asset_it->asset_item_ids_listed_auct.end(), item_id);
		if(item_id_it != asset_it->asset_item_ids_listed_auct.end()) {		// if item_id found
			asset_table.modify(asset_it, get_self(), [&](auto &row){
				row.asset_item_ids_listed_auct.erase(item_id_it);
			});
		}	
	}

	// delete the auction after successful purchase
	auction_table.erase(auction_it);
}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::raisefund(
				uint64_t creator_id,
				const name& collection_name,
				uint64_t asset_id,
				const name& pay_mode,
				const asset& required_fund_crypto,
				float required_fund_fiat_usd
			)

{
	require_auth(get_self());

	// check for valid collection name
	collection_index collection_table(get_self(), creator_id);
	auto collection_it = collection_table.find(collection_name.value);

	check(collection_it != collection_table.end(), "The collection is not present.");

	// add fund info into asset table
	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for this auction\'s collection name");

	check( (pay_mode == "crypto"_n) || (pay_mode == "fiat"_n), "invalid pay mode.");

	// NOTE: Here, both crypto & fiat funding is also allowed at the same time.
	if(pay_mode == "crypto"_n) {
		check( required_fund_crypto.amount > 0, "amount in crypto must be positive.");

		asset_table.modify(asset_it, get_self(), [&](auto &row){
			creatify_balances_map(row.required_fund_crypto, required_fund_crypto, 1);		// 1 for add balance
		});

	} 
	if(pay_mode == "fiat"_n) {
		check(required_fund_fiat_usd > 0, "amount in fiat USD must be positive.");

		asset_table.modify(asset_it, get_self(), [&](auto &row){
			row.required_fund_fiat_usd += required_fund_fiat_usd;
		});
	}


}
// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::propshareast(
				uint64_t investor_id,
				const name& collection_name,
				uint64_t asset_id,
				float proposed_share,
				const name& pay_mode,
				const asset& proposed_fund_crypto,
				float proposed_fund_fiat_usd
			)
{
	require_auth(get_self());

	// check for valid collection name
	// collection_index collection_table(get_self(), creator_id);
	// auto collection_it = collection_table.find(collection_name.value);

	// check(collection_it != collection_table.end(), "The collection is not present.");

	// check for valid asset id
	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for this collection name");

	check( (proposed_share >= 0) && (proposed_share <= 1), "proposed share must be between 0 and 1");

	check( (pay_mode == "crypto"_n) || (pay_mode == "fiat"_n), "invalid pay mode.");

	funding_index funding_table(get_self(), collection_name.value);
	auto ast_inv_idx = funding_table.get_index<"byastinvestr"_n>();
	auto ast_inv_it = ast_inv_idx.find(combine_ids(asset_id, investor_id));

	if(ast_inv_it == ast_inv_idx.end()) {
		ast_inv_idx.emplace(get_self(), [&](auto &row){
			row.asset_id = asset_id;
			row.investor_id = investor_id;
			row.creator_id = asset_it->creator_id;
			row.proposed_share = proposed_share;
			if(pay_mode == "crypto"_n) {
				check(proposed_fund_crypto.amount > 0, "proposed crypto fund must be positive");

				// check the amount present in required_fund_crypto map's value is >= proposed_fund_crypto
				check_amount_in_map( asset_it->required_fund_crypto, proposed_fund_crypto );
				row.proposed_fund_crypto = proposed_fund_crypto;
			}
			if(pay_mode == "fiat"_n) {
				check(proposed_fund_fiat_usd > 0, "proposed fiat fund must be positive");

				check(asset_it->required_fund_fiat_usd >= proposed_fund_fiat_usd, 
					"the proposed fiat fund by investor must be less than or equal to required fund.");
				row.proposed_fund_fiat_usd = proposed_fund_fiat_usd;
			}
		});	
	}
	else {
		check(!ast_inv_it->is_funding_closed, "the funding is closed. So, investor can\'t propose");
		ast_inv_idx.modify(ast_inv_it, get_self(), [&](auto &row){
			row.proposed_share = proposed_share;			// it could be 0 or 1 as well, in case of angel funding or something else. 
			if(pay_mode == "crypto"_n) {
				check(proposed_fund_crypto.amount > 0, "proposed crypto fund must be positive");

				// check the amount present in required_fund_crypto map's value is >= proposed_fund_crypto
				check_amount_in_map( asset_it->required_fund_crypto, proposed_fund_crypto );
				row.proposed_fund_crypto = proposed_fund_crypto;
			}
			if(pay_mode == "fiat"_n) {
				check(proposed_fund_fiat_usd > 0, "proposed fiat fund must be positive");

				check(asset_it->required_fund_fiat_usd >= proposed_fund_fiat_usd, 
					"the proposed fiat fund by investor must be less than or equal to required fund.");
				row.proposed_fund_fiat_usd = proposed_fund_fiat_usd;
			}
		});

	}

}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::negoshareast(
				uint64_t creator_id,
				uint64_t investor_id,
				const name& collection_name,
				uint64_t asset_id,
				float proposed_share,
				const asset& proposed_fund_crypto,
				float proposed_fund_fiat_usd
			)
{
	require_auth(get_self());

	// // check for valid collection name
	// collection_index collection_table(get_self(), creator_id);
	// auto collection_it = collection_table.find(collection_name.value);

	// check(collection_it != collection_table.end(), "The collection is not present.");

	// check for valid asset id
	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for this collection name");
	check(asset_it->creator_id == creator_id, "fund negotiation can only be done by the asset creator");

	check( (proposed_share >= 0) && (proposed_share <= 1), "proposed share must be between 0 and 1");

	funding_index funding_table(get_self(), collection_name.value);
	auto ast_inv_idx = funding_table.get_index<"byastinvestr"_n>();
	auto ast_inv_it = ast_inv_idx.find(combine_ids(asset_id, investor_id));

	check(ast_inv_it != ast_inv_idx.end(), "there is no row for this investor with this asset.");
	check(!ast_inv_it->is_funding_closed, "the funding is closed. So, creator can\'t negotiate");
	
	ast_inv_idx.modify(ast_inv_it, get_self(), [&](auto &row){
		row.proposed_share = proposed_share;			// it could be 0 or 1 as well, in case of angel funding or something else. 
		if(ast_inv_it->proposed_fund_crypto.amount > 0) {			// crypto mode chosen by investor
			check(proposed_fund_crypto.amount > 0, "proposed crypto fund must be positive");
			
			// check the amount present in required_fund_crypto map's value is >= proposed_fund_crypto
			check_amount_in_map( asset_it->required_fund_crypto, proposed_fund_crypto );
			row.proposed_fund_crypto = proposed_fund_crypto;
		}
		if(ast_inv_it->proposed_fund_fiat_usd > 0) {				// fiat mode chosen by investor
			check(proposed_fund_fiat_usd > 0, "proposed fiat fund must be positive");

			check(asset_it->required_fund_fiat_usd >= proposed_fund_fiat_usd, 
				"the proposed fiat fund by creator must be less than or equal to required fund.");
			row.proposed_fund_fiat_usd = proposed_fund_fiat_usd;
		}
	});

}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::invconfirmsf(
				uint64_t investor_id,
				const name& collection_name,
				uint64_t asset_id,
				uint8_t scfa			// 0/1/2/3 (share/crypto/fiat/all)
			)
{
	// // check for valid collection name
	// collection_index collection_table(get_self(), creator_id);
	// auto collection_it = collection_table.find(collection_name.value);

	// check(collection_it != collection_table.end(), "The collection is not present.");

	// check for valid asset id
	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for this collection name");

	check( (scfa == 0) || (scfa == 1) || (scfa == 2) || (scfa == 3), "scfa must be 0 or 1 or 2 or 3");

	funding_index funding_table(get_self(), collection_name.value);
	auto ast_inv_idx = funding_table.get_index<"byastinvestr"_n>();
	auto ast_inv_it = ast_inv_idx.find(combine_ids(asset_id, investor_id));

	check(ast_inv_it != ast_inv_idx.end(), "there is no row for this investor with this asset.");
	check(!ast_inv_it->is_funding_closed, "the funding is closed. So, investor can\'t confirm");

	ast_inv_idx.modify(ast_inv_it, get_self(), [&](auto &row){
		if ((scfa == 0) || (scfa == 3)) {	// share or all
			check(ast_inv_it->confirmed_share_by_investor != 1, "the investor has already confirmed the creator share for this funding.");
			// share can be inclusive of 0 or 1 i.e. [0, 1]
			if((ast_inv_it->proposed_share >= 0) && (ast_inv_it->proposed_share <= 1)) row.confirmed_share_by_investor = 1;
		}
		if ((scfa == 1) || (scfa == 3)) {	// fund_crypto or all
			check(ast_inv_it->confirmed_fund_crypto_by_investor != 1, "the investor has already confirmed the creator crypto fund for this funding.");			
			if(ast_inv_it->proposed_fund_crypto.amount > 0)			// crypto mode chosen by creator
				row.confirmed_fund_crypto_by_investor = 1;
		}
		if ((scfa == 2) || (scfa == 3)) {	// fund_fiatusd or all
			check(ast_inv_it->confirmed_fund_fiat_usd_by_investor != 1, "the investor has already confirmed the creator fiat fund for this funding.");			
			if(ast_inv_it->proposed_fund_fiat_usd > 0)				// fiat mode chosen by creator
				row.confirmed_fund_fiat_usd_by_investor = 1;
		}
	});
}

// --------------------------------------------------------------------------------------------------------------------
void oyanftmarket::creconfirmsf(
				uint64_t creator_id,
				uint64_t investor_id,
				const name& collection_name,
				uint64_t asset_id,
				uint8_t scfa			// 0/1/2/3 (share/crypto/fiat/all)
			)
{
	// // check for valid collection name
	// collection_index collection_table(get_self(), creator_id);
	// auto collection_it = collection_table.find(collection_name.value);

	// check(collection_it != collection_table.end(), "The collection is not present.");

	// check for valid asset id
	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for this collection name");
	check(asset_it->creator_id == creator_id, "investor fund confirmation must be done by the asset creator only");

	check( (scfa == 0) || (scfa == 1) || (scfa == 2) || (scfa == 3), "scfa must be 0 or 1 or 2 or 3");

	funding_index funding_table(get_self(), collection_name.value);
	auto ast_inv_idx = funding_table.get_index<"byastinvestr"_n>();
	auto ast_inv_it = ast_inv_idx.find(combine_ids(asset_id, investor_id));

	check(ast_inv_it != ast_inv_idx.end(), "there is no row for this investor with this asset.");
	check(!ast_inv_it->is_funding_closed, "the funding is closed. So, creator can\'t confirm");

	ast_inv_idx.modify(ast_inv_it, get_self(), [&](auto &row){
		if ((scfa == 0) || (scfa == 3)) {	// share or all
			check(ast_inv_it->confirmed_share_by_creator != 1, "the creator has already confirmed the investor share for this funding.");
			// share can be inclusive of 0 or 1 i.e. [0, 1]
			if((ast_inv_it->proposed_share >= 0) && (ast_inv_it->proposed_share <= 1)) row.confirmed_share_by_creator = 1;
		}
		if ((scfa == 1) || (scfa == 3)) {	// fund_crypto or all
			check(ast_inv_it->confirmed_fund_crypto_by_creator != 1, "the creator has already confirmed the investor crypto fund for this funding.");			
			if(ast_inv_it->proposed_fund_crypto.amount > 0)			// crypto mode chosen by investor
				row.confirmed_fund_crypto_by_creator = 1;
		}
		if ((scfa == 2) || (scfa == 3)) {	// fund_fiatusd or all
			check(ast_inv_it->confirmed_fund_fiat_usd_by_creator != 1, "the creator has already confirmed the investor fiat fund for this funding.");			
			if(ast_inv_it->proposed_fund_fiat_usd > 0)				// fiat mode chosen by creator
				row.confirmed_fund_fiat_usd_by_creator = 1;
		}
	});
}


// --------------------------------------------------------------------------------------------------------------------
// TODO: should it be done by the creator [NOW implemented] or 
// 		investor [RECOMMENDED, as the investor is allowing to deduct the money from it's account]
void oyanftmarket::finalizefund(
				uint64_t creator_id,
				uint64_t investor_id,
				const name& collection_name,
				uint64_t asset_id
			)
{
	// // check for valid collection name
	// collection_index collection_table(get_self(), creator_id);
	// auto collection_it = collection_table.find(collection_name.value);

	// check(collection_it != collection_table.end(), "The collection is not present.");

	// check for valid asset id
	asset_index asset_table(get_self(), collection_name.value);
	auto asset_it = asset_table.find(asset_id);

	check(asset_it != asset_table.end(), "there is no such asset id for this collection name");
	check(asset_it->creator_id == creator_id, "investor fund confirmation must be done by the asset creator only");

	funding_index funding_table(get_self(), collection_name.value);
	auto ast_inv_idx = funding_table.get_index<"byastinvestr"_n>();
	auto ast_inv_it = ast_inv_idx.find(combine_ids(asset_id, investor_id));

	check(ast_inv_it != ast_inv_idx.end(), "there is no row for this investor with this asset.");
	check(!ast_inv_it->is_funding_closed, "the funding is closed. So, creator can\'t confirm");

	// share
	check( confirmed_share_by_investor && confirmed_share_by_creator, "Either creator or investor has not yet confirmed the share.");

	// fund_crypto
	check( confirmed_fund_crypto_by_investor && confirmed_fund_crypto_by_creator, "Either creator or investor has not yet confirmed the crypto fund.");
	
	// check the amount present in required_fund_crypto map's value is >= proposed_fund_crypto
	check_amount_in_map( asset_it->required_fund_crypto, proposed_fund_crypto );

	// fund_fiatusd
	check( confirmed_fund_fiat_usd_by_investor && confirmed_fund_fiat_usd_by_creator, "Either creator or investor has not yet confirmed the fiat usd fund.");
	check(asset_it->required_fund_fiat_usd >= proposed_fund_fiat_usd, 
		"the proposed fiat fund by creator must be less than or equal to required fund.");


	// transfer the money using tip as inline action
	action(
		permission_level{get_self(), "active"_n},
		get_self(),
		"tip"_n,
		std::make_tuple(investor_id, creator_id, "", "", ast_inv_it->proposed_fund_crypto , 
										"invest ".append(ast_inv_it->proposed_fund_crypto.to_string()).append(" for the asset id: ").append(asset_id).append(" as sponsor"))
	).send();

	// move the required info to asset table
	// 1. crypto
	if (ast_inv_it->proposed_fund_crypto.amount > 0) {
		asset_table.modify(asset_it, get_self(), [&](auto &row){
			creatify_balances_map(row.required_fund_crypto, ast_inv_it->proposed_fund_crypto, 0);		// 0 for sub balance
		});
	}

	// 2. fiat usd
	if (ast_inv_it->proposed_fund_fiat_usd > 0) {
		asset_table.modify(asset_it, get_self(), [&](auto &row){
			row.required_fund_fiat_usd -= ast_inv_it->proposed_fund_fiat_usd;
		});
	}

	// 3. investor info
	asset_table.modify(asset_it, get_self(), [&](auto &row){
		investor_t i1{};
		i1.share = ast_inv_it->proposed_share;
		creatify_balances_map(i1.fund_crypto, ast_inv_it->proposed_fund_crypto, 1);		// 1 for add balance
		i1.fund_usd += fund_usd;
		creatify_investor_map(row.map_investorid_info, investor_id, i1);
	});


	// delete the row
	ast_inv_idx.erase(ast_inv_it);
}
