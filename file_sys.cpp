// $Id: file_sys.cpp,v 1.5 2016-01-14 16:16:52-08 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <queue>

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
   // create root inode and set cwd == root.
   root = make_shared<inode>(file_type::DIRECTORY_TYPE);
	root->contents->setSelfNode(root);
	root->contents->setParentNode(root);
   cwd = root;
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
}

string inode_state::getPWD() {

	string pathName = "";
	inode_ptr thisNode = cwd;
	inode_ptr currentNode = cwd;
	inode_ptr parentNode = currentNode->contents->getNodeByName("..");

	do{
		if (parentNode != currentNode)
		{
			pathName.insert(0, parentNode->contents->getNameOfNode(cwd));
			currentNode = parentNode;
			cwd = currentNode;
			parentNode = currentNode->contents->getNodeByName("..");
		}

		pathName.insert(0, "/");

	} while (parentNode != currentNode);

	cwd = thisNode;

	return pathName;
}

// With given path, it will find the corresponding NODE containing FOLDER type contents ONLY
// if client want to find a file inside a folder, this is how to use this function to get the parent folder of the file:
// "/fd1/fd2/fl1" --> input to getTargetNode should be: "/fd/f2"
// if path starts with "/", it will start searching from root.
// otherwise, it will start the search from cwd.
// empty path will return cwd immediately.
inode_ptr inode_state::getTargetNode(const string& path) {

	DEBUGF ('i', "path = " << path);
	DEBUGF ('i', "path size = " << path.length());

	inode_ptr targetNode = cwd;

	// see if any path is specified
	// if no path is specified, we should return cwd
	if (path.length() > 0)
	{
		// if this is true, we have to search from root node
		if (path[0] == '/')
		{
			targetNode = root;
		}

		// tokenize path by delimiter '/'
		wordvec pathTokens = split(path, "/");

		for (unsigned int i = 0; i < pathTokens.size(); ++i)
		{
			DEBUGF ('i', "path token = " << pathTokens[i]);

			string fdName = pathTokens[i];

			inode_ptr nextNode = targetNode->contents->getNodeByName(fdName);

			if (nextNode != nullptr)
			{
				targetNode = nextNode;
			}
			else
			{
				throw file_error (path+" does not exist!");
			}
		}
	}

	return targetNode;
}

vector<string> inode_state::getLS(const string& path) {
	vector<string> result;
	this->getTargetNode(path)->getLS(path, result);
	return result;
}

vector<string> inode_state::getLSR(const string& path){
	vector<string> result;
	this->getTargetNode(path)->getLSR_inode(path, result);
	return result;
}

const string& inode_state::prompt() { return prompt_; }

void inode_state::setPrompt(string newPrompt)
{
	prompt_ = newPrompt;

}

void inode_state::mkdir(const string& path)
{
	size_t found = path.find("/");
	inode_ptr targetFolder;
	string folderName = "";
	if (found == string::npos)
	{
		targetFolder = cwd;
		folderName = path;
	}
	else
	{
		size_t found2 = path.find_last_of("/");
		string path_dirOnly = path.substr(0, found2);
		folderName = path.substr(found2 + 1);
		targetFolder = getTargetNode(path_dirOnly);
	}

	targetFolder->mkDir(folderName);
}

void inode_state::make(const string& path, const wordvec& newdata)
{
	size_t found = path.find("/");
	inode_ptr targetFolder;
	string fileName = "";
	if (found == string::npos)
	{
		targetFolder = cwd;
		fileName = path;
	}
	else
	{
		size_t found2 = path.find_last_of("/");
		string path_dirOnly = path.substr(0, found2);
		fileName = path.substr(found2 + 1);
		targetFolder = getTargetNode(path_dirOnly);
	}

	targetFolder->mkFile(fileName, newdata);
}

void inode_state::cat(const string& path)
{
	size_t found = path.find("/");
	inode_ptr targetFolder;
	string fileName = "";
	if (found == string::npos)
	{
		targetFolder = cwd;
		fileName = path;
	}
	else
	{
		size_t found2 = path.find_last_of("/");
		string path_dirOnly = path.substr(0, found2);
		fileName = path.substr(found2 + 1);
		targetFolder = getTargetNode(path_dirOnly);
	}

	targetFolder->catenate(fileName);
}

