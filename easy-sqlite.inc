/*
	Author: (creator)
 	* ThePhenix AKA ThreeKingz

	Credits:
	* SA-MP team for sqlite functions
	
	Version: 2.0
	
	(c) Copyright 2015
 	* This file is provided as is (no warranties).

	native SL::Connect(database[]);
	native SL::DeleteRow(const table[], const column[], columnID, DB:database = DB:1);
	native SL::DeleteRowEx(const table[], const column[], columnID[], DB:database = DB:1);
	native SL::GetIntEntry(const table[], const field[], const column[], columnID, DB:database = DB:1);
	native Float:SL::GetFloatEntry(const table[], const field[], const column[], columnID, DB:database = DB:1);
	native SL::GetStringEntry(const table[], const field[], const column[], columnID, dest[], len = sizeof(dest), DB:database = DB:1);
	native SL::GetStringEntryEx(const table[], const field[], const column[], const scolumn[], dest[], len = sizeof(dest), DB:database = DB:1)
	native SL::GetIntEntryEx(const table[], const field[], const column[], scolumn[], DB:database = DB:1);
	native Float:SL::GetFloatEntryEx(const table[], const field[], const column[], scolumn[], DB:database = DB:1);
	native SL::CreateTable(const tablename[], DB:database = DB:1);
	native SL::AddTableEntry(handle, const field[], SL::datatypes: type = SL_TYPE_INT, maxlength = 11, bool:auto_increment = false, bool:setprimary = false);
	native SL::OpenTable_Read(const table[], const column[], columnID, DB:database = DB:1);
	native SL::OpenTable_ReadEx(const table[], const column[], columnID[], DB:database = DB:1);
	native SL::ReadInt(handle, const field[], &dest);
	native SL::ReadFloat(handle, const field[], &Float:dest);
	native SL::ReadString(handle, const field[], dest[], len = sizeof(dest));
	native SL::OpenTable_Update(const table[], const column[], columnID, DB:database = DB:1);
	native SL::OpenTable_UpdateEx(const table[], const column[], columnID[], DB:database = DB:1);
	native SL::Open(SL::qtypes:type, const table[], const column[] = "", columnID = -1, DB:database = DB:1);
	native SL::ToggleAutoIncrement(handle, bool:toggle);
	native SL::OpenEx(SL::qtypes:type, const table[], const column[] = "", columnID[] = "", DB:database = DB:1)
	native SL::UpdateIntEntry(handle, const field[], value);
	native SL::UpdateFloatEntry(handle, const field[], Float:value);
	native SL::UpdateStringEntry(handle, const field[], const value[], bool:use_real_escape = true);
	native SL::Close(handle);
	native SL::SetIntEntry(const table[], const field[], value, const column[], columnID, DB:database = DB:1);
	native SL::SetIntEntryEx(const table[], const field[], value, const column[], columnID[], DB:database = DB:1);
	native SL::SetFloatEntry(const table[], const field[], Float:value, const column[], columnID, DB:database = DB:1);
	native SL::SetFloatEntryEx(const table[], const field[], Float:value, const column[], columnID[], DB:database = DB:1);
	native SL::SetStringEntry(const table[], const field[], const value[], const column[], columnID, bool:use_real_escape = true, DB:database = DB:1);
	native SL::SetStringEntryEx(const table[], const field[], const value[], const column[], columnID[], bool:use_real_escape = true, DB:database = DB:1);
	native SL::OpenTable_Insert(const table[], DB:database = DB:1);
	native SL::InsertIntEntry(handle, const field[], value);
	native SL::InsertFloatEntry(handle, const field[], Float:value);
	native SL::InsertStringEntry(handle, const field[], const value[], bool:use_real_escape = true);
	native SL::ExistsTable(const tablename[], DB:database = DB:1);
	native SL::CountRows(const tablename[], DB:database = DB:1);
	native SL::CountTables(DB:database = DB:1);
	native SL::DropTable(const tablename[], DB:database = DB:1);
	
	v2.0
	
	native SL::Open(SL::qtypes:type, const table[], const column[] = "", columnID = -1, DB:database = DB:1);
	native SL::ToggleAutoIncrement(handle, bool:toggle);
	native SL::OpenEx(SL::qtypes:type, const table[], const column[] = "", columnID[] = "", DB:database = DB:1)
	
	
	The following functions replace Insert and Update functions:
	
	SL::WriteFloat(handle, const field[], Float:value);
	SL::WriteInt(handle, const field[], Float:value);
	SL::WriteString(handle, const field[], const value[]);
	
	Added:
	
	SL::Begin(DB:database = DB:1)
	SL::Commit(DB:database = DB:1)
	
	This include is mainly for people without experience with sqlite as it simplifies its usage.
*/

#if defined _sqlitei_included//Avoid including it again!
	#endinput
#endif
#define _sqlitei_included

#if !defined _samp_included
	#tryinclude <a_samp>
	#if !defined _samp_included
		#error  "<a_samp.inc> was not found. Make sure the include is correctly placed on your includes folder."
	#endif
#endif


//By Y_Less
//native strcpy(dest[], src[], len = sizeof(dest));
#if !defined strcpy
#define strcpy(%0,%1) \
	strcat((%0[0] = '\0', %0), %1)
