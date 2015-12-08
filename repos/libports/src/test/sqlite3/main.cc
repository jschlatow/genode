#include <base/printf.h>
#include <sqlite3.h>
#include <unistd.h>

int main() 
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_open_v2("test.sqlite", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, "unix-none");
	if( rc ){
		PERR("Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return(1);
	}
	rc = sqlite3_exec(db, "CREATE TABLE Testing(Id INTEGER);", 0, 0, &zErrMsg);
	if( rc!=SQLITE_OK ){
		PERR("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db);
		return(1);
	}
	else {
		PDBG("Table created");
	}

	rc = sqlite3_exec(db, "INSERT INTO Testing VALUES (3);", 0, 0, &zErrMsg);
	if( rc!=SQLITE_OK ){
		PERR("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else {
		PDBG("Row inserted\n");
	}

	/* FIXME The last db modification is not commited to the database.
	 * We therefore use an arbitrary SQL statement to commit the previous one. */
	rc = sqlite3_exec(db, "INSERT INTO Testing VALUES (3);", 0, 0, &zErrMsg);

	rc = sqlite3_close(db);
	if (rc!=SQLITE_OK){
		PERR("Cannot close database");
	}

	return 0;
}
