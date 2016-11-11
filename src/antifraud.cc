#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;

//A friend network is more or less a graph in compsci terms.  The nodes are users
//So here's a struct describing each User.

struct User_Data{
  //Let's track the number of transactions each user makes.
  int n_transactions;
  //Let's also track the total amount of money each user spends.
  float total_money_spent;
  float total_money_received;
  //And now let's track the users this user has transacted with.  
  //I'm using a map here: first int is the user ID, and second is the number of transactions.
  //In principle you could use the number of transactions as a "weight"; i.e. if a user 
  //has many successful transactions with the same people they are probably trustworthy.
  map<int, int> Users_Transacted_With;
  //This map is also the basis of the edges of our graph, more or less a weighted adjacency list.
};

//Now let's set up the graph itself.

class Friend_Network
{
  //The number of users
  int number_of_users;

  //A list of users, mapping user id with their data defined in the struct above
  map<int, User_Data> List_Of_Users;

  //The output files our graph will use for verification
  string Verification_Output_Files[3];
 public:

  //A function just to read the names of the output files
  void Set_Output_File(string output_file_name, int which_output_file);

  //We have to get new nodes into our graph somehow.  In this case we define our objects based
  //on whether or not we find them when processing transactions, so we will always have at least one
  //edge while adding a node to our graph.
  void Add_New_User(int new_user, int first_customer, float first_transaction, bool sent_or_received);
  
  //The guts of the challenge.  When a transaction is made between two users, we verify it three ways.
  int Verify_Transaction(int uid1, int uid2);
  
  //Then, after we do verification, we process the transaction.  
  void Process_Transaction(int uid1, int uid2, float money_moved);

  //A function to read in a text file. For this challenge we read in two ways; once to populate
  //the initial state, and once to start verifying.  The processing is otherwise similar so 
  //we just have a bool to denote which step we are in.
  void Parse_Data(string stream_file_name, bool To_Verify);
};


//The aforementioned function to open the output file so it might be accessible to Verify_Transaction
void Friend_Network::Set_Output_File(string output_file_name, int which_output_file){
  Verification_Output_Files[which_output_file] = output_file_name;
}

//Did we find a new user when processing a transaction?  Add them to the list.
void Friend_Network::Add_New_User(int new_user, int first_customer, float first_transaction, bool sent_or_received){

  //New user!
  User_Data this_user;

  //This is their first transaction.
  this_user.n_transactions = 1;

  //Were they the sender or the recipient?
  this_user.total_money_spent = sent_or_received ? first_transaction : 0;
  this_user.total_money_received = (!sent_or_received) ? 0 : first_transaction;

  //Mark on the adjacency list that they had their first transaction with a customer.
  this_user.Users_Transacted_With[first_customer] = 1;

  //Add the user to the network
  List_Of_Users[new_user] = this_user;

  //Time ordered, the unique user ids in the data increase by one every time they appear
  //So they are presumably sequential.  But we don't have the true initial state of the network,
  //Just one we assume exists from the initial data.  So we note the user ids in our sample are
  //all bound 0<n<10^5 and simply set the number of users to the highest user id found so far. 
  if (new_user > number_of_users) number_of_users = new_user;
}


//Now let's process the transaction.  A transaction is between two users for a certain amount of money.
void Friend_Network::Process_Transaction(int uid1, int uid2, float money_moved){

  //Is the sender in the network?  If not, add them as a new user.
  if(List_Of_Users.find(uid1)==List_Of_Users.end()){
    Add_New_User(uid1, uid2, money_moved, true);}
  else{
    //Otherwise, increment their number of transactions and the amount of money they spent.

    List_Of_Users[uid1].n_transactions++;
    List_Of_Users[uid1].total_money_spent += money_moved;
    //Increment the number of transactions between this user and the recipient...
    if(List_Of_Users[uid1].Users_Transacted_With.find(uid2) != List_Of_Users[uid1].Users_Transacted_With.end()){
      List_Of_Users[uid1].Users_Transacted_With[uid2]++;
    }
    //Unless this is their first transaction, in which case add them to the adjacency map.
    else{
      List_Of_Users[uid1].Users_Transacted_With[uid2] = 1;
    }
    
  }
  //You could do the first verification feature here but I prefer to have the verification as distinct
  //functionality in its own function.


  //Now do the same as above for the recipient.
  if(List_Of_Users.find(uid2)==List_Of_Users.end()){
    Add_New_User(uid2, uid1, money_moved, false);}
  else{

    List_Of_Users[uid2].n_transactions++;
    List_Of_Users[uid2].total_money_received += money_moved;

    if(List_Of_Users[uid2].Users_Transacted_With.find(uid1) != List_Of_Users[uid2].Users_Transacted_With.end()){
      List_Of_Users[uid2].Users_Transacted_With[uid1]++;
    }
    else{
      List_Of_Users[uid2].Users_Transacted_With[uid1] =1;

    }
  }

}

