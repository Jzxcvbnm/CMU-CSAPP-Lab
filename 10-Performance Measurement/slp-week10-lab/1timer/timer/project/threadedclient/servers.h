/*
	servers.h

	Open a TCP/IP connection to a server, request the specified
	information, and return the information required in a struct.
 */
#include <string>
using namespace std;

/*
	Personal

	Personal information about the owner of an account.
 */
struct Personal {
	Personal( int account, string *firstname, string *lastname,
		string *address ) {

		FirstName = firstname;
		LastName = lastname;
		Account = account;
		Address = address;
	}
	~Personal() {
		delete FirstName;
		delete LastName;
		delete Address;
	}
	string *FirstName;
	string *LastName;
	string *Address;
	int Account;
};

/*
	Personal *GetPersonalInformation( int account )

	Retrieve personal information about the user of a certain account.
	Return NULL if there is no such account.
 */
Personal *GetPersonalInformation( int account );

/*
	AccountInfo

	A struct to hold account-related information.
 */
struct AccountInfo {
	AccountInfo( int account, int balance_in_cents, int share, int pending ) {
		Account = account;
		Balance = balance_in_cents;
		Share = share;
		Pending = pending;
	}
	int Account;
	int Balance;
	int Share;
	int Pending;
};

AccountInfo *GetAccountInformation( int account );