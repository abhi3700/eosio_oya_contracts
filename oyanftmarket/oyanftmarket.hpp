#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
// #include <eosio/crypto.hpp>
#include <string>
#include <vector>
#include <cstdlib>



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

public:
	using contract::contract;

	oyanftmarket(name receiver, name code, datastream<const char*> ds) : 
				contract(receiver, code, ds)
				{}


	/**
	 * @brief - Add or modify collection
	 * @details - Add or modify collection
	 * 
	 * @param author_id - author telegram_id (fetched directly from private chat)
	 * @param collection_name - collection name
	 * @param collection_desc - collection description
	 * @param collection_url - collection url
	 * 
	 */
	ACTION addmodcol(
				uint64_t author_id,
				const name& collection_name,
				const string& collection_desc,
				const string& collection_url,
			);

	/**
	 * @brief - delete collection
	 * @details - delete collection
	 * 
	 * @param author_id - author telegram_id (fetched directly from private chat)
	 * @param collection_name - collection name
	 * 
	 * @pre - ensure that collection (search by asset_id in sales & auction tables) is not listed in sale or auction
	 */
	ACTION delcol(
				uint64_t author_id,
				const name& collection_name,
			);

	/**
	 * @brief - Add or modify asset in a collection
	 * @details - Add or modify asset in a collection
	 * 
	 * @param collection_name - collection name
	 * @param asset_id - asset id (to be created from outside) as 9210<current_timestamp>
	 * @param author_id - author telegram_id (fetched directly from private chat)
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
			);

	/**
	 * @brief - delete asset from a collection
	 * @details - delete asset from a collection
	 * 
	 * @param collection_name - collection name
	 * @param asset_id - asset id
	 * @param author_id - author id (fetched directly from private chat)
	 * 
	 * @pre - match the 'chat_id' in telegram with the 'author_id' for user verification
	 * @pre - ensure that any of the asset's items (search by asset_id in sales & auction tables) is not listed
	 */
	ACTION delasset(
				const name& collection_name,
				uint64_t asset_id,
				uint64_t author_id
			);

	/**
	 * @brief - delete item from an asset
	 * @details - delete item from an asset
	 * 
	 * @param collection_name - collection name
	 * @param item_id - item id
	 * @param author_id - author id (fetched directly from private chat)
	 * 
	 * @pre - match the chat_id in telegram with the author_id for user verification
	 * @pre - ensure that the item_id is not listed
	 * @pre - ensure that the item_id is not listed. Search by item_id in sales & auctions tables
	 */
	ACTION delitem(
				const name& collection_name,
				uint64_t item_id,
				uint64_t author_id
			);


	/**
	 * @brief - add/modify asset into nftownership table
	 * @details - add/modify asset into nftownership table
	 * 
	 * @param owner_id - owner id
	 * @param collection_name - collection name
	 * @param asset_id - asset id
	 */
	ACTION addastnftown(
				uint64_t owner_id,
				const name& collection_name,
				uint64_t asset_id,
			);

	/**
	 * @brief - delete asset into nftownership table
	 * @details - delete asset into nftownership table
	 * 
	 * @param owner_id - owner id
	 * @param collection_name - collection name
	 * @param asset_id - asset id
	 */
	ACTION delastnftown(
				uint64_t owner_id,
				const name& collection_name,
				uint64_t asset_id,
			);

	/**
	 * @brief - add/modify item into nftownership table
	 * @details - add/modify item into nftownership table
	 * 
	 * @param owner_id - owner id
	 * @param asset_id - asset id
	 * @param item_id - item id
	 */
	ACTION additmnftown(
				uint64_t owner_id,
				uint64_t asset_id,
				uint64_t item_id
			);


	/**
	 * @brief - delete item into nftownership table
	 * @details - delete item into nftownership table
	 * 
	 * @param owner_id - owner id
	 * @param asset_id - asset id
	 * @param item_id - item id
	 */
	ACTION delitmnftown(
				uint64_t owner_id,
				uint64_t asset_id,
				uint64_t item_id
			);


	/**
	 * @brief - list item on sale
	 * @details - list item on sale
	 * 
	 * @param item_id - item id
	 * @param collection_name - collection name
	 * @param author_id - author id
	 * @param listing_price_crypto - price in crypto
	 * @param listing_price_fiat - price in fiat
	 * 
	 * @pre - match the chat_id in telegram with the author_id for user verification
	 * @pre - item must not be listed before (in 'sales' & 'auction' TABLE)
	 */
	ACTION listitemsale(
				uint64_t item_id,
				const name& collection_name,
				const name& author_id,
				const asset& listing_price_crypto,
				const string& listing_price_fiat,
			);

	/**
	 * @brief - unlist item on sale
	 * @details - unlist item on sale
	 * 
	 * @param item_id - item id
	 * @param author_id - author id
	 * 
	 * 
	 * @pre - match the chat_id in telegram with the author_id for user verification
	 * @pre - item must be listed before on 'sales' TABLE
	 */
	ACTION ulistitmsale(
				uint64_t item_id,
				const name& author_id
			);

	/**
	 * @brief - list item on auction
	 * @details - list item on auction
	 * 
	 * @param item_id - item id
	 * @param collection_name - collection name
	 * @param author_id - author id
	 * @param current_bid_crypto - price in crypto
	 * @param current_bid_fiat - price in fiat
	 * 
	 * @pre - match the chat_id in telegram with the author_id for user verification
	 * @pre - item must not be listed before (in 'sales' & 'auction' TABLE)
	 */
	ACTION listitemauct(
				uint64_t item_id,
				const name& collection_name,
				const name& author_id,
				const asset& current_bid_crypto,
				const string& current_bid_fiat,
			);


	/**
	 * @brief - unlist item on auction
	 * @details - unlist item on auction
	 * 
	 * @param item_id - item id
	 * @param author_id - author id
	 * 
	 * @pre - match the chat_id in telegram with the author_id for user verification
	 * @pre - item must be listed before in 'auction' TABLE
	 */
	ACTION ulistitmauct(
				uint64_t item_id,
				const name& author_id,
			);



	// -----------------------------------------------------------------------------------------------------------------------
	// scope: <telegram_id>
	TABLE balance
	{
        asset balance;

        uint64_t primary_key()const { return balance.symbol.code().raw(); }
	};

	using balance_index = multi_index<"balances"_n, balance>

	// -----------------------------------------------------------------------------------------------------------------------
	// scope: <person_telegram_id>
	TABLE oyanonauthor
	{
		name collection_name;			// collection name
		uint64_t asset_id;				// asset id
		vector<uint64_t> item_ids;			// list of item id for the asset_id
		bool is_author;				// yes/no i.e. 1/0

		auto primary_key() const { return collection_name.value; }
		uint64_t by_asset() const { return asset_id; }
	};

	using oyanonauthor_index = multi_index<"oyanonauthor"_n, oyanonauthor,
								indexed_by< "byasset"_n, const_mem_fun<oyanonauthor, uint64_t, &oyanonauthor::by_asset>>
								>;

	// -----------------------------------------------------------------------------------------------------------------------
	// scope: <author_telegram_id>
	TABLE collection
	{
		// name author;					// collection author name [DISABLED for Telegram Bot, as there is no EOSIO account for users needed]
		name collection_name;			// collection name
		string collection_desc;			// collection description
		string collection_url;			// collection url
		// uint64_t collection_item_qty;	// collection item qty
		// vector<uint64_t> collection_item_ids;	// collection item ids	


		auto primary_key() const { return collection_name.value; }
	};

	using collection_index = multi_index<"collections"_n, collection>;

	// -----------------------------------------------------------------------------------------------------------------------
	// scope: <collection_name>
	TABLE asset
	{
		uint64_t asset_id;				// asset id. Another is asset_id i.e. 9210<start_time><max copies upto 99999> E.g. if max_copies = 100, then 9210<start_time>1 is the 1st asset_id. This is shown in the auctions, sales TABLE
		uint64_t author_id;				// author telegram_id		
		uint64_t current_owner_id;		// current owner telegram_id
		string asset_name;				// asset name
		string asset_desc;				// asset description
		checksum256 asset_img_hash;		// asset image hash
		checksum256 asset_vid_hash;		// asset video hash
		checksum256 asset_gif_hash;		// asset gif hash
		// map<string, string> asset_trade_ids_success;		// asset trade ids list with order ids (successful ones)
		// map<string, string> asset_trade_ids_ongoing;		// asset trade ids list with order ids (ongoing ones)
		uint64_t asset_copies_qty_used;	// asset copies used qty
		uint64_t asset_copies_qty_total;		// asset copies total qty (if burned an item, then qty is decreased here)
		float asset_royaltyfee;			// asset royalty fee
		string asset_artist;				// asset artist


		auto primary_key() const { return asset_id; }
		uint64_t by_author() const { return author_id; }
		uint64_t by_curr_owner() const { return current_owner_id; }
	};

	using asset_index = multi_index<"assets"_n, asset,
								indexed_by< "byauthor"_n, const_mem_fun<asset, uint64_t, &asset::by_author>>,	
								indexed_by< "bycurrowner"_n, const_mem_fun<asset, uint64_t, &asset::by_curr_owner>>	
								>;



	// -----------------------------------------------------------------------------------------------------------------------
	// scope: self i.e. oyanftmarket
	TABLE sale
	{
		uint64_t sale_id;		// sale_id id. 3700<start_time>
		uint64_t item_id;			// item_id format is "<item_id>999999" E.g. if max_copies = 100, then "<asset_id>1" is the 1st item_id. If max_copies = 1, then item_id is "<asset_id>1"
		uint64_t asset_id;			// asset id
		uint64_t seller_id;				// seller telegram_id
		asset listing_price_crypto;		// listing price of asset in crypto (if opted for crypto)
		string listing_price_fiat;		// listing price of asset in fiat (if opted for fiat)
		uint64_t buyer_id;				// buyer telegram_id
		name collection_name;		// collection name
		float royalty_fee;			// collection/royalty fee


		auto primary_key() const { return sale_id; }
		uint64_t by_item() const { return item_id; }
		uint64_t by_asset() const { return asset_id; }
		uint64_t by_seller() const { return seller_id; }
		uint64_t by_buyer() const { return buyer_id; }
		uint64_t by_collection() const { return collection_name.value; }
	};

	using sale_index = multi_index<"sales"_n, sale>,
								indexed_by< "byitem"_n, const_mem_fun<sale, uint64_t, &sale::by_item>>,	
								indexed_by< "byasset"_n, const_mem_fun<sale, uint64_t, &sale::by_asset>>,
								indexed_by< "byseller"_n, const_mem_fun<sale, uint64_t, &sale::by_seller>>,
								indexed_by< "bybuyer"_n, const_mem_fun<sale, uint64_t, &sale::by_buyer>>,
								indexed_by< "bycollection"_n, const_mem_fun<sale, uint64_t, &sale::by_collection>>,
								>;


	// -----------------------------------------------------------------------------------------------------------------------
	// scope: self i.e. oyanftmarket
	TABLE auction
	{
		uint64_t auction_id;		// auction id. 3701<start_time>
		uint64_t item_id;			// item_id format is "<item_id>99999" E.g. if max_copies = 100, then "<asset_id>1" is the 1st item_id. If max_copies = 1, then item_id is "<asset_id>1"
		uint64_t asset_id;			// asset id
		uint64_t seller_id;				// seller telegram_id
		uint64_t start_time;		// auction start time
		uint64_t end_time;			// auction end time
		asset current_bid_crypto;			// current bid of asset in crypto (if opted for crypto)
		string current_bid_fiat;			// current bid of asset in fiat (if opted for fiat)
		uint64_t current_bidder_id;		// current bidder telegram_id
		bool claimed_by_seller;		// claimed by seller
		bool claimed_by_buyer;		// claimed by buyer
		name collection_name;		// collection name
		float royalty_fee;		// collection/royalty fee


		auto primary_key() const { return auction_id; }
		uint64_t by_item() const { return item_id; }
		uint64_t by_asset() const { return asset_id; }
		uint64_t by_seller() const { return seller_id; }
		uint64_t by_buyer() const { return buyer_id; }
		uint64_t by_collection() const { return collection_name.value; }
	};

	using auction_index = multi_index<"auctions"_n, auction>,
								indexed_by< "byitem"_n, const_mem_fun<auction, uint64_t, &auction::by_item>>,	
								indexed_by< "byasset"_n, const_mem_fun<auction, uint64_t, &auction::by_asset>>,
								indexed_by< "byseller"_n, const_mem_fun<auction, uint64_t, &auction::by_seller>>,
								indexed_by< "bybuyer"_n, const_mem_fun<auction, uint64_t, &auction::by_buyer>>,
								indexed_by< "bycollection"_n, const_mem_fun<auction, uint64_t, &auction::by_collection>>,
								>;


	// -----------------------------------------------------------------------------------------------------------------------
	inline uint64_t str_to_uint64t(const string& s) {
		uint64_t num = strtoull(s.c_str(), NULL, 10);
		return num;
	}

};