void inode_state::rm(const string& path)
{
	size_t found = path.find("/");
	inode_ptr targetFolder;
	string fileName = "";
	if (found == string::npos)
	{
		targetFolder = cwd;
		fileName = path;
	}
	else
	{
		size_t found2 = path.find_last_of("/");
		string path_dirOnly = path.substr(0, found2);
		fileName = path.substr(found2 + 1);
		targetFolder = getTargetNode(path_dirOnly);
	}

	targetFolder->remove(fileName);
}

void inode_state::rmr(const string& path)
{
	size_t found = path.find("/");
	inode_ptr targetFolder;
	string fileName = "";
	if (found == string::npos)
	{
		targetFolder = cwd;
		fileName = path;
	}
	else
	{
		size_t found2 = path.find_last_of("/");
		string path_dirOnly = path.substr(0, found2);
		fileName = path.substr(found2 + 1);
		targetFolder = getTargetNode(path_dirOnly);
	}

	targetFolder->rmr_inode(fileName);
}

void inode_state::cd(const string& path)
{
	cwd = getTargetNode(path);
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
	}
	else
	{
		currentFolder = path;
	}

	contents->getLS(currentFolder, result);
}

void inode::getLSR_inode(string path, vector<string>& result)
{
	vector<string> lsInfo;
	string currentFolder = "";

	if(path.empty())
	{
		currentFolder = "/";
	}
	else
	{
		currentFolder = path;
	}

	contents->getLSR_dir(currentFolder, result);
}

void inode::mkDir(const string& folderName) {

	inode_ptr newNode = this->contents->mkdir(folderName);

	if(newNode != nullptr)
	{
		newNode->contents->setParentNode(this->contents->getNodeByName("."));
		newNode->contents->setSelfNode(newNode);
	}
}

void inode::mkFile(const string& fileName, const wordvec& newdata)
{
	inode_ptr newFile = this->contents->mkfile(fileName);
	newFile->contents->writefile(newdata);
}


void inode::catenate(const string& fileName)
{
	inode_ptr targetFile = this->contents->fn_catenate(fileName);
	wordvec data = targetFile->contents->readfile();
	for (auto it = data.begin(); it != data.end(); ++it)
	{
		cout << *it;
	}
	cout << '\n';
}

void inode::remove(const string& fileName)
{
	this->contents->remove(fileName);
}

void inode::rmr_inode(const string &fileName)
{
	this->contents->rmr_dir(fileName);
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
	this->data = words;
}

void plain_file::remove (const string&) {
   throw file_error ("is a plain file");
}

void plain_file::rmr_dir(const string&){
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
	throw file_error ("is a plain file");
}

void plain_file::getLSR_dir(const string&, vector<string>&){
	// it is a no-op for plain file
	throw file_error ("is a plain file");
}

void plain_file::setSelfNode(inode_ptr) {
}

void plain_file::setParentNode(inode_ptr) {

}

inode_ptr plain_file::fn_catenate(const string&) {
	throw file_error ("is a plain file");
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

	selfNode = current;
}
void directory::setParentNode(inode_ptr parent) {

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

	bool is_file_already_present = false;

	// check if name already exists
	is_file_already_present = dirents.find(filename) != dirents.end() ? true : false;

	if (is_file_already_present)
	{
		inode_ptr existingFile = dirents[filename];
		if (existingFile->getContentType() == file_type::DIRECTORY_TYPE)
		{
			size_t size_existingItem = existingFile->getContentSize();
			if (size_existingItem > 2)
			{
				throw file_error (filename+" is not an empty directory");
			}
		}
		dirents.erase(filename);

	}
	else
	{
		throw file_error (filename+" is not a valid file/directory");
	}
}

void directory::rmr_dir(const string &filename)
{
	DEBUGF ('i', filename);

	bool is_file_already_present = false;

	// check if name already exists
	is_file_already_present = dirents.find(filename) != dirents.end() ? true : false;

	if (is_file_already_present)
	{
		dirents.erase(filename);
	}
	else
	{
		throw file_error (filename+" is not a valid file/directory");
	}
}

