/**
 * @author See Contributors.txt for code contributors and overview of BadgerDB.
 *
 * @section LICENSE
 * Copyright (c) 2012 Database Group, Computer Sciences Department, University of Wisconsin-Madison.
 */

#include "btree.h"
#include "filescan.h"
#include "exceptions/bad_index_info_exception.h"
#include "exceptions/bad_opcodes_exception.h"
#include "exceptions/bad_scanrange_exception.h"
#include "exceptions/no_such_key_found_exception.h"
#include "exceptions/scan_not_initialized_exception.h"
#include "exceptions/index_scan_completed_exception.h"
#include "exceptions/file_not_found_exception.h"
#include "exceptions/end_of_file_exception.h"
#include "exceptions/file_exists_exception.h"
#include "string.h"
#include "exceptions/invalid_page_exception.h"
#include <typeinfo>
//#define DEBUG

namespace badgerdb
{


// -----------------------------------------------------------------------------
// BTreeIndex::BTreeIndex -- Constructor
// -----------------------------------------------------------------------------

  /**
   * BTreeIndex Constructor. 
	 * Check to see if the corresponding index file exists. If so, open the file.
	 * If not, create it and insert entries for every tuple in the base relation using FileScan class.
   *
   * @param relationName        Name of file.
   * @param outIndexName        Return the name of index file.
   * @param bufMgrIn						Buffer Manager Instance
   * @param attrByteOffset			Offset of attribute, over which index is to be built, in the record
   * @param attrType						Datatype of attribute over which index is built
   * @throws  BadIndexInfoException     If the index file already exists for the corresponding attribute, but values in metapage(relationName, attribute byte offset, attribute type etc.) do not match with values received through constructor parameters.
   */
BTreeIndex::BTreeIndex(const std::string & relationName,
		std::string & outIndexName,
		BufMgr *bufMgrIn,
		const int attrByteOffset,
		const Datatype attrType)
{
	// initialize all the local variables
	std::ostringstream idxStr;
	idxStr<<relationName<<'.'<<attrByteOffset;
	outIndexName = idxStr.str();
	headerPageNum = 1;
	bufMgr = bufMgrIn;
	attributeType = attrType;
	BTreeIndex::attrByteOffset = (int)attrByteOffset;
	if(attributeType == INTEGER){
		leafOccupancy = INTARRAYLEAFSIZE;
		nodeOccupancy = INTARRAYNONLEAFSIZE;
	}
	else if(attributeType == DOUBLE){
		leafOccupancy = DOUBLEARRAYLEAFSIZE;
		nodeOccupancy = DOUBLEARRAYNONLEAFSIZE;
	}
	else if(attributeType == STRING){
		leafOccupancy = STRINGARRAYLEAFSIZE;
		nodeOccupancy = STRINGARRAYNONLEAFSIZE;
	}
	else{
		std::cout << "Attribute Type does not fit the enum! \n";
		return;
	}
	scanExecuting = false;
	std::cout<< outIndexName << "\n";

	try{
		BlobFile* newFile = new BlobFile(outIndexName, true);
		file = (File*)newFile;
		Page* empty_page;
		PageId meta_pageId;
		bufMgr->allocPage(file, meta_pageId, empty_page);
		bufMgr->unPinPage(file, meta_pageId, true);
		bufMgr->readPage(file, meta_pageId, empty_page);
		IndexMetaInfo* metapage = (IndexMetaInfo*)empty_page;
		
		std::copy(relationName.begin(), relationName.end(), (*metapage).relationName);
		(*metapage).attrByteOffset = attrByteOffset;
		(*metapage).attrType = attrType;
		Page* root_page;
		bufMgr->allocPage(file, rootPageNum, root_page);
		bufMgr->unPinPage(file,rootPageNum, true);
		bufMgr->readPage(file, rootPageNum, root_page);
		(*metapage).rootPageNo = rootPageNum;
		(*metapage).isRootLeafPage = true;
		// create an empty leaf node at root
		if((*metapage).attrType == INTEGER){
			LeafNodeInt* leafNode = (LeafNodeInt*) root_page;
			(*leafNode).validEntry_num = 0;
		}
		else if((*metapage).attrType == DOUBLE){
			LeafNodeDouble* leafNode = (LeafNodeDouble*) root_page;
			(*leafNode).validEntry_num = 0;
		}
		else if((*metapage).attrType == STRING){
			LeafNodeString* leafNode = (LeafNodeString*) root_page;
			(*leafNode).validEntry_num = 0;
		}
		else{
			std::cout <<"INVALID type in constructor\n";
		}
		
		
		bufMgr->unPinPage(file, rootPageNum, true );
		bufMgr->unPinPage(file, meta_pageId, true);

		BTreeIndex::createInitialTree(relationName, bufMgrIn);

		std::cout << "File created: " << (*metapage).relationName << "\n";
		bufMgr->flushFile(file);
	}
	catch(FileExistsException ex){
		//check for metafile
		BlobFile* newfile = new BlobFile(outIndexName, false);
		file = (File*) newfile;
		IndexMetaInfo* metapage_info;
		Page *meta_page;
		bufMgrIn->readPage(file, headerPageNum, meta_page);
		metapage_info = (IndexMetaInfo*) meta_page;
		rootPageNum = (*metapage_info).rootPageNo;
		std::cout << (*metapage_info).relationName << "  "<< relationName<< "\n";
		if(relationName.compare((*metapage_info).relationName)==0 
			&& (*metapage_info).attrByteOffset==attrByteOffset 
			&& (*metapage_info).attrType==attrType){
			rootPageNum = (*metapage_info).rootPageNo;

			return;
		}
		else{
			throw BadIndexInfoException("Metafile does not match!!");
		}
		bufMgr->unPinPage(file, headerPageNum, false);

	}catch(FileNotFoundException ex){
		std::cout<<"You should not get FileNotFoundException in constructor!!\n";
	}	

}


// Create initial tree
const void BTreeIndex:: createInitialTree(const std::string &relationName, BufMgr *bufMgrIn){
	//build the tree
		FileScan fscan(relationName, bufMgrIn);
		try{
			RecordId scan_rid;
			while(1){
				fscan.scanNext(scan_rid);
				std::string recordStr = fscan.getRecord();
				const char* record = recordStr.c_str();
				if(attributeType == INTEGER){
					int key = *((int*) (record + attrByteOffset));
					std::cout << "Inserting : " << key << " into tree\n";
					BTreeIndex::insertEntry(&key, scan_rid);
				}
				else if (attributeType == DOUBLE){
					double key = *((double*) (record + attrByteOffset));
					std::cout << "Inserting : " << key <<" into tree\n";
					BTreeIndex::insertEntry(&key, scan_rid);
				}
				else{
					char keyHolder[10];
					char* src = (char*)(record + attrByteOffset);
					strncpy(keyHolder, src, sizeof(keyHolder));
					std::string key = std::string(keyHolder);
					std::cout << "Inserting : " << key << " into tree\n";
					BTreeIndex::insertEntry(&key, scan_rid);
				}
			}
		}
		catch(EndOfFileException e){
			scanExecuting = false;
			std::cout << "All records had been read in constructors\n";
		}


}

// -----------------------------------------------------------------------------
// BTreeIndex::~BTreeIndex -- destructor
// -----------------------------------------------------------------------------

