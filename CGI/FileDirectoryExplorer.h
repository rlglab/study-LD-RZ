#ifndef FILEDIRECTORYEXPLOER_H
#define FILEDIRECTORYEXPLOER_H

#include "SpinLock.h"
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

/*
	This class is for traverse each file form root directory.
	When you get a new file, it will lock it.
	Currently for Train mode using.
*/

class SgfInformation {
public:
	int m_iFileLine;
	string m_sFilename;
	string m_sSgfString;

	SgfInformation() {
		m_iFileLine = 1;
		m_sFilename = m_sSgfString = "";
	}
	string getFullyName() { return m_sFilename+" "+ToString(m_iFileLine); }
	string getDetail() { return getFullyName()+"\n"+m_sSgfString; }
};

class FileDirectoryExplorer {
private:
	bool m_bIsEndSgf;
	bool m_bHasSetDirectory;
	uint m_nSgf;
	uint m_nFile;
	SpinLock m_lock;
	SpinLock m_sgfLock;
	fstream m_fSgfFile;
	string m_sFileName;
	int m_iFileLine;
	boost::filesystem::recursive_directory_iterator m_dirIterator;
	vector<string> m_files;

public:
	FileDirectoryExplorer() { reset(); }

	inline void reset()
	{
		m_bIsEndSgf = false;
		m_bHasSetDirectory = false;
		m_nSgf = 0;
		m_nFile = 0;
		m_fSgfFile.close();
		m_iFileLine = 1;
	}
	inline uint getSgfCount() { return m_nSgf; }
	inline uint getfileCount() { return m_nFile; }

	void setRootDirectory( string sDirectory ) {
		m_bHasSetDirectory = true;
		m_dirIterator = boost::filesystem::recursive_directory_iterator(sDirectory);				

		m_files.clear();
		while( m_dirIterator!=boost::filesystem::recursive_directory_iterator() ) {
			if( is_directory(m_dirIterator->status()) ) { ++m_dirIterator; continue; }

			m_sFileName = m_dirIterator->path().string();			
			m_files.push_back(m_sFileName);
			++m_dirIterator;
		}

		sort(m_files.begin(), m_files.end());
		m_dirIterator = boost::filesystem::recursive_directory_iterator(sDirectory);
	}

	SgfInformation getNextSgfInformation( bool bDisplayProgress=false )
	{
		SgfInformation sgfInformation;
		if (!m_bHasSetDirectory) { return sgfInformation; }

		sgfLock();
		string sSgfString = "";
		while( (sSgfString=getSgfString())=="" ) {
			string sFileName = getNextFile();
			if (sFileName == "") { break; }
			//if (sFileName.find(".sgf") == string::npos) { continue; }
			m_fSgfFile.close();
			m_fSgfFile.open(sFileName.c_str(),ios::in);
			m_iFileLine = 0;
		}
		m_iFileLine++;

		if( sSgfString!="" ) { ++m_nSgf; }
		else { m_bIsEndSgf = true; }
		if( bDisplayProgress ) { cerr << "total read " << m_nSgf << " sgf(s).           \r"; }

		sgfInformation.m_sFilename = m_sFileName;
		sgfInformation.m_iFileLine = m_iFileLine;
		sgfInformation.m_sSgfString = sSgfString;
		sgfUnLock();

		return sgfInformation;
	}

	string getNextSgfString()
	{
		return getNextSgfInformation().m_sSgfString;
	}

	string getNextFile()
	{
		if( !m_bHasSetDirectory ) { return ""; }
		
		fileLock();
		m_sFileName = "";
		while( m_dirIterator!=boost::filesystem::recursive_directory_iterator() ) {
			if( is_directory(m_dirIterator->status()) ) { ++m_dirIterator; continue; }

			//m_sFileName = m_dirIterator->path().string();
			m_sFileName = m_files[m_nFile];
			++m_dirIterator;
			++m_nFile;

			break;
		}
		fileUnLock();

		return m_sFileName;
	}

	bool isEndSgf() { return m_bIsEndSgf; }
	bool isEndFile() { return (m_dirIterator==boost::filesystem::recursive_directory_iterator()); }
	string getFileName() { return m_sFileName; }
	int getLineCount() { return m_iFileLine; }

private:
	inline void fileLock() { m_lock.lock(); }
	inline void sgfLock() { m_sgfLock.lock(); }
	inline void sgfUnLock() { m_sgfLock.unlock(); }
	inline void fileUnLock() { m_lock.unlock(); }

	inline string getSgfString()
	{
		string sSgfString = "";
		if( !m_fSgfFile.is_open() ) { return sSgfString; }

		char c;
		while( (c=m_fSgfFile.get())!=EOF ) {
			if( c=='(' ) { break; }
		}

		bool bSkip = false;
		int leftBrackets = 1;
		while( (c=m_fSgfFile.get())!=EOF ) {
			sSgfString += c;
			if( c=='[' ) { bSkip = true; }
			else if( c==']' ) { bSkip = false; }
			if( !bSkip ) {
				if( c=='(' ) { ++leftBrackets; }
				else if( c==')' ) { --leftBrackets; }
			}
			if( leftBrackets==0 ) { break; }
		}

		if( sSgfString=="" ) { return sSgfString; }
		else { return ("(" + sSgfString); }

		/*string sSgfString = "";
		if( !m_fSgfFile.is_open() ) { return sSgfString; }
		getline(m_fSgfFile,sSgfString);

		return sSgfString;*/
	}
};

#endif 
