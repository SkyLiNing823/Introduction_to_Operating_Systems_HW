#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <openssl/sha.h>
using namespace std;

struct fileNode
{
    string path;
    string hash;
    int linkCount;
};

string fileHash(string path)
{
    ifstream ifs(path);
    int SIZE = 4096;
    char buffer[SIZE];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    while (ifs.read(buffer, SIZE))
        SHA256_Update(&sha256, buffer, SIZE);
    SHA256_Update(&sha256, buffer, ifs.gcount());
    SHA256_Final(hash, &sha256);

    string hashStr;
    char hexChar[3];

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(hexChar, "%02x", hash[i]);
        hashStr += hexChar;
    }
    return hashStr;
}

void listFiles(vector<fileNode *> &files, string path)
{
    DIR *dir = opendir(path.c_str());
    dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            if (entry->d_type == DT_DIR)
                listFiles(files, path + "/" + entry->d_name);
            else if (entry->d_type == DT_REG)
            {
                fileNode *node = new fileNode;
                node->path = path + "/" + entry->d_name;
                node->hash = fileHash(node->path);
                node->linkCount = 1;
                files.push_back(node);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char **argv)
{
    (void)argc;
    vector<fileNode *> files;
    listFiles(files, argv[1]);
    for (int i = 0; i < files.size(); i++)
    {
        if (files[i]->linkCount < 1)
            continue;
        for (int j = i + 1; j < files.size(); j++)
        {
            if (strcmp(files[i]->hash.c_str(), files[j]->hash.c_str()) == 0)
            {
                unlink(files[j]->path.c_str());
                link(files[i]->path.c_str(), files[j]->path.c_str());
                files[i]->linkCount++;
                files[j]->linkCount--;
            }
        }
    }

    return 0;
}