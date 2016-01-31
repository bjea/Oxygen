// $Id: file_sys.cpp,v 1.5 2016-01-14 16:16:52-08 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

/*======================================================================================================================
 *
 =====================================================================================================================*/


inode_state::inode_state() {
   // create root inode and set cwd == root
   root = make_shared<inode>(file_type::DIRECTORY_TYPE);
	root->contents->setSelfNode(root);
	root->contents->setParentNode(root);
   cwd = root;
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
}

string inode_state::getPWD() {

	string pathName = "";
	inode_ptr currentNode = cwd;
	inode_ptr parentNode = currentNode->contents->getNodeByName("..");

	do{
		if(parentNode != currentNode)
		{
			pathName.insert(0, parentNode->contents->getNameOfNode(cwd));
			currentNode = parentNode;
			parentNode = currentNode->contents->getNodeByName("..");
		}

		pathName.insert(0, "/");

	}while(parentNode != currentNode);

	return pathName;
}

inode_ptr inode_state::getTargetNode(const string& path) {

	DEBUGF ('i', "path = " << path);
	DEBUGF ('i', "path size = " << path.length());
	inode_ptr targetNode = cwd;

	// see if any path is specified
	// if no path is specified, we should return cwd
	if(path.length() > 0)
	{
		inode_ptr searchNode = cwd;

		// if this is true, we have to search from root node
		if(path[0] == '/')
		{
			searchNode = root;
		}

		// tokenize path by delimiter '/'
		wordvec pathTokens = split(path, "/");

		for(unsigned int i = 0; i < pathTokens.size(); i++)
		{
			DEBUGF ('i', "path token = " << pathTokens[i]);
		}
		// find the right inode to do LS
		// if token == ".", we will not change the current node
		// if token == "..", we will move up 1 level, and do nothing when we reach to root node.
		// else, find node with corresponding name, check if it contains file or folder, throw exception when the
		// content type of a node is PLAIN_TYPE

	}
	return targetNode;
}

vector<string> inode_state::getLS(const string& path) {
	vector<string> result;
	this->getTargetNode(path)->getLS(path, result);
	return result;
}


const string& inode_state::prompt() { return prompt_; }

void inode_state::setPrompt(string newPrompt)
{
	//string newPrompt_str;
	//newPrompt_str = newPrompt[1];
	prompt_ = newPrompt;

}

inode_ptr inode_state::getCWD(inode_state state)
{
	return cwd;
}

void inode_state::mkdir(const string& path)
{
	// process path: extract path and find correct node
	inode_ptr targetFolder = cwd;
	string folderName= path;

	targetFolder->mkDir(folderName);

}

/*======================================================================================================================
 *
 =====================================================================================================================*/

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

/*======================================================================================================================
 *
 =====================================================================================================================*/


inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
	contentType = type;
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

size_t inode::getContentSize() {
	return contents->size();
}

void inode::getLS(string path, vector<string>& result) {

	vector<string> lsInfo;
	string currentFolder = "";

	if(path.empty())
	{
		currentFolder = "/";

		if(contents->getNodeByName("..") != contents->getNodeByName("."))
		{

		}
	}
	else
	{
		currentFolder = path;
	}


	contents->getLS(currentFolder, result);
}

void inode::mkDir(const string& folderName) {

	inode_ptr newNode = this->contents->mkdir(folderName);

	if(newNode != nullptr)
	{
		newNode->contents->setParentNode(this->contents->getNodeByName("."));
		newNode->contents->setSelfNode(newNode);
	}
}

/*======================================================================================================================
 *
 =====================================================================================================================*/


file_error::file_error (const string& what):
            runtime_error (what) {
}

size_t plain_file::size() const {
   size_t size {0};
	size = data.size();
   DEBUGF ('i', "size = " << size);
   return size;
}

/*======================================================================================================================
 *
 =====================================================================================================================*/


const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
}

void plain_file::remove (const string&) {
   throw file_error ("is a plain file");
}

