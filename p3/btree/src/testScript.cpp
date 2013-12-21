#include <vector>
#include "btree.h"
#include "page.h"
#include "filescan.h"
#include "page_iterator.h"
#include "file_iterator.h"
#include "exceptions/insufficient_space_exception.h"
#include "exceptions/index_scan_completed_exception.h"
#include "exceptions/file_not_found_exception.h"
#include "exceptions/no_such_key_found_exception.h"
#include "exceptions/bad_scanrange_exception.h"
#include "exceptions/bad_opcodes_exception.h"
#include "exceptions/scan_not_initialized_exception.h"
#include "exceptions/end_of_file_exception.h"

using namespace badgerdb;
int testNum = 1;
const std::string relationName = "relA";

typedef struct tuple {
	int i;
	double d;
	char s[64];
}RECORD;

BufMgr* bufMgr = new BufMgr(100);

int main(int argc, char **argv){
	if( argc != 2 )
	{
		std::cout << "Expects one argument as a number between 1 to 3 to choose datatype of key.\n";
		std::cout << "For INTEGER keys run as: ./badgerdb_main 1\n";
		std::cout << "For DOUBLE keys run as: ./badgerdb_main 2\n";
		std::cout << "For STRING keys run as: ./badgerdb_main 3\n";
		return 0;
	}

	sscanf(argv[1],"%d",&testNum);
	
	switch(testNum)
	{
		case 1:
			std::cout << "leaf size:" << INTARRAYLEAFSIZE << " non-leaf size:" << INTARRAYNONLEAFSIZE << std::endl;
			break;
		case 2:
			std::cout << "leaf size:" << DOUBLEARRAYLEAFSIZE << " non-leaf size:" << DOUBLEARRAYNONLEAFSIZE << std::endl;
			break;
		case 3:
			std::cout << "leaf size:" << STRINGARRAYLEAFSIZE << " non-leaf size:" << STRINGARRAYNONLEAFSIZE << std::endl;
			break;
	}
 
}