  /**
   * BTreeIndex Destructor. 
	 * End any initialized scan, flush index file, after unpinning any pinned pages, from the buffer manager
	 * and delete file instance thereby closing the index file.
	 * Destructor should not throw any exceptions. All exceptions should be caught in here itself. 
	 * */
BTreeIndex::~BTreeIndex()
{
	scanExecuting = false;
	bufMgr->unPinPage(file, currentPageNum, false);
	bufMgr->flushFile(file);
}

// -----------------------------------------------------------------------------
// BTreeIndex::insertEntry
// -----------------------------------------------------------------------------

  /**
	 * Insert a new entry using the pair <value,rid>. 
	 * Start from root to recursively find out the leaf to insert the entry in. The insertion may cause splitting of leaf node.
	 * This splitting will require addition of new leaf page number entry into the parent non-leaf, which may in-turn get split.
	 * This may continue all the way upto the root causing the root to get split. If root gets split, metapage needs to be changed accordingly.
	 * Make sure to unpin pages as soon as you can.
   * @param key			Key to insert, pointer to integer/double/char string
   * @param rid			Record ID of a record whose entry is getting inserted into the index.
	**/
const void BTreeIndex::insertEntry(const void *key, const RecordId rid) 
{
	
	//get the rootpage
	Page* meta_page;
	bufMgr->readPage(file, headerPageNum, meta_page);
	IndexMetaInfo* metapage = (IndexMetaInfo*)meta_page;
	//rootPageNum = (*metapage).rootPageNo;
	bool leafIsRoot = (*metapage).isRootLeafPage;
	bufMgr->unPinPage(file, headerPageNum, false);

	if(leafIsRoot){
		switch(attributeType){
			case INTEGER:	BTreeIndex::insertLeafPage<int, struct LeafNodeInt, struct NonLeafNodeInt>(rootPageNum, (void*)key, rid);
					break;
			case DOUBLE:	BTreeIndex::insertLeafPage<double, struct LeafNodeDouble, struct NonLeafNodeDouble>(rootPageNum, (void*)key, rid);
					break;
			case STRING:	BTreeIndex::insertLeafPage<std::string, struct LeafNodeString, struct NonLeafNodeString>(rootPageNum, (void*)key, rid);
					break;
			default:	std::cout << "something wrong with the attribute type!!!\n";
		}
	}
	else{
		// the root is non leaf, go deep to the target leaf node
		switch(attributeType){
			case INTEGER:	{
					RIDKeyPair<int>* Intrid_pair = new RIDKeyPair<int>();
					Intrid_pair->set(rid, *((int*)key) );
					PageId target_leafNode = BTreeIndex::lookForTargetLeaf<int, struct LeafNodeInt, struct NonLeafNodeInt>(rootPageNum, (RIDKeyPair<int>*) Intrid_pair);
					BTreeIndex::insertLeafPage<int, struct LeafNodeInt, struct NonLeafNodeInt>(target_leafNode, (void*)key, rid);
					}
					break;

			case DOUBLE:	{
					RIDKeyPair<double>* Doublerid_pair = new RIDKeyPair<double>();
					Doublerid_pair->set(rid, *((double*)key) );
					PageId target_leafNode = BTreeIndex::lookForTargetLeaf<double, struct LeafNodeDouble, struct NonLeafNodeDouble>(rootPageNum, (RIDKeyPair<double>*) Doublerid_pair);
					BTreeIndex::insertLeafPage<double, struct LeafNodeDouble, struct NonLeafNodeDouble>(target_leafNode, (void*)key, rid);
					}
					break;

			case STRING:	{
					RIDKeyPair<std::string>* Stringrid_pair = new RIDKeyPair<std::string>();
					Stringrid_pair->set(rid, *((char**)key) );
					PageId target_leafNode = BTreeIndex::lookForTargetLeaf<std::string, struct LeafNodeString, struct NonLeafNodeString>(rootPageNum, (RIDKeyPair<std::string>*) Stringrid_pair);
					BTreeIndex::insertLeafPage<std::string, struct LeafNodeString, struct NonLeafNodeString>(target_leafNode, (void*)key, rid);
					}
					break;

			default:	std::cout << "something wrong with the attribute type!!!\n";
		}
	}
}


/* traverse the level and compare the key
 * */
template<class T, class T_leafFormat, class T_nonleafFormat>
const PageId BTreeIndex::lookForTargetLeaf(PageId &root_id, RIDKeyPair<T>* rid_pair){
	Page* root_page;
	bufMgr->readPage(file, root_id, root_page);
	T_nonleafFormat* rootPage = reinterpret_cast<T_nonleafFormat*> (root_page);
	
	//iterate deep into the levels
	T obj_key = (*rid_pair).key;
	int level = (*rootPage).level;
	while(level >= 1){
		//compare the key of no leaf
		std::uint32_t index = 0;
		while(index < sizeof((*rootPage).keyArray)){
			if(obj_key >= (*rootPage).keyArray[index]){
				index++;
			}
			else{
				break;
			}
		}
		PageId next_pageId = (*rootPage).pageNoArray[index+1];
		bufMgr->unPinPage(file, root_id, false);
		// if this node is at level 1 (means that it is just 1 level up from non leaf), return pageID
		if(level == 1){
			Page* nonleaf_page;
			bufMgr->readPage(file, next_pageId, nonleaf_page);
			T_nonleafFormat* nonleafPage = reinterpret_cast<T_nonleafFormat*> (nonleaf_page);
			for(int i =0; i<(*nonleafPage).validEntry_num; i++){
				if(obj_key < (*nonleafPage).keyArray[i]){
					return (*nonleafPage).pageNoArray[i];
				}
			}
		}
		else{	// if this node is not at level 1, do deeper
			return BTreeIndex::lookForTargetLeaf<T, T_leafFormat, T_nonleafFormat>(next_pageId, rid_pair);
		}
	}
	
	std::cout << "Something wrong in the lookForTargetLeaf\n";
	PageId invalidPageId = 0;
	return invalidPageId;
}

//generic copy function for leaf
const void BTreeIndex::copyToLeaf(Page* currentPage, int index, void *key, const RecordId &rid){
	if(attributeType == STRING){
		LeafNodeString* leafString_page = (LeafNodeString*) currentPage;
		strncpy((*leafString_page).keyArray[index], (char*)key, 10);
		(*leafString_page).ridArray[index] = rid;
	}
	else if (attributeType == INTEGER){
		LeafNodeInt* leafInt_page = (LeafNodeInt*) currentPage;
		(*leafInt_page).keyArray[index] = *((int*) key);
		(*leafInt_page).ridArray[index] = rid;
	}
	else if (attributeType == DOUBLE){
		LeafNodeDouble* leafDouble_page = (LeafNodeDouble*) currentPage;
		(*leafDouble_page).keyArray[index] = *((double*)key);
		(*leafDouble_page).ridArray[index] = rid;
	}
	else{
		std::cout<<"Copy not success!!\n";
	}
}

//generic copy function for non leaf
const void BTreeIndex::copyToNonLeaf(Page* currentPage, std::uint32_t index, void *key, const PageId &pid){
	if(attributeType == STRING){
		NonLeafNodeString* nonleafString_page = (NonLeafNodeString*) currentPage;
		strncpy((*nonleafString_page).keyArray[index], *((char**)key), 10);
		(*nonleafString_page).pageNoArray[index+1] = pid;
	}
	else if (attributeType == INTEGER){
		NonLeafNodeInt* nonleafInt_page = (NonLeafNodeInt*) currentPage;
		(*nonleafInt_page).keyArray[index] = *((int*)key);
		(*nonleafInt_page).pageNoArray[index+1] = pid;
	}
	else if (attributeType == DOUBLE){
		NonLeafNodeDouble* nonleafDouble_page = (NonLeafNodeDouble*) currentPage;
		(*nonleafDouble_page).keyArray[index] = *((double*)key);
		(*nonleafDouble_page).pageNoArray[index+1] = pid;
	}
	else{
		std::cout<<"Copy not success!!\n";
	}
}

/* insert leaf pages
 *
 * */
template<class T, class T_leafFormat, class T_nonleafFormat>
const void BTreeIndex::insertLeafPage(PageId &firstLeaf_pageId, void *key, const RecordId &rid){
	
	Page* current_page;
	bufMgr->readPage(file, firstLeaf_pageId, current_page);
	T_leafFormat* first_leafPage = reinterpret_cast<T_leafFormat*> (current_page);
	int index = (*first_leafPage).validEntry_num;
	
	if(index < (int)sizeof((*first_leafPage).ridArray)){
		BTreeIndex::copyToLeaf(current_page, index, key, rid);
		//(*first_leafPage).ridArray[index] = rid;
		(*first_leafPage).validEntry_num++;
		bufMgr->unPinPage(file, firstLeaf_pageId, true);
		//std::cout << "I am in insertLeafPage " << sizeof((*first_leafPage).ridArray) << "\n"; 
	}
	else{
		bufMgr->unPinPage(file, firstLeaf_pageId, false);
		BTreeIndex::splitAndInsertLeafPage<T, T_leafFormat, T_nonleafFormat>( firstLeaf_pageId, key, rid);
	}
}

/* Insert leaf if the leaf is full
 *
 * */
template<class T, class T_leafFormat, class T_nonleafFormat>
const void BTreeIndex::splitAndInsertLeafPage(PageId &firstLeaf_pageId, void *key, const RecordId rid){
	//read the first leaf node
	Page* currentPage;
	bufMgr->readPage(file, firstLeaf_pageId, currentPage);
	T_leafFormat* first_leafPage = reinterpret_cast<T_leafFormat*> (currentPage);
	
	// create a new leaf node
	PageId secondLeaf_pageId;
	Page* newPage;
	bufMgr->allocPage(file, secondLeaf_pageId, newPage);
	T_leafFormat* second_leafPage = reinterpret_cast<T_leafFormat*> (newPage);
		
	int second_arrayIndex =0;
	int end_of_array = 0;
	if(sizeof((*first_leafPage).keyArray)%2 > 0){
		end_of_array = ((int)sizeof((*first_leafPage).keyArray)/2)+1;
	}
	else{
		end_of_array = ((int)sizeof((*first_leafPage).keyArray))/2;
	}			
	(*first_leafPage).validEntry_num = end_of_array;
	(*second_leafPage).validEntry_num = (int)sizeof((*first_leafPage).keyArray) - end_of_array;
	
	
	//copy the entry from array from node 1 to node 2
	while(second_arrayIndex < end_of_array){
//		(*second_leafPage).ridArray[second_arrayIndex] 
//			= (*first_leafPage).ridArray[second_arrayIndex+end_of_array];
		BTreeIndex::copyToLeaf((Page*) second_leafPage, second_arrayIndex,
			(void*)&((*first_leafPage).keyArray[second_arrayIndex+end_of_array]), 
			(*first_leafPage).ridArray[second_arrayIndex+end_of_array] );  //<<===========PICKUP FROM HERE
		
		//delete rid from first leaf page
		//(*first_leafPage).ridArray[second_arrayIndex+end_of_array] = reinterpret_cast<PageId> (0); 

//		(*second_leafPage).keyArray[second_arrayIndex] 
//			= (*first_leafPage).keyArray[second_arrayIndex+end_of_array];
		second_arrayIndex++;
	}
				
	//compare key to go to the correct leaf 
	T_leafFormat* target_leafPage;
	if(*((T*)key) < (*second_leafPage).keyArray[0]){
		target_leafPage = first_leafPage;
	}
	else{
		target_leafPage = second_leafPage;
	}
	(*target_leafPage).validEntry_num += 1;
		
	// insert entry to leaf node
	std::uint32_t newArrayCounter = 0;
	T newKeyArray[sizeof( (*target_leafPage).keyArray )];
	RecordId newRidArray[sizeof( (*target_leafPage).ridArray )];
	bool newEntryInserted = false;
	for(int i = 0, newArray_counter=0; i < (*target_leafPage).validEntry_num ; newArrayCounter++){
		if((*target_leafPage).keyArray[i] < *((T*)key) || newEntryInserted == true){
			newKeyArray[newArray_counter] = (*target_leafPage).keyArray[i];
			newRidArray[newArray_counter] = (*target_leafPage).ridArray[i];
			i++;
		}
		else{
			newKeyArray[newArray_counter] = *((T*)key);
			newRidArray[newArray_counter] = rid;
			newEntryInserted = true;
		}
	}
	std::cout<< "I am in split and insert leaf, valid entry is "<< (*first_leafPage).validEntry_num <<" \n";
	memcpy((*target_leafPage).keyArray, newKeyArray, (*target_leafPage).validEntry_num);
	memcpy((*target_leafPage).ridArray, newRidArray, (*target_leafPage).validEntry_num);
	//delete[] newKeyArray;
	//delete[] newRidArray; 
		
	// connect node 1 to node 2
	(*second_leafPage).rightSibPageNo = (*first_leafPage).rightSibPageNo;
	(*first_leafPage).rightSibPageNo = secondLeaf_pageId;
	
	// read metapage to set root page to non leaf if the rootpage is leaf
	Page* metapage;
	bufMgr->readPage(file, headerPageNum, metapage);
	IndexMetaInfo* metapage_info = (IndexMetaInfo*) metapage;
	
	//create a non leaf node
	PageId nonleaf_pageid;
	Page* nonleaf_page;
	bufMgr->allocPage(file, nonleaf_pageid, nonleaf_page);
	T_nonleafFormat* first_nonleafPage = reinterpret_cast<T_nonleafFormat*> (nonleaf_page);
	(*first_nonleafPage).level = 1;
	(*first_nonleafPage).validEntry_num = 0;

	std::uint32_t validEntry_index = (*first_nonleafPage).validEntry_num; 
	//(*first_nonleafPage).keyArray[validEntry_index] = (*second_leafPage).keyArray[0];
	//(*first_nonleafPage).pageNoArray[validEntry_index+1] = secondLeaf_pageId;
	BTreeIndex::copyToNonLeaf((Page*)first_nonleafPage, validEntry_index,
			(void*) &((*second_leafPage).keyArray[0]), secondLeaf_pageId); //<================see here
	(*first_nonleafPage).validEntry_num++;

	// first leaf and second leaf is configured correctly
	bufMgr->unPinPage(file, secondLeaf_pageId, true);
	bufMgr->unPinPage(file, firstLeaf_pageId, true);
	
	// if leaf is the root
	if((*metapage_info).isRootLeafPage == true){
		(*metapage_info).isRootLeafPage = false;
		(*metapage_info).rootPageNo = nonleaf_pageid;
		std::cout << "Set to non leaf as root\n";
		rootPageNum = nonleaf_pageid;
		bufMgr->unPinPage(file, headerPageNum, true);
		(*first_nonleafPage).pageNoArray[0] = firstLeaf_pageId;
		// we are done creating non leaf as root, unpin
		bufMgr->unPinPage(file, nonleaf_pageid, true);
	}
	else{
		// if the nonleaf is root
		int level = (*first_nonleafPage).level;
		bufMgr->unPinPage(file, headerPageNum, false);
		bufMgr->unPinPage(file, nonleaf_pageid, true);
		// go through the tree and find the suitble space to insert the none leaf node
		BTreeIndex::insertNonLeafPage<T, T_leafFormat, T_nonleafFormat>(nonleaf_pageid, level);

	}
}

/* Insert non leaf
 *
 * */
template<class T, class T_leafFormat, class T_nonleafFormat>
const void BTreeIndex::insertNonLeafPage(PageId &first_nonleafId, int &level){
	
	
	//read from metafile and find the suitable spot to insert non root
	Page* current_rootPage;
	bufMgr->readPage(file, rootPageNum, current_rootPage);
	T_nonleafFormat* rootPage = reinterpret_cast<T_nonleafFormat*> (current_rootPage);

	// if the level is higher than the root page, it becomes the root
	if(level >= (*rootPage).level){
		Page* current_metapage;
		bufMgr->readPage(file, headerPageNum, current_metapage);
		IndexMetaInfo* metapage = (IndexMetaInfo*)current_metapage;
		(*metapage).rootPageNo = first_nonleafId;
		rootPageNum = first_nonleafId;
		bufMgr->unPinPage(file, headerPageNum, true);
		bufMgr->unPinPage(file, rootPageNum, false);
		return;
	}

	Page* current_page;
	bufMgr->readPage(file, first_nonleafId, current_page);
	T_nonleafFormat* first_nonleafPage = reinterpret_cast<T_nonleafFormat*> (current_page);
	void* key = reinterpret_cast<void*> (&(*first_nonleafPage).keyArray[0]);

	PageId target_nonleafId = BTreeIndex::lookForNonLeafAboveLeaf<T, T_leafFormat, T_nonleafFormat>(rootPageNum, key, level);
	Page* target_page;
	bufMgr->readPage(file, target_nonleafId, target_page);
	T_nonleafFormat* target_nonleafPage = reinterpret_cast<T_nonleafFormat*> (target_page);
	//if target non leaf is not full, then just insert
	int index = (*target_nonleafPage).validEntry_num;
	if( index < (int) sizeof( (*target_nonleafPage).keyArray ) ){
		//copy over the key and pageid
		(*target_nonleafPage).pageNoArray[index] = (*first_nonleafPage).pageNoArray[0];
		for(int j=0; j<(*first_nonleafPage).validEntry_num; j++){
			BTreeIndex::copyToNonLeaf((Page*) target_nonleafPage, (index+j),
				(void*)&(*first_nonleafPage).keyArray[j], (*first_nonleafPage).pageNoArray[j+1] );
			(*target_nonleafPage).validEntry_num++;
		}
		//(*target_nonleafPage).keyArray[index] = (*first_nonleafPage).keyArray[1];
		bufMgr->unPinPage(file, first_nonleafId, false);
		//(*target_nonleafPage).validEntry_num++;
		bufMgr->unPinPage(file, target_nonleafId, true);
		//delete current page from file
		try{
			BlobFile* b_file = (BlobFile*) file;
			b_file->deletePage(first_nonleafId);
		}catch(InvalidPageException ex){
			
		}
	}
	else{
		//else call split and Insert
		bufMgr->unPinPage(file, first_nonleafId, false);
		bufMgr->unPinPage(file, target_nonleafId, false);
		BTreeIndex::splitAndInsertNonLeafPage<T, T_leafFormat, T_nonleafFormat>(target_nonleafId, first_nonleafId);
	}
}

/* Look for target non leaf
 *
 * */
template<class T, class T_leafFormat, class T_nonleafFormat>
const PageId BTreeIndex::lookForNonLeafAboveLeaf(PageId &root_id, void* key, int &target_level){ //<==================Look for nonleaf that fits the level
	//check metafile for empty Tree
	Page* meta_page;
	bufMgr->readPage(file, headerPageNum, meta_page);
	IndexMetaInfo* metapage = (IndexMetaInfo*) meta_page;
	if((*metapage).isRootLeafPage){
		std::cout << "You got empty tree man@.@\n";
		return (*metapage).rootPageNo;
	}


	Page* root_page;
	bufMgr->readPage(file, root_id, root_page);
	T_nonleafFormat* rootPage = reinterpret_cast<T_nonleafFormat*> (root_page);

	T obj_key = *((T*)key);
	
	int level = (*rootPage).level;
	while(level >= target_level){
		//compare the key of no leaf
		int index = 0;
		while(index < (*rootPage).validEntry_num){
			if(obj_key >= (*rootPage).keyArray[index]){
				index++;
			}
			else{
				break;
			}
		}
		PageId next_pageId = (*rootPage).pageNoArray[index];
		bufMgr->unPinPage(file, root_id, false);
		// if this node is at level 1 (means that it is just 1 level up from non leaf), return pageID
		if(level == target_level){
			return next_pageId;
		}
		else{	// if this node is not at target level, go deeper
			return BTreeIndex::lookForNonLeafAboveLeaf<T, T_leafFormat, T_nonleafFormat>(next_pageId, key, target_level);
		}
	}
	
	std::cout << "Something wrong in the lookForNonLeafAboveLeaf\n" << root_id << "\t" << level << "\n";
	PageId empty_pageId = 0;
	return empty_pageId;
}

/* Insert non leaf if full
 *
 * */
template<class T, class T_leafFormat, class T_nonleafFormat>
const void BTreeIndex::splitAndInsertNonLeafPage(PageId &first_nonleafId, PageId &newAlloc_pageId){
	//create new non-leaf parent node
	Page* newPage;
	PageId parent_nonleafId;
	bufMgr->allocPage(file, parent_nonleafId, newPage);
	T_nonleafFormat* parent_nonleafPage = reinterpret_cast<T_nonleafFormat*>(newPage);

	//read the first non leaf page & second non leaf page
	Page* first_page;
	bufMgr->readPage(file, first_nonleafId, first_page);
	T_nonleafFormat* first_nonleafPage = reinterpret_cast<T_nonleafFormat*>(first_page);

	Page* second_page;
	PageId second_nonleafId;
	bufMgr->allocPage(file, second_nonleafId, second_page);
	T_nonleafFormat* second_nonleafPage = reinterpret_cast<T_nonleafFormat*>(second_page);


	// check for odd size array then set the correct valid entry num 
	std::uint32_t sizeOfArray = sizeof((*first_nonleafPage).keyArray);
	int indexAfterDivide;
	if(sizeOfArray%2 > 0){
		indexAfterDivide = (sizeOfArray/2)+1;
	}
	else{
		indexAfterDivide = (sizeOfArray/2);
	}
	(*first_nonleafPage).validEntry_num = indexAfterDivide;
	(*second_nonleafPage).validEntry_num = (int)sizeOfArray - indexAfterDivide;

	// copy from first node to second node
	std::uint32_t second_pageIndex = 0;
	(*second_nonleafPage).pageNoArray[0] = (*first_nonleafPage).pageNoArray[indexAfterDivide];
	while((int)second_pageIndex < indexAfterDivide){
		BTreeIndex::copyToNonLeaf((Page*) second_nonleafPage, second_pageIndex, 
			(void*)&((*first_nonleafPage).keyArray[second_pageIndex + indexAfterDivide]), 
			(*first_nonleafPage).pageNoArray[second_pageIndex + 1 + indexAfterDivide]);
		second_pageIndex++;
	}

	//put first node and second node into parent node
	//(*parent_nonleafPage).keyArray[0] = (*second_nonleafPage).keyArray[0];
	(*parent_nonleafPage).pageNoArray[0] = first_nonleafId;
	std::uint32_t parent_index = 0;
	BTreeIndex::copyToNonLeaf((Page*) parent_nonleafPage, parent_index,
		(void*)&((*second_nonleafPage).keyArray[0]), second_nonleafId);
	//(*parent_nonleafPage).pageNoArray[1] = second_nonleafId;
	int level = (*first_nonleafPage).level + 1;
	(*parent_nonleafPage).level = level;
	bufMgr->unPinPage(file, parent_nonleafId, true);

	//compare and set the target non leaf node to insert
	Page* entry_page;
	bufMgr->readPage(file, newAlloc_pageId, entry_page);
	T_nonleafFormat* entry_nonleafPage = reinterpret_cast<T_nonleafFormat*> (entry_page);
	T_nonleafFormat* target_nonleafPage;
	//int indexOf_entry = (*entry_nonleafPage).validEntry_num; 
	if((T)((*entry_nonleafPage).keyArray[0]) >= (T)((*second_nonleafPage).keyArray[0]) ){
		target_nonleafPage = second_nonleafPage;
	}
	else{
		target_nonleafPage = first_nonleafPage;
	}
	(*target_nonleafPage).validEntry_num += 1;

	//lets the compare begin after found the target node
	T newKeyArray[sizeof((*target_nonleafPage).keyArray)];
	PageId newPageNoArray[sizeof((*target_nonleafPage).pageNoArray)];
	int index_forTarget = 0;
	bool newEntry_inserted = false;
	for(int i=0; index_forTarget < (*target_nonleafPage).validEntry_num; index_forTarget++){
		//insert pageId to the left side/ first entry in pageid array (only for first case)
		if(index_forTarget == 0){
			if((*entry_nonleafPage).keyArray[0] <= (*target_nonleafPage).keyArray[0]){
				newPageNoArray[0] = newAlloc_pageId;
			}
			else{
				newPageNoArray[0] = (*target_nonleafPage).pageNoArray[0];
			}
		}

		//insert pageid to the right side and insert key
		if((*entry_nonleafPage).keyArray[0] > (*target_nonleafPage).keyArray[index_forTarget] || newEntry_inserted == true){
			newKeyArray[index_forTarget+i] = (*target_nonleafPage).keyArray[index_forTarget];
			newPageNoArray[(index_forTarget+1)+i] = (*target_nonleafPage).pageNoArray[index_forTarget + 1];
		}
		else{
			newKeyArray[index_forTarget+i] = (*entry_nonleafPage).keyArray[0];
			newPageNoArray[(index_forTarget+1)+i] = newAlloc_pageId;
			newEntry_inserted = true;
			i++;
		}
	}
	memcpy((*target_nonleafPage).keyArray, newKeyArray, (*target_nonleafPage).validEntry_num);
	memcpy((*target_nonleafPage).pageNoArray, newPageNoArray, (*target_nonleafPage).validEntry_num);
	bufMgr->unPinPage(file, first_nonleafId, true);
	bufMgr->unPinPage(file, second_nonleafId, true);
	bufMgr->unPinPage(file, newAlloc_pageId, true);

	//find the non leaf of that level and insert it
	BTreeIndex::insertNonLeafPage<T, T_leafFormat, T_nonleafFormat>(parent_nonleafId, level);
}

const void BTreeIndex::printTree(){
	//check whether the root is leaf or not
	Page* meta_page;
	bufMgr->readPage(file, headerPageNum, meta_page);
	IndexMetaInfo* metapage = (IndexMetaInfo*) meta_page;
	bool isRootLeaf = (*metapage).isRootLeafPage;
	bufMgr->unPinPage(file, headerPageNum, false);

	Page* root_page;
	bufMgr->readPage(file, rootPageNum, root_page);
	if(isRootLeaf){
		//print tree at base level
	}
}

template<class T, class T_leafFormat, class T_nonleafFormat>
const void BTreeIndex::printTreeFrom(PageId &pid){
	Page* newPage;
	bufMgr->readPage(file, pid, newPage);
	T_nonleafFormat nonleaf_page = reinterpret_cast<T_nonleafFormat>(newPage);

	int validEntry_index = (*nonleaf_page).validEntry_num;
	PageId pageNoQueue[sizeof((*nonleaf_page).pageNoArray)+1];
	memcpy(pageNoQueue, (*nonleaf_page).pageNoArray, validEntry_index+1);

	int level = (*nonleaf_page).level;
	if(level > 0){
		std::cout << "\n\n";
		for(int i=0; i < validEntry_index; i++){
			std::cout << (*nonleaf_page).keyArray[i] << " ";
		}
		bufMgr->unPinPage(file, pid, false);

		if(level == 1){
			//go to leaf
			Page* leaf_page;
			PageId leaf_pageId = pageNoQueue[0];
			bufMgr->readPage(file, leaf_pageId, leaf_page);
			PageId nextLeaf_pageId;
			T_leafFormat leafPage = reinterpret_cast<T_leafFormat>(leaf_page);
			do{
				int validEntry_leaf = (*leafPage).validEntry_num;
				for(int k=0; k<validEntry_leaf; k++){
					std::cout << (*leafPage).keyArray[k] << " ";
				}
				std::cout << "| ";
				nextLeaf_pageId = (*leafPage).rightSibPageNo;
				bufMgr->unPinPage(file, leaf_pageId, false);
				bufMgr->readPage(file, nextLeaf_pageId, leaf_page);
				leafPage = reinterpret_cast<T_leafFormat>(leaf_page);
				leaf_pageId = nextLeaf_pageId;
			
			}while((*leafPage).rightSibPageNo!=0);
			bufMgr->unPinPage(file, leaf_pageId, false);
		}
		else{
			for(int j=0; j < validEntry_index+1; j++)
				BTreeIndex::printTreeFrom(pageNoQueue[j]);
		}
	}

}
// -----------------------------------------------------------------------------
// BTreeIndex::startScan
// -----------------------------------------------------------------------------