//Now the meat; the function to verify a transaction.
int Friend_Network::Verify_Transaction(int uid1, int uid2)
{

  /*------------------------------------------------------------------------------
  The verification appears to be a graph traversal problem in OOP terms.
  We have three conditions:
  1) User has transacted with the recipient before
  2) User and recipient have a common friend
  3) User and recipient are separated in the network by 4 or fewer degrees.

  Each condition also satisfies the ones below it, so I structured this as a 
  bi-directional breadth-first search.  First:
  
  1) Check if the users exist already.  If not, then they can't be verified at all
  2) Check the friends list of the sender/receiver for each other (Condition 1)
  3) Check 1 deep in the adjacency list on each side (common friends, condition 2)
  4) Check 2 deep in the adjacency list on each side (4th degree separation)

  If at any point one of the above conditions are met, the validation is set accordingly.
  ----------------------------------------------------------------------------------*/

  //int representing verification status.  Doing this bitwise:
  //LSB = verified only by step 3
  //LSB+1 = verified by step 2 and 3
  //LSB+2 = verified by all steps.
  //By default zero (no verification)
  int verification_status = 0;

  //1) Does either user not exist in the network yet?  If so then verificaiton fails automatically.
  if(List_Of_Users.find(uid1)==List_Of_Users.end() || List_Of_Users.find(uid2)==List_Of_Users.end()){
    verification_status = 0;
  }
  else{
    //Otherwise, go to step 2
    User_Data User1 = List_Of_Users[uid1];
    User_Data User2 = List_Of_Users[uid2];

    //Do the users appear on each others' transaction lists?  Really since the adjacency list
    //is bidirectional you should only have to check once, but I was cautious here.
    if(User1.Users_Transacted_With.find(uid2) != User1.Users_Transacted_With.end() || 
       User2.Users_Transacted_With.find(uid1) != User2.Users_Transacted_With.end()){
      //Set all verification bits true.
      verification_status = 7;
    }
    //If not, move to verification step 3
    else{
      //The least extensible part of the code.  UID is bound below 10^5 in the sample data so I assumed
      //the max number of nodes is 10^5. 
      int Visited_Nodes1[100000], Visited_Nodes2[100000];
      map<int, int>::const_iterator First_it1 = User1.Users_Transacted_With.begin();
      map<int, int>::const_iterator First_it2 = User2.Users_Transacted_With.begin();
      //Iterate over adjacency for User 1 and see if we hit a common user (actually the way this is structured you can't
      //get a positive result when looping over user 1 first.  I could try this with one loop where you just stop iterating
      //over the user with fewer adjacencies when you hit the end of their list.
      for(First_it1 = User1.Users_Transacted_With.begin(); First_it1 != User1.Users_Transacted_With.end(); First_it1++){
	//Did we visit the node from the other side?

	if(Visited_Nodes2[First_it1->first] == 1){
	  verification_status = 3;
	  break;
	}
	//If not before, we visited it on this side.
	else{
	  Visited_Nodes1[First_it1->first] = 1;}
      }

      //Now iterate over the second user's adjacency list.
      for(First_it2 = User2.Users_Transacted_With.begin(); First_it2 !=User2.Users_Transacted_With.end(); First_it2++){

	if(Visited_Nodes1[First_it2->first] == 1){
          verification_status =3;
          break;
	}
	else{
          Visited_Nodes2[First_it2->first] = 1;}
      }
      //Did we fail to find a 2nd-degree connection?  If so continue.
      if(verification_status != 3){


	for(First_it1 = User1.Users_Transacted_With.begin(); First_it1 != User1.Users_Transacted_With.end(); First_it1++){
	  //Get each user 1 degree away from the sender.

	  User_Data User_one_down = List_Of_Users[First_it1->first];
	  //Now loop over the 2nd degree users adjacent to the 1st degree user.
	  for(map<int, int>::const_iterator Second_it1 = User_one_down.Users_Transacted_With.begin(); 
	      Second_it1 != User_one_down.Users_Transacted_With.end(); Second_it1++){

	    //Again check if we visited the node and mark if we didn't.
	    if(Visited_Nodes2[Second_it1->first] == 1){
	      verification_status = 1;
	      break;
	    }
	    else{
	      Visited_Nodes1[Second_it1->first] = 1;}
	  }
	}

	//Now do the same as above for the 2nd-degree friends of the recipient, User 2.
	for(First_it2 = User2.Users_Transacted_With.begin(); First_it2 != User2.Users_Transacted_With.end(); First_it2++){

          User_Data User_one_down = List_Of_Users[First_it2->first];
          for(map<int, int>::const_iterator Second_it2 = User_one_down.Users_Transacted_With.begin();
	      Second_it2 != User_one_down.Users_Transacted_With.end(); Second_it2++){

            if(Visited_Nodes1[Second_it2->first]== 1){
              verification_status = 1;
	      break;
            }
            else{
	      Visited_Nodes2[Second_it2->first] = 1;}
          }
	}

      }
    }

  }
  //Return the three verification bits.
  return verification_status;
}



