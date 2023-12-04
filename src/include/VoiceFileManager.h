/*
 * VoiceFileManager.h
 *
 *  Created on: 2023-04-15
 *      Author: Kacper Stark
 */
#ifndef VFM_H_
#define VFM_H_

#include <filesystem>
#include <vector>
#include <wx/string.h>
#include <random>

namespace fs = std::filesystem;

struct VoiceFileInfo
{
    std::string shortName;
    std::string fullName;
    fs::path path;
    std::string transcription;
};

enum VoiceFileTypes
{
    Logatoms = 1,
    Sentences = 2
};

/// @brief a class that handles managing audio files
class VoiceFileManager
{
public:
    VoiceFileManager(){};
    /// @brief A basic constructor
    /// @param parentPath a directory that contains both logatoms and sentences directories.
    /// @param logatomsDirName name of the directory containing logatoms.
    /// @param sentencesDirName name of the directory containing sentences.
    /// @param fileExtension selected file extension, with a leading dot (e.g.".wav"). Used to list compatible files.
    /// @throws std::invalid_argument, when the directories do not exist in the filesystem.
    VoiceFileManager(std::string parentPath, std::string logatomsDirName = "Logatomy", std::string sentencesDirName = "Zdania", std::string fileExtension = ".wav");

    /// @brief Sets the logatoms directory name and checks for its existence.
    /// @param name name of the directory containing logatoms. Must be available in the parent path.
    /// @return true if the directory exists in parentPath, false otherwise.
    bool setLogatomsDirName(std::string name)
    {
        // check if the directory exists in parentPath
        if (fs::exists(parentPath / name) && fs::is_directory(parentPath / name))
        {
            this->logatomsDirName = name;
            return true;
        }
        return false;
    };


    /// @brief Sets the sentences directory name and checks for its existence.
    /// @param name name of the directory containing logatoms. Must be available in the parent path.
    /// @return true if the directory exists in parentPath, false otherwise.
    bool setSentencesDirName(std::string name)
    {
        // check if the directory exists in parentPath
        if (fs::exists(parentPath / name) && fs::is_directory(parentPath / name))
        {
            this->sentencesDirName = name;
            return true;
        }
        return false;
    };



    /// @brief Sets the file extension used for file listing.
    /// @param newExtension new file extension with a leading dot (e.g. ".wav")
    void setFileExtension(std::string newExtension)
    {
        this->selectedFileExtension = newExtension;
        return;
    }
    bool setParentPath(std::string newPath)
    {
    if (fs::exists(newPath)){
        this->parentPath = newPath;
        return true;
        }
        return false;
    }

    /// @brief Lists the subfolders in parentPath/fileType directory. 
    /// @param FileType
    /// @return a vector containing names of found subdirectories.
    std::vector<wxString> listVoiceTypes(VoiceFileTypes FileType) const;


    /// @brief Lists files in a given directory (selected voiceType)
    /// @param fileType 
    /// @param voiceType  
    /// @return a vector conatining names of found files.
    std::vector<std::string> listFiles(VoiceFileTypes fileType, std::string voiceType) const;

    /// @brief gets random file from the specified path, then saves the information to fileInfo.
    /// @param fileType selected file type
    /// @param voiceType selected voice type
    /// @param fileInfo output
    void getRandomFileInfo(VoiceFileTypes fileType, std::string voiceType, VoiceFileInfo &fileInfo) const;


    /// @brief looks for the file specified by fileInfo in selected directory. If nothing is found, a random file from the directory is selected. 
    /// @param fileType 
    /// @param voiceType 
    /// @param fileInfo 
    void getSelectedFileInfo(VoiceFileTypes fileType, std::string voiceType, VoiceFileInfo &fileInfo) const;

    /// @brief generate a std::filesystem::path for selected file.
    /// @param fileType selected file type.
    /// @param voiceType selected voice type.
    /// @param fileName selected file name.
    /// @param includeParent whether to include the parentPath in output path.
    /// @return path to file.
    fs::path makePath(VoiceFileTypes fileType, std::string voiceType, std::string fileName, bool includeParent = false) const
    {
        fs::path tmp;
        tmp = ((includeParent) ? parentPath : "") / ((fileType == VoiceFileTypes::Logatoms) ? logatomsDirName : sentencesDirName) / voiceType / fileName;
        return tmp;
    }

    /// @brief generate a std::filesystem::path for selected voiceType directory.
    /// @param fileType selected file type.
    /// @param voiceType selected voice type.
    /// @param includeParent whether to include the parentPath in output path.
    /// @return path to directory.
    fs::path makePath(VoiceFileTypes fileType, std::string voiceType, bool includeParent = false) const
    {
        fs::path tmp;
        tmp = ((includeParent) ? parentPath : "") / ((fileType == VoiceFileTypes::Logatoms) ? logatomsDirName : sentencesDirName) / voiceType;
        return tmp;
    }

private:
    std::string logatomsDirName;
    std::string sentencesDirName;
    std::string selectedFileExtension;
    fs::path parentPath;
};

#endif