/*======================================================================================================================
 *
 =====================================================================================================================*/


inode_ptr plain_file::mkdir (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&) {
   throw file_error ("is a plain file");
}

string plain_file::getNameOfNode(inode_ptr) {
	throw file_error ("is a plain file");
}
inode_ptr plain_file::getNodeByName(const string&) {
	throw file_error ("is a plain file");
}

void plain_file::getLS(const string&, vector<string>&) {
	// it is a no-op for plain file
	return;
}

void plain_file::setSelfNode(inode_ptr) {
}
void plain_file::setParentNode(inode_ptr) {

}

/*======================================================================================================================
 *
 =====================================================================================================================*/


directory::directory() {
   dirents.clear();
}

size_t directory::size() const {
   size_t size {0};
	size = dirents.size();
	size += 2;
   DEBUGF ('i', "size = " << size);
   return size;
}

void directory::setSelfNode(inode_ptr current) {
	//dirents["."] = current;
	selfNode = current;
}
void directory::setParentNode(inode_ptr parent) {
	//dirents[".."] = parent;
	parentNode = parent;
}

const wordvec& directory::readfile() const {
   throw file_error ("is a directory");
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
}


inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);

	bool is_dir_already_present = false;

	// check if dirname alrady exists
	is_dir_already_present = dirents.find(dirname) != dirents.end() ? true : false;

	if(is_dir_already_present)
	{
		cout<< "TODO: throw error when mkdir for existing folder"<<endl;
		return nullptr;
	}

	inode_ptr newNode = make_shared<inode>(file_type::DIRECTORY_TYPE);
	dirents[dirname] = newNode;

   return newNode;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);
   return nullptr;
}

string directory::getNameOfNode(inode_ptr node) {
	 for (map<string,inode_ptr>::iterator it=dirents.begin(); it!=dirents.end(); ++it)
	 {
		 if(it->second == node)
		 {
			 return it->first;
		 }
	 }

	 return "";
}

inode_ptr directory::getNodeByName(const string& nodeName) {
	if(nodeName == "..")
	{
		return parentNode.lock();
	}

	if(nodeName == ".")
	{
		return selfNode.lock();
	}

	return dirents.find(nodeName) != dirents.end() ? dirents[nodeName] : nullptr;
}

void directory::constructLSInfo(const string& name, const string& delimiter, inode_ptr node, string& result) {
	result += delimiter;
	result += std::to_string(node->get_inode_nr());
	result += delimiter;
	result += std::to_string(node->getContentSize());
	result += delimiter;
	result += name;

	if(shouldAppendSlash(name, node))
	{
		result +="/";
	}
}


// for each directory, we will append a string with the following format (each field is separated by a space)
// [folderName] [nodeNumber] [size] [elementName] [nodeNumber] [size] [elementName]
// "     /            1         2          .            1         2         ..    "
void directory::getLS(const string& currentFolderName, vector<string>& result) {

	 static const string SP=" ";

	 string ls = "";
	 ls += currentFolderName;

	 inode_ptr me = selfNode.lock();
	 inode_ptr myParent = parentNode.lock();
	constructLSInfo(".", SP, me, ls);
	constructLSInfo("..", SP, myParent, ls);


	 for (map<string,inode_ptr>::iterator it=dirents.begin(); it!=dirents.end(); ++it)
	 {
		 /*
		 ls += SP;
		 ls += std::to_string((it->second)->get_inode_nr());
		 ls += SP;
		 ls += std::to_string(this->size());
		 ls += SP;
		 ls += it->first;
		  */

		 constructLSInfo(it->first, SP, it->second, ls);
	 }
	 result.push_back(ls);
}

bool directory::shouldAppendSlash(const string& folderName, inode_ptr folderNode) {
	if(folderName == "." or folderName == "..")
	{
		return false;
	}

	if(folderNode->getContentType() == file_type::DIRECTORY_TYPE)
	{
		return true;
	}

	return false;
}

