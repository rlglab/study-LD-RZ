#ifndef LOG_SQLITE_H_
#define LOG_SQLITE_H_

#include "sqlite3.h"
#include <string>

class SQLite
{
private:
	sqlite3* m_SqliteDb;
	sqlite3_stmt* m_SqliteStmt;
	int m_iErrorNumber;
	char* m_ErrorMessage;

public:
	bool m_bIsOpen;

public:
	SQLite()
	{
		m_SqliteDb = NULL;
		m_SqliteStmt = NULL;
		m_iErrorNumber = SQLITE_OK;
		m_ErrorMessage = NULL;
		m_bIsOpen = false;
	}

	~SQLite()
	{
		Finalize();
		Close();
		m_bIsOpen = false;
	}

public:
	int Open(const std::string dbname)
	{
		m_iErrorNumber = sqlite3_open_v2(dbname.c_str(), &m_SqliteDb, SQLITE_OPEN_READONLY, NULL);
		if(m_iErrorNumber == SQLITE_OK) m_bIsOpen = true;
		return m_iErrorNumber;
	}

	void Close()
	{
		if (m_SqliteDb != NULL)
		{
			sqlite3_close(m_SqliteDb);
			m_bIsOpen = false;
			m_SqliteDb = NULL;
		}
	}

	void Finalize()
	{
		if (m_SqliteStmt != NULL)
		{
			sqlite3_finalize(m_SqliteStmt);
			m_SqliteStmt = NULL;
		}
	}

	int Exec(const std::string &query)
	{
		if (m_SqliteDb != NULL)
		{
			m_iErrorNumber = sqlite3_exec(m_SqliteDb, query.c_str(), 0, 0, &m_ErrorMessage);
		}
		return m_iErrorNumber;
	}

	int Prepare(const std::string &query)
	{
		if (m_SqliteDb != NULL)
		{
			const char* tail = NULL;
			m_iErrorNumber = sqlite3_prepare(m_SqliteDb, query.c_str(), -1, &m_SqliteStmt, &tail);
		}
		return m_iErrorNumber;
	}

public:
	int Step()
	{
		return sqlite3_step(m_SqliteStmt);
	}

	long ColumnInt32(int field)
	{
		return sqlite3_column_int(m_SqliteStmt, field);
	}

	/*
	unsigned long long ColumnUInt64(int field)
	{		
		return sqlite3_column_int64(m_SqliteStmt, field);
	}
	*/

	long long ColumnInt64(int field)
	{
		return sqlite3_column_int64(m_SqliteStmt, field);
	}

	double ColumnDouble(int field)
	{
		return sqlite3_column_double(m_SqliteStmt, field);
	}

	const char* ColumnText(int field)
	{
		return (const char*)sqlite3_column_text(m_SqliteStmt, field);
	}

	const unsigned char* ColumnBlob(int field)
	{
		return (const unsigned char*)sqlite3_column_blob(m_SqliteStmt, field);
	}
};

#endif  // LOG_SQLITE_H_ 
