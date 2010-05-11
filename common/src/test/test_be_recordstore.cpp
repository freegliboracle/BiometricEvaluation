/*
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */

#include <iostream>

#include <stdlib.h>
#include <be_filerecstore.h>

using namespace BiometricEvaluation;

/*
 * Test the read and write operations of a Bitstore, hopefully stressing
 * it enough to gain confidence in its operation. This program should be
 * able to test any implementation of the abstract Bitstore by creating
 * an object of the appropriate implementation class.
 */
int main (int argc, char* argv[]) {

	/*
	 * Other types of Bitstore objects can be created here and
	 * accessed via the Bitstore interface.
	 */
	/* Call the constructor that will create a new FileRecordStore. */
	string frstest("frs_test");
	RecordStore *rs;
	try {
		rs = new FileRecordStore(frstest, "RW Test Dir");
	} catch (ObjectExists) {
		cout << "The RecordStore already exists; using it." << endl;
		try {
			rs = new FileRecordStore(frstest);
		} catch (StrategyError e) {
			cout << "A strategy error occurred: " << e.getInfo() << endl;
		}
	} catch (StrategyError e) {
		cout << "A strategy error occurred: " << e.getInfo() << endl;
	}

	/*
	 * From this point forward, all access to the store object, no matter
	 * what subclass, is done via the RecordStore interface.
	 */

	/*
	 * Insert a record to the RecordStore so we can read/write it.
	 */
	string firstRec("firstRec");
	char *wdata = "ABCDEFGHIJKLMNOPQWSTUVWXYZ";
	uint64_t count = strlen(wdata);
	try {
		cout << "insert(" << firstRec << ")";
		rs->insert(firstRec, wdata, count);
	} catch (ObjectExists) {
		cout << " exists; deleting." << endl;
		rs->remove(firstRec);
		rs->insert(firstRec, wdata, count);
	} catch (StrategyError e) {
		cout << " failed:" << e.getInfo() << "." << endl;
		exit (EXIT_FAILURE);
	}
	cout << endl;

	char rdata[64];
	bzero(rdata, 64);
	try {
		cout << "read(" << firstRec << ")";
		count = rs->read(firstRec, rdata);
	} catch (ParameterError e) {
		cout << " failed: " << e.getInfo() << "." << endl;
		exit (EXIT_FAILURE);
	} catch (StrategyError e) {
		cout << " failed: " << e.getInfo() << "." << endl;
		exit (EXIT_FAILURE);
	}
	cout << " succeeded, read " << rdata << endl;
	delete rs;

	exit(EXIT_SUCCESS);
}