#endif
#if !defined SL_MAX_HANDLES
	#define SL_MAX_HANDLES              (3)//Maximun concurrent handles running.
#endif

#define SL_MAX_TABLE_NAME 					(64)
#define SL_MAX_QUERY_LENGTH				(4096)
#define SL_INVALID_HANDLE					(-1)
#define SL_FORM_LENGTH						(128)
#define SL:: 	SL_TK
#define SL_Warning(%0)      		(printf("[SQLITE] - WARNING: " %0))
#define SL_Error(%0)        		(printf("[SQLITE] - ERROR: " %0))
#define SL_Notice(%0)				(printf("[SQLITE] - NOTICE: " %0))

enum SL::datatypes {SL_TYPE_INT, SL_TYPE_VCHAR, SL_TYPE_FLOAT} 
enum SL::qtypes {SL::UPDATE, SL::CREATE, SL::INSERT, SL::MINSERT, SL_TYPE_DELETE, SL::READ}

static stock
	bool:SL::UsedHandle[SL_MAX_HANDLES],
	DB:SL::database[SL_MAX_HANDLES],
	SL::upd_table[SL_MAX_HANDLES][SL_MAX_TABLE_NAME],
	SL::upd_rowidentifier[SL_MAX_HANDLES][SL_FORM_LENGTH],
	SL::upd_query[SL_MAX_HANDLES][SL_MAX_QUERY_LENGTH],
	SL::upd_query_2[SL_MAX_HANDLES][SL_MAX_QUERY_LENGTH],
	SL::upd_increment_key[SL_MAX_HANDLES][SL_FORM_LENGTH],
	SL::upd_form[SL_FORM_LENGTH],
	SL::upd_datacount[SL_MAX_HANDLES],
	SL::qtypes:SL::upd_type[SL_MAX_HANDLES],
	SL::upd_pos[SL_MAX_HANDLES],
	DBResult:SL::ReadCache[SL_MAX_HANDLES],
	bool:SL::upd_useautoincrement[SL_MAX_HANDLES],
	SL::primarykey[SL_MAX_HANDLES][64],
	SL::isset_primarykey[SL_MAX_HANDLES]
	
;

static stock SL::GetFreeUpdatingSlot()
{
	new i = 0;
	while (i < sizeof (SL::UsedHandle) && SL::UsedHandle[i] == true)
	{
		i++;
	}
	if (i == sizeof (SL::UsedHandle)) return SL_INVALID_HANDLE;
	return i;
}
static stock SL::IsValidUpdatingSlot(handle)
{
	return SL::UsedHandle[handle];
}
stock DB:SL::Connect(database[])
{
	new DB:db;
	if((db = db_open(database)) == DB:0)
	{
		SL_Warning("Could not connect to database %s", database);
	
	}
	else
	{
		SL_Notice("Successfully connected to database %s!", database);
	}
	return db;
}
stock SL::DeleteRow(const table[], const column[], columnID, DB:database = DB:1)
{
	format(SL::upd_form, sizeof(SL::upd_form), "DELETE FROM `%s` WHERE `%s`='%d' ", table, column, columnID);
	db_free_result(db_query(DB:database, SL::upd_form));
	return 1;
}
stock SL::Open(SL::qtypes:type, const table[], const column[] = "", columnID = -1, DB:database = DB:1)
{
	new handle;
	switch(type)
	{
		case SL::CREATE:
		{
			handle = SL::CreateTable(table, DB:database);
		}
		case SL::READ:
		{
			handle = SL::OpenTable_Read(table, column, columnID, DB:database);
		}
		case SL::INSERT:
		{
			handle = SL::OpenTable_Insert(table, false, DB:database);
		}
		case SL::UPDATE:
		{
			handle = SL::OpenTable_Update(table, column, columnID, DB:database);
		}
	}
	return handle;
}
stock SL::Begin(DB:database = DB:1)
{
	db_free_result(db_query(DB:database, "BEGIN"));
	return 1;
}
stock SL::Commit(DB:database = DB:1)
{
	db_free_result(db_query(DB:database, "COMMIT"));
	return 1;
}
stock SL::OpenEx(SL::qtypes:type, const table[], const column[] = "", columnID[] = "", DB:database = DB:1)
{
	new handle;
	switch(type)
	{
		case SL::CREATE:
		{
			handle = SL::CreateTable(table, DB:database);
		}
		case SL::READ:
		{
			handle = SL::OpenTable_ReadEx(table, column, columnID, DB:database);
		}
		case SL::INSERT:
		{
			handle = SL::OpenTable_Insert(table, false, DB:database);
		}
		case SL::UPDATE:
		{
			handle = SL::OpenTable_UpdateEx(table, column, columnID, DB:database);
		}
	}
	return handle;
}

