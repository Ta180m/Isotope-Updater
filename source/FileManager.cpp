// Isotope Updater
// Copyright (C) 2020 Nichole Mattera
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <SimpleIniParser.hpp>
#include <string.h>
#include <switch.h>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>

#include "FileManager.hpp"

#include "ConfigManager.hpp"

using namespace simpleIniParser;
using namespace std;
namespace fs = std::filesystem;

namespace dsu
{
    vector<char> FileManager::readFile(string path)
    {
        ifstream file;
        file.open(path, ios::in | ios::binary | ios::ate);

        auto size = file.tellg();
        file.seekg(0, ios::beg);

        vector<char> buffer(size);
        file.read(buffer.data(), size);
        file.close();

        return buffer;
    }

    // Overwrites a file if it already exists!
    bool FileManager::writeFile(string filename, string data)
    {
        deleteFile(filename);

        FILE *file = fopen(filename.c_str(), "wb");
        if (!file)
        {
            return false;
        }

        size_t result = fwrite(data.c_str(), sizeof(char), data.size(), file);

        fflush(file);
        fsync(fileno(file));
        fclose(file);

        return (result == data.size());
    }

    bool FileManager::appendFile(string filename, string data)
    {
        ofstream file;
        // Make sure we are only adding, not overwriting.
        file.open(filename, ios_base::app);
        
        if(!file.is_open()) return false;

        file << data;

        file.close();
        return true;
    }

    bool FileManager::deleteFile(string filename)
    {
        if (fs::exists(filename))
        {
            return remove(filename.c_str()) == 0;
        }

        return false;
    }

    bool FileManager::fileExists(string filename)
    {
        if(fs::exists(filename)) return true;
        return false;
    }

    // This scans all FILES in the given directory and it's respective subdirs.
    vector<string> FileManager::scanDirectoryRecursive(string path)
    {
        vector<string> files;

        // First check if the dir even exists.
        if(fs::exists(path))
        {   
            // Then make sure it's actually a directory, and not a file. All before
            // iterating all of the files in the directory.
            if(fs::is_directory(path))
            {
                for (auto ft : fs::recursive_directory_iterator(path))
                {
                    if(ft.is_directory()) continue;
                    string path = ft.path().string();
                    
                    files.push_back(path);
                }
            } 
        }
        return files;
    } 

    // http://stackoverflow.com/a/11366985
    bool FileManager::createSubfolder(string path)
    {
        bool bSuccess = false;
        int nRC = ::mkdir(path.c_str(), 0775);
        if (nRC == -1)
        {
            switch (errno)
            {
            case ENOENT:
                //parent didn't exist, try to create it
                if (createSubfolder(path.substr(0, path.find_last_of('/'))))
                    //Now, try to create again.
                    bSuccess = 0 == ::mkdir(path.c_str(), 0775);
                else
                    bSuccess = false;
                break;
            case EEXIST:
                //Done!
                bSuccess = true;
                break;
            default:
                bSuccess = false;
                break;
            }
        }
        else
            bSuccess = true;

        return bSuccess;
    }

    bool FileManager::extract(string zipFilename, string destination)
    {
        unzFile unz = unzOpen(zipFilename.c_str());

        vector<string> filesToIgnore = ConfigManager::getFilesToIgnore();
        vector<string> filesInstalled = ConfigManager::getInstalledFiles();

        int i = 0;
        for (;;)
        {
            int code;
            if (i == 0)
            {
                code = unzGoToFirstFile(unz);
            }
            else
            {
                code = unzGoToNextFile(unz);
            }
            i++;

            if (code == UNZ_END_OF_LIST_OF_FILE)
            {
                break;
            }
            else
            {
                unz_file_pos pos;
                unzGetFilePos(unz, &pos);
            }

            unz_file_info_s *fileInfo = _getFileInfo(unz);

            string filename = destination;
            filename += _getFullFileName(unz, fileInfo);

            if (find(begin(filesToIgnore), end(filesToIgnore), filename) != end(filesToIgnore))
            {
                free(fileInfo);
                continue;
            }

            // No need to extract Hekate's payload.
            if (filename.compare(0, 12, "sdmc:/hekate") == 0 && filename.compare(filename.length() - 4, 4, ".bin") == 0)
            {
                free(fileInfo);
                continue;
            }

            // No need to extract Isotope Updater.
            if (filename.compare(0, 29, "sdmc:/switch/Isotope-Updater/") == 0)
            {
                free(fileInfo);
                continue;
            }

            if (filename.back() != '/')
            {
                filesInstalled.push_back(filename);

                int result = _extractFile(filename.c_str(), unz, fileInfo);
                if (result < 0)
                {
                    free(fileInfo);
                    unzClose(unz);
                    return false;
                }
            }

            free(fileInfo);
        }

        if (i <= 0)
        {
            unzClose(unz);
            return false;
        }

        ConfigManager::setInstalledFiles(filesInstalled);
        unzClose(unz);
        return true;
    }

    void FileManager::cleanUpFiles()
    {
        vector<string> installedFiles = ConfigManager::getInstalledFiles();
        vector<string> filesToIgnore = ConfigManager::getFilesToIgnore();

        for (auto const &fileName : installedFiles)
        {
            if (find(begin(filesToIgnore), end(filesToIgnore), fileName) != end(filesToIgnore))
            {
                continue;
            }

            deleteFile(fileName);
        }

        vector<string> blankVector;
        ConfigManager::setInstalledFiles(blankVector);
    }

    void FileManager::applyNoGC()
    {
        Ini *ini = Ini::parseFile(HEKATE_FILE);
        for (auto const &section : ini->sections)
        {
            if (section->type != IniSectionType::Section)
                continue;

            if (section->value == "config")
            {
                bool patchApplied = false;
                for (auto const &option : section->options)
                {
                    if (option->key == "autonogc")
                    {
                        option->value = "1";
                        patchApplied = true;
                        break;
                    }
                }

                if (!patchApplied)
                {
                    section->options.push_back(new IniOption(IniOptionType::Option, "autonogc", "1"));
                }

                break;
            }
        }

        ini->writeToFile(HEKATE_FILE);
        delete ini;
    }

    unz_file_info_s *FileManager::_getFileInfo(unzFile unz)
    {
        unz_file_info_s *fileInfo = (unz_file_info_s *)malloc(sizeof(unz_file_info_s));
        unzGetCurrentFileInfo(unz, fileInfo, NULL, 0, NULL, 0, NULL, 0);
        return fileInfo;
    }

    string FileManager::_getFullFileName(unzFile unz, unz_file_info_s *fileInfo)
    {
        char filePath[fileInfo->size_filename + 1];

        unzGetCurrentFileInfo(unz, fileInfo, filePath, fileInfo->size_filename, NULL, 0, NULL, 0);
        filePath[fileInfo->size_filename] = '\0';

        string path(filePath);
        path.resize(fileInfo->size_filename);

        return path;
    }

    bool FileManager::_makeDirectoryParents(string path)
    {
        bool bSuccess = false;
        int nRC = ::mkdir(path.c_str(), 0775);
        if (nRC == -1)
        {
            switch (errno)
            {
            case ENOENT:
                //parent didn't exist, try to create it
                if (_makeDirectoryParents(path.substr(0, path.find_last_of('/'))))
                    //Now, try to create again.
                    bSuccess = 0 == ::mkdir(path.c_str(), 0775);
                else
                    bSuccess = false;
                break;
            case EEXIST:
                //Done!
                bSuccess = true;
                break;
            default:
                bSuccess = false;
                break;
            }
        }
        else
            bSuccess = true;

        return bSuccess;
    }

    int FileManager::_extractFile(const char *path, unzFile unz, unz_file_info_s *fileInfo)
    {
        //check to make sure filepath or fileInfo isnt null
        if (path == NULL || fileInfo == NULL)
            return -1;

        if (unzOpenCurrentFile(unz) != UNZ_OK)
            return -2;

        char folderPath[strlen(path) + 1];
        strcpy(folderPath, path);
        char *pos = strrchr(folderPath, '/');
        if (pos != NULL)
        {
            *pos = '\0';
            _makeDirectoryParents(string(folderPath));
        }

        u32 blocksize = 0x8000;
        u8 *buffer = (u8 *)malloc(blocksize);
        if (buffer == NULL)
            return -3;
        u32 done = 0;
        int writeBytes = 0;
        FILE *fp = fopen(path, "w");
        if (fp == NULL)
        {
            free(buffer);
            return -4;
        }

        while (done < fileInfo->uncompressed_size)
        {
            if (done + blocksize > fileInfo->uncompressed_size)
            {
                blocksize = fileInfo->uncompressed_size - done;
            }
            unzReadCurrentFile(unz, buffer, blocksize);
            writeBytes = write(fileno(fp), buffer, blocksize);
            if (writeBytes <= 0)
            {
                break;
            }
            done += writeBytes;
        }

        fflush(fp);
        fsync(fileno(fp));
        fclose(fp);

        free(buffer);
        if (done != fileInfo->uncompressed_size)
            return -4;

        unzCloseCurrentFile(unz);
        return 0;
    }
} // namespace dsu
