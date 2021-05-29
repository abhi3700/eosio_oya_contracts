#pragma once
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
// #include <eosio/crypto.hpp>
#include <string>
#include <vector>


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
// using eosio::current_time_point;
using eosio::action;
using eosio::same_payer;
using eosio::symbol;
using eosio::extended_symbol;
// using eosio::require_recipient;
// using eosio::checksum256;
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
	TABLE nftownership
	{
		name collection_name;			// collection name
		uint64_t item_id;				// item id
		vector<uint64_t> item_sub_ids;			// list of item sub id
		bool author;				// yes/no i.e. 1/0

		auto primary_key() const { return collection_name.value; }
	};

	using nftownership_index = multi_index<"nftownership"_n, nftownership>;

	// -----------------------------------------------------------------------------------------------------------------------
	// scope: <author_telegram_id>
	TABLE collection
	{
		// name author;					// collection author name [DISABLED for Telegram Bot, as there is no EOSIO account for users needed]
		name collection_name;			// collection name
		uint64_t author_id;				// author telegram_id
		string collection_desc;			// collection description
		string collection_url;			// collection url
		uint64_t collection_item_qty;	// collection item qty
		vector<uint64_t> collection_item_ids;	// collection item ids	


		auto primary_key() const { return collection_name.value; }
	};

	using collection_index = multi_index<"collections"_n, collection>;

	// -----------------------------------------------------------------------------------------------------------------------
	// scope: <collection_name>
	TABLE asset
	{
		uint64_t item_id;				// item/asset id. Another is asset_sub_id i.e. 921<start_time>999999 E.g. if max_copies = 100, then 922<start_time>001 is the 1st asset_sub_id. This is shown in the auctions, sales TABLE
		uint64_t author_id;				// author telegram_id		
		uint64_t current_owner_id;		// current owner telegram_id
		string item_name;				// item name
		string item_desc;				// item description
		checksum256 item_img_hash;		// item image hash
		checksum256 item_vid_hash;		// item video hash
		checksum256 item_gif_hash;		// item gif hash
		vector<uint64_t> item_trade_ids;// item trade ids list with order ids (successful ones)
		uint64_t item_mint_qty_used;	// item copies used qty
		uint64_t item_copies_qty_total;		// item copies total qty (if burned an item, then qty is decreased here)
		float item_royaltyfee;			// item royalty fee
		string item_artist;				// item artist


		auto primary_key() const { return item_id; }
		uint64_t by_author() const {return author_id; }
		uint64_t by_curr_owner() const {return current_owner_id; }
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
		uint64_t asset_sub_id;			// item/asset sub id format is "<item_id>999999" E.g. if max_copies = 100, then "<item_id>001" is the 1st asset_sub_id. If max_copies = 1, then asset_sub_id is "<item_id>1"
		uint64_t asset_id;
		uint64_t seller_id;				// seller telegram_id
		asset listing_price_crypto;		// listing price of asset in crypto (if opted for crypto)
		string listing_price_fiat;		// listing price of asset in fiat (if opted for fiat)
		uint64_t buyer_id;				// buyer
		name collection_name;		// collection name
		float royalty_fee;			// collection/royalty fee


		auto primary_key() const { return sale_id; }
	};

	using sale_index = multi_index<"sales"_n, sale>;


	// -----------------------------------------------------------------------------------------------------------------------
	// scope: self i.e. oyanftmarket
	TABLE auction
	{
		uint64_t auction_id;		// auction id. 3701<start_time>
		uint64_t asset_sub_id;			// item/asset sub id format is "<item_id>999999" E.g. if max_copies = 100, then "<item_id>001" is the 1st asset_sub_id. If max_copies= 1, then asset_sub_id is "<item_id>1"
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
	};

	using auction_index = multi_index<"auctions"_n, auction>;



};