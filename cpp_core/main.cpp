#include "ttwFunctions.h"
#include "ttwFileFunctions.h"
#include "ttwCustomFunctions.h"
#include "ttwClasses.h"
#include "ttwDeclarations.h"

int main(int argc, char *argv[]){

fileInformations fileInfos;
vector<tagClass> containerTags{};
vector<lineClass> containerLines{};
vector<footNoteClass> footnoteAdressContainer{};
documentSectionsClass documentSections;
vector<string> articleFile{};
    
    
    //Process arguments__________________________________________
    
    while(nextRunIsSet) {

    //Get parameter entered in the command line when starting ...
    string parameter;
    vector<string> parameterVector;
    int terminationFlag=1;
    
    if(firstRun==true)  {
	   int i=1;
       while(argv[i]){
            parameter=argv[i++];
            parameterVector.push_back(parameter);
            parameter.clear();
        }
    }

    //... and on the second run via console input...
    if(firstRun==false){
		cout << "Please enter one or more of the following parameters:" << endl;
		show_options();

        parameterVector.clear();
        string input;
        
		getline(cin, input);
        parameterVector=identifyParameters(input);

     }
	
    //Process parameters and set functions...
    terminationFlag=processParameters(parameterVector, fileInfos);

	if(terminationFlag==0){
        cout << "\n";
        cout << "Application was stopped, please restart" << endl;
        return 0;
    }


	//Start__________________________________________
    
	console_print("\nTagTool_WiZArD is starting...");
    
   	fileInfos.set_lapCounter();

    size_t numberOfLines;
    
	if(firstRun==true){
        				
		fileInfos.fileNameArticleFile_ = fileInfos.fileNameSourceFile_;
		
        if(callFromWebSelected==true){
			fileInfos.set_tempPath();
		}
		
		if(callFromPythonSelected==true){
			fileInfos.set_projectPath();
		}
		articleFile=loadFileContent(fileInfos.fileNameArticleFile_);
            
        load_resources(fileInfos);
         
        firstRun=false;
        
    }

    analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);

	//Custom function: Apply functions on DAI citation style?____________________________________________________
	if(applyCitationStyleSelected==true){
	
		apply_citation_style_hyphens(articleFile, documentSections);
		
		//After alterating the file analyze again
    	analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
    	console_print("DAI citation style features applied successfully...");
		applyCitationStyleDone=true;
	}	

    //Set author year tags?__________________________________________________________________
    if (authorYearTagsSelected==true && authorYearTagsSet==false){

        //Get bibliography/author year list...

        vector<authorYearListClass> authorYearList;

        authorYearList=load_value_list(fileInfos.fileNameAuthorYearList_, fileInfos.ttwRootPath_, authorYearList);
             
        if(htmlSelected==true){
		set_authorYearTags(articleFile, authorYearList, documentSections);	
		}
		
		if(htmlSelected==false){
	    set_authorYearTags_XML(articleFile, authorYearList, documentSections, 
		footnoteAdressContainer, containerTags, containerLines);		
		}
		        
        //After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
        console_print("Author year tags set successfully...");
        authorYearTagsSet=true;
    }

    //Set customized journal Tags?______________________________________
    if(customBodyTagsSelected==true && customBodyTagsSet==false){

        //Insert text body tags.....

        numberOfLines=articleFile.size();

        int posA=0;
        int posB=0;
        int y=0;

        numberOfLines=articleFile.size();

        for(size_t i=0; i<numberOfLines-1; i++){
            
            if(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "paragraphBegin" 
			&& containerLines.at(i).lineCategory_!="noTextParagraph" 
			&&	i<documentSections.lineNrTextEnd_) {
			   		
					posA=containerLines.at(i).tagContainerLine_.at(0).addressTagBegin_;
                	posB=containerLines.at(i).tagContainerLine_.at(0).addressTagEnd_;
				
               		articleFile.at(i) = set_custom_bodyTag(articleFile.at(i), posA, posB);

            }
        }
        
	
		//After alterating the file analyze again
       	analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);

		//Insert headlines tags
        set_custom_HeadlineTags(articleFile, containerLines, documentSections, containerTags, footnoteAdressContainer);
        
		    
        //After alterating the file analyze again
       	analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
				        
        //Convert ordered and unordered lists
        convert_lists(articleFile, containerLines, documentSections);
                        
        //After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
		        
        //Convert italic passages
        convert_selected_tags("emphasizedBegin", articleFile, containerLines, documentSections);
       	analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
        
        //Convert bold passages
        convert_selected_tags("strongBegin", articleFile, containerLines, documentSections);
      	analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
        
        //Convert subscript passages
        convert_selected_tags("subscriptBegin", articleFile, containerLines, documentSections);
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
      
       	console_print("Custom journal body tags set successfully...");
        customBodyTagsSet=true;

    }

    //Set paragraph numbers?________________________________________
    if(paragraphNumbersSelected==true && paragraphNumbersSet==false){

            int paragrNumber = 0;
            int* pointerParagrNumber;
            pointerParagrNumber=&paragrNumber;

            int posA=0;
            int posB=0;

            numberOfLines=articleFile.size();

            for(size_t i=0; i<numberOfLines-1; i++){
                
                if(containerLines.at(i).toBeNumbered==true) {
                    posA=containerLines.at(i).tagContainerLine_.at(0).addressTagBegin_;
                    posB=containerLines.at(i).tagContainerLine_.at(0).addressTagEnd_;
                    articleFile.at(i) = set_paragraphNumbers(articleFile.at(i), posA, posB, newParagraphTag, paragraphNumberTagBegin, paragraphNumberTagEnd, pointerParagrNumber);
                }
            }


            //After alterating the file analyze again
            analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
            console_print("Paragraph numbers set successfully...");
            paragraphNumbersSet=true;

    }


    //Set figure references tags?_______________
    if(figureReferenceTagsSelected==true && figureReferenceTagsSet==false){

        set_figureReferencesTags(articleFile);

        //After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
        numberOfLines=articleFile.size();
        console_print("Figure references tags set successfully...");
        figureReferenceTagsSet=true;

    }
    
    
    //insert tagged illustration credits value list?______________________________________________
	if(insertCreditListSelected==true && imageContainerInserted==false){
		
	    insert_image_credit_list(articleFile, fileInfos, documentSections);
		
		//After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);	
		console_print("Illustration credit tags set successfully...");	
		figureReferenceTagsSet=true;
	}	
    
	

    //Convert footnotes to MS WORD or xml_____________________________________________________________
    if(footnoteTagsSet==false && documentSections.lineNrFootnotesBeginIsSet==true){
		
		//Make sure that footnote section doesn´t contain line breaks
		check_footnotes_for_manual_paragraph_marks(articleFile, documentSections);
		
		//After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
		
		insert_FootnoteTags(articleFile, footnoteAdressContainer, documentSections);
	
        //After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
        console_print("Footnotes converted successfully...");
        footnoteTagsSet=true;
    }


	//After having the footnotes converted, convert superscript passages...
	//(needs to be done after converting the footnotes and the <sup>-tags)
	if(customBodyTagsSelected==true && superscriptTagsSet==false){
		
		convert_selected_tags("superscriptBegin", articleFile, containerLines, documentSections);
		
		console_print("Superscript passages converted successfully...");
		superscriptTagsSet=true;
		
		//After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
	}
	
	//Additional search an replace____________________________________
	if(toSearchAndReplaceSelected==true && searchAndReplaceDone==false){
		vector<reducedValueClass> values;
		
		values=load_reduced_value_list(fileInfos.fileNameToSearchAndReplaceList_, fileInfos.ttwRootPath_, values);
				
		callFromAddSearchReplace=true;		
		
		search_replace(articleFile, values);
			
		searchAndReplaceDone=true;
		callFromAddSearchReplace=false;
		
		console_print("Additional search and replace done...");
		
		//After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
       		
	}
	
	
    //Prepare to write file an folder target names and save article and ___________________________________
    
    create_target_file_and_folder_names(fileInfos);
    
    if(customBodyTagsSet==true){
		
		if(htmlSelected==true){
			replace_HtmlHead(articleFile, fileInfos, documentSections);
        	htmlHeadReplaced=true;
        
        	search_replace(articleFile, fileInfos.toReplaceInHtmlHead_, fileInfos.newFileNameFor_);	
        
        	//After alterating the file analyze again        
        	analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
        
        	insert_metadataTemplates(articleFile, fileInfos, documentSections);
        	
        	//After alterating the file analyze again        
        	analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
        
		}
        
        if(htmlSelected==false){
        	
			structure_xml_output_file(articleFile, fileInfos, documentSections);
									
			articleFile=remove_blankLines(articleFile);
			
			//After alterating the file analyze again        
        	analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);	
        	
        	//custom function for tagebuecher project
        	xml_preparation_tagebuecher (articleFile, documentSections);
        	
        	//After alterating the file analyze again        
        	analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);	
        	

				
		}
    }

	
	saveFile(articleFile, fileInfos);
    
    write_resources(fileInfos);
    	
	
    //Success report and resetting the tool for next run

    console_print("\n+ + + + + + + + + + + + + + + + + + + +");
    console_print("+     Article successfully edited!    +");
    console_print("+ + + + + + + + + + + + + + + + + + + +\n");

    customBodyTagsSelected=false;
    figureReferenceTagsSelected=false;
    authorYearTagsSelected=false;
    paragraphNumbersSelected=false;
	reducedCreditListSelected=false; 
	figureReferenceTagsSelected=false;
	
	// Suppressed this dialogue until ttw is prepared better for re-editing the same content multiple times
	/*
	char input;
	if(silentModeSelected==false){	
		cout << "\n";
	    cout << "Do you wish further alterations (y/n)? ";
	    
	    cin >> input;
	    cin.ignore(1,'\n');
	    cout << "\n";
	
	    if(input=='n' || input=='N'){
	    nextRunIsSet=false;
	    }
	}
	if(silentModeSelected==true){
		nextRunIsSet=false;
	}
	*/
	nextRunIsSet=false;
}
	
    console_print("TagTool_WiZArD terminated successfully");

    return 0;
}
