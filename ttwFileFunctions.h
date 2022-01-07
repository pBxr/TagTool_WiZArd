#pragma once
#ifndef TTWFILEFUNCTIONS_H
#define TTWFILEFUNCTIONS_H

#include "ttwClasses.h"
#include "ttwDeclarations.h"


namespace fs = std::filesystem;

void create_target_file_and_folder_names(fileInformations &fileInfo) {

    fileInfo.fileNameArticleFile_ = fileInfo.fileNameSourceFile_;

    int pos1;
    pos1=fileInfo.fileNameArticleFile_.find(".html");
    string counter = std::to_string(fileInfo.lapCounter_);
    string suffix = "_edited_"+counter+"_.html";

    fileInfo.fileNameArticleFile_.replace(pos1, 5, suffix);
    
       
    //for ressources folder...
    
    int pos2;

	string appendix;
	
	appendix = fileInfos.fileNameArticleFile_;
	
	pos2=appendix.find(".");
    appendix.erase(pos2, 5);
    appendix = appendix + "_ress";
    
    fileInfos.newFileNameForRessources_= appendix;
    	
	fileInfos.folderWritingRessources_ = fileInfos.workingPath_ + "/" + appendix;

    
}

string get_current_path(fileInformations &fileInfo){
	
	fs::path workingPathTemp;
	
	workingPathTemp = fs::current_path();
	
	fileInfos.workingPath_= workingPathTemp.string();
	
	//Convert slashes 
	
	int pos;
	
	pos=fileInfos.workingPath_.find("\\");

	while(pos>=0){
	fileInfos.workingPath_.replace(pos, 1, "/");
	pos=fileInfos.workingPath_.find("\\");
	}
	
}

vector<authorYearListClass> load_authorYearList(string fileName, vector<authorYearListClass> authorYearList){

    vector<string> RefListFile;

    RefListFile=loadFileContent(fileName);

    size_t numberOfLines;
    int pos1=0;
    int pos2=0;
    int index=0;
    int posA=0;
    int posB=0;
    int counter=0;
    
    string toCheck;
    string authorYear;
    string fullCitation;
    string zenonID;

    numberOfLines=RefListFile.size();

    string::iterator endLine;
    string iteratorContent;
    bool rowOneDetected=false;
    
    for(size_t i=0; i<numberOfLines-1; i++){

        //Erase spaces...

        toCheck=RefListFile[i][0];

        if(toCheck==" "){
        while(toCheck==" "){
        RefListFile[i].erase(0,1);
        toCheck=RefListFile[i][0];
        }

        }

        posA=RefListFile[i].find(" %");

        while(posA>=0){
            RefListFile[i].replace(posA, 2, "%");
            posA=RefListFile[i].find(" %");
            }

        posB=RefListFile[i].find("% ");

        while(posB>=0){
            RefListFile[i].replace(posB, 2, "%");
            posB=RefListFile[i].find("% ");
            }

        endLine=RefListFile[i].end();

        for(string::iterator it=RefListFile[i].begin(); it!=endLine; ++it){

            iteratorContent = *it;

            if(iteratorContent=="%"){
                counter++;
				if(rowOneDetected==false){
                    pos1=index;
                    rowOneDetected=true;
                }

                if(rowOneDetected==true){
                    pos2=index;
                }
            }

            index++;
        }

		if(counter==1){
			cout << "\nWARNING: A component of the list of references is missing. Follwing dummy entry was created: \"###_CHECK_ENTRY_###" << endl;
			pos2=RefListFile[i].size()-1;
			RefListFile[i].pop_back();
			RefListFile[i] = RefListFile[i] + "%###_CHECK_ENTRY_###";
			cout << RefListFile[i] << "\n" <<  endl;
		}

        for(int x=0; x<pos1; x++){
            authorYear=authorYear+RefListFile[i][x];

        }

        for(int y=(pos1+1); y<pos2; y++){
            fullCitation=fullCitation+RefListFile[i][y];
        }

        for(size_t z=(pos2+1); z<RefListFile[i].size(); z++){
            zenonID=zenonID+RefListFile[i][z];
        }

        authorYearList.push_back( authorYearListClass { i, authorYear, fullCitation, zenonID });

        authorYear.clear();
        fullCitation.clear();
        zenonID.clear();
        index=0;
        pos1=0;
        pos2=0;
        counter=0;
        rowOneDetected=false;
       
    }

    return authorYearList;

}