  /**
	 * Begin a filtered scan of the index.  For instance, if the method is called 
	 * using ("a",GT,"d",LTE) then we should seek all entries with a value 
	 * greater than "a" and less than or equal to "d".
	 * If another scan is already executing, that needs to be ended here.
	 * Set up all the variables for scan. Start from root to find out the leaf page that contains the first RecordID
	 * that satisfies the scan parameters. Keep that page pinned in the buffer pool.
   * @param lowVal	Low value of range, pointer to integer / double / char string
   * @param lowOp		Low operator (GT/GTE)
   * @param highVal	High value of range, pointer to integer / double / char string
   * @param highOp	High operator (LT/LTE)
   * @throws  BadOpcodesException If lowOp and highOp do not contain one of their their expected values 
   * @throws  BadScanrangeException If lowVal > highval
	 * @throws  NoSuchKeyFoundException If there is no key in the B+ tree that satisfies the scan criteria.
	**/
const void BTreeIndex::startScan(const void* lowValParm,
				   const Operator lowOpParm,
				   const void* highValParm,
				   const Operator highOpParm)
{
	if(scanExecuting){
		return;
	}
	else{
		scanExecuting = true;
		lowOp = lowOpParm;
		highOp = highOpParm;
	}
	if((lowOp!=LT && lowOp!=LTE && lowOp!=GTE && lowOp!=GT) ||
		(highOp!=LT && highOp!=LTE && highOp!=GTE && highOp!=GT)){
		scanExecuting = false;
		throw BadOpcodesException();	
	}	
	//check whether root is leaf
	bool rootIsLeaf = false;
	Page* meta_page;
	bufMgr->readPage(file, headerPageNum, meta_page);
	IndexMetaInfo* metapage = (IndexMetaInfo*) meta_page;
	if((*metapage).isRootLeafPage){
		rootIsLeaf = true;
	}

	// start casting using attributeType
	if(attributeType == INTEGER){
		lowValInt = *((int*)lowValParm);
		highValInt = *((int*) highValParm);
		if(lowValInt > highValInt){
			scanExecuting = false;
			throw BadScanrangeException();
		}
		int base_level = 1;
		void* lowValKey = reinterpret_cast<void*>(&lowValInt);
		std::cout << "Looking at lowValKey " << *((int*)lowValKey) << "\n";
		if(!rootIsLeaf){
			PageId target_pageId = BTreeIndex::lookForNonLeafAboveLeaf<int, struct LeafNodeInt, struct NonLeafNodeInt>(rootPageNum, lowValKey, base_level);
			Page* nonleaf_page;
			bufMgr->readPage(file, target_pageId, nonleaf_page);
			NonLeafNodeInt* nonleafPage = (NonLeafNodeInt*) nonleaf_page;

			int targetIndex = 0;
			int validEntry_count = (*nonleafPage).validEntry_num;
			//go though the key in non leaf to find the suitable leaf node
			while(targetIndex < validEntry_count){
				if((lowOp == GT || lowOp == GTE) && lowValInt < (*nonleafPage).keyArray[targetIndex]){
					break;
				}
				else{
					targetIndex++;
				}
			}

			if(targetIndex >= validEntry_count){
				throw BadScanrangeException();
			}
			else{
				currentPageNum = (*nonleafPage).pageNoArray[targetIndex];
				bufMgr->unPinPage(file, target_pageId, false);
				bufMgr->readPage(file, currentPageNum, currentPageData);
				LeafNodeInt* leaf_node = (LeafNodeInt*) currentPageData;
				for(int i =0; i < (*leaf_node).validEntry_num; i++){
					if(lowValInt >= (*leaf_node).keyArray[i]){
						if(lowOp == GTE && lowValInt == (*leaf_node).keyArray[i]){
							nextEntry = i;
							break;
						}
						nextEntry = i;
					}
				}
			
				bufMgr->unPinPage(file, currentPageNum, false);
			
			}
		}
		else{
			//leaf is root, look through the list of keys then set the point
			currentPageNum = rootPageNum;
			bufMgr->readPage(file, currentPageNum, currentPageData);
			LeafNodeInt* leaf_node = (LeafNodeInt*) currentPageData;
			std::cout << "There are " << (*leaf_node).validEntry_num << " in this leaf!\n and start with ";
			for(int i =0; i < (*leaf_node).validEntry_num; i++){
				std::cout << (*leaf_node).keyArray[i] << "\t";
				if(lowValInt >= (*leaf_node).keyArray[i]){
					if(lowOp == GTE && lowValInt == (*leaf_node).keyArray[i]){
						nextEntry = i;
						break;
					}
					nextEntry = i;
				}
			}
			std::cout<<"\n";
			bufMgr->unPinPage(file, currentPageNum, false);
		}

	}
	else if(attributeType == DOUBLE){
		lowValDouble = *((double*)lowValParm);
		highValDouble = *((double*) highValParm);
		if(lowValDouble>highValDouble){
			scanExecuting = false;
			throw BadScanrangeException();
		}
	}
	else if(attributeType == STRING){
		lowValString = *((std::string*)lowValParm);
		highValString = *((std::string*) highValParm);
		if(lowValString.compare(0,9,highValString) > 0){
			scanExecuting = false;
			throw BadScanrangeException();
		}
	}
}


// -----------------------------------------------------------------------------
// BTreeIndex::scanNext
// -----------------------------------------------------------------------------

