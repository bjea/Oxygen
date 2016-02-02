// $Id: commands.cpp,v 1.16 2016-01-14 16:10:40-08 - - $

#include "commands.h"
#include "debug.h"

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr  },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string path = "";

   if (words.size() > 1)
   {
      path = words[1];
   }
   DEBUGF ('c', path);
   state.cat(path);
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string path = "";

   if (words.size() == 1)
   {
      path = "/";
   }
   else if (words.size() > 2)
   {
      cout << "Too many operands! Please give just 1 folderName or 1 pathName." << endl;
      return;
   }
   else
   {
      path = words[1];
   }

   DEBUGF ('c', path);
   state.cd(path);
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   int stateCode = 0;

   if (words.size() > 1)
   {
      try {
         stateCode = std::stoi(words[1]);
      }
      catch (const std::invalid_argument& ex)
      {
         stateCode = 127;
      }
   }

   exit_status::set(stateCode);
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string path = "";

   if (words.size() > 1)
   {
      path = words[1];
   }

   DEBUGF ('c', path);

   vector<string> lsInfo = state.getLS(path);

   vector<string> lsInfo_split = split(lsInfo[0], "\t");
   cout << lsInfo_split[0] << ":" << endl;

   for (unsigned int i = 1; i < lsInfo_split.size(); ++i)
   {
      cout << lsInfo_split[i] << endl;
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string path = "";

   if (words.size() > 1)
   {
      path = words[1];
   }

   DEBUGF ('c', path);

   vector<string> lsInfo = state.getLSR(path);

   for (unsigned int i = 0; i < lsInfo.size(); ++i)
   {
      vector<string> lsInfo_split = split(lsInfo[i], "\t");
      cout << lsInfo_split[0] << ":" << endl;
      for (unsigned int i = 1; i < lsInfo_split.size(); ++i)
      {
         cout << lsInfo_split[i] << endl;
      }
   }
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() > 1)
   {
      wordvec newdata;
      for (auto it = words.begin()+2; it != words.end(); ++it)
      {
         newdata.push_back(*it);
         newdata.push_back(" ");
      }
      state.make(words[1], newdata);
   }
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   // words could be a full pathname or relative path

   if (words.size() > 1)
   {
      state.mkdir(words[1]);
   }
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string newPrompt = "";
   if (words.size() > 1)
   {
      for (auto it = ++words.begin(); it != words.end(); ++it)
      {
         newPrompt.append(*it);
         newPrompt.append(" ");
      }
   }
   DEBUGF ('c', newPrompt);
   state.setPrompt(newPrompt);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   cout << state.getPWD() << endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() > 1)
   {
      state.rm(words[1]);
   }
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() > 1)
   {
      state.rmr(words[1]);
   }
}