vector<string> loadFileContent(string fileName){

vector<string> workingFile_;
string inputLine;

    ifstream fileIn(fileName);
    if(!fileIn){
        std::cerr << "Error opening " << fileName << ". Check filename or path" << endl;
        }
    while(fileIn) {
       getline(fileIn, inputLine);
       workingFile_.push_back(inputLine+"\n");
       inputLine.clear();
    }

fileIn.close();

return workingFile_;

}

void load_ressources(){
	
	fileInfos.fileNameTemplMetadBegin_= fileInfos.workingPath_ + "/ressources/" + "MetadataTextBegin.txt";
 	fileInfos.fileNameTemplMetadEnd_= fileInfos.workingPath_ + "/ressources/" + "MetadataTextEnd.txt";
	fileInfos.fileNameNewHtmlHead_ = fileInfos.workingPath_ + "/ressources/" +  "New_Html_Head.html";
	fileInfos.fileNameColorschememapping_ = fileInfos.workingPath_ + "/ressources/" +  "colorschememapping.xml";
	fileInfos.fileNameFilelist_ = fileInfos.workingPath_ + "/ressources/" + "filelist.xml";
	fileInfos.fileNameHeader_ = fileInfos.workingPath_ + "/ressources/" + "header.html";
	fileInfos.fileNameItem0001_ = fileInfos.workingPath_ + "/ressources/" + "item0001.xml";
	
	
	fileInfos.metadataBegin = loadFileContent(fileInfos.fileNameTemplMetadBegin_);
	fileInfos.metadataEnd = loadFileContent(fileInfos.fileNameTemplMetadEnd_);
	
	fileInfos.colorschememapping=loadFileContent(fileInfos.fileNameColorschememapping_);
	fileInfos.filelist=loadFileContent(fileInfos.fileNameFilelist_);
	fileInfos.header=loadFileContent(fileInfos.fileNameHeader_);
	fileInfos.item0001=loadFileContent(fileInfos.fileNameItem0001_);	
	
}

void saveFile(vector<string> &articleFile, fileInformations &fileInfos){

     ofstream fileOut(fileInfos.fileNameArticleFile_, std::ios::trunc);
     if(!fileOut){
         std::cerr << "Error writing " << fileInfos.fileNameArticleFile_ << ". Check filename or path"<< endl;
         }

     for(string &p:articleFile) {
     fileOut << p;
     }

     fileOut.close();

     
}







void write_ressources(fileInformations &fileInfos){

	
    search_replace(fileInfos.filelist, fileInfos.toReplaceInRessources_, fileInfos.newFileNameForRessources_);
    search_replace(fileInfos.header, fileInfos.toReplaceInRessources_, fileInfos.newFileNameForRessources_);
    search_replace(fileInfos.item0001, fileInfos.toReplaceInRessources_, fileInfos.newFileNameForRessources_);

    size_t charlength=fileInfos.folderWritingRessources_.size();
    char folderNameRessources_[charlength];
    for(size_t i=0; i<charlength; i++){
        folderNameRessources_[i]=fileInfos.folderWritingRessources_[i];
    }
    folderNameRessources_[charlength]='\0';

    mkdir(folderNameRessources_); 

    //write ressources
    string pathToWrite1=fileInfos.folderWritingRessources_+"/"+"filelist.xml";
    string pathToWrite2=fileInfos.folderWritingRessources_+"/"+"header.html";
    string pathToWrite3=fileInfos.folderWritingRessources_+"/"+"item0001.xml";
    string pathToWrite4=fileInfos.folderWritingRessources_+"/"+"colorschememapping.xml";

    ofstream fileOut1(pathToWrite1);
    for(string &p:fileInfos.filelist) {
    fileOut1 << p << "\n";
    }
    fileOut1.close();

    ofstream fileOut2(pathToWrite2);
    for(string &p:fileInfos.header) {
    fileOut2 << p << "\n";
    }
    fileOut2.close();

    ofstream fileOut3(pathToWrite3);
    for(string &p:fileInfos.item0001) {
    fileOut3 << p << "\n";
    }
    fileOut3.close();

    ofstream fileOut4(pathToWrite4);
    for(string &p:fileInfos.colorschememapping) {
    fileOut4 << p << "\n";
    }
    fileOut4.close();
    
    fileInfos.toReplaceInRessources_ = fileInfos.fileNameArticleFile_;
    
}



#endif // TTWFILEFUNCTIONS_H
