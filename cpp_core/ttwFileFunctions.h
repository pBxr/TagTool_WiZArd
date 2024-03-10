#pragma once
#ifndef TTWFILEFUNCTIONS_H
#define TTWFILEFUNCTIONS_H

#include "ttwClasses.h"
#include "ttwDeclarations.h"


namespace fs = std::filesystem;

void create_target_file_and_folder_names(fileInformations &fileInfos) {

    fileInfos.fileNameArticleFile_ = fileInfos.fileNameSourceFile_;

    int pos1;
    pos1=fileInfos.fileNameArticleFile_.find(".html");
    string counter = std::to_string(fileInfos.lapCounter_);
    string suffix;
	
	if(htmlSelected==true){
		suffix = "_edited_"+counter+"_.html";
		}
	
	if(htmlSelected==false){
		suffix = "_edited_"+counter+"_.xml";
		}	
		

    fileInfos.fileNameArticleFile_.replace(pos1, 5, suffix);
    
       
    //for  folder...
    
    int pos2;

	string appendix;
	
	appendix = fileInfos.fileNameArticleFile_;
	
	pos2=appendix.find(".");
    appendix.erase(pos2, 5);
    appendix = appendix + "_ress";
    
    fileInfos.newFileNameFor_= appendix;
    	
	if(callFromWebSelected==true){
		fileInfos.folderWriting_ = fileInfos.pathTempDirectory_ + "/" + appendix;	
	}
	else if(callFromPythonSelected==true){
		fileInfos.folderWriting_ = appendix;	
	}
	else{
		fileInfos.folderWriting_ = ".\\" + appendix;
	}
	
}
void get_current_path(fileInformations &fileInfos){
	
	fs::path workingPathTemp;
	
	workingPathTemp = fs::current_path();
		
	fileInfos.ttwRootPath_= workingPathTemp.string();
	
	//Convert slashes 
	
	int pos;
	
	pos=fileInfos.ttwRootPath_.find("\\");

	while(pos>=0){
	fileInfos.ttwRootPath_.replace(pos, 1, "/");
	pos=fileInfos.ttwRootPath_.find("\\");
	}

	if(callFromWebSelected == true){
		fileInfos.pathTempDirectory_ = fileInfos.ttwRootPath_ + "/" +fileInfos.nameTempDirectory_ + "/";
		//fileInfos.pathTempDirectory_ = "C:\\xampp\\htdocs\\Ordner_PB\\ttw\\20230416_tempu63v\\";
	}
	
	if(callFromPythonSelected == true){
		fileInfos.pathProjectDirectory_ = fileInfos.nameProjectDirectory_ + fileInfos.fileNameArticleFile_;
		}
				
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

void load_resources(fileInformations& fileInfos){
	
	if(callFromWebSelected==true){
	
		fileInfos.fileNameTemplMetadBegin_= fileInfos.ttwRootPath_ + "/resources/" + "MetadataTextBegin.txt";
 		fileInfos.fileNameTemplMetadEnd_= fileInfos.ttwRootPath_ + "/resources/" + "MetadataTextEnd.txt";
		fileInfos.fileNameNewHtmlHead_ = fileInfos.ttwRootPath_ + "/resources/" + "New_Html_Head.html";
		fileInfos.fileNameNewXMLHead_ = fileInfos.ttwRootPath_ + "/resources/" + "New_XML_Head.xml";
		fileInfos.fileNameColorschememapping_ = fileInfos.ttwRootPath_ + "/resources/" + "colorschememapping.xml";
		fileInfos.fileNameFilelist_ = fileInfos.ttwRootPath_ + "/resources/" + "filelist.xml";
		fileInfos.fileNameHeader_ = fileInfos.ttwRootPath_ + "/resources/" + "header.html";
		fileInfos.fileNameItem0001_ = fileInfos.ttwRootPath_ + "/resources/" + "item0001.xml";
		
		fileInfos.fileNameHTML_XML_valueList_ = fileInfos.ttwRootPath_ + "/resources/" + "html2xmlValueList.csv";
	}
	else{
		fileInfos.fileNameTemplMetadBegin_= fileInfos.fileNameTemplMetadBegin_ + ".\\resources\\" + "MetadataTextBegin.txt";
 		fileInfos.fileNameTemplMetadEnd_= fileInfos.fileNameTemplMetadEnd_ + ".\\resources\\" + "MetadataTextEnd.txt";
		fileInfos.fileNameNewHtmlHead_ = fileInfos.fileNameNewHtmlHead_ + ".\\resources\\" + "New_Html_Head.html";
		fileInfos.fileNameNewXMLHead_ = fileInfos.fileNameNewXMLHead_ + ".\\resources\\" + "New_XML_Head.xml";
		fileInfos.fileNameColorschememapping_ = fileInfos.fileNameColorschememapping_ + ".\\resources\\" + "colorschememapping.xml";
		fileInfos.fileNameFilelist_ = fileInfos.fileNameFilelist_ + ".\\resources\\" + "filelist.xml";
		fileInfos.fileNameHeader_ = fileInfos.fileNameHeader_ + ".\\resources\\" + "header.html";
		fileInfos.fileNameItem0001_ = fileInfos.fileNameItem0001_ + ".\\resources\\" + "item0001.xml";
		
		fileInfos.fileNameHTML_XML_valueList_ = fileInfos.fileNameHTML_XML_valueList_ + ".\\resources\\" + "html2xmlValueList.csv";
	}
	
	fileInfos.metadataBegin = loadFileContent(fileInfos.fileNameTemplMetadBegin_);
	fileInfos.metadataEnd = loadFileContent(fileInfos.fileNameTemplMetadEnd_);
	
	fileInfos.colorschememapping=loadFileContent(fileInfos.fileNameColorschememapping_);
	fileInfos.filelist=loadFileContent(fileInfos.fileNameFilelist_);
	fileInfos.header=loadFileContent(fileInfos.fileNameHeader_);
	fileInfos.item0001=loadFileContent(fileInfos.fileNameItem0001_);
	
}

vector<reducedValueClass> load_reduced_value_list(string fileName, string path, vector<reducedValueClass> valueList){
	
	vector<string> valueListFile;
	
	valueListFile=loadFileContent(fileName);

    size_t numberOfLines;
    
    int posA=0;
    int posB=0;
        
    string toCheck;
    string entry1;
    string entry2;
    
    numberOfLines=valueListFile.size();

    for(size_t i=0; i<numberOfLines-1; i++){

        //Erase spaces...

        toCheck=valueListFile[i][0];

        if(toCheck==" "){
        while(toCheck==" "){
        valueListFile[i].erase(0,1);
        toCheck=valueListFile[i][0];
        }

        }

        posA=valueListFile[i].find(" |");

        while(posA>=0){
            valueListFile[i].replace(posA, 2, "|");
            posA=valueListFile[i].find(" |");
            }

        posB=valueListFile[i].find("| ");

        while(posB>=0){
            valueListFile[i].replace(posB, 2, "|");
            posB=valueListFile[i].find("| ");
            }

        
        
        //Get addresses... 
        
        vector<int> positions;
        
        for(int y=0; y<valueListFile[i].size(); ++y){
            
            string toCheck; 
            toCheck = valueListFile[i][y];
             		
             		
            if(toCheck=="|"){
                positions.push_back(y);
                }
			}
				
		
		for(int x=0; x<positions[0]; x++){
            entry1=entry1+valueListFile[i][x];
			}

        for(size_t zz=(positions[0]+1); zz<valueListFile[i].size(); zz++){
            entry2=entry2+valueListFile[i][zz];
            }
		
		
		if (!entry2.empty() && entry2[entry2.length()-1] == '\n') {
    		entry2.erase(entry2.length()-1);
			}
		
		valueList.push_back(reducedValueClass {entry1, entry2});
			
		entry1.clear();
        entry2.clear();
                
        positions.clear();
         
	}
	
	return valueList;
			
}

vector<authorYearListClass> load_value_list(string fileName, string path, vector<authorYearListClass> authorYearList){

    vector<string> valueListFile;

    valueListFile=loadFileContent(fileName);
    
    size_t numberOfLines;
    
    int posA=0;
    int posB=0;
    vector<int> positions;
         
    string toCheck;
    string entry1;
    string entry2;
    string entry3;

    numberOfLines=valueListFile.size();
    
    for(size_t i=0; i<numberOfLines-1; i++){

        //Erase spaces...

        toCheck=valueListFile[i][0];

        if(toCheck==" "){
        while(toCheck==" "){
        valueListFile[i].erase(0,1);
        toCheck=valueListFile[i][0];
        }

        }

        posA=valueListFile[i].find(" |");

        while(posA>=0){
            valueListFile[i].replace(posA, 2, "|");
            posA=valueListFile[i].find(" |");
            }

        posB=valueListFile[i].find("| ");

        while(posB>=0){
            valueListFile[i].replace(posB, 2, "|");
            posB=valueListFile[i].find("| ");
            }

        //Get addresses... 
        
        
        for(int y=0; y<valueListFile[i].size(); ++y){
            
            string toCheck; 
            toCheck = valueListFile[i][y];
             		
             		
            if(toCheck=="|"){
                positions.push_back(y);
                }
			}
		
		
		//Check for missing entries and create dummy entries
		
		int vecSize = positions.size();
				
		
		if(vecSize<2){
		valueListFile[i].pop_back();
		
		cout << "\nWARNING: A component of the list " << fileName << " is missing. Follwing dummy entry was created: \"###_CHECK_ENTRY_###\"\n" << endl;	
		
			for(int y=0; y<(2-vecSize); y++){
			
			valueListFile[i].append("|###_CHECK_ENTRY_###");
			
						
				positions.clear();
				
				for(int y=0; y<valueListFile[i].size(); ++y){
            
            	string toCheck; 
            	toCheck = valueListFile[i][y];
             		if(toCheck=="|"){
                	positions.push_back(y);
                	}
			}
			}
			}

		
				

        for(int x=0; x<positions[0]; x++){
            entry1=entry1+valueListFile[i][x];
            }

        for(int y=(positions[0]+1); y<positions[1]; y++){
            entry2=entry2+valueListFile[i][y];
            }


		for(size_t zz=(positions[1]+1); zz<valueListFile[i].size(); zz++){
            entry3=entry3+valueListFile[i][zz];
            }


        authorYearList.push_back( authorYearListClass { i, entry1, entry2, entry3 });

        entry1.clear();
        entry2.clear();
        entry3.clear();
        
       positions.clear();
    }

    return authorYearList;

}

vector<illustrationCreditsClass> load_value_list(string fileName, string path, vector<illustrationCreditsClass> illustrationCreditList){
	
	vector<string> valueListFile;

    valueListFile=loadFileContent(fileName);
 
    size_t numberOfLines;
    
    int posA=0;
    int posB=0;
        
    string toCheck;
    string entry1;
    string entry2;
    string entry3;
    string entry4;

    numberOfLines=valueListFile.size();

    for(size_t i=0; i<numberOfLines-1; i++){

        //Erase spaces...

        toCheck=valueListFile[i][0];

        if(toCheck==" "){
        	while(toCheck==" "){
        	valueListFile[i].erase(0,1);
        	toCheck=valueListFile[i][0];
        	}

        }

        posA=valueListFile[i].find(" |");

        while(posA>=0){
            valueListFile[i].replace(posA, 2, "|");
            posA=valueListFile[i].find(" |");
            }

        posB=valueListFile[i].find("| ");

        while(posB>=0){
            valueListFile[i].replace(posB, 2, "|");
            posB=valueListFile[i].find("| ");
            }

                
        //Get addresses... 
        
        vector<int> positions;
        for(int y=0; y<valueListFile[i].size(); ++y){
            
            string toCheck; 
            toCheck = valueListFile[i][y];
             		
             		
            if(toCheck=="|"){
                positions.push_back(y);
                }
			}
		
		//Check for missing entries and create dummy entries
		
		int vecSize = positions.size();
		if(vecSize<3){
		valueListFile[i].pop_back();
		
		cout << "\nWARNING: A component of the list " << fileName << " is missing. Follwing dummy entry was created: \"###_CHECK_ENTRY_###\"\n" << endl;	
		
			for(int y=0; y<(3-vecSize); y++){
			
				valueListFile[i].append("|###_CHECK_ENTRY_###");	
				
			}
			
				positions.clear();
				
				for(int y=0; y<valueListFile[i].size(); ++y){
            
            	string toCheck; 
            	toCheck = valueListFile[i][y];
             		if(toCheck=="|"){
                	positions.push_back(y);
                	}
			}
		
		}
		

        for(int x=0; x<positions[0]; x++){
            entry1=entry1+valueListFile[i][x];
			}

        for(int y=(positions[0]+1); y<positions[1]; y++){
            entry2=entry2+valueListFile[i][y];
        	}

        for(size_t z=(positions[1]+1); z<positions[2]; z++){
            entry3=entry3+valueListFile[i][z];
        	}
        
        for(size_t zz=(positions[2]+1); zz<valueListFile[i].size(); zz++){
            entry4=entry4+valueListFile[i][zz];
        }
		
			if (!entry4.empty() && entry4[entry4.length()-1] == '\n') {
    		entry4.erase(entry4.length()-1);
			}
		
        illustrationCreditList.push_back( illustrationCreditsClass { i, entry1, entry2, entry3, entry4 });

        entry1.clear();
        entry2.clear();
        entry3.clear();
        entry4.clear();
        
        positions.clear();
       
    }

    return illustrationCreditList;
	
}

void saveFile(vector<string> &articleFile, fileInformations &fileInfos){
	
	string toSave = fileInfos.fileNameArticleFile_;
	
	if(callFromWebSelected==true){
		toSave = fileInfos.pathTempDirectory_ + fileInfos.fileNameArticleFile_;
	}
	
	if(callFromPythonSelected==true){
		toSave = fileInfos.fileNameArticleFile_;
	}
	ofstream fileOut(toSave, std::ios::trunc);
    if(!fileOut){
        std::cerr << "\n****ERROR WRITING " << fileInfos.fileNameArticleFile_ << ". Check filename or path or if file ist opened****\n"<< endl;
        return;
		}

    for(string &p:articleFile) {
    fileOut << p;
    }

    fileOut.close();

    console_print("Article successfully written...");

}
void write_resources(fileInformations &fileInfos){
	
    search_replace(fileInfos.filelist, fileInfos.toReplaceInResources_, fileInfos.newFileNameFor_);
    search_replace(fileInfos.header, fileInfos.toReplaceInResources_, fileInfos.newFileNameFor_);
    search_replace(fileInfos.item0001, fileInfos.toReplaceInResources_, fileInfos.newFileNameFor_);
         
    size_t charlength=fileInfos.folderWriting_.size();
    char folderName_[charlength];
    for(size_t i=0; i<charlength; i++){
        folderName_[i]=fileInfos.folderWriting_[i];
    }
    folderName_[charlength]='\0';
    
    mkdir(folderName_); 
   
    //write 
    string pathToWrite1=fileInfos.folderWriting_+"/"+"filelist.xml";
    string pathToWrite2=fileInfos.folderWriting_+"/"+"header.html";
    string pathToWrite3=fileInfos.folderWriting_+"/"+"item0001.xml";
    string pathToWrite4=fileInfos.folderWriting_+"/"+"colorschememapping.xml";
    
	ofstream fileOut1(pathToWrite1, std::ios::trunc);
    for(string &p:fileInfos.filelist) {
    fileOut1 << p << "\n";
    }
    fileOut1.close();
	
	ofstream fileOut2(pathToWrite2, std::ios::trunc);
    for(string &p:fileInfos.header) {
    fileOut2 << p << "\n";
    }
    fileOut2.close();

	ofstream fileOut3(pathToWrite3, std::ios::trunc);
    for(string &p:fileInfos.item0001) {
    fileOut3 << p << "\n";
    }
    fileOut3.close();
	
	ofstream fileOut4(pathToWrite4, std::ios::trunc);
    for(string &p:fileInfos.colorschememapping) {
    fileOut4 << p << "\n";
    }
    fileOut4.close();
    
    fileInfos.toReplaceInResources_ = fileInfos.fileNameArticleFile_;
    
}

#endif // TTWFILEFUNCTIONS_H