  /**
	 * Fetch the record id of the next index entry that matches the scan.
	 * Return the next record from current page being scanned. If current page has been scanned to its entirety, move on to the right sibling of current page, if any exists, to start scanning that page. Make sure to unpin any pages that are no longer required.
   * @param outRid	RecordId of next record found that satisfies the scan criteria returned in this
	 * @throws ScanNotInitializedException If no scan has been initialized.
	 * @throws IndexScanCompletedException If no more records, satisfying the scan criteria, are left to be scanned.
	**/
const void BTreeIndex::scanNext(RecordId& outRid) 
{
	if(!scanExecuting){
		throw ScanNotInitializedException();
	}

	bufMgr->readPage(file, currentPageNum, currentPageData);
	
	switch(attributeType){
		case INTEGER:	{
					LeafNodeInt* leaf_node = (LeafNodeInt*) currentPageData;
					// check if nextIndex is exceeding the valid entry size
					if(nextEntry >= (*leaf_node).validEntry_num){
						//read next page and reset next entry
						PageId temp_pageId = (*leaf_node).rightSibPageNo;
						bufMgr->unPinPage(file, currentPageNum, false);
						currentPageNum = temp_pageId;
						bufMgr->readPage(file, currentPageNum, currentPageData);
						leaf_node = (LeafNodeInt*) currentPageData;
						nextEntry = 0;
					}

					if(nextEntry < (*leaf_node).validEntry_num){
						if(highOp == LT){
							if((*leaf_node).keyArray[nextEntry] == highValInt){
								bufMgr->unPinPage(file, currentPageNum, false);
								throw IndexScanCompletedException();
							}
						}

						if((*leaf_node).keyArray[nextEntry] > highValInt){
							bufMgr->unPinPage(file, currentPageNum, false);
							throw IndexScanCompletedException();
						}
						else{
							//return the rid
							outRid = (*leaf_node).ridArray[nextEntry];
							nextEntry++;
							
						}
					}
					else{
						std::cout << "NextIndex overflow in scanNext!!!\n";	
					}
				}
				break;
		default:	std::cout << "Invalid type in scanNext!!\n";
	}
}

// -----------------------------------------------------------------------------
// BTreeIndex::endScan
// -----------------------------------------------------------------------------
//

  /**
	 * Terminate the current scan. Unpin any pinned pages. Reset scan specific variables.
	 * @throws ScanNotInitializedException If no scan has been initialized.
	**/
const void BTreeIndex::endScan() 
{
	if(!scanExecuting){
		throw ScanNotInitializedException();
	}
	bufMgr->unPinPage(file, currentPageNum, false);
	scanExecuting = false;
}

}
