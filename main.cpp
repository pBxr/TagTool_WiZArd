#include "ttwFunctions.h"
#include "ttwFileFunctions.h"
#include "ttwClasses.h"
#include "ttwDeclarations.h"

int main(int argc, char *argv[]){

    cout << "\n\n********************************************************************************************************\n" << endl;
    cout << "   Welcome to TagTool_WiZArD application (Version 1.0.0)"<< endl;
    cout << "\n\n********************************************************************************************************\n" << endl;

    
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

		show_options();

        parameterVector.clear();
        string input;
        
		getline(cin, input);
        parameterVector=identifyParameters(input);

     }

    //Process parameters and set functions...

    terminationFlag=processParameter(parameterVector);

    if(terminationFlag==0){
        cout << "\n";
        cout << "Application was stopped, please restart" << endl;
        return 0;
    }


	//Start__________________________________________

    cout << "\nTagTool_WiZArD is starting..." << endl;
    
   	fileInfos.set_lapCounter();

    size_t numberOfLines;

    if(firstRun==true){
        
		//Get article and ressources
        fileInfos.fileNameArticleFile_ = fileInfos.fileNameSourceFile_;

        articleFile=loadFileContent(fileInfos.fileNameArticleFile_);
        
        get_current_path(fileInfos);
        
        load_ressources();
        
        firstRun=false;
    }



    analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);


    //Remove dispensable formattings/tags?____________________________________________
    if (removeDispensableTagsSelected==true && dispensableTagsRemoved==false){

        articleFile=remove_disp_formattings(articleFile, containerLines);

        //After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
        cout << "Dispensable formattings/tags successfully removed..." << endl;
        dispensableTagsRemoved=true;
    }


    //Set author year tags?-----------------------------------------------------
    if (authorYearTagsSelected==true && authorYearTagsSet==false){

        //Get bibliography/author year list...

        vector<authorYearListClass> authorYearList;

        authorYearList=load_authorYearList(fileInfos.fileNameAuthorYearList_, authorYearList);

        set_authorYearTags(articleFile, authorYearList, documentSections);

        //After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
        cout << "Author year tags set successfully..." << endl;
        authorYearTagsSet=true;
    }


    //Set customized journal Tags?______________________________________
    if(customBodyTagsSelected==true && customBodyTagsSet==false){

        //Text-Body-Tags einfÃ¼gen.....

        numberOfLines=articleFile.size();

        int posA=0;
        int posB=0;
        int y=0;

        numberOfLines=articleFile.size();

        for(size_t i=0; i<numberOfLines-1; i++){
            
            if(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "paragraphBegin" && containerLines.at(i).lineCategory_!="noTextParagraph") {
                posA=containerLines.at(i).tagContainerLine_.at(0).addressTagBegin_;
                posB=containerLines.at(i).tagContainerLine_.at(0).addressTagEnd_;

                articleFile.at(i) = set_custom_bodyTag(articleFile.at(i), posA, posB, newParagraphTag);

            }
        }
        //After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);



        //Insert tags at the beginning of the headlines
        numberOfLines=articleFile.size();
        for(size_t i=0; i<numberOfLines-1; i++){

            if(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head1Begin" ||
                    containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head2Begin" ||
                    containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head3Begin") {

                posA=containerLines.at(i).tagContainerLine_.at(0).addressTagBegin_;
                posB=containerLines.at(i).tagContainerLine_.at(0).addressTagEnd_;

                if(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head1Begin"){
                    y=0; //= first record in vector
                }
                if(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head2Begin"){
                    y=1; // s. o.
                }
                if(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head3Begin"){
                    y=2; // s. o.
                }

                articleFile.at(i) = set_custom_HeadlineTags(articleFile.at(i), posA, posB, newHeadlineTags, y);
            }


        }

        //After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);


        //Insert tags at the end of the headlines
        numberOfLines=articleFile.size();
        for(size_t i=0; i<numberOfLines-1; i++){

            size_t containerSize;
            containerSize=containerLines.at(i).tagContainerLine_.size();

            for(size_t z=0; z<containerSize; z++){

                if(containerLines.at(i).tagContainerLine_.at(z).typeOfTag_ == "head1End" ||
                        containerLines.at(i).tagContainerLine_.at(z).typeOfTag_ == "head2Ende" ||
                        containerLines.at(i).tagContainerLine_.at(z).typeOfTag_ == "head3Ende" ) {

                    posA=containerLines.at(i).tagContainerLine_.at(z).addressTagBegin_;
                    posB=(containerLines.at(i).tagContainerLine_.at(z).addressTagEnd_);
                    y=3;

                    articleFile.at(i) = set_custom_HeadlineTags(articleFile.at(i), posA, posB, newHeadlineTags, y);

                }
            }
        }


        //After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
        cout << "Custom journal body tags set successfully..." << endl;
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
                
                if(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "paragraphBegin" && containerLines.at(i).lineCategory_!="blankLine" && containerLines.at(i).lineCategory_!="noTextParagraph") {
                    posA=containerLines.at(i).tagContainerLine_.at(0).addressTagBegin_;
                    posB=containerLines.at(i).tagContainerLine_.at(0).addressTagEnd_;
                    articleFile.at(i) = set_paragraphNumbers(articleFile.at(i), posA, posB, newParagraphTag, paragraphNumberTagBegin, paragraphNumberTagEnd, pointerParagrNumber);
                }
            }


            //After alterating the file analyze again
            analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
            cout << "Paragraph numbers set successfully..." << endl;
            paragraphNumbersSet=true;

    }


    //Set figure references tags?_______________
    if(figureReferenceTagsSelected==true && figureReferenceTagsSet==false){

        set_figureReferencesTags(articleFile, figureReferencesClass::figReferenceTagBegin_, figureReferencesClass::figReferenceTagEnd_);

        //After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
        numberOfLines=articleFile.size();
        cout << "Figure references tags set successfully..." << endl;
        figureReferenceTagsSet=true;

    }

    //Remove blank lines?_________________________
    if(removeBlankLinesSelected==true && blankLinesRemoved==false){

        articleFile=remove_blankLines(articleFile, containerTags, containerLines);

        //After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
        cout << "Blank lines removed successfully..." << endl;
        blankLinesRemoved=true;

    }

    //Convert pandoc footnotes to MS WORD_____________________________________________________________
    if(footnoteTagsSet==false){

        insert_MSWordFootnoteTags(articleFile, footnoteAdressContainer);

        //After alterating the file analyze again
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
        cout << "Footnotes converted successfully..." << endl;
        footnoteTagsSet=true;
    }


    //Prepare to write file an folder target names and save article and ressources___________________________________
    
    create_target_file_and_folder_names(fileInfos);

    if(customBodyTagsSet==true){

        replace_HtmlHead(articleFile, fileInfos.fileNameNewHtmlHead_);
        htmlHeadReplaced=true;
        search_replace(articleFile, fileInfos.toReplaceInHtmlHead_, fileInfos.newFileNameForRessources_);
        
		//After alterating the file analyze again        
        analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);

    }

    insert_metadataTemplates(articleFile, fileInfos);

	//After alterating the file analyze again 
    analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);

    saveFile(articleFile, fileInfos);
    cout << "Article successfully written..." << endl;

    write_ressources(fileInfos);
    cout << "Ressources successfully written..." << endl;

    //Success report and resetting the tool for next run

    cout << "\n+ + + + + + + + + + + + + + + + + + + +" << endl;
    cout << "+ Article successfully edited!  +" << endl;;
    cout << "+ + + + + + + + + + + + + + + + + + + +\n" << endl;

    //customBodyTagsSelected=false;
    figureReferenceTagsSelected=false;
    removeBlankLinesSelected=false;
    authorYearTagsSelected=false;
    removeDispensableTagsSelected=false; 
    paragraphNumbersSelected=false; 

    cout << "\n";
    cout << "Do you wish further alterations (y/n)? ";
    char input;
    cin >> input;
    cin.ignore(1,'\n');
    cout << "\n";

    if(input=='n' || input=='N'){
    nextRunIsSet=false;
    }

    }

    cout << "TagTool_WiZArD terminated successfully" << endl;

    return 0;
}