stock SL::ToggleAutoIncrement(handle, bool:toggle)
{
	if(!SL::IsValidUpdatingSlot(handle)) return 0;
	if(SL::upd_type[handle] != SL::INSERT) return 0;
	SL::upd_useautoincrement[handle] = toggle;
	return 1;
}
stock SL::DeleteRowEx(const table[], const column[], columnID[], DB:database = DB:1)
{
	format(SL::upd_form, sizeof(SL::upd_form), "DELETE FROM `%s` WHERE `%s`='sd' ", table, column, columnID);
	db_free_result(db_query(DB:database, SL::upd_form));
	return 1;
}
stock SL::GetIntEntry(const table[], const field[], const column[], columnID, DB:database = DB:1)
{
	new DBResult:result, int;
	format(SL::upd_form, sizeof(SL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%d' ", field, table, column, columnID);
	result = db_free_result(db_query(DB:database, SL::upd_form));
	if (result) 
	{
		if (db_num_rows(result))
		{
			int = db_get_field_assoc_int(result, field);
		}
		else
		{
			int = -1;
		}
		db_free_result(result);
	}
	return int;
}
stock Float:SL::GetFloatEntry(const table[], const field[], const column[], columnID, DB:database = DB:1)
{
	new 
		DBResult:result, 
		Float:int
	;
	format(SL::upd_form, sizeof(SL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%d' ", field, table, column, columnID);
	
	
	result = db_free_result(db_query(DB:database, SL::upd_form));
	
	
	if (result) 
	{
		if (db_num_rows())
		{
			int = db_get_field_assoc_float(result, field);
		}
		else
		{
			int = 0.0;
		}
		db_free_result(result);
	}
	return int;
}	
stock SL::GetStringEntry(const table[], const field[], const column[], columnID, dest[], len = sizeof(dest), DB:database = DB:1)
{
	new DBResult:result;
	format(SL::upd_form, sizeof(SL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%d' ", field, table, column, columnID);
	result = db_free_result(db_query(DB:database, SL::upd_form));
	if (result) 
	{
		if (db_num_rows(result))
		{
			db_get_field_assoc(result, field, SL::upd_form, sizeof(SL::upd_form));
			strcpy(dest, SL::upd_form, len);
			db_free_result(result);
		}
		else
		{
			db_free_result(result);
			return 0;
		}
	}
	return 1;
}	
stock SL::GetStringEntryEx(const table[], const field[], const column[], const scolumn[], dest[], len = sizeof(dest), DB:database = DB:1)
{
	new DBResult:result;
	format( SL::upd_form, sizeof(SL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%q' ", field, table, column, scolumn);
	result = db_free_result(db_query(DB:database, SL::upd_form));
	if (result) 
	{
		if (db_num_rows(result))
		{
			db_get_field_assoc(result, field, SL::upd_form, sizeof(SL::upd_form));
			strcpy(dest, SL::upd_form, len);
			db_free_result(result);
		}
		else
		{
			db_free_result(result);
			return 0;
		}
	}
	return 1;
}	

stock SL::GetIntEntryEx(const table[], const field[], const column[], scolumn[], DB:database = DB:1)
{
	new DBResult:result, int;
	format( SL::upd_form, sizeof(SL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%q' ", field, table, column, scolumn);
	result = db_free_result(db_query(DB:database, SL::upd_form));
	if (result) 
	{
		if (db_num_rows(result))
		{
			int = db_get_field_assoc_int(result, field);
		}
		else
		{
			int = -1;
		}
		db_free_result(result);
	}
	return int;
}
stock Float:SL::GetFloatEntryEx(const table[], const field[], const column[], scolumn[], DB:database = DB:1)
{
	new DBResult:result, Float:int;
	format( SL::upd_form, sizeof(SL::upd_form), "SELECT `%s` FROM `%s` WHERE `%s`='%q' ", field, table, column, scolumn);
	result = db_free_result(db_query(DB:database, SL::upd_form));
	if (result) 
	{
		if (db_num_rows(result))
		{
			int = db_get_field_assoc_float(result, field);
		}
		else
		{
			int = -1.0;
		}
		db_free_result(result);
	}
	return int;
}
stock SL::CreateTable(const tablename[], DB:database = DB:1)
{
	if(DB:database == DB:0)
	{
		SL_Error("(SL::CreateTable) No active connection.");
		return SL_INVALID_HANDLE;
	} 
	if(strlen(tablename) > SL_MAX_TABLE_NAME)
	{
		SL_Error("(SL::CreateTable) Invalid table length.");
		return SL_INVALID_HANDLE;
	} 
	new 
		i = SL::GetFreeUpdatingSlot()
	;
	if(i == SL_INVALID_HANDLE) 
	{
		return SL_INVALID_HANDLE;
	}
	SL::upd_type[i] = SL::CREATE;
	strcpy(SL::upd_table[i], tablename);
	SL::database[i] = DB:database;
	format(SL::upd_query[i], SL_MAX_QUERY_LENGTH, "CREATE TABLE %s (", SL::upd_table[i]);
	SL::upd_datacount[i] = 0;
	SL::isset_primarykey[i] = 0;
	SL::UsedHandle[i] = true;
	return i;
}
stock SL::AddTableEntry(handle, const field[], SL::datatypes: type = SL_TYPE_INT, maxlength = 11, bool:auto_increment = false, bool:setprimary = false)
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::AddTableEntry) Invalid handle. Make sure you used (SL::OpenTable) first.");
		return 0;
	}
	if(SL::upd_type[handle] != SL::CREATE) return 0;
	if(SL::isset_primarykey[handle] && (setprimary == true || auto_increment == true))
	{
		SL_Error("(SL::AddTableEntry) A primary field was created already. There can only be one primary field! [Key %s]", field);
		return 0;
	}
	if(auto_increment == true && type == SL_TYPE_VCHAR)
	{
		SL_Error("(SL::AddTableEntry) You can't set auto increment if the type is varchar! [Key %s]", field);
		return 0;
	}
	switch(type)
	{
		case SL_TYPE_INT:
		{
			if(auto_increment == true && setprimary == true)
			{
				format(SL::upd_form, sizeof(SL::upd_form), "%s INTEGER PRIMARY KEY AUTOINCREMENT,", field);
				strcat(SL::upd_query[handle], SL::upd_form);
				SL::upd_datacount[handle]++;
				SL::isset_primarykey[handle] = 1;
			}
			else if(auto_increment == false && setprimary == true)
			{
				format(SL::upd_form, sizeof(SL::upd_form), "%s INTEGER PRIMARY KEY,", field);
				strcat(SL::upd_query[handle], SL::upd_form);
				SL::upd_datacount[handle]++;
				SL::isset_primarykey[handle] = 1;
			}
			else if(auto_increment == true && setprimary == false)
			{
				format(SL::upd_form, sizeof(SL::upd_form), "%s INTEGER PRIMARY KEY AUTOINCREMENT,", field);
				strcat(SL::upd_query[handle], SL::upd_form);
				SL::upd_datacount[handle]++;
				SL::isset_primarykey[handle] = 1;
			}
			else
			{
				format(SL::upd_form, sizeof(SL::upd_form), "%s int(%d),", field, maxlength);
				strcat(SL::upd_query[handle], SL::upd_form);
				SL::upd_datacount[handle]++;
			}
		}
		case SL_TYPE_VCHAR:
		{
			if(auto_increment == true) return 0;
			if(setprimary == true)
			{
				format(SL::upd_form, sizeof(SL::upd_form), "%s varchar(%d) PRIMARY KEY,", field, maxlength);
				strcat(SL::upd_query[handle], SL::upd_form);
				SL::upd_datacount[handle]++;
				SL::isset_primarykey[handle] = 1;
				strcpy(SL::primarykey[handle], field);
			}
			else
			{
				format(SL::upd_form, sizeof(SL::upd_form), "%s varchar(%d),", field, maxlength);
				strcat(SL::upd_query[handle], SL::upd_form);
				SL::upd_datacount[handle]++;

			}
		}
		case SL_TYPE_FLOAT:
		{
			if(auto_increment == true) return 0;
			if(setprimary == true)
			{
				format(SL::upd_form, sizeof(SL::upd_form), "%s float(%d) PRIMARY KEY,", field, maxlength);
				strcat(SL::upd_query[handle], SL::upd_form);
				SL::upd_datacount[handle]++;
				SL::isset_primarykey[handle] = 1;
				strcpy(SL::primarykey[handle], field);
			}
			else
			{
				format(SL::upd_form, sizeof(SL::upd_form), "%s float(%d),", field, maxlength);
				strcat(SL::upd_query[handle], SL::upd_form);
				SL::upd_datacount[handle]++;
			}
		}
	}
	return 1;
}
stock SL::OpenTable_Read(const table[], const column[], columnID = -1, DB:database = DB:1)
{
	if(DB:database == DB:0)
	{
		SL_Error("(SL::OpenTable) No active connection.");
		return SL_INVALID_HANDLE;
	} 
	if(strlen(table) > SL_MAX_TABLE_NAME)
	{
		SL_Error("(SL::OpenTable) Invalid table length.");
		return SL_INVALID_HANDLE;
	} 
	new 
		i = SL::GetFreeUpdatingSlot()
	;
	if(i == SL_INVALID_HANDLE) 
	{
		return SL_INVALID_HANDLE;
	}
	SL::upd_type[i] = SL::READ;
	SL::database[i] = DB:database;
	if(column[0] == '\0' && columnID -1)
	{
		format(SL::upd_form, sizeof(SL::upd_form), "SELECT * FROM `%s`", table);
		SL::ReadCache[i] = db_query(DB:database, SL::upd_form);
	}
	else
	{
		format(SL::upd_form, sizeof(SL::upd_form), "SELECT * FROM `%s` WHERE `%s` = '%d' ", table, column, columnID);
		SL::ReadCache[i] = db_query(DB:database, SL::upd_form);
	}
	SL::ReadCache[i] = db_query(DB:database, SL::upd_form);
	SL::upd_datacount[i] = 0;
	if((SL::ReadCache[i]))
	{
		SL::UsedHandle[i] = true;
		return i;
	}
	return SL_INVALID_HANDLE;
}
stock SL::OpenTable_ReadEx(const table[], const column[], columnID[], DB:database = DB:1)
{
	if(DB:database == DB:0)
	{
		SL_Error("(SL::OpenTable_ReadEx) No active connection.");
		return SL_INVALID_HANDLE;
	} 
	if(strlen(table) > SL_MAX_TABLE_NAME)
	{
		SL_Error("(SL::OpenTable_ReadEx) Invalid table length.");
		return SL_INVALID_HANDLE;
	} 
	if(column[0] == '\0')
	{
		SL_Error("(SL::OpenTable_ReadEx) No format has been entered. Format example: (pID)");
		return SL_INVALID_HANDLE;
	}
	new 
		i = SL::GetFreeUpdatingSlot()
	;
	if(i == SL_INVALID_HANDLE) 
	{
		return SL_INVALID_HANDLE;
	}
	SL::upd_type[i] = SL::READ;
	SL::database[i] = DB:database;
	format(SL::upd_form, sizeof(SL::upd_form), "SELECT * FROM `%s` WHERE `%s` = '%s' ", table, column, columnID);
	SL::ReadCache[i] = db_query(DB:database, SL::upd_form);
	SL::upd_datacount[i] = 0;
	if((SL::ReadCache[i]))
	{
		SL::UsedHandle[i] = true;
		return i;
	}
	return SL_INVALID_HANDLE;
}
stock SL::ReadInt(handle, const field[], &dest)
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::ReadInt) Invalid handle. Make sure you opened the table first.");
		return 0;
	}
	if(SL::upd_type[handle] != SL::READ) return 0;
	dest = db_get_field_assoc_int(SL::ReadCache[handle], field);
	SL::upd_datacount[handle]++;
	return 1;
}
stock SL::ReadFloat(handle, const field[], &Float:dest)
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::ReadInt) Invalid handle. Make sure you opened the table first.");
		return 0;
	}
	if(SL::upd_type[handle] != SL::READ) return 0;
	dest = db_get_field_assoc_float(SL::ReadCache[handle], field);
	SL::upd_datacount[handle]++;
	return 1;
}
stock SL::ReadString(handle, const field[], dest[], len = sizeof(dest))
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::ReadInt) Invalid handle. Make sure you opened the table first.");
		return 0;
	}
	if(SL::upd_type[handle] != SL::READ) return 0;
	db_get_field_assoc(SL::ReadCache[handle], field, dest, len);
	SL::upd_datacount[handle]++;
	return 1;
}