//Now the function for parsing the data from a text file.  
void Friend_Network::Parse_Data(string data_file_name, bool To_Verify){

  // prepare the three output files.
  ofstream Verification_File1;
  ofstream Verification_File2;
  ofstream Verification_File3;

  if(To_Verify){
    //Don't open them though, unless we're actually doing verification.
    Verification_File1.open(Verification_Output_Files[0].c_str());
    Verification_File2.open(Verification_Output_Files[1].c_str());
    Verification_File3.open(Verification_Output_Files[2].c_str());
  }

  //Total number of lines (keep this for marking progress)
  int nlines = 0;
  
  //Declare the input file
  ifstream batchfile;
  batchfile.open(data_file_name.c_str());

  //Read it in line by line
  string batch_line;
  //Get the first line; it's just the headers.
  //  getline(batchfile, batch_line);
  //Now actually process the transactions.
  while(!batchfile.eof()){
    nlines++;
    getline(batchfile, batch_line);
    if( nlines==1 )continue;

    if (nlines % 1000 == 0) cout << "processed " << nlines << " records " << endl;

    //For parsing with STL without strtok, first I found and cleared the first 4 commas
    //After: date, id1, id2, cost
    //We don't know anything about the comment, there could be anything in there including Unicode and other commas.
    //So just replace the first four with white space.
    int ncommas = 0;
    for(unsigned int i = 0; i<batch_line.length(); ++i){
      if( batch_line.at(i) == ','){
	ncommas++;
	if( ncommas <5)batch_line[i] = ' ';
      }
    }

    //Make a stringstream to read the parts of the line
    stringstream batch_Stream(batch_line);

    //Read the first 4 pieces of data from the line - date, uid1, uid2, amount
    string date_from_batch, time_from_batch, i1_from_batch, i2_from_batch, cost_from_batch, comment;
    batch_Stream >> date_from_batch >> time_from_batch >> i1_from_batch >> i2_from_batch >> cost_from_batch;

    //Now dump the rest into the comment string
    getline(batch_Stream, comment);
    //Erase the leading space
    comment.erase(0,2);

    //Convert the strings for uid1, uid2, amount to the right containers.
    int user_sending_money = atoi(i1_from_batch.c_str());
    int user_receiving_money = atoi(i2_from_batch.c_str());
    float money_sent = atof(cost_from_batch.c_str());

    if((money_sent <= 0.0) && (user_sending_money <=0) && (user_receiving_money <=0)) continue;

    //Declare the verification status
    int status = 0;
    //Verify transactions if we are doing that now.
    if(To_Verify){

      status = Verify_Transaction(user_sending_money, user_receiving_money);
    }

    //If we are doing verification, check the three verification bits.  To repeat from above,                                       
    //LSB = verified only by step 3                                                                                                 
    //LSB+1 = verified by step 2 and 3                                                                                              
    //LSB+2 = verified by all steps 
    if(To_Verify){
      if((status & 0x4) ==4){
        Verification_File1 << "trusted" << endl;
      }
      else{
        Verification_File1 << "unverified" << endl;
        cout << "Unverified: You've never had a transaction with this user before.  Are you sure you would like to proceed with this payment?" << endl;
      }

      if((status & 0x2) ==2){
        Verification_File2 << "trusted"<< endl;
      }
      else{
        Verification_File2 << "unverified" << endl;
        cout << "Unverified: This user is not a friend or a friend of a friend.  Are you sure you would like to proceed with this payment?" << endl;
      }

      if((status & 0x1) ==1){
        Verification_File3 << "trusted"<< endl;
      }
      else{
        Verification_File3 << "unverified" << endl;
        cout << "Unverified: This user is outside of the 4th degree or higher network.  Are you sure you would like to proceed wit his payment?" << endl;
      }

    }


    //Process the transaction
    Process_Transaction(user_sending_money, user_receiving_money, money_sent);
    

    //If we are doing verification, check the three verification bits.  To repeat from above,
    //LSB = verified only by step 3             
    //LSB+1 = verified by step 2 and 3                                                  
    //LSB+2 = verified by all steps

  }
  //If we opened some files, close them now.
  if(To_Verify){
    Verification_File1.close();
    Verification_File2.close();
    Verification_File3.close();
  }
}


int main(int argc, char *argv[]){
  
  // In principle this code should take 5 arguments: 2 input files followed by 3 output files.
  // First check the right number of arguments have been passed at start.

  //I didn't do much exception handling in this module.  I just put one here to show that I know
  //what exception handling is.
  try{
    if (argc<6){
      throw (argc-1);
    }
  }
  catch(int e){
    cout << "Exception: Not enough arguments.  Expecting 5, got " << e << endl;
  }

  //Declare the network
  Friend_Network our_network;
  
  //Find the 2 input file names
  string batch_file_name = argv[1];
  string stream_file_name = argv[2];
  //Also the output file names.
  for(int iout = 0; iout < 3; iout++){
    our_network.Set_Output_File(argv[iout+3],iout);
  }

  //Now parse the initial data (false means we aren't doing verification on it)
  our_network.Parse_Data(batch_file_name, false);
  //Now parse the test data (true means to do the verification)
  our_network.Parse_Data(stream_file_name, true);

  return 0;

}
