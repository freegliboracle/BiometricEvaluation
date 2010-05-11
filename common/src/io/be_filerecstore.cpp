/******************************************************************************
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 ******************************************************************************/

#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <be_filerecstore.h>

const string controlFileName(".frscontrol");

BiometricEvaluation::FileRecordStore::FileRecordStore(
    const string &name,
    const string &description)
    throw (ObjectExists, StrategyError) : RecordStore(name, description)
{
	struct stat sb;

	/* The directory where the store is rooted is just the name of
	 * the store, created in the current working directory of the
	 * process.
	 */
	_directory = name;

	/*
	 * The RecordStore is implemented as a directory in the current
	 * working directory.
	 */
	/* Check that the directory doesn't already exist */
	if (stat(_directory.c_str(), &sb) == 0)
		throw ObjectExists("Named object already exists");

	/* Make the new directory, checking for errors */
	if (mkdir(_directory.c_str(), 0700) != 0)
		throw StrategyError("Could not create directory");
	try {
		(void)writeControlFile();
	} catch (StrategyError e) {
		throw e;
	}
	
}

BiometricEvaluation::FileRecordStore::FileRecordStore(
    const string &name)
    throw (ObjectDoesNotExist, StrategyError)
{

	_directory = name;

	/* Check that the directory exists, throwing an error if not */
	if (!fileExists(name))
		throw ObjectDoesNotExist();

	try {
		(void)readControlFile();
	} catch (StrategyError e) {
		throw e;
	}
}

void
BiometricEvaluation::FileRecordStore::insert( 
    const string &key,
    const void *data,
    const uint64_t size)
    throw (ObjectExists, StrategyError)
{
	string pathname = canonicalName(key);
	if (fileExists(pathname))
		throw ObjectExists();

	std::FILE *fp = std::fopen(pathname.c_str(), "wb");
	if (fp == NULL)
		throw StrategyError("Could not create " + pathname);

	std::size_t sz = fwrite(data, 1, size, fp);
	std::fclose(fp);
	if (sz != size)
		throw StrategyError("Could not write " + pathname);

	_count++;

}

void
BiometricEvaluation::FileRecordStore::remove( 
    const string &key)
    throw (ObjectDoesNotExist, StrategyError)
{
	string pathname = canonicalName(key);
	if (!fileExists(pathname))
		throw ObjectDoesNotExist();

}

uint64_t
BiometricEvaluation::FileRecordStore::read(
    const string &key,
    void * data)
    throw (ParameterError, StrategyError)
{
}

void
BiometricEvaluation::FileRecordStore::replace(
    const string &key,
    void * data,
    const uint64_t size)
    throw (ParameterError, StrategyError)
{
}

void
BiometricEvaluation::FileRecordStore::flush(
    const string &key)
    throw (ParameterError, StrategyError)
{
}

/******************************************************************************/
/* Private method implementations.                                            */
/******************************************************************************/
/*
 * Turn a Bitstore object name into a complete pathname/filename.
 */
string
BiometricEvaluation::FileRecordStore::canonicalName(
    const string &name)
{
	return (_directory + '/' + name);
}

bool
BiometricEvaluation::FileRecordStore::fileExists(const string &pathname)
{
	struct stat sb;

	if (stat(pathname.c_str(), &sb) == 0)
		return (true);
	else
		return (false);
}

/*
 * Read/Write the control file. Write always writes a new file, overwriting an
 * existing file.
 */
void
BiometricEvaluation::FileRecordStore::readControlFile()
    throw (StrategyError)
{
	/* Read the directory name and description from the control file */
	std::ifstream ifs(canonicalName(controlFileName).c_str());
	if (!ifs)
		throw StrategyError("Could not open control file");

	std::getline(ifs, _directory);
	if (ifs.eof())
		throw StrategyError("Premature EOF on control file");
		
	std::getline(ifs, _description);
	
	ifs.close();
}

void
BiometricEvaluation::FileRecordStore::writeControlFile()
    throw (StrategyError)
{
	std::ofstream ofs(canonicalName(controlFileName).c_str());
	if (!ofs)
		throw StrategyError("Could not create control file");

	/* Write the directory name and description into the control file */
	ofs << _directory << '\n';
	ofs << _description << '\n';
	ofs.close();
}

/*
 * Get the size of an object managed by this class, a record.
 */
uint64_t
BiometricEvaluation::FileRecordStore::getObjSize(const string &name)
    throw (ObjectDoesNotExist, StrategyError)
{
	string pathname = canonicalName(name);
	if (!fileExists(pathname))
		throw ObjectDoesNotExist();

	struct stat sb;
	if (stat(pathname.c_str(), &sb) != 0)
		throw StrategyError("Getting stats on file");
	return ((uint64_t)sb.st_size);

}
