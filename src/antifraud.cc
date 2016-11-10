#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>

using namespace std;

//A friend network is more or less a graph in compsci terms.  The nodes are users
//So here's a struct describing each User.

struct User_Data{
  //Users are identified by ID.  We will represent this by having a map of IDs to User_Data.
  //Let's track the number of transactions each user makes.
  int n_transactions;
  //Let's also track the total amount of money each user spends.
  float total_money_spent;
  list<pair<int,int>> Users_Transacted_With;
};

//If the nodes in the graph are users, the adjacencies (edges) are Transactions.
//So let's have a struct for Transactions too.

struct Transaction_History{
  string date;
  string time;
  int id1;
  int id2;
  float amount;
  string comment;
};

class Friend_Network
{
  int number_of_users;
 public:
  void Setup_Initial_Graph(string batch_file_name);
  void Parse_New_Data(string stream_file_name);
};


void Friend_Network::Setup_Initial_Graph(string batch_file_name){

  cout << batch_file_name << endl;
  ifstream batchfile;
  batchfile.open(batch_file_name.c_str());
  string batch_line;                                                                                                                 
  while(!batchfile.eof()){
    getline(batchfile, batch_line);

    int position_of_fourth_comma = 0;
    int ncommas = 0;
    for(int i = 0; i<batch_line.length(); ++i){
      if( batch_line.at(i) == ','){
	ncommas++;
	if( ncommas <5)batch_line[i] = ' ';
      }
    }

    stringstream batch_Stream(batch_line);

    string date_from_batch, time_from_batch, i1_from_batch, i2_from_batch, cost_from_batch, comment;
    batch_Stream >> date_from_batch >> time_from_batch >> i1_from_batch >> i2_from_batch >> cost_from_batch;

    getline(batch_Stream, comment);
    comment.erase(0,2);
    int 

    cout << date << "|" << time << "|" << atoi(i1.c_str()) << "|" << atoi(i2.c_str()) << "|" << atof(cost.c_str()) << "|" << comment << endl;
  }
  
    
};

void Friend_Network::Parse_New_Data(string stream_file_name){

  ifstream streamfile;
  streamfile.open(stream_file_name.c_str());

  string stream_line;
  while(!streamfile.eof()){
    getline(streamfile, stream_line);
    cout << stream_line << endl;
  }


}


int main(int argc, char *argv[]){
  
  // In principle this code should take 5 arguments: 2 input files followed by 3 output files.
  // First check the right number of arguments have been passed at start.
  try{
    if (argc<6){
      throw (argc-1);
    }
  }
  catch(int e){
    cout << "Exception: Not enough arguments.  Expecting 5, got " << e << endl;
  }

  Friend_Network our_network;
  
  string batch_file_name = argv[1];
  our_network.Setup_Initial_Graph(batch_file_name);
  
  string stream_file_name = argv[2];
  
  our_network.Parse_New_Data(stream_file_name);
  
  return 0;

}
