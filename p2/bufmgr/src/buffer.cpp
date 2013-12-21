/**
 * @author See Contributors.txt for code contributors and overview of BadgerDB.
 *
 * @section LICENSE
 * Copyright (c) 2012 Database Group, Computer Sciences Department, University of Wisconsin-Madison.
 */

///////////////////////////////////////////
// File: buffer.cpp
// Time stamp: Feb 20, 11.12PM
//  Description: This program create a buffer manager above I/O layer provided. This buffer manager will help to handle the pages that being read into main memory and also deallocation of the pages.
//  Student Name: Liang Zheng Gooi	 
//  UW Campus ID: 9066169518	
//  email: lgooi@wisc.edu
/////////////////////////////////////////// 

#include <memory>
#include <iostream>
#include "buffer.h"
#include "exceptions/buffer_exceeded_exception.h"
#include "exceptions/page_not_pinned_exception.h"
#include "exceptions/page_pinned_exception.h"
#include "exceptions/bad_buffer_exception.h"
#include "exceptions/hash_not_found_exception.h"

namespace badgerdb { 

/*
 *
 * Constructor of BufMgr class
 */
BufMgr::BufMgr(std::uint32_t bufs)
	: numBufs(bufs) {
	bufDescTable = new BufDesc[bufs];

  for (FrameId i = 0; i < bufs; i++) 
  {
  	bufDescTable[i].frameNo = i;
  	bufDescTable[i].valid = false;
  }

  bufPool = new Page[bufs];

	int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
  hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table

  clockHand = bufs - 1;
}

/**
  * Destructor of BufMgr class
  */
BufMgr::~BufMgr() {
	for(std::uint32_t i =0 ; i<numBufs; i++){
		if(bufDescTable[i].dirty == true){
			flushFile(bufDescTable[i].file);		
		}
	}
	delete[] bufDescTable;
	delete[] bufPool;
	delete hashTable;
}

/*
 *
 * Advance clock to next frame in the buffer pool
 */
void BufMgr::advanceClock()
{
	clockHand++;
	clockHand = clockHand % (numBufs);
}

/*
 *
 * Allocate a free frame.  
 *
 * @param frame   	Frame reference, frame ID of allocated frame returned via this variable
 * @throws BufferExceededException If no such buffer is found which can be allocated
 */
void BufMgr::allocBuf(FrameId & frame) 
{
	std::uint32_t clock_count = 0;
	//clock algorithm
	while(1){
		advanceClock();
		if(bufDescTable[clockHand].valid == true){
			if(bufDescTable[clockHand].refbit == true){
				bufDescTable[clockHand].refbit = false;
				//skip all the steps below and loop again
				continue;
			}
			if(bufDescTable[clockHand].pinCnt > 0){
				clock_count++;
				//if every pages is being pinned in buffer pool, throw exception
				if(clock_count > numBufs){
					throw BufferExceededException();
				}
				continue;
			}
			if(bufDescTable[clockHand].dirty == true){
				flushFile(bufDescTable[clockHand].file);
				bufDescTable[clockHand].dirty = false;
			}
			else{
				hashTable->remove(bufDescTable[clockHand].file, bufDescTable[clockHand].pageNo);
			}
			break;
		}
		else{
			break;
		}
	}

	frame = bufDescTable[clockHand].frameNo;
}

/**
 * Reads the given page from the file into a frame and returns the pointer to page.
 * If the requested page is already present in the buffer pool pointer to that frame is returned
 * otherwise a new frame is allocated from the buffer pool for reading the page.
 *
 * @param file   	File object
 * @param PageNo  Page number in the file to be read
 * @param page  	Reference to page pointer. Used to fetch the Page object in which requested page from file is read in.
 */
void BufMgr::readPage(File* file, const PageId pageNo, Page*& page)
{
	FrameId frame_check;
	try{
		hashTable->lookup(file, pageNo, frame_check);
	}catch(HashNotFoundException ex){
		
	//the page is not in the pool, so allocate one
	FrameId frame_no;
	allocBuf(frame_no);
	bufPool[frame_no] = file->readPage(pageNo);
	hashTable->insert(file, pageNo, frame_no);
	bufDescTable[frame_no].Set(file, pageNo);
	page = &bufPool[frame_no];
	return;
	}

	bufDescTable[frame_check].pinCnt++;
	bufDescTable[frame_check].refbit = true;
	page = &bufPool[frame_check];
}

/**
 * Unpin a page from memory since it is no longer required for it to remain in memory.
 *
 * @param file   	File object
 * @param PageNo  Page number
 * @param dirty		True if the page to be unpinned needs to be marked dirty	
  * @throws  PageNotPinnedException If the page is not already pinned
 */
void BufMgr::unPinPage(File* file, const PageId pageNo, const bool dirty) 
{
	//search for the page then clear
	FrameId frame_no;
	try{
		hashTable->lookup(file, pageNo, frame_no);
		if(bufDescTable[frame_no].pinCnt == 0){
			throw PageNotPinnedException(bufDescTable[frame_no].file->filename(), bufDescTable[frame_no].pageNo, frame_no);
		}
		bufDescTable[frame_no].pinCnt--;
		if(dirty == true){
			bufDescTable[frame_no].dirty = true;
		}
	}
	catch(HashNotFoundException ex){
		//do nothing
	}
}

/**
 * Delete page from file and also from buffer pool if present.
 * Since the page is entirely deleted from file, its unnecessary to see if the page is dirty.
 *
 * @param file   	File object
 * @param PageNo  Page number
 * @throws PagePinnedException If the page is pinned in the buffer pool
*/
void BufMgr::disposePage(File *file, const PageId pageNo){
	FrameId frame_no;
	try{
		hashTable->lookup(file, pageNo, frame_no);
		if(bufDescTable[frame_no].pinCnt > 0)
			throw PagePinnedException(bufDescTable[frame_no].file->filename(), bufDescTable[frame_no].pageNo, frame_no);
		else{
			//if the page is still in buffer pool
			bufDescTable[frame_no].Clear();
			hashTable->remove(file, pageNo);
		}
		
	}catch(HashNotFoundException ex){
		//do nothing
	}
	
	file->deletePage(pageNo);
}

/**
 * Writes out all dirty pages of the file to disk.
 * All the frames assigned to the file need to be unpinned from buffer pool before this function can be successfully called.
 * Otherwise Error returned.
 *
 * @throws  PagePinnedException If any page of the file is pinned in the buffer pool 
 * @throws BadBufferException If any frame allocated to the file is found to be invalid
 */
void BufMgr::flushFile(const File* file) 
{
	
		int pages_found = 0;
		for(std::uint32_t i = 0; i<numBufs; i++){
			if(bufDescTable[i].file == file){
				pages_found++;
				if(bufDescTable[i].pinCnt > 0){
					throw PagePinnedException(bufDescTable[i].file->filename(), bufDescTable[i].pageNo, i);
				}
				else if(bufPool[i].page_number() != bufDescTable[i].pageNo || bufDescTable[i].valid == false){
					throw BadBufferException(i, bufDescTable[i].dirty, bufDescTable[i].valid, bufDescTable[i].refbit);
				}

				if(bufDescTable[i].dirty == true){
					bufDescTable[i].file->writePage(bufPool[i]);
				}
				hashTable->remove(file, bufDescTable[i].pageNo);
				bufDescTable[i].Clear();
			}
		}	
}

/**
 * Allocates a new, empty page in the file and returns the Page object.
 * The newly allocated page is also assigned a frame in the buffer pool.
 *
 * @param file   	File object
 * @param PageNo  Page number. The number assigned to the page in the file is returned via this reference.
 * @param page  	Reference to page pointer. The newly allocated in-memory Page object is returned via this reference.
 */
void BufMgr::allocPage(File* file, PageId &pageNo, Page*& page) 
{
	FrameId frame;
	allocBuf(frame);
	
	bufPool[frame] = file->allocatePage();
	pageNo = bufPool[frame].page_number();

	bufDescTable[frame].Set(file, pageNo);
	hashTable->insert(file, pageNo, frame);
	page = &bufPool[frame];
}

/*
 *
 * Print member variable values. 
 */
void BufMgr::printSelf(void) 
{
  BufDesc* tmpbuf;
	int validFrames = 0;
  
  for (std::uint32_t i = 0; i < numBufs; i++)
	{
  	tmpbuf = &(bufDescTable[i]);
		std::cout << "FrameNo:" << i << " ";
		tmpbuf->Print();

  	if (tmpbuf->valid == true)
    	validFrames++;
  }

	std::cout << "Total Number of Valid Frames:" << validFrames << "\n";
}

}
