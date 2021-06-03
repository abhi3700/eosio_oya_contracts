#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
// #include <eosio/crypto.hpp>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm>



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
// using eosio::extended_symbol;
// using eosio::require_recipient;
using eosio::checksum256;
// using eosio::action_wrapper;

using std::string;
using std::vector;

CONTRACT oyanftmarket : public contract
{
private:
	const vector<symbol> crypto_token_symbol_list;
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
				platform_commission_rate(0.01)
				{}


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
				const string& collection_url,
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



	ACTION setitmprisale(
				uint64_t sale_id,
				uint64_t seller_id,
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
	ACTION buyitemsale(
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
	ACTION ulistitemsale(
				uint64_t sale_id,
				uint64_t seller_id
			);

	/**
	 * @brief - list item on auction
	 * @details - list item on auction
	 * 
	 * @param item_id - item id
	 * @param collection_name - collection name
	 * @param seller_id - seller(creator/non-creator) id
	 * @param current_bid_crypto - price in crypto
	 * @param current_bid_usd - price in fiat (usd)
	 * 
	 * @pre - match the chat_id in telegram with the creator_id for user verification
	 * @pre - item must not be listed before (in 'sales' & 'auction' TABLE)
	 */
	ACTION listitemauct(
				uint64_t item_id,
				const name& collection_name,
				const name& seller_id,
				const asset& current_bid_crypto,
				float current_bid_fiat_usd,
			);


	/**
	 * @brief - unlist item on auction
	 * @details - unlist item on auction
	 * 
	 * @param item_id - item id
	 * @param seller_id - seller(creator/owner) id
	 * 
	 * @pre - match the chat_id in telegram with the creator_id for user verification
	 * @pre - item must not be listed before in 'auction' TABLE
	 */
	ACTION ulistitmauct(
				uint64_t auction_id,
				uint64_t seller_id
			);



private:
	// -----------------------------------------------------------------------------------------------------------------------
	// scope: get_self()
	TABLE cryptobal
	{
		uint64_t user_id;
        asset balance;

        uint64_t primary_key()const { return user_id; }
	};

	using cryptobal_index = multi_index<"cryptobal"_n, cryptobal>

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
	// scope: self i.e. oyanftmarket
	TABLE auction
	{
		uint64_t auction_id;		// auction id. 3701<start_time><last_3_digit_tg_id>
		vector<uint64_t> item_ids;			// list of item_id format is "<item_id>99999" E.g. if max_copies = 100, then "<asset_id>1" is the 1st item_id. If max_copies = 1, then item_id is "<asset_id>1"
		uint64_t asset_id;			// asset id
		uint64_t seller_id;				// seller telegram_id
		uint64_t start_time;		// auction start time
		uint64_t end_time;			// auction end time
		asset current_bid_crypto;			// current bid of asset in crypto (if opted for crypto)
		float current_bid_fiat_usd;			// current bid of asset in fiat in USD (if opted for fiat)
		uint64_t current_bidder_id;		// current bidder telegram_id
		bool claimed_by_seller;		// claimed by seller
		bool claimed_by_buyer;		// claimed by buyer
		name collection_name;		// collection name
		float royalty_fee;		// collection/royalty fee


		auto primary_key() const { return auction_id; }
		uint64_t by_asset() const { return asset_id; }
		uint64_t by_seller() const { return seller_id; }
		uint64_t by_buyer() const { return buyer_id; }
		uint64_t by_collection() const { return collection_name.value; }
	};

	using auction_index = multi_index<"auctions"_n, auction>,
								indexed_by< "byasset"_n, const_mem_fun<auction, uint64_t, &auction::by_asset>>,
								indexed_by< "byseller"_n, const_mem_fun<auction, uint64_t, &auction::by_seller>>,
								indexed_by< "bybuyer"_n, const_mem_fun<auction, uint64_t, &auction::by_buyer>>,
								indexed_by< "bycollection"_n, const_mem_fun<auction, uint64_t, &auction::by_collection>>,
								>;


	// ============================================================================================================================
	template <typename T>
	inline bool has_item_in_vector( const vector<T>& vec, T item) {
		bool found = false;
		if (std::find(vec.begin(), vec.end(), item) !=  vec.end())
			found = true;
		return found;
	}

	// -----------------------------------------------------------------------------------------------------------------------
	// get the current timestamp
	inline uint32_t now() const {
		return current_time_point().sec_since_epoch();
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
	void sub_balance( const name& owner, const asset& value );
	void add_balance( const name& owner, const asset& value, const name& ram_payer );


};