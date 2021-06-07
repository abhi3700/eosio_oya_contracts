#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
// #include <eosio/crypto.hpp>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>		// for strtoull
// #include <algorithm>



using eosio::contract;
using eosio::print;
using eosio::name;
using eosio::multi_index;
using eosio::const_mem_fun;
using eosio::indexed_by;
using eosio::asset;
using eosio::check;
using eosio::permission_level;
using eosio::datastream;
using eosio::current_time_point;
using eosio::action;
using eosio::same_payer;
using eosio::symbol;
using eosio::extended_symbol;
// using eosio::require_recipient;
using eosio::checksum256;
// using eosio::action_wrapper;

using std::string;
using std::vector;
using std::map;
using std::make_pair;

CONTRACT oyanftmarket : public contract
{
private:
	const vector<symbol> crypto_token_symbol_list;
	const symbol cryptopay_token_symbol;
	float platform_commission_rate;

public:
	using contract::contract;

	oyanftmarket(name receiver, name code, datastream<const char*> ds) : 
				contract(receiver, code, ds),
				crypto_token_symbol_list(vector<symbol>{
														symbol("EOS", 4), 
														symbol("TLOS", 4), 
														symbol("WAX", 4)}
										),
				cryptopay_token_symbol(symbol("EOS", 4))
				platform_commission_rate(0.01)
				{}

	// ==================CRYPTO Wallet===============================================

	/**
	 * @brief - deposit money to the contract ac
	 * @details - deposit money to the contract ac with memo - telegram user_id e.g. 452435325
	 * 			- accepts any token i.e. EOSIO token e.g. "EOS", "TOE", "FUTBOL" created on a chain
	 * @param from - user account
	 * @param contract_ac - contract ac
	 * @param quantity - in eosio tokens - EOS, TOE, etc.
	 * @param memo - purpose which should include telegram user_id
	 */
	[[eosio::on_notify("*::transfer")]]
	void deposit( const name& from_ac, 
					const name& contract_ac, 
					const asset& quantity,
					const string& memo );


	/**
	 * @brief - withdraw amount
	 * @details - withdraw amount from_id to to_ac
	 * 
	 * @param from_id - from telegram_id
	 * @param from_username - from telegram username
	 * @param to_ac - to eosio account
	 * @param quantity - qty
	 * @param memo - memo
	 */
	ACTION withdraw( uint64_t from_id,
					 const string& from_username,
					 const name& to_ac,
					 const asset& quantity,
					 const string& memo 
					);

	
	/**
	 * @brief - tip money to a person via just telegram_id
	 * @details - no eosio account needed for both from & to users
	 * 
	 * @param contract_ac - contract account name
	 * @param from_id - from telegram_id
	 * @param to_id - to telegram_id
	 * @param from_username - from telegram username
	 * @param to_username - to telegram username
	 * @param quantity - qty
	 * @param memo - memo
	 */
	ACTION tip( /*const name& contract_ac,*/
				uint64_t from_id,
				uint64_t to_id,
				const string& from_username,
				const string& to_username,
				const asset& quantity,
				const string& memo );

	// ==================NFT===============================================

	/**
	 * @brief - Add or modify collection
	 * @details - Add or modify collection
	 * 
	 * @param creator_id - creator telegram_id (fetched directly from private chat)
	 * @param collection_name - collection name
	 * @param collection_displayname - collection display name
	 * @param collection_desc - collection description
	 * @param collection_url - collection url
	 * 
	 */
	ACTION addmodcol(
				uint64_t creator_id,
				const name& collection_name,
				const string& collection_displayname,
				const string& collection_desc,
				const string& collection_url
			);

	/**
	 * @brief - delete collection
	 * @details - delete collection
	 * 
	 * @param creator_id - creator telegram_id (fetched directly from private chat)
	 * @param collection_name - collection name
	 * 
	 * @pre - ensure that collection (search by asset_id in sales & auction tables) is not listed in sale or auction
	 */
	ACTION delcol(
				uint64_t creator_id,
				const name& collection_name
			);

	/**
	 * @brief - Add or modify asset in a collection
	 * @details - Add or modify asset in a collection
	 * 
	 * @param collection_name - collection name
	 * @param creator_id - creator telegram_id (fetched directly from private chat)
	 * @param current_owner_id - current owner id
	 * @param asset_name - asset name
	 * @param asset_desc - asset description
	 * @param asset_img_hash - asset image hash
	 * @param asset_vid_hash - asset video hash
	 * @param asset_gif_hash - asset gif hash
	 * @param asset_file_hash - asset gif hash
	 * @param asset_copies_qty_total - asset total copies qty (max. 99999)
	 * @param asset_royaltyfee - asset royalty fee
	 * @param asset_artist - asset artist name
	 * 
	 * @pre - collection_name must exist
	 * @pre - total asset copies must be <= 99999
	 * 
	 */
	ACTION addmodasset(
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
			);

	/**
	 * @brief - delete asset from a collection
	 * @details - delete asset from a collection
	 * 
	 * @param collection_name - collection name
	 * @param asset_id - asset id
	 * @param creator_id - creator id (fetched directly from private chat)
	 * 
	 * @pre - match the 'chat_id' in telegram with the 'creator_id' for user verification
	 * @pre - ensure that any of the asset's items (search by asset_id in sales & auction tables) is not listed
	 */
	ACTION delasset(
				const name& collection_name,
				uint64_t asset_id,
				uint64_t creator_id
			);

	/**
	 * @brief - delete item (copies) from an asset
	 * @details - delete item (copies) from an asset or burn asset copy/copies
	 * 
	 * @param collection_name - collection name
	 * @param asset_id - asset id
	 * @param creator_id - creator id (fetched directly from private chat)
	 * @param item_qty - item qty
	 * 
	 * @pre - match the chat_id in telegram with the creator_id for user verification
	 * @pre - ensure that the remaining item to be listed is >= parsed item_qty
	 */
	ACTION delitem(
				const name& collection_name,
				uint64_t asset_id,
				uint64_t creator_id,
				uint64_t item_qty,
			);


	// /**
	//  * @brief - add/modify asset into oyanocreator table for non-creator
	//  * @details - add/modify asset into oyanocreator table for non-creator
	//  * 
	//  * @param noncreator_id - noncreator id
	//  * @param collection_name - collection name
	//  * @param asset_id - asset id
	//  */
	// ACTION addastother(
	// 			uint64_t noncreator_id,
	// 			const name& collection_name,
	// 			uint64_t asset_id,
	// 		);

	// /**
	//  * @brief - delete asset into oyanocreator table for non-creator
	//  * @details - delete asset into oyanocreator table for non-creator
	//  * 
	//  * @param owner_id - noncreator id
	//  * @param collection_name - collection name
	//  * @param asset_id - asset id
	//  */
	// ACTION delastother(
	// 			uint64_t noncreator_id,
	// 			const name& collection_name,
	// 			uint64_t asset_id,
	// 		);

	/**
	 * @brief - add/modify item into oyanocreator table for non-creator
	 * @details - add/modify item into oyanocreator table for non-creator
	 * 
	 * @param owner_id - noncreator id
	 * @param collection_name - collection name
	 * @param item_id - item id
	 */
	ACTION additemnctor(
				uint64_t noncreator_id,
				const name& collection_name,
				uint64_t item_id
			);


	/**
	 * @brief - remove/burn/destroy item_id into oyanocreator table for non-creator
	 * @details - remove/burn/destroy item_id into oyanocreator table for non-creator
	 * 
	 * @param noncreator_id - noncreator id
	 * @param item_id - item id
	 */
	ACTION rmitemnctor(
				uint64_t noncreator_id,
				uint64_t item_id
			);


	/**
	 * @brief - list item_id(s) on sale
	 * @details - list item_id(s) on sale
	 * 
	 * @param seller_id - creator/non-creator id
	 * @param collection_name - collection name
	 * @param item_ids - item ids (list multiple items into the market)
	 * @param price_mode - price mode (fiat/crypto)
	 * @param listing_price_crypto - price in crypto
	 * @param listing_price_fiat_usd - price in fiat (usd)
	 * 
	 * @pre - match the chat_id in telegram with the creator_id for user verification
	 * @pre - item(s) must not be listed before (in 'sales' TABLE)
	 * @pre - item(s) must not be in asset_item_ids_listed_sale in asset table
	 * @pre - items must be owned by the seller_id
	 * 
	 */
	ACTION listitemsale(
				const name& seller_id,
				const name& collection_name,
				const vector<uint64_t> item_ids,
				const name& price_mode,
				const asset& listing_price_crypto,
				float listing_price_fiat,
			);

	/**
	 * @brief - add item(s) to an existing sale
	 * @details - add item(s) to an existing sale
	 * 
	 * @param sale_id - sale id
	 * @param seller_id - creator/non-creator id
	 * @param item_ids - item ids (list multiple items into the market)
	 * 
	 * @pre - match the chat_id in telegram with the creator_id for user verification
	 * @pre - item(s) must not be in asset_item_ids_listed_sale in asset table
	 * @pre - item(s) must not be listed before (in 'sales' TABLE)
	 * 
	 */
	ACTION additemsale(
				uint64_t sale_id, 
				uint64_t seller_id,
				const vector<uint64_t> item_ids
			);

	/**
	 * @brief - remove item(s) from an existing sale
	 * @details - remove item(s) from an existing sale
	 * 
	 * @param sale_id - sale id
	 * @param seller_id - creator/non-creator id
	 * @param item_ids - item ids (list multiple items into the market)
	 * 
	 * @pre - match the chat_id in telegram with the creator_id for user verification
	 * @pre - item(s) must be in asset_item_ids_listed_sale in asset table
	 * @pre - item(s) must be listed before (in 'sales' TABLE)
	 * 
	 */
	ACTION rmitemsale(
				uint64_t sale_id, 
				uint64_t seller_id,
				const vector<uint64_t> item_ids
			);


	/**
	 * @brief - Set price for a sale
	 * @details - set price for sale in fiat or cypto
	 * 
	 * @param sale_id - sale id
	 * @param seller_id - seller id
	 * @param price_mode - price mode (fiat/crypto)
	 * @param listing_price_crypto - listing price in crypto
	 * @param listing_price_fiat_usd - listing price in USD
	 */
	ACTION setpricesale(
				uint64_t sale_id,
				uint64_t seller_id,
				const name& price_mode,
				const asset& listing_price_crypto,
				float listing_price_fiat_usd
			);

	/**
	 * @brief - Buyer buy a sale (with item(s))
	 * @details: main objectives:
	 * 			- transfer of price amount from buyer to seller, creator (as royalty fee)
				- transfer of assets from seller to buyer
				- update the info wherever needed like sale table, asset table, oyanocreator table cryptobal table
	 * 
	 * @param sale_id - sale id
	 * @param buyer_id - buyer id
	 * @param pay_mode - pay mode (fiat/crypto)
	 */
	ACTION buysale(
				uint64_t sale_id,
				uint64_t buyer_id,
				const name& pay_mode
			);

	/**
	 * @brief - unlist a sale
	 * @details - unlist a sale
	 * 
	 * @param sale_id - sale id
	 * @param seller_id - seller(creator/non-creator) id. Seller is the owner
	 * 
	 * @pre - match the chat_id fetched from telegram with the parsed seller_id for user verification
	 * @pre - checked the seller is original
	 * 
	 * @post - remove the item_id(s) also from asset_item_ids_listed_sale in asset table
	 * 
	 */
	ACTION unlistsale(
				uint64_t sale_id,
				uint64_t seller_id
			);


	/**
	 * @brief - platform delete the sale just after `buyitemsale` is executed successfully
	 * @details - platform delete the sale just after `buyitemsale` is executed successfully
	 * 
	 * @param sale_id - sale id
	 */
	ACTION delsale(
				uint64_t sale_id
			);


	/**
	 * @brief - list item_id(s) on sale
	 * @details - list item_id(s) on sale
	 * 
	 * @param seller_id - creator/non-creator id
	 * @param collection_name - collection name
	 * @param item_ids - item ids (merge multiple items into the market)
	 * @param end_time - auction end time
	 * @param price_mode - price mode (fiat/crypto)
	 * @param current_price_crypto - bid price in crypto
	 * @param current_price_fiat_usd - bid price in fiat (usd)
	 * 
	 * @pre - match the chat_id in telegram with the creator_id for user verification
	 * @pre - item(s) must not be listed before (in 'sales' TABLE)
	 * @pre - item(s) must not be in asset_item_ids_listed_sale in asset table
	 * @pre - items must be owned by the seller_id
	 * 
	 */
	ACTION listitemauct(
				const name& seller_id,
				const name& collection_name,
				const vector<uint64_t> item_ids,
				uint32_t end_time,
				const name& price_mode,
				const asset& current_price_crypto,
				float current_price_fiat_usd,
			);

	/**
	 * @brief - add item(s) to an existing auction
	 * @details - add item(s) to an existing auction
	 * 
	 * @param auction_id - auction id
	 * @param seller_id - creator/non-creator id
	 * @param item_ids - item ids (merge multiple items into the market)
	 * 
	 * @pre - match the chat_id in telegram with the creator_id for user verification
	 * @pre - item(s) must not be in asset_item_ids_listed_auct in asset table
	 * @pre - item(s) must not be listed before (in 'auction' TABLE)
	 * 
	 */
	ACTION additemauct(
				uint64_t auction_id, 
				uint64_t seller_id,
				const vector<uint64_t> item_ids
			);

	/**
	 * @brief - remove item(s) from an existing auction
	 * @details - remove item(s) from an existing auction
	 * 
	 * @param auction_id - auction id
	 * @param seller_id - creator/non-creator id
	 * @param item_ids - item ids (list multiple items into the market)
	 * 
	 * @pre - match the chat_id in telegram with the creator_id for user verification
	 * @pre - item(s) must be in asset_item_ids_listed_auct in asset table
	 * @pre - item(s) must be listed before (in 'auction' TABLE)
	 * 
	 */
	ACTION rmitemauct(
				uint64_t auction_id, 
				uint64_t seller_id,
				const vector<uint64_t> item_ids
			);


	/**
	 * @brief - Set price for a auction
	 * @details - set price for auction in fiat or cypto
	 * 
	 * @param auction_id - auction id
	 * @param seller_id - seller id
	 * @param price_mode - price mode (fiat/crypto)
	 * @param current_bid_crypto - bid in crypto
	 * @param current_bid_fiat_usd - bid in fiat
	 */
	ACTION setpriceauct(
				uint64_t auction_id,
				uint64_t seller_id,
				const name& price_mode,
				const asset& current_price_crypto,
				float current_price_fiat_usd
			);

	/**
	 * @brief - bid for auction
	 * @details - bid for auction
	 * 
	 * @param auction_id - auction id
	 * @param bidder_id - bidder id
	 * @param pay_mode - pay mode
	 * @param bid_price_crypto - bid price in crypto
	 * @param bid_price_fiat_usd - bid price in fiat usd
	 */
	ACTION bidforauct(
				uint64_t auction_id,
				uint64_t bidder_id,
				const name& pay_mode,
				const asset& bid_price_crypto,
				float bid_price_fiat_usd
			);

	/**
	 * @brief - seller claim auction
	 * @details - seller claim auction
	 * 
	 * @param auction_id - auction id
	 * @param bidder_id - bidder id
	 */
	ACTION sclaimauct(
				uint64_t auction_id,
				uint64_t bidder_id,
			);


	/**
	 * @brief - Bidder claim a auction
	 * @details: main objectives:
	 * 			- transfer of price amount from buyer to seller, creator (as royalty fee)
				- transfer of assets from seller to buyer
				- update the info wherever needed like sale table, asset table, oyanocreator table cryptobal table
	 * 
	 * @param auction_id - auction id
	 * @param buyer_id - buyer id
	 * @param pay_mode - pay mode (fiat/crypto)
	 */
	ACTION bclaimauct(
				uint64_t auction_id,
				uint64_t bidder_id,
				const name& pay_mode
			);

	/**
	 * @brief - unlist item on auction
	 * @details - unlist item on auction
	 * 
	 * @param item_id - item id
	 * @param seller_id - seller(creator/non-creator) id. Seller is the current owner
	 * 
	 * @pre - match the chat_id in telegram with the creator_id for user verification
	 * @pre - checked the seller is original
	 * 
	 * @post - remove the item_id(s) also from `asset_item_ids_listed_auct` in asset table
	 */
	ACTION ulistauction(
				uint64_t auction_id,
				uint64_t seller_id
			);

	/**
	 * @brief - platform delete the auction just after `buyitemauct` is executed successfully
	 * @details - platform delete the auction just after `buyitemauct` is executed successfully
	 * 
	 * @param auction_id - auction id
	 */
	ACTION delauction(
				uint64_t auction_id
			);



private:
	// -----------------------------------------------------------------------------------------------------------------------
	// scope: get_self()
	// TABLE cryptobal
	// {
	// 	uint64_t user_id;
 //        asset balance;

 //        uint64_t primary_key()const { return user_id; }
	// };

	// using cryptobal_index = multi_index<"cryptobal"_n, cryptobal>
	TABLE account
	{
		uint64_t owner;		// telegram_id, e.g. 452435325.

		/*
			[ 
				{ "key": { "symbol": "4,SOV", "contract": "sovmintofeos" }, "value": 30000 }, 
				{ "key": { "symbol": "4,FROG", "contract": "frogfrogcoin" }, "value": 3500000 }, 
				{ "key": { "symbol": "4,PEOS", "contract": "thepeostoken" }, "value": 100000 }, 
				{ "key": { "symbol": "4,KROWN", "contract": "krowndactokn" }, "value": 7169 } 
			]
			
			Here, quantity amount is 30000/10^4 = 3 i.e. asset is "3.0000 SOV"
		*/
		map<extended_symbol, uint64_t> balances; // map with extended_symbol, uint64_t

		auto primary_key() const { return owner; }
	};

	using account_index = multi_index<"accounts"_n, account>;

	// -----------------------------------------------------------------------------------------------------------------------
	// Table for non-creator with asset_id (with item_ids)
	// scope: <noncreator_telegram_id>
	TABLE oyanocreator
	{
		uint64_t asset_id;				// asset id
		name collection_name;			// collection name
		vector<uint64_t> item_ids;			// list of item ids for the asset_id

		auto primary_key() const { return asset_id; }
		uint64_t by_collection() const { return collection_name.value; }
	};

	using oyanocreator_index = multi_index<"oyanocreator"_n, oyanocreator,
								indexed_by< "bycollection"_n, const_mem_fun<oyanocreator, uint64_t, &oyanocreator::by_collection>>
								>;

	// -----------------------------------------------------------------------------------------------------------------------
	// scope: <creator_telegram_id>
	TABLE collection
	{
		// name creator;					// collection creator name [DISABLED for Telegram Bot, as there is no EOSIO account for users needed]
		name collection_name;			// collection name
		string collection_displayname;	// collection display name
		string collection_desc;			// collection description
		string collection_url;			// collection url


		auto primary_key() const { return collection_name.value; }
	};

	using collection_index = multi_index<"collections"_n, collection>;

	// -----------------------------------------------------------------------------------------------------------------------
	typedef struct {
		float share;
		map<extended_symbol, uint64_t> fund_crypto; 	// [OPTIONAL] map with extended_symbol, uint64_t. if works fine, otherwise, the transferred amount will be searched by telegram_id
		float fund_usd;
	} investor_t;

	// scope: <collection_name>
	TABLE asset
	{
		uint64_t asset_id;				// asset id. Another is item_id i.e. 9210<current_time><max copies upto 99999> E.g. if max_copies = 100, then 9210<start_time>1 is the 1st item_id. This is shown in the auctions, sales TABLE
		uint64_t creator_id;			// creator telegram_id		
		// uint64_t current_owner_id;		// current owner telegram_id [DEPRECATED], there can be many current owners for different item_id(s)
		string asset_name;				// asset name
		string asset_desc;				// asset description
		checksum256 asset_img_hash;		// asset image hash
		checksum256 asset_vid_hash;		// asset video hash
		checksum256 asset_gif_hash;		// asset gif hash
		checksum256 asset_file_hash;		// asset file hash (for mp3, mp4, good quality file)
		vector<uint64_t> asset_item_ids_listed_sale;	// asset copies listed qty by owner (creator (only for 1st sale) or seller). only modifyable when there is a new sale or auction
		vector<uint64_t> asset_item_ids_listed_auct;	// asset copies listed qty by owner (creator (only for 1st sale) or seller). only modifyable when there is a new sale or auction
		vector<uint64_t> asset_item_ids_transferred;	// asset copies transferred from creator i.e. sold/gifted at least once
		uint64_t asset_copies_qty_total;		// asset copies total qty (if burned an item, then qty is decreased here)
		float asset_royaltyfee;			// asset royalty fee
		string asset_artist;				// asset artist
		/*
			[ 
				{ "key": { "symbol": "4,SOV", "contract": "sovmintofeos" }, "value": 30000 }, 
				{ "key": { "symbol": "4,FROG", "contract": "frogfrogcoin" }, "value": 3500000 }, 
				{ "key": { "symbol": "4,PEOS", "contract": "thepeostoken" }, "value": 100000 }, 
				{ "key": { "symbol": "4,KROWN", "contract": "krowndactokn" }, "value": 7169 } 
			]		
		*/
		map<extended_symbol, uint64_t> required_fund_crypto;		// ["25 EOS", "56 TLOS"]
		float required_fund_fiat_usd;			// required fund in fiat USD
		map<uint64_t, investor_t> map_investorid_info;

		auto primary_key() const { return asset_id; }
		uint64_t by_creator() const { return creator_id; }
		uint64_t by_curr_owner() const { return current_owner_id; }
	};

	using asset_index = multi_index<"assets"_n, asset,
								indexed_by< "bycreator"_n, const_mem_fun<asset, uint64_t, &asset::by_creator>>,	
								indexed_by< "bycurrowner"_n, const_mem_fun<asset, uint64_t, &asset::by_curr_owner>>	
								>;



	// -----------------------------------------------------------------------------------------------------------------------
	// scope: self i.e. oyanftmarket
	TABLE sale
	{
		uint64_t sale_id;		// sale_id id. 3700<start_time><last_3_digit_tg_id>
		vector<uint64_t> item_ids;			//list of item_id format is "<item_id>999999" E.g. if max_copies = 100, then "<asset_id>1" is the 1st item_id. If max_copies = 1, then item_id is "<asset_id>1"
		uint64_t asset_id;			// asset id
		uint64_t seller_id;				// seller telegram_id
		asset listing_price_crypto;		// listing price of asset in crypto (if opted for crypto)
		float listing_price_fiat_usd;		// listing price of asset in fiat in USD (if opted for fiat)
		uint64_t buyer_id;				// buyer telegram_id
		name collection_name;		// collection name
		float royalty_fee;			// collection/royalty fee, E.g. 5% = 0.05


		auto primary_key() const { return sale_id; }
		uint64_t by_asset() const { return asset_id; }
		uint64_t by_seller() const { return seller_id; }
		uint64_t by_buyer() const { return buyer_id; }
		uint64_t by_collection() const { return collection_name.value; }
	};

	using sale_index = multi_index<"sales"_n, sale>,
								indexed_by< "byasset"_n, const_mem_fun<sale, uint64_t, &sale::by_asset>>,
								indexed_by< "byseller"_n, const_mem_fun<sale, uint64_t, &sale::by_seller>>,
								indexed_by< "bybuyer"_n, const_mem_fun<sale, uint64_t, &sale::by_buyer>>,
								indexed_by< "bycollection"_n, const_mem_fun<sale, uint64_t, &sale::by_collection>>,
								>;


	// -----------------------------------------------------------------------------------------------------------------------
	typedef struct {
		bool claimed_by_bidder;
		asset bid_crypto_price;
		float bid_fiat_price_usd;
	} bid_t;

	// scope: self i.e. oyanftmarket
	TABLE auction
	{
		uint64_t auction_id;		// auction id. 3701<start_time><last_3_digit_tg_id>
		vector<uint64_t> item_ids;			// list of item_id format is "<item_id>99999" E.g. if max_copies = 100, then "<asset_id>1" is the 1st item_id. If max_copies = 1, then item_id is "<asset_id>1"
		uint64_t asset_id;			// asset id
		uint64_t seller_id;				// seller telegram_id
		uint32_t start_time;		// auction start time
		uint32_t end_time;			// auction end time
		asset current_price_crypto;			// current of asset in crypto (if opted for crypto)
		float current_price_fiat_usd;			// current of asset in fiat in USD (if opted for fiat)
		// map<uint64_t, bool> map_bidderid_claimedbybidder;		// map of bidderid, claimedbybidder. NOTE: claimedbybidder is set to '1' only if done after claimed_by_seller, after which the price would be deducted 
		// map<uint64_t, asset> map_bidderid_cprice;				// map of bidderid, cryptoprice. 
		// map<uint64_t, float> map_bidderid_fprice;				// map of bidderid, fiatprice. 
		map<uint64_t, bid_t> map_bidderid_info;						// map of bidderid, info (claimed_by_buyer, bid_crypto_price, bid_fiat_price_usd)
		bool claimed_by_seller;		// claimed by seller
		uint64_t confirmed_bidder_id_by_seller;		// seller confirmed buyer id
		// bool claimed_by_buyer;	// claimed by buyer
		bool status;				// auction status
		name collection_name;		// collection name
		float royalty_fee;		// collection/royalty fee


		auto primary_key() const { return auction_id; }
		uint64_t by_asset() const { return asset_id; }
		uint64_t by_seller() const { return seller_id; }
		// uint64_t by_buyer() const { return buyer_id; }
		uint64_t by_collection() const { return collection_name.value; }
	};

	using auction_index = multi_index<"auctions"_n, auction>,
								indexed_by< "byasset"_n, const_mem_fun<auction, uint64_t, &auction::by_asset>>,
								indexed_by< "byseller"_n, const_mem_fun<auction, uint64_t, &auction::by_seller>>,
								// indexed_by< "bybuyer"_n, const_mem_fun<auction, uint64_t, &auction::by_buyer>>,
								indexed_by< "bycollection"_n, const_mem_fun<auction, uint64_t, &auction::by_collection>>,
								>;

	// -----------------------------------------------------------------------------------------------------------------------
	// scope: collection_name
	TABLE funding
	{
		uint64_t asset_id;			// asset id
		uint64_t investor_id;		// investor telegram_id
		uint64_t creator_id;		// creator id
		float proposed_share;
		asset proposed_fund_crypto;
		float proposed_fund_fiat_usd;
		bool confirmed_share_by_investor;
		bool confirmed_share_by_creator;
		bool confirmed_fund_crypto_by_investor;
		bool confirmed_fund_crypto_by_creator;
		bool confirmed_fund_fiat_usd_by_investor;
		bool confirmed_fund_fiat_usd_by_creator;
		bool is_funding_closed;		// 0/1 open/closed

		auto primary_key() const { return asset_id; }
		uint64_t by_investor() const { return investor_id; }
		uint128_t by_assetid_investorid() const { return combine_ids(asset_id, investor_id); }

		// uint64_t by_creator() const { return creator_id; }
	};

	using funding_index = multi_index<"funding"_n, funding>,
								indexed_by< "byinvestor"_n, const_mem_fun<funding, uint64_t, &funding::by_investor>>,
								indexed_by< "byastinvestr"_n, const_mem_fun<funding, uint128_t, &funding::by_assetid_investorid>>
								>;
	// -----------------------------------------------------------------------------------------------------------------------
	// UTILITY functions
	// -----------------------------------------------------------------------------------------------------------------------
	// get the current timestamp
	inline uint32_t now() const {
		return current_time_point().sec_since_epoch();
	}

	// -----------------------------------------------------------------------------------------------------------------------
	// count no. of alphabets in a string
	inline uint64_t count_alpha( const string& str ) {
		uint64_t count = 0;		// no. of alphabet chars

	    for (int i=0; i<=str.size(); ++i) {
	        if (isalpha(str[i]))
	            ++count;
	    }

	    return count;
	}

	// -----------------------------------------------------------------------------------------------------------------------
	// string to uint64_t
	inline uint64_t str_to_uint64t(const string& s) {
		// M-1
		// char* end;
		uint64_t num = strtoull(s.c_str(), NULL, 10);
	
		//----------------------------------------------
		// M-2
		// uint64_t num = lexical_cast<uint64_t>(s);

		return num;
	}

	// -----------------------------------------------------------------------------------------------------------------------
	template <typename T>
	inline bool has_item_in_vector( const vector<T>& vec, T item) {
		bool found = false;
		if (std::find(vec.begin(), vec.end(), item) !=  vec.end())
			found = true;
		return found;
	}

	// -----------------------------------------------------------------------------------------------------------------------
	inline uint64_t create_astsaleauc_id(int init_num, uint64_t creator_id) {
		// capture last 3 digits of telegram id
		// 1. divide by 1000, & save as string
		double res = (double)creator_id/1000;
		auto res_str = std::to_string(res);
		// 2. snip the last 3 digits after decimal now. find position of .
		string creator_id_last3 = res_str.substr(res_str.find(".")+1, 3);  

		// create unique sale id i.e. 3700<current_time><last_3_digit_tg_id>
		// create unique auction id i.e. 3701<current_time><last_3_digit_tg_id>
		uint64_t sale_id = str_to_uint64t(std::to_string(init_num).append(std::to_string(now())).append(creator_id_last3));

		return saleauc_id;
	}

	// -----------------------------------------------------------------------------------------------------------------------
	void sub_balance( uint64_t owner_id, const asset& qty ) {
		cryptobal_index from_cryptobal_table(get_self(), get_self().value);
		auto from_cryptobal_it = from_cryptobal_table.find(owner_id);

		check(from_cryptobal_it != from_cryptobal_table.end(), "user not found in cryptobal table.")
		check(from_cryptobal_it->balance.symbol.raw() == qty.symbol.raw(), "no crypto balance object found for buyer");
		check( from_cryptobal_it->balance.amount >= qty.amount, "overdrawn crypto balance" );

		from_cryptobal_table.modify( from_cryptobal_it, get_self(), [&]( auto& row ) {
			 row.balance -= qty;
		});
	}


	// -----------------------------------------------------------------------------------------------------------------------
	void add_balance( uint64_t owner_id, const asset& qty, const name& ram_payer )
	{
		cryptobal_index to_cryptobal_table(get_self(), get_self().value);
		auto to_cryptobal_it = to_cryptobal_table.find(owner_id);

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
	// -----------------------------------------------------------------------------------------------------------------------
	template<typename T1, typename T2>
	inline void creatify_map( map<T1, T2>& m, const T1& item_key, const T2& item_val, char item_val_type ) 
	{
		// auto s_it = std::find_if(m.begin(), m.end(), [&](auto& ms) {return ms.first == item_key;});
		auto s_it = m.find(item_key);
		if(s_it != m.end()) {		// key found
			if (item_val_type == 'b') s_it->second.claimed_by_bidder = item_val.claimed_by_bidder;
			else if (item_val_type == 'c') s_it->second.bid_crypto_price = item_val.bid_crypto_price;
			else if (item_val_type == 'f') s_it->second.bid_fiat_price_usd = item_val.bid_fiat_price_usd;
		}
		else {						// key NOT found
			m.insert( make_pair(item_key, item_val) );
		}
	}

	// -----------------------------------------------------------------------------------------------------------------------
	template<typename T1, typename T2>
	inline bool key_found_in_map(map<T1, T2>& m, const T1& item_key) {
		bool found = false;
		// auto s_it = std::find_if(m.begin(), m.end(), [&](auto& ms) {return ms.first == item_key;});
		auto it = m.find(item_key)

		if (s_it != m.end()) {			// key found
			found = true;
		}

		return found;
	}

	// -----------------------------------------------------------------------------------------------------------------------
	template<typename T1, typename T2>
	inline bool crypto_found_in_map(map<T1, T2>& m, const T1& item_key) {
		bool found = false;
		// auto s_it = std::find_if(m.begin(), m.end(), [&](auto& ms) {return ms.first == item_key;});
		auto it = m.find(item_key)

		if (s_it != m.end()) {			// key found
			if (s_it->second.bid_crypto_price.amount == 0) found = true;
		}

		return found;
	}

	// -----------------------------------------------------------------------------------------------------------------------
	template<typename T1, typename T2>
	inline bool fiat_found_in_map(map<T1, T2>& m, const T1& item_key) {
		bool found = false;
		// auto s_it = std::find_if(m.begin(), m.end(), [&](auto& ms) {return ms.first == item_key;});
		auto it = m.find(item_key)

		if (s_it != m.end()) {			// key found
			if (s_it->second.bid_fiat_price_usd == 0) found = true;
		}

		return found;
	}

	// -----------------------------------------------------------------------------------------------------------------------
	/*	
		Here, 2 cases are covered in which the balances map is modified when the row with (owner, balances) exist: 
			- case-1: if the row exists & key is found. i.e. the parsed quantity symbol is found
				- add/sub quantity amount is done by an arithmetic_op (0/1) => (-/+) 
			- case-2: if the row exists & key is NOT found. i.e. the parsed quantity symbol is NOT found 
	*/	
	inline void creatify_balances_map( map<extended_symbol, uint64_t>& m, const asset& qty, 
								bool arithmetic_op 			// add/sub balance from existing quantity
								) {
		auto s_it = std::find_if(m.begin(), m.end(), 
							[&](auto& ms) {return ms.first.get_symbol() == qty.symbol;});
		
		if(s_it != m.end()) {		// key found
			if (arithmetic_op == 1)
				s_it->second += qty.amount;
			else if (arithmetic_op == 0)
				s_it->second -= qty.amount;
		}
		else {						// key NOT found
			m.insert( make_pair(extended_symbol(qty.symbol, get_first_receiver()), qty.amount) );
		}
	}

	// -----------------------------------------------------------------------------------------------------------------------
	/*	
		Here, 2 cases are covered in which the balances map is checked for <>= than the given quantity, when the row with (owner, balances) exist: 
			- case-1: if the row exists & key is found. i.e. the parsed quantity symbol is found
				- check for >= , else throw message
			- case-2: if the row exists & key is NOT found. i.e. the parsed quantity symbol is NOT found 
				- throw message saying that there is no balances available
	*/	
	inline void check_amount_in_map( map<extended_symbol, uint64_t> m, const asset& qty ) {
		auto s_it = std::find_if(m.begin(), m.end(), 
							[&](auto& ms) {return ms.first.get_symbol() == qty.symbol;});
		
		if(s_it != m.end()) {		// key found
			check( s_it->second >= qty.amount, "Insufficient balance in from\'s account." );
		}
		else {						// key NOT found
			check( false, "there is no balances map available corresponding to the parsed quantity symbol" );
		}
	}

	// -----------------------------------------------------------------------------------------------------------------------
	/*	
		Here, capture the token contract name in which the balances map exists in the row with (owner, balances) exist: 
			- case-1: if the row exists & key is found. i.e. the parsed quantity symbol is found
				- capture the contract ac name from the key
	*/	
	inline name capture_contract_in_map( map<extended_symbol, uint64_t> m, const asset& qty ) {
		name token_contract_ac = ""_n;

		auto s_it = std::find_if(m.begin(), m.end(), 
							[&](auto& ms) {return ms.first.get_symbol() == qty.symbol;});
		
		if(s_it != m.end()) {		// key found
			token_contract_ac = s_it->first.get_contract();
		}
		else {
			check(false, "there is no contract account found with this quantity");
		}

		return token_contract_ac;
	}

	// -----------------------------------------------------------------------------------------------------------------------
	// concatenation of ids
	static uint128_t combine_ids(const uint64_t &x, const uint64_t &y) {
	    return (uint128_t{x} << 64) | y;
	}

};