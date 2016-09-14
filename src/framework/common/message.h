#ifndef _MESSAGE_INCLUDED_H_
#define _MESSAGE_INCLUDED_H_

enum MessagePackID
{
	ID_NONE = ID_USER_PACKET_ENUM,
	
	//DATABASE PACKET
	ID_QUERY,		 //Service -> DatabaseService -> Logic
	ID_QUERY_OK,     //Logic -> DatabaseService -> Service
	ID_EXECUTE,      //同上
	ID_EXECUTE_OK,   //同上
	ID_UNPACKED_FAILED,  //Logic -> DatabaseService -> Service

	//AUTH SERVER
	ID_AUTH_OK,
};



enum ErrorCodeEnum
{
	ERROR_FIRST = 1,
	ERROR_INVALID_PARAMETER
};
#endif
