/* 
 *  Simple register/login system using easy_mySL.inc! 
*/ 
#include <a_samp> 
#include <easy-sqlite> 
enum p_info 
{ 
    p_id, 
    p_name[24], 
    p_password[64], 
    p_score, 
    Float:p_posx, 
    Float:p_posy, 
    Float:p_posz, 
    p_loggedin 
}; 
new UserInfo[MAX_PLAYERS][p_info];
#define DIALOG_LOGIN 0 
#define DIALOG_REGISTER 1 
stock ret_pName(playerid) 
{ 
    new name[24]; 
    GetPlayerName(playerid, name, sizeof(name)); 
    return name; 
} 
main() 
{ 
     
} 
public OnGameModeInit() 
{ 
    //Connecting to the database 
	SL::Connect("server.db"); 
    //Checking if the table 'players' exists 
     
	 
    //Checking if the table 'players' exists 
	if(!SL::ExistsTable("players")) 
    { 
        //If not, then create a table called 'players'. 
        new handle = SL::Open(SL::CREATE, "players"); //Opening a valid handle to create a table called 'players' 
		SL::AddTableEntry(handle, "p_id", SL_TYPE_INT, 11, true); 
        SL::AddTableEntry(handle, "p_name", SL_TYPE_VCHAR, 24); 
        SL::AddTableEntry(handle, "p_password", SL_TYPE_VCHAR, 64); 
        SL::AddTableEntry(handle, "p_score", SL_TYPE_INT); 
        SL::AddTableEntry(handle, "p_posx", SL_TYPE_FLOAT); 
        SL::AddTableEntry(handle, "p_posy", SL_TYPE_FLOAT); 
        SL::AddTableEntry(handle, "p_posz", SL_TYPE_FLOAT); 
        SL::Close(handle);//Closing the previous opened handle. 
		print("Table 'players' was successfully created");
    } 
	else 
	{
		print("Table 'players' was successfully loaded");
	}
	SL::SetStringEntryEx("players", "p_password", "XD2LOL", "p_name", "[AV]Thee");
	//SL::SetIntEntry(const table[], const field[], value, const column[], columnID
    return 1; 
} 
public OnPlayerConnect(playerid) 
{ 
    UserInfo[playerid][p_loggedin] = 0; UserInfo[playerid][p_score] = 0;  UserInfo[playerid][p_posx] = 1958.3783; 
    UserInfo[playerid][p_posy] = 1343.1572; UserInfo[playerid][p_posz] = 15.3746;  
    if(SL::RowExistsEx("players", "p_name", ret_pName(playerid))) //Check if the name is registered in the database 
    { 
        //Get the player password and unique ID. 
        new handle = SL::OpenEx(SL::READ, "players", "p_name", ret_pName(playerid)); 
        SL::ReadString(handle, "p_password", UserInfo[playerid][p_password], 64); 
        SL::ReadInt(handle, "p_id", UserInfo[playerid][p_id]); 
        SL::Close(handle); 
        //Show the login dialog 
        ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, "{0080FF}Login", "Please input your password below to log in.", "Login", "Exit"); 
    } 
    else 
    { 
        //If not registered, then show the register DIALOG. 
        ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD, "{0080FF}Register", "Please input a password below to register in.", "Login", "Exit"); 
    } 
    return 1; 
} 
public OnPlayerSpawn(playerid) 
{ 
    SetPlayerPos(playerid, UserInfo[playerid][p_posx], UserInfo[playerid][p_posy], UserInfo[playerid][p_posz]); 
    return 1; 
} 
public OnPlayerDisconnect(playerid, reason) 
{ 
    if(UserInfo[playerid][p_loggedin] == 1) 
    { 
        //Save the player data. 
        GetPlayerPos(playerid, UserInfo[playerid][p_posx], UserInfo[playerid][p_posy], UserInfo[playerid][p_posz]); 
        new handle = SL::Open(SL::UPDATE, "players", "p_id", UserInfo[playerid][p_id]); 
        SL::WriteInt(handle, "p_score", GetPlayerScore(playerid)); 
        SL::WriteFloat(handle, "p_posx", UserInfo[playerid][p_posx]); 
        SL::WriteFloat(handle, "p_posy", UserInfo[playerid][p_posy]); 
        SL::WriteFloat(handle, "p_posz", UserInfo[playerid][p_posz]); 
        SL::Close(handle); 
    } 
    return 1; 
} 
public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]) 
{ 
    switch(dialogid) 
    { 
        case DIALOG_REGISTER: 
        { 
            if(!response) return Kick(playerid); 
            if(strlen(inputtext) < 5) 
            { 
                ShowPlayerDialog(playerid, DIALOG_REGISTER, DIALOG_STYLE_PASSWORD, "{0080FF}Register", "Please input a password below to register in.", "Login", "Exit"); 
                return 1; 
            } 
            SHA256_PassHash(inputtext, "", UserInfo[playerid][p_password], 64); 
            new handle = SL::Open(SL::INSERT, "players"); 
			SL::ToggleAutoIncrement(handle, true);
            SL::WriteString(handle, "p_name", ret_pName(playerid)); 
            SL::WriteString(handle, "p_password", UserInfo[playerid][p_password]); 
            SL::WriteInt(handle, "p_score", 0); 
            SL::WriteFloat(handle, "p_posx", 0.0); 
            SL::WriteFloat(handle, "p_posy", 0.0); 
            SL::WriteFloat(handle, "p_posz", 0.0); 
            UserInfo[playerid][p_id] = SL::Close(handle);  
            SendClientMessage(playerid, -1, "Successfully registered in!"); 
            UserInfo[playerid][p_loggedin] = 1; 
        } 
        case DIALOG_LOGIN: 
        { 
            if(!response) Kick(playerid);  
            new hash[64]; 
            SHA256_PassHash(inputtext, "", hash, 64); 
            if(!strcmp(hash, UserInfo[playerid][p_password])) 
            {  
                //Load player data 
                new handle = SL::Open(SL::READ, "players", "p_id", UserInfo[playerid][p_id]); 
                SL::ReadInt(handle, "p_score", UserInfo[playerid][p_score]); 
                SL::ReadFloat(handle, "p_posx", UserInfo[playerid][p_posx]); 
                SL::ReadFloat(handle, "p_posy", UserInfo[playerid][p_posy]); 
                SL::ReadFloat(handle, "p_posz", UserInfo[playerid][p_posz]); 
                SL::Close(handle);//You must close the handle. 
                SetPlayerScore(playerid, UserInfo[playerid][p_score]); 
                UserInfo[playerid][p_loggedin] = 1; 
                SendClientMessage(playerid, -1, "Successfully logged in!"); 
                 
            } 
            else  
            { 
                ShowPlayerDialog(playerid, DIALOG_LOGIN, DIALOG_STYLE_PASSWORD, "{0080FF}Login", "Please input your password below to log in.", "Login", "Exit"); 
            } 
        } 
    } 
    return 1; 
}  