inode_ptr directory::mkdir (const string& dirname) {
   DEBUGF ('i', dirname);

	bool is_dir_already_present = false;

	// check if dirname already exists
	is_dir_already_present = dirents.find(dirname) != dirents.end() ? true : false;

	if (is_dir_already_present)
	{
		throw file_error (dirname+" already exists");
	}

	inode_ptr newNode = make_shared<inode>(file_type::DIRECTORY_TYPE);
	dirents[dirname] = newNode;

   return newNode;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);

	bool is_file_already_present = false;

	// check if filename already exists
	is_file_already_present = dirents.find(filename) != dirents.end() ? true : false;

	if (is_file_already_present)
	{
		inode_ptr existingFile = dirents[filename];
		if (existingFile->getContentType() != file_type::PLAIN_TYPE)
		{
			throw file_error ("is a directory");
		}

		return existingFile;
	}

	inode_ptr newFile = make_shared<inode>(file_type::PLAIN_TYPE);
	dirents[filename] = newFile;

   return newFile;
}

string directory::getNameOfNode(inode_ptr node) {
	 for (map<string,inode_ptr>::iterator it=dirents.begin(); it!=dirents.end(); ++it)
	 {
		 if (it->second == node)
		 {
			 return it->first;
		 }
	 }

	 return "";
}

inode_ptr directory::getNodeByName(const string& nodeName) {
	if (nodeName == "..")
	{
		return parentNode.lock();
	}

	if (nodeName == ".")
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
		result += "/";
	}

	result += "\t";
}


// for each directory, we will append a string with the following format (each field is separated by a space)
// [folderName] [nodeNumber] [size] [elementName] [nodeNumber] [size] [elementName]
// "     /            1         2          .            1         2         ..    "
void directory::getLS(const string& currentFolderName, vector<string>& result) {

	 static const string SP = " ";

	 string ls = "";
	 ls += currentFolderName;
	 ls += "\t";

	 inode_ptr me = selfNode.lock();
	 inode_ptr myParent = parentNode.lock();
	constructLSInfo(".", SP, me, ls);
	constructLSInfo("..", SP, myParent, ls);

	 for (map<string,inode_ptr>::iterator it=dirents.begin(); it!=dirents.end(); ++it)
	 {
		 constructLSInfo(it->first, SP, it->second, ls);
	 }
	 result.push_back(ls);
}

void directory::getLSR_dir(const string &currentFolderName, vector<string> &result){

	static const string SP = " ";

	string ls = "";
	ls += currentFolderName;
	ls += "\t";

	inode_ptr me = selfNode.lock();
	inode_ptr myParent = parentNode.lock();

	constructLSInfo(".", SP, me, ls);
	constructLSInfo("..", SP, myParent, ls);

	queue<inode_ptr> dirQueue;
	queue<string> dirNameQueue;
	for (map<string,inode_ptr>::iterator it=dirents.begin(); it!=dirents.end(); ++it)
	{
		inode_ptr childNode = it->second;
		constructLSInfo(it->first, SP, childNode, ls);
		file_type fileType = childNode->getContentType();
		if (fileType == file_type::DIRECTORY_TYPE)
		{
			dirQueue.push(childNode);
			dirNameQueue.push(it->first);
		}
	}
	result.push_back(ls);

	while (dirQueue.size() != 0)
	{
		inode_ptr nextDir = dirQueue.front();
		dirQueue.pop();
		string nextDirName;
		nextDirName += currentFolderName;
		if (currentFolderName != "/")
		{
			nextDirName += "/";
		}
		nextDirName += dirNameQueue.front();
		dirNameQueue.pop();
		nextDir->getLSR_inode(nextDirName, result);
	}
}


bool directory::shouldAppendSlash(const string& folderName, inode_ptr folderNode) {
	if (folderName == "." or folderName == "..")
	{
		return false;
	}

	if (folderNode->getContentType() == file_type::DIRECTORY_TYPE)
	{
		return true;
	}

	return false;
}

inode_ptr directory::fn_catenate(const string& fileName)
{
	bool is_file_already_present = false;

	// check if filename already exists.
	is_file_already_present = dirents.find(fileName) != dirents.end() ? true : false;

	if (is_file_already_present)
	{
		inode_ptr existingFile = dirents[fileName];
		if (existingFile->getContentType() != file_type::PLAIN_TYPE)
		{
			throw file_error ("is a directory");
		}

		return existingFile;
	}
	else
	{
		// Required Format: "cat: food: No such file or directory"
		throw file_error ("cat: No such file or directory");
	}
}