stock SL::OpenTable_Update(const table[], const column[], columnID, DB:database = DB:1)
{
	if(DB:database == DB:0)
	{
		SL_Error("(SL::OpenTable) No active connection.");
		return SL_INVALID_HANDLE;
	} 
	if(strlen(table) > SL_MAX_TABLE_NAME)
	{
		SL_Error("(SL::OpenTable) Invalid table length.");
		return SL_INVALID_HANDLE;
	} 
	if(column[0] == '\0')
	{
		SL_Error("(SL::OpenTable) No format has been entered. Format example: (pID)");
		return SL_INVALID_HANDLE;
	}
	new 
		i = SL::GetFreeUpdatingSlot()
	;
	if(i == SL_INVALID_HANDLE) 
	{
		return SL_INVALID_HANDLE;
	}
	SL::upd_type[i] = SL::UPDATE;
	strcpy(SL::upd_table[i], table);
	SL::database[i] = DB:database;
	format(SL::upd_query[i], SL_MAX_QUERY_LENGTH, "UPDATE `%s` SET ", SL::upd_table[i]);
	format(SL::upd_rowidentifier[i], SL_FORM_LENGTH, " WHERE `%s` = '%d' ", column, columnID);
	SL::upd_datacount[i] = 0;
	SL::UsedHandle[i] = true;
	return i;
}
stock SL::OpenTable_UpdateEx(const table[], const column[], columnID[], DB:database = DB:1)
{
	if(DB:database == DB:0)
	{
		SL_Error("(SL::OpenTable_UpdateEx) No active connection.");
		return SL_INVALID_HANDLE;
	} 
	if(strlen(table) > SL_MAX_TABLE_NAME)
	{
		SL_Error("(SL::OpenTable_UpdateEx) Invalid table length.");
		return SL_INVALID_HANDLE;
	} 
	if(column[0] == '\0')
	{
		SL_Error("(SL::OpenTable_UpdateEx) No format has been entered. Format example: (pID)");
		return SL_INVALID_HANDLE;
	}
	new 
		i = SL::GetFreeUpdatingSlot()
	;
	if(i == SL_INVALID_HANDLE) 
	{
		return SL_INVALID_HANDLE;
	}
	SL::upd_type[i] = SL::UPDATE;
	strcpy(SL::upd_table[i], table);
	SL::database[i] = DB:database;
	format(SL::upd_query[i], SL_MAX_QUERY_LENGTH, "UPDATE `%s` SET ", SL::upd_table[i]);
	format(SL::upd_rowidentifier[i], SL_FORM_LENGTH, " WHERE `%s` = '%s' ", column, columnID);
	SL::upd_datacount[i] = 0;
	SL::UsedHandle[i] = true;
	return i;
}
stock SL::UpdateIntEntry(handle, const field[], value)
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::UpdateIntEntry) Invalid handle. Make sure you used (SL::OpenTable) first.");
		return 0;
	}
	if(SL::upd_type[handle] != SL::UPDATE) return 0;
	format(SL::upd_form, sizeof(SL::upd_form), "`%s`='%i',", field, value);
	strcat(SL::upd_query[handle], SL::upd_form);
	SL::upd_datacount[handle]++;
	return 1;
}
stock SL::WriteInt(handle, const field[], value)
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::WriteInt) Invalid handle. Make sure you used (SL::Open) first.");
		return 0;
	}
	switch(SL::upd_type[handle])
	{
		case SL::UPDATE:
		{
			return SL::UpdateIntEntry(handle, field, value);
		}
		case SL::INSERT:
		{
			return SL::InsertIntEntry(handle, field, value);
		}
	}
	return 0;
	
}
stock SL::WriteFloat(handle, const field[], Float:value)
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::WriteFloat) Invalid handle. Make sure you used (SL::Open) first.");
		return 0;
	}
	switch(SL::upd_type[handle])
	{
		case SL::UPDATE:
		{
			return SL::UpdateFloatEntry(handle, field, value);
		}
		case SL::INSERT:
		{
			return SL::InsertFloatEntry(handle, field, value);
		}
	}
	return 0;
}
stock SL::WriteString(handle, const field[], const value[])
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::WriteFloat) Invalid handle. Make sure you used (SL::Open) first.");
		return 0;
	}
	switch(SL::upd_type[handle])
	{
		case SL::UPDATE:
		{
			return SL::UpdateStringEntry(handle, field, value);
		}
		case SL::INSERT:
		{
			return SL::InsertStringEntry(handle, field, value);
		}
	}
	return 0;
}
stock SL::UpdateFloatEntry(handle, const field[], Float:value)
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::UpdateFloatEntry) Invalid handle. Make sure you used (SL::OpenTable) first.");
		return 0;
	}
	if(SL::upd_type[handle] != SL::UPDATE) return 0;
	format(SL::upd_form, sizeof(SL::upd_form), "`%s`='%f',", field, value);
	strcat(SL::upd_query[handle], SL::upd_form);
	SL::upd_datacount[handle]++;
	return 1;
}
stock SL::UpdateStringEntry(handle, const field[], const value[], bool:use_real_escape = true)
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::UpdateStringEntry) Invalid handle. Make sure you used (SL::OpenTable) first.");
		return 0;
	}
	if(SL::upd_type[handle] != SL::UPDATE) return 0;
	if(use_real_escape == true)
	{
		format(SL::upd_form, sizeof(SL::upd_form), "`%s`='%q',", field, value);
	}
	else
	{
		format(SL::upd_form, sizeof(SL::upd_form), "`%s`='%s',", field, value);
	}
	strcat(SL::upd_query[handle], SL::upd_form);
	SL::upd_datacount[handle]++;
	return 1;
}
stock SL::Close(handle)
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::Close) Invalid handle. Make sure you opened a handle first.");
		return 0;
	}
	if(!SL::upd_datacount[handle])
	{
		SL_Error("(SL::Close) No data has been updated/inserted/deleted.");
		return 0;
	}
	if(SL::upd_type[handle] == SL::UPDATE)
	{
		//Remove last comma 
		new pos = strlen(SL::upd_query[handle])-1;
		SL::upd_query[handle][pos] = ' ';
		strcat(SL::upd_query[handle], SL::upd_rowidentifier[handle]);

		db_free_result(db_query(SL::database[handle], SL::upd_query[handle]));
		SL::upd_table[handle][0] = '\0';
		SL::database[handle] = DB:0;
		SL::upd_query[handle][0] = '\0';
		SL::upd_rowidentifier[handle][0] = '\0';
		SL::upd_datacount[handle] = 0;
		SL::UsedHandle[handle] = false;
	}
	else if(SL::upd_type[handle] == SL::CREATE)
	{
		new pos = strlen(SL::upd_query[handle])-1;
		SL::upd_query[handle][pos] = ' ';
		strcat(SL::upd_query[handle], ");");
		db_free_result(db_query(SL::database[handle], SL::upd_query[handle]));
		SL::upd_table[handle][0] = '\0';
		SL::database[handle] = DB:0;
		SL::primarykey[handle][0] = '\0';
		SL::isset_primarykey[handle] = 0;
		SL::upd_query[handle][0] = '\0';
		SL::upd_rowidentifier[handle][0] = '\0';
		SL::upd_datacount[handle] = 0;
		SL::UsedHandle[handle] = false;
		
	}
	else if(SL::upd_type[handle] == SL::INSERT)
	{
		new pos = strlen(SL::upd_query[handle])-1;
		SL::upd_query[handle][pos] = ')';
		pos = strlen(SL::upd_query_2[handle])-1;
		SL::upd_query_2[handle][pos] = ' ';
		strcat(SL::upd_query_2[handle], ");");
		strcat(SL::upd_query[handle], SL::upd_query_2[handle]);
		SL::upd_query_2[handle][0] = '\0';
		SL::upd_rowidentifier[handle][0] = '\0';
		SL::upd_datacount[handle] = 0;
		new id;
		if(SL::upd_useautoincrement[handle] == true)
		{
			db_free_result(db_query(SL::database[handle], SL::upd_query[handle]));
			new DBResult:result;
			format(SL::upd_form, sizeof(SL::upd_form), "SELECT last_insert_rowid() FROM %s LIMIT 1", SL::upd_table[handle]);
			result = db_query(SL::database[handle], SL::upd_form);
			if(result)
			{
				id = db_get_field_int(result);
				db_free_result(result);
				SL::database[handle] = DB:0;
				SL::upd_query[handle][0] = '\0';
				SL::UsedHandle[handle] = false;
				SL::upd_table[handle][0] = '\0';
				return id;
			}
		}
		else
		{
			db_free_result(db_query(SL::database[handle], SL::upd_query[handle]));
			SL::UsedHandle[handle] = false;
		}
		SL::database[handle] = DB:0;
		SL::upd_query[handle][0] = '\0';
		SL::upd_table[handle][0] = '\0';
		SL::UsedHandle[handle] = false;
		
	}
	else if(SL::upd_type[handle] == SL::READ)
	{
		if((SL::ReadCache[handle]))
		{
			db_free_result(SL::ReadCache[handle]);
		}
		SL::UsedHandle[handle] = false;
	}
	return -1;
}
stock SL::SetIntEntry(const table[], const field[], value, const column[], columnID, DB:database = DB:1)
{
	if(!DB:database)
	{
		SL_Error("(SL::SetIntEntry) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	format(SL::upd_form, sizeof(SL::upd_form), "UPDATE `%s` SET `%s`='%d' WHERE `%s`='%d' ", table, field, value, column, columnID);
	db_free_result(db_query(DB:database, SL::upd_form));
	return 1;
}
stock SL::SetIntEntryEx(const table[], const field[], value, const column[], columnID[], DB:database = DB:1)
{
	if(!DB:database)
	{
		SL_Error("(SL::SetIntEntryEx) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	format( SL::upd_form, sizeof(SL::upd_form), "UPDATE `%s` SET `%s`='%d' WHERE `%s`='%q' ", table, field, value, column, columnID);
	db_free_result(db_query(DB:database, SL::upd_form));
	return 1;
}
stock SL::SetStringEntry(const table[], const field[], const value[], const column[], columnID, bool:use_real_escape = true, DB:database = DB:1)
{
	if(!DB:database)
	{
		SL_Error("(SL::SetStringEntry) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	new SL::str[256];
	if(use_real_escape == true)
	{
		//Not using mysql_real_escape_string as the %q specifier is faster.
		format(SL::str, sizeof(SL::str), "UPDATE `%s` SET `%s`='%q' WHERE `%s`='%d' ", table, field, value, column, columnID);
	}
	else
	{
		format(SL::str, sizeof(SL::str), "UPDATE `%s` SET `%s`='%s' WHERE `%s`='%d' ", table, field, value, column, columnID);
	}
	db_free_result(db_query(DB:database, SL::str));
	return 1;
}
stock SL::SetStringEntryEx(const table[], const field[], const value[], const column[], columnID[], bool:use_real_escape = true, DB:database = DB:1)
{
	if(!DB:database)
	{
		SL_Error("(SL::SetStringEntryEx) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	if(use_real_escape == true)
	{
		//Not using mysql_real_escape_string as the %q specifier is faster.
		format(SL::upd_form, sizeof(SL::upd_form), "UPDATE `%s` SET `%s`='%q' WHERE `%s`='%q' ", table, field, value, column, columnID);
	}
	else
	{
		format(SL::upd_form, sizeof(SL::upd_form), "UPDATE `%s` SET `%s`='%s' WHERE `%s`='%q' ", table, field, value, column, columnID);
	}
	db_free_result(db_query(DB:database, SL::upd_form));
	return 1;
}
stock SL::SetFloatEntry(const table[], const field[], Float:value, const column[], columnID, DB:database = DB:1)
{
	if(!DB:database)
	{
		SL_Error("(SL::SetFloatEntry) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	format(SL::upd_form, sizeof(SL::upd_form), "UPDATE `%s` SET `%s`='%f' WHERE `%s`='%d' ", table, field, value, column, columnID);
	db_free_result(db_query(DB:database, SL::upd_form));
	return 1;
}
stock SL::SetFloatEntryEx(const table[], const field[], Float:value, const column[], columnID[], DB:database = DB:1)
{
	if(!DB:database)
	{
		SL_Error("(SL::SetFloatEntryEx) No active connection.");
		return 0;
	} 
	if(!table[0]) return 0;
	if(!field[0]) return 0;
	format(SL::upd_form, sizeof(SL::upd_form), "UPDATE `%s` SET `%s`='%f' WHERE `%s`='%q' ", table, field, value, column, columnID);
	db_free_result(db_query(DB:database, SL::upd_form));
	return 1;
}

stock SL::OpenTable_Insert(const table[], bool:use_autoincrement = false, DB:database = DB:1)
{
	if(!DB:database)
	{
		SL_Error("(SL::OpenTable_Insert) No active connection.");
		return SL_INVALID_HANDLE;
	} 
	if(strlen(table) > SL_MAX_TABLE_NAME)
	{
		SL_Error("(SL::OpenTable_Insert) Invalid table length.");
		return SL_INVALID_HANDLE;
	} 
	new 
		i = SL::GetFreeUpdatingSlot()
	;
	if(i == SL_INVALID_HANDLE) 
	{
		return SL_INVALID_HANDLE;
	}
	SL::upd_type[i] = SL::INSERT;
	strcpy(SL::upd_table[i], table);
	SL::database[i] = DB:database;
	format(SL::upd_query[i], SL_MAX_QUERY_LENGTH, "INSERT INTO `%s` (", SL::upd_table[i]);
	strcpy(SL::upd_query_2[i], " VALUES (");
	SL::upd_datacount[i] = 0;
	if(use_autoincrement == true)
	{
		SL::upd_useautoincrement[i] = true;
	}
	else
	{
		SL::upd_useautoincrement[i] = false;
	}
	SL::UsedHandle[i] = true;
	return i;
}
stock SL::InsertIntEntry(handle, const field[], value)
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::UpdateIntEntry) Invalid handle. Make sure you used (SL::OpenTable) first.");
		return 0;
	}
	if(SL::upd_type[handle] != SL::INSERT) return 0;
	format(SL::upd_form, sizeof(SL::upd_form), "`%s`,", field);
	strcat(SL::upd_query[handle], SL::upd_form);
	format(SL::upd_form, sizeof(SL::upd_form), "'%i',", value);
	strcat(SL::upd_query_2[handle], SL::upd_form);
	SL::upd_datacount[handle]++;
	return 1;
}
stock SL::InsertFloatEntry(handle, const field[], Float:value)
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::UpdateIntEntry) Invalid handle. Make sure you used (SL::OpenTable) first.");
		return 0;
	}
	if(SL::upd_type[handle] != SL::INSERT) return 0;
	format(SL::upd_form, sizeof(SL::upd_form), "`%s`,", field);
	strcat(SL::upd_query[handle], SL::upd_form);
	format(SL::upd_form, sizeof(SL::upd_form), "'%f',", value);
	strcat(SL::upd_query_2[handle], SL::upd_form);
	SL::upd_datacount[handle]++;
	return 1;
}
stock SL::InsertStringEntry(handle, const field[], const value[], bool:use_real_escape = true)
{
	if(!SL::IsValidUpdatingSlot(handle))
	{
		SL_Error("(SL::UpdateIntEntry) Invalid handle. Make sure you used (SL::OpenTable) first.");
		return 0;
	}
	if(SL::upd_type[handle] != SL::INSERT) return 0;
	format(SL::upd_form, sizeof(SL::upd_form), "`%s`,", field);
	strcat(SL::upd_query[handle], SL::upd_form);
	if(use_real_escape == true)
	{
		format(SL::upd_form, sizeof(SL::upd_form), "'%q',", value);
	}
	else
	{
		format(SL::upd_form, sizeof(SL::upd_form), "'%s',", value);
	}
	strcat(SL::upd_query_2[handle], SL::upd_form);
	SL::upd_datacount[handle]++;
	return 1;
}
stock SL::ExistsTable(const tablename[], DB:database = DB:1)
{
	new query[85], DBResult:result;
	format(query, sizeof(query), "SELECT * FROM sqlite_master WHERE type='table' AND name='%s';", tablename);
	result = db_query(database, query);
	new rows = db_num_rows(result);
	db_free_result(result);
	if(rows > 0) 
	{
		return 1;
	}
	return 0;
}
stock SL::CountRows(const tablename[], DB:database = DB:1)
{
	new query[SL_FORM_LENGTH], DBResult:result;
	format(query, sizeof(query), "SELECT * FROM %s", tablename);
	result = db_query(DB:database, query);
	new rows = db_num_rows(result);
	db_free_result(result);
	return rows;
}
stock SL::RowExistsEx(const table[], const column[], const columnID[], DB:database = DB:1)
{
	new query[SL_FORM_LENGTH], DBResult:result;
	format( query, sizeof(query),"SELECT * FROM `%s` WHERE `%s` = '%q' ", table, column, columnID);
	result = db_query(DB:database, query);
	new rows = db_num_rows(result);
	db_free_result(result);
	if(rows > 0) 
	{
		return 1;
	}
	return 0;
}
stock SL::DropTable(const table[], DB:database = DB:1)
{
	new query[SL_FORM_LENGTH];
	format(query, sizeof(query), "DROP TABLE %s;", table);
	db_query(DB:database, query);
	return 0;
}
stock SL::RowExists(const table[], const column[], columnID[], DB:database = DB:1)
{
	new query[SL_FORM_LENGTH], DBResult:result;
	format( query, sizeof(query),"SELECT * FROM `%s` WHERE `%s` = '%d' ", table, column, columnID);
	result = db_query(DB:database, query);
	new rows = db_num_rows(result);
	db_free_result(result);
	if(rows > 0) 
	{
		return 1;
	}
	return 0;
}
stock SL::CountTables(DB:database = DB:1)
{
	new DBResult:result;
	result = db_query(DB:database, "SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name != 'sqlite_sequence'");
	new tables = db_get_field_int(result);
	db_free_result(result);
	return tables;
}

#undef SL_INVALID_HANDLE
#undef SL_MAX_TABLE_NAME
#undef SL_MAX_HANDLES
#undef SL_Warning
#undef SL_Error
