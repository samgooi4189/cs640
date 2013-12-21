	else if(metapage.attrType == Datatype.STRING){
		NonLeafNodeString non_leafNode = *(NonLeafNodeString*) currentPageData;
	}
	else{
		std::cout <<"INVALID type in constructor\n";
	}
	
	else if(metapage.isRootLeafPage){
		if(metapage.attrType == Datatype.INTEGER){
			//if root page is not created, readPage will automatically allocate a page
			bufMger->readPage(file, rootPageNum, currentPageData);		// remember to unpin this
			LeafNodeInt leafPage = *(LeafNodeInt*)currentPageData;
			// if the root node is empty, create a leaf node as root
			if(leafPage == NULL){
				leafPage.ridArray[0] = rid;
				leafPage.keyArray[0] = key;
			}
			else{
				//go through every entry in array inside leaf node
				int arrayIndex = 0;
				while(arrayIndex < INTARRAYLEAFSIZE && leafPage.keyArray[arrayIndex]!= NULL){
					arrayIndex++;
				}
				// if array is less than half full, insert entry to the node
				if(arrayIndex < INTARRAYLEAFSIZE){
					leafPage.ridArray[arrayIndex] = rid;
					leafPage.keyArray[arrayIndex] = key;
				}
				else{
					// create another node and aggregate a non-leaf node
					PageId page_id;
					Page newPage;
					bufMgr->allocPage(file, page_id, newPage);
					bufMgr->readPage(file, page_id, newPage);	//remember to unpin this
					LeafNodeInt second_leafPage = *(LeafNodInt*)newPage;
					
					int second_arrayIndex =0;
					int end_of_array = 0;
					if(isOddNumber(INTARRAYLEAFSIZE)){
						end_of_array = (INTARRAYLEAFSIZE/2)+1;
					}
					else{
						end_of_array = INTARRAYLEAFSIZE/2;
					}
					
					//copy the entry from array from node 1 to node 2
					while(second_arrayIndex < end_of_array){
						second_leafPage.ridArray[second_arrayIndex] 
							= leafPage.ridArray[second_arrayIndex+end_of_array];
						second_leafPage.keyArray[second_arrayIndex] 
							= leafPage.keyArray[second_arrayIndex+end_of_array];
					}
					// insert entry at the appropriate index	<========MISSING CODE

					// connect node 1 to node 2
					second_leafPage.rightSibPageNo = leafPage.rightSibPageNo;
					leafPage.rightSibPageNo = page_id;

					//create non leafpage for the first time because root node is leafnode
					PageId nonLeaf_pageId;
					Page nonLeaf_page;
					bufMgr->allocPage(file, nonLeaf_pageId, nonLeaf_page);
					bufMgr->readPage(file, nonLeaf_pageId, nonLeaf_page);
					NonLeafNodeInt nonLeaf_node = *(nonLeaf_page*) nonLeafPage;
					nonLeaf_node.level = 0;
					nonLeaf_node.keyArray[0] = second_leafPage.keyArray[0];
					nonLeaf_node.pageNoArray[0] = rootPageNum;
					nonLeaf_node.pageNoArray[1] = page_id;

					//set the root page to the newly created non leaf node
					bufMgr->readPage(file, headerPageNum, currentPageData);
					metapage_info = *(IndexMetaInfo*)currentPageData;
					metapage_info.rootPageNo = nonLeaf_pageId;
					rootPageNum = nonLeaf_pageId;
					bufMgr->unPinPage(file, headerPageNum, true);
				}
			}
		}
	}







	if(metapage.attrType == INTEGER){
		//if the rootpage is leaf
		if(metapage.isRootLeafPage){
			// create first leaf node
			int curr_entryNum = (*root_page).validEntry_num;
			if(curr_entryNum < leafOccupancy){
				(*root_page).ridArray[curr_entryNum] = rid;
				(*root_page).keyArray[curr_entryNum] = *((int *)key);
				(*root_page).validEntry_num++;
			}
			else{
				// if the leaf node is full
				// create another node and aggregate a non-leaf node
				PageId page_id;
				Page* newPage;
				bufMgr->allocPage(file, page_id, newPage);
				bufMgr->unPinPage(file, page_id, true);
				bufMgr->readPage(file, page_id, newPage);	//remember to unpin this
				LeafNodeInt* second_leafPage = (LeafNodeInt*)newPage;
					
				int second_arrayIndex =0;
				int end_of_array = 0;
				if(INTARRAYLEAFSIZE%2 > 0){
					end_of_array = (INTARRAYLEAFSIZE/2)+1;
				}
				else{
					end_of_array = INTARRAYLEAFSIZE/2;
				}			
				(*root_page).validEntry_num = end_of_array;
				(*second_leafPage).validEntry_num = INTARRAYLEAFSIZE - end_of_array;

				//copy the entry from array from node 1 to node 2
				while(second_arrayIndex < end_of_array){
					(*second_leafPage).ridArray[second_arrayIndex] 
						= (*root_page).ridArray[second_arrayIndex+end_of_array];
					//<===============================================================================Pick up from here
					(*second_leafPage).keyArray[second_arrayIndex] 
						= (*root_page).keyArray[second_arrayIndex+end_of_array];
				}
				
				//compare key to go to the correct leaf 
				LeafNodeInt* target_leafPage;
				if(*((int*)key) < (*second_leafPage).keyArray[0]){
					target_leafPage = root_page;
				}
				else{
					target_leafPage = second_leafPage;
				}
				(*target_leafPage).validEntry_num += 1;
					
				// insert entry to leaf node
				int newArrayCounter = 0;
				int newKeyArray[INTARRAYLEAFSIZE];
				RecordId newRidArray[INTARRAYLEAFSIZE];
				for(int i = 0, newArray_counter=0; i < second_arrayIndex; newArrayCounter++){
					if((*root_page).keyArray[i] < *((int*)key)){
						newKeyArray[newArray_counter] = (*target_leafPage).keyArray[i];
						newRidArray[newArray_counter] = (*target_leafPage).ridArray[i];
						i++;
					}
					else{
						newKeyArray[newArray_counter] = *(int*)key;
						newRidArray[newArray_counter] = rid;	
					}
				}
				memcpy((*target_leafPage).keyArray, newKeyArray, (*target_leafPage).validEntry_num);
				memcpy((*target_leafPage).ridArray, newRidArray, (*target_leafPage).validEntry_num);
				delete[] newKeyArray;
				delete[] newRidArray; 
					

				// connect node 1 to node 2
				second_leafPage.rightSibPageNo = leafPage.rightSibPageNo;
				leafPage.rightSibPageNo = page_id;
				// set the root non leaf
				non_leafPage.keyArray[0] = second_leafPage.keyArray[0];
				non_leafPage.pageNoArray[1] = page_id;
				bufMgr->unPinPage(file, page_id, true);


				//create a non leaf node
				PageId nonleaf_pageid;
				Page* nonleaf_page;
				bufMgr->allocPage(file, nonleaf_pageid, nonleaf_page);
				(*nonleaf_page).level = 0;
				(*nonleaf_page).validPage_num = 2;
				(*nonleaf_page).pageNoArray[0] = rootPageNum;
				(*nonleaf_page).pageNoArray[1] = page_id;
				(*nonleaf_page).
				bufMgr->unPinPage(file, nonleaf_pageid, true);
				
				
			}
		bufMgr->unPinPage(file, rootPageNum, true);
		return;
		}
	}
