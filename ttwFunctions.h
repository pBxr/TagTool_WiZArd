#pragma once
#ifndef TTWFUNCTIONS_H
#define TTWFUNCTIONS_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream> 
#include <vector>
#include <regex>

#include "ttwClasses.h"
#include "ttwDeclarations.h"


using std::cout; using std::cin; using std::vector; using std::string; using std::ifstream; using std::ofstream;
using std::endl; using std::stringstream; using std::ostringstream;


void analyze_articleFile(vector<string> &articleFile, vector<tagClass> &containerTags, vector<lineClass> &containerLines, struct documentSectionsClass &documentSections, vector<footNoteClass> &footnoteAdressContainer) {

	containerTags.clear();
	containerLines.clear();
	documentSections.clear();
	footnoteAdressContainer.clear();
	footnoteAdressContainer.push_back( footNoteClass {0, 0, 0, 0}); 
	//Footnote 0 remains unused, so the index is corresponding with the article´s footnote numbers
	
	unsigned int footnoteNumber=1;

	bool footnoteBeginDetected=false;
	
	documentSections.lineNrFootnotesBeginIsSet=false;

	for(size_t i=0; i<articleFile.size(); i++) {

		string tagType;
		string tagContent;
		string lineContent;
		string lineType;

		//Isolate plain text
		lineContent=isolate_plainText(articleFile.at(i));

		//Classify type of tags and extract footnotes
		unsigned int index=0;
		unsigned int tagNr=0;
		unsigned int pos1=0;
		unsigned int pos2=0;
		int testPos1, testPos2;
		string iteratorContent;
		string::iterator end1=articleFile.at(i).end();

		for(string::iterator it=articleFile.at(i).begin(); it!=end1; ++it) {

			iteratorContent = *it;

			if(iteratorContent=="<") {
				pos1=index;
				testPos1=articleFile[i].find(">");
				if(testPos1==-1) {
					pos2=articleFile[i].size();
					tagType="incomplete Tag (end missing)";
					tagContent="##not recorded##";
					containerTags.push_back( tagClass {i,tagNr, pos1, pos2, tagContent, tagType});
					tagNr++;
				}

			}

			if(iteratorContent==">") {
				pos2=index;
				testPos2=articleFile[i].find("<");
				if(testPos2==-1) {
					pos2=0;
					tagType="incomplete Tag (begin missing)";
					tagContent="##not recorded##";
					containerTags.push_back( tagClass {i,tagNr, pos1, pos2, tagContent, tagType});
					tagNr++;
				}

				else {

					for(unsigned int ii=pos1; ii<=pos2; ii++) {
						tagContent=tagContent+articleFile.at(i).at(ii);
					}

					

					tagType=classify_tag(tagContent, i, documentSections);

					containerTags.push_back( tagClass {i,tagNr, pos1, pos2, tagContent, tagType});

					//Save pandoc footnote references and their line numbers

					if(tagType=="footnoteReferencePandoc") {
						
						footnoteAdressContainer.push_back( footNoteClass {footnoteNumber, i, pos1, pos2});
												
						footnoteNumber++;
					}


				if(tagType=="footnotePandoc") {
						detect_footnotes(tagContent, i, pos1, pos2, footnoteAdressContainer);
						
					}

					if(tagType=="footnotePandocBacklink") {
						detect_footnoteBacklinks(tagContent, pos1, pos2, footnoteAdressContainer);
					}
					
					tagContent.clear();
				}
				tagNr++;
			}
			index++;
		}


		if(pos1==0 && pos2==0) {
			containerTags.push_back( tagClass {i, tagNr, 0, 0, " ", "noTag"});
		}

		containerLines.push_back( {i, containerTags, lineContent, lineType} );
		lineContent.clear();
		containerTags.clear();
	}

	//Detect section beginnings...
	for(size_t i=0; i<articleFile.size(); i++) {
		if(i>=documentSections.lineNrBodyTextBegin_) {
			get_lineNumbers_documentSections(containerLines[i].plainTextLine_, i, documentSections);
		}
	}
	
	
	//In case article contains no different sections
	if(documentSections.lineNrTextEnd_==0){
		documentSections.lineNrTextEnd_=documentSections.lineNrBodyTextEnd_;
	}
	
	//Check if paragraph is to be flagged as numerable
	detect_numberable_paragraphs(articleFile);
		
}

string classify_tag(string tagContent, size_t i, documentSectionsClass &documentSections) {

	int checkPos1=-1;
	int checkPos2=-1;

	//Document sections...
	if((checkPos1=tagContent.find("<html"))==0) {
		return "htmlBegin";
	}
	
	if((checkPos1=tagContent.find("/html>"))>0) {
		return "htmlEnd";
	}
	
	if((checkPos1=tagContent.find("<article article-type"))==0) {
		return "xmlBegin";
	}
	
	if((checkPos1=tagContent.find("/article>"))>0) {
		return "xmlEnd";
	}
	
	if((checkPos1=tagContent.find("<head"))==0) {
		return "headBegin";
	}
	
	if((checkPos1=tagContent.find("/head>"))>0) {
		documentSections.lineNrHtmlHeadEnd_=i;
		return "headEnd";
	}
	
	if((checkPos1=tagContent.find("<front"))==0) {
		return "frontBegin";
	}
	
	if((checkPos1=tagContent.find("/front>"))>0) {
		documentSections.lineNrHtmlHeadEnd_=i;
		return "frontEnd";
	}
	
	if((checkPos1=tagContent.find("<body"))==0) {
		documentSections.lineNrBodyTextBegin_=i;
		documentSections.lineNrBodyTextIsSet=true;
		return "bodyBegin";
	}
	
	if((checkPos1=tagContent.find("/body>"))>0) {
		if(documentSections.lineNrBodyTextEndIsSet==false){
			documentSections.lineNrBodyTextEnd_=i;
			}
		return "bodyEnd";
	}
	
	if((checkPos1=tagContent.find("<ref id=\"ref"))==0) {
		if(documentSections.lineNrReferencesBeginIsSet==false){
		documentSections.lineNrReferencesBegin_=i;
		documentSections.lineNrReferencesBeginIsSet=true;	
		}
		return "bibReference";
	}
	
		
	if((checkPos1=tagContent.find("<fn-group content-type=\"footnotes\">"))==0){
		documentSections.lineNrFootnotesBegin_=i;
		documentSections.lineNrFootnotesBeginIsSet=true;
		return "beginnFootnoteGroupXML";
	}
	
	if((checkPos1=tagContent.find("<section class=\"footnotes\" role=\"doc-endnotes\">"))==0){
		documentSections.lineNrFootnotesBegin_=i;
		documentSections.lineNrFootnotesBeginIsSet=true;
		return "beginnFootnoteGroupPandoc";
	}
			
	if((checkPos1=tagContent.find("/section>"))>0) {
		return "sectionEndHtml";
	}
	
	
	//footnotes....
	if((checkPos1=tagContent.find("<a href=\"#fn")==0)&&(checkPos2=tagContent.find("<a href=\"#fnref")==-1)) {
		return "footnoteReferencePandoc";
	}

	if((checkPos1=tagContent.find("<li id=\"fn")==0)&&(checkPos2=tagContent.find("doc-endnote"))>0) {
		return "footnotePandoc";
	}

	if((checkPos1=tagContent.find("<a href=\"#fnref")==0)&&(checkPos2=tagContent.find("doc-backlink"))>0) {
		return "footnotePandocBacklink";
	}
	
	
	if((checkPos1=tagContent.find("<xref ref-type=\"fn\" rid="))==0) {
		return "footnoteReferenceXML";
	}

	if((checkPos1=tagContent.find("<fn id=\"fn\""))==0){
		return "footnoteXML";
	}

	
	if((checkPos1=tagContent.find("/xref>"))>0) {
		return "referenceEndXML";
	}
	

	if((checkPos1=tagContent.find("/sec>"))>0) {
		return "sectionEndXML";
	}


	//headlines...
	if((checkPos1=tagContent.find("<h1"))==0 ||(checkPos1=tagContent.find("<p class=DAIbody-h1>")==0)) {
		return "head1Begin";
	}
	if((checkPos1=tagContent.find("/h1>"))>0) {
		return "head1End";
	}

	if((checkPos1=tagContent.find("<h2"))==0 ||(checkPos1=tagContent.find("<p class=DAIbody-h2>")==0) ) {
		return "head2Begin";
	}
	if((checkPos1=tagContent.find("/h2>"))>0) {
		return "head2End";
	}

	if((checkPos1=tagContent.find("<h3"))==0 ||(checkPos1=tagContent.find("<p class=DAIbody-h3>")==0)) {
		return "head3Begin";
	}
	if((checkPos1=tagContent.find("/h3>"))>0) {
		return "head3End";
	}

	if((checkPos1=tagContent.find("<h4"))==0 || (checkPos1=tagContent.find("<p class=DAIbody-h4>")==0)) {
		return "head4Begin";
	}
	if((checkPos1=tagContent.find("/h4>"))>0) {
		return "head4End";
	}

	if((checkPos1=tagContent.find("<h5"))==0 || (checkPos1=tagContent.find("<p class=DAIbody-h1>")==0)) {
		return "head5Begin";
	}
	if((checkPos1=tagContent.find("/h5>"))>0) {
		return "head5End";
	}
	
	
	//numberes text paragraohs
	if((checkPos1=tagContent.find("<p id="))==0) {
		return "countedTextParagraphXML";
	}
	
	if((checkPos1=tagContent.find("<p class=DAIbody-text>"))==0) {
		return "countedTextParagraphHTML";
	}
		
		
	//xml images container
	if((checkPos1=tagContent.find("<sec id=\"images-container\">"))==0) {
		documentSections.lineNrImageContainerXML_=i;
			if(documentSections.lineNrBodyTextEndIsSet==false){
			documentSections.lineNrBodyTextEnd_=i;
			}
		return "imageContainerXMLBegin";
	}	
	
	//Other tags....
	if((checkPos1=tagContent.find("<meta"))==0) {
		return "metaBegin";
	}
	if((checkPos1=tagContent.find("/meta>"))>0) {
		return "metaEnd";
	}

	if((checkPos1=tagContent.find("<title"))==0) {
		return "titleBegin";
	}
	if((checkPos1=tagContent.find("/title>"))>0) {
		return "titleEnd";
	}
	
	if((checkPos1=tagContent.find("<div"))==0) {
		if((checkPos2=tagContent.find("ootnote"))>0) {
			return "divFootnoteBegin";
		} 
		else {
			return "divBegin";
		}
	}

	if((checkPos1=tagContent.find("/div>"))>0) {
		return "divEnd";
	}

	if((checkPos1=tagContent.find("<span"))==0) {
		if((checkPos2=tagContent.find("ootnote"))>0) {
			return "spanFootnoteBegin";
		} else {
			return "spanBegin";
		}
	}
	if((checkPos1=tagContent.find("/span>"))>0) {
		return "spanEnd";
	}

	if((checkPos1=tagContent.find("<style"))==0) {
		return "styleBegin";
	}
	if((checkPos1=tagContent.find("/style>"))>0) {
		return "styleEnd";
	}

	if((checkPos1=tagContent.find("<p>"))==0) {
		return "paragraphBegin";
	}
	if((checkPos1=tagContent.find("/p>"))>0) {
		return "paragraphEnd";
	}

	if((checkPos1=tagContent.find("<p lang"))==0) {
		return "languageBegin";
	}

	if((checkPos1=tagContent.find("<p class"))==0) {
		if((checkPos2=tagContent.find("ootnote"))>0) {
			return "footnoteParagrBegin";
		} 
		else {
			return "paragraphBegin";
		}
	}

	if((checkPos1=tagContent.find("<a class"))==0) {
		if((checkPos2=tagContent.find("ootnote"))>0) {
			return "footnoteAnchorBegin";
		} 
		else {
			return "anchorBegin";
		}
	}

	if((checkPos1=tagContent.find("/a>"))>0) {
		return "anchorEnd";
	}

	if((checkPos1=tagContent.find("<a style"))==0) {
		if((checkPos2=tagContent.find("ootnote"))>0) {
			return "footnoteStyleBegin";
		} 
		else {
			return "styleBegin";
		}
	}

	if((checkPos1=tagContent.find("<i>"))==0) {
		return "italicBegin";
	}
	if((checkPos1=tagContent.find("<i "))==0) {
		return "italicWithFormatBegin";
	}
	if((checkPos1=tagContent.find("/i>"))>0) {
		return "italicEnd";
	}

	if((checkPos1=tagContent.find("<b>"))==0) {
		return "boldBegin";
	}
	if((checkPos1=tagContent.find("/b>"))>0) {
		return "boldEnd";
	}
	
	if((checkPos1=tagContent.find("<strong>"))==0) {
		return "strongBegin";
	}
	if((checkPos1=tagContent.find("/strong>"))>0) {
		return "strongEnd";
	}

	if((checkPos1=tagContent.find("<em>"))==0) {
		return "emphasizedBegin";
	}
	if((checkPos1=tagContent.find("</em>"))>0) {
		return "emphasizedEnd";
	}

	if((checkPos1=tagContent.find("<sup"))==0) {
		return "superscriptBegin";
	}
	if((checkPos1=tagContent.find("/sup>"))>0) {
		return "superscriptEnd";
	}

	if((checkPos1=tagContent.find("<br/>"))==0) {
		return "break";
	}

	if((checkPos1=tagContent.find("<tab"))==0) {
		return "tableBegin";
	}
	if((checkPos1=tagContent.find("/tab>"))>0) {
		return "tableEnd";
	}

	if((checkPos1=tagContent.find("<tr"))==0) {
		return "row1Begin";
	}
	if((checkPos1=tagContent.find("/tr>"))>0) {
		return "row1End";
	}

	if((checkPos1=tagContent.find("<td"))==0) {
		return "row2Begin";
	}
	if((checkPos1=tagContent.find("/td>"))>0) {
		return "row2End";
	}

	if((checkPos1=tagContent.find("<li"))==0) {
		return "listBegin";
	}
	if((checkPos1=tagContent.find("/li>"))>0) {
		return "listEnd";
	}

	if((checkPos1=tagContent.find("<ul"))==0) {
		return "list2Begin";
	}
	if((checkPos1=tagContent.find("/ul>"))>0) {
		return "list2End";
	}

	if((checkPos1=tagContent.find("<ol"))==0) {
		return "list3Begin";
	}
	if((checkPos1=tagContent.find("/ol>"))>0) {
		return "list3End";
	}

	if((checkPos1=tagContent.find("<a href"))==0) {
		return "linkBegin";
	}
	
	if((checkPos1=tagContent.find("<ext-link"))==0) {
		return "linkBeginXML";
	}

	if((checkPos1=tagContent.find("/ext-link>"))>0) {
		return "linkEndnXML";
	}
			
	if((checkPos1=tagContent.find("<p id=\"p"))==0) {
		return "paragraphBeginXMLWithNumbers";
	}

	return "toDelete";
}

void detect_footnotes(string tagContent, size_t lineNumber, unsigned int pos1, unsigned int pos2, vector<footNoteClass> &footnoteAdressContainer) {

	int footnoteNumber=0;

	std::regex pattern{"fn[0-9]{1,3}"};
	std::sregex_iterator begin{ tagContent.cbegin(), tagContent.cend(), pattern};
	std::sregex_iterator end;

	for (std::sregex_iterator i = begin; i != end; ++i) {
		std::smatch match = *i;
		std::string match_str = match.str();
		match_str.erase(0,2);

		footnoteNumber=std::stoi(match_str);

	}

	footnoteAdressContainer[footnoteNumber].lineNumberFootnote_=lineNumber;
	footnoteAdressContainer[footnoteNumber].adressFNBegin_=pos1;
	footnoteAdressContainer[footnoteNumber].adressFNEnd_=pos2;

}

void detect_footnoteBacklinks(string tagContent, unsigned int pos1, unsigned int pos2, vector<footNoteClass> &footnoteAdressContainer) {

	int footnoteNumber=0;

	std::regex pattern{"fnref[0-9]{1,3}"};
	std::sregex_iterator begin{ tagContent.cbegin(), tagContent.cend(), pattern};
	std::sregex_iterator end;

	for (std::sregex_iterator i = begin; i != end; ++i) {
		std::smatch match = *i;
		std::string match_str = match.str();
		match_str.erase(0,5);
		footnoteNumber=std::stoi(match_str);
	}

	if(footnoteNumber>=0) {
		footnoteAdressContainer[footnoteNumber].adressFNBacklinkBegin_=pos1;
		footnoteAdressContainer[footnoteNumber].adressFNBacklinkEnd_=pos2;
	}

}

void detect_numberable_paragraphs(vector<string> articleFile){
	
	//Categorize line categories
	for(int i=0; i<articleFile.size(); i++){
			
		if(containerLines[i].tagContainerLine_.size()==1 && containerLines[i].tagContainerLine_[0].typeOfTag_=="noTag") {
		containerLines[i].lineCategory_ = "noTextParagraph";
		}
		
		if(containerLines[i].tagContainerLine_.size()==0) {
		containerLines[i].lineCategory_ = "noTextParagraph";
		}

		if(i>documentSections.lineNrTextEnd_) {
		containerLines[i].lineCategory_ = "noTextParagraph";
		}

		if(containerLines[i].tagContainerLine_.at(0).typeOfTag_=="paragraphBegin" && containerLines[i].plainTextLine_.size()<125) {
		containerLines[i].lineCategory_ = "noTextParagraph";
		}

		else{
		containerLines[i].lineCategory_ = "other";
		}
	}

	//detect numberable paragraphs
	for(size_t i=0; i<documentSections.lineNrTextEnd_; i++){
      		        
        if(
		(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "paragraphBegin" 
		&& containerLines.at(i).lineCategory_!="noTextParagraph"
		&& i<documentSections.lineNrTextEnd_)
		|| (containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "countedTextParagraphHTML")
		|| (containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "countedTextParagraphXML")
		) 	{
        	containerLines.at(i).toBeNumbered=true;
			}
    }
}

void get_lineNumbers_documentSections(string articleFileLine, size_t i, struct documentSectionsClass &documentSections) {

	//If one of the following elements is detected (more than 10 lines from the beginning of the text body)
	//the flag for the end of the body text will be set
	//The footnote section will be detected separately

	bool abstract=false;
	bool abbreviations=false;
	bool address=false;
	bool keywords=false;
	bool illusCredits=false;
	int y;

	for(string toCheck : documentSections.checkStringAbstract) {
		y=articleFileLine.find(toCheck);
		if((i>documentSections.lineNrBodyTextBegin_+10) && y>=0 && y<5) {
			abstract=true;
			documentSections.lineNrAbstractBeginIsSet_=true;
			documentSections.lineNrAbstractBegin_=i;
		}
	}


	for(string toCheck : documentSections.checkStringAbbreviations) {
		y=articleFileLine.find(toCheck);
		if((i>documentSections.lineNrBodyTextBegin_+10) && y>=0 && y<10) {
			abbreviations=true;
			documentSections.lineNrAbbreviationsBeginIsSet=true;
			documentSections.lineNrAbbreviationsBegin_ =i;
		}
	}

	for(string toCheck : documentSections.checkStringAddress) {
		y=articleFileLine.find(toCheck);
		if((i>documentSections.lineNrBodyTextBegin_+10) && y>=0 && y<5) {
			address=true;
			documentSections.lineNrAddressIsSet=true;
			documentSections.lineNrAddressBegin_ =i;
		}
	}

	for(string toCheck : documentSections.checkStringKeywords) {
		y=articleFileLine.find(toCheck);
		if((i>documentSections.lineNrBodyTextBegin_+10) && y>=0 && y<5) {
			keywords=true;
			documentSections.lineNrKeywordsBegin_ =i;
			documentSections.lineNrKeywordsBeginIsSet =true;
		}
	}

	for(string toCheck : documentSections.checkStringSourcesIllustrations) {
		y=articleFileLine.find(toCheck);
		if((i>documentSections.lineNrBodyTextBegin_+10) && y>=0 && y<5) {
			documentSections.lineNrSourcesIllustrationsBegin_ =i;
			illusCredits=true;
			documentSections.lineNrSourcesIllustrationsIsSet =true;
		}
	}

	//...and find the first one that marks the end of the body text
	vector<int> vectorDocumentSections = {documentSections.lineNrAbstractBegin_,
	                                       documentSections.lineNrKeywordsBegin_,
	                                       documentSections.lineNrAbbreviationsBegin_,
	                                       documentSections.lineNrAddressBegin_,
	                                       documentSections.lineNrSourcesIllustrationsBegin_
	                                      };
	
	sort(vectorDocumentSections.begin(), vectorDocumentSections.end());
	
	for(int i=0; i<vectorDocumentSections.size(); i++) {
		if(vectorDocumentSections[i]>0) {
			documentSections.lineNrTextEnd_=vectorDocumentSections[i];
			documentSections.lineNrTextEndIsSet=true;
			break;
		}
	}

}

vector<string> identifyParameters(string inputLine) {

	vector<string> parameterVector;
	int lenght;
	string toCheck;

	//Delete blank spaces at the begin/end
	toCheck=inputLine[0];
	while(toCheck==" ") {
		inputLine.erase(0,1);
		toCheck=inputLine[0];
	}

	toCheck=inputLine[inputLine.size()-1];
	while(toCheck==" ") {
		inputLine.erase(inputLine.size()-1,1);
		toCheck=inputLine[inputLine.size()-1];
	}

	//Extract parameters
	lenght=inputLine.size();
	int posA=0;
	string parameter;

	for(int i=0; i<lenght; i++) {
		toCheck=inputLine[i];
		if(toCheck==" " ) {
			parameter = inputLine.substr(posA, (i-posA));
			parameterVector.push_back(parameter);
			posA=i+1;
		}
	}

	parameter=inputLine.substr(posA, lenght);
	parameterVector.push_back(parameter);

	return parameterVector;

}

void insert_image_credit_list(vector<string> &articleFile){
	
	vector<illustrationCreditsClass> illustrationCreditList;
	
	illustrationCreditList=load_value_list(fileInfos.fileNameCreditList_, illustrationCreditList);
	
	string nr;

	int number;
	int offset;
	string figNumberLabelTagBeginXML;
	string pathFilenameSourcesXML;
	string toInsert;
	
	vector<string> insertVector;
	vector<string> insertVector2;
	
	if(htmlSelected==false){
		insertVector.push_back("<sec id=\"images-container\"><title>Illustration Credits</title>\n");
		
		for(int i=1; i<illustrationCreditList.size(); i++){
		
		
		//extract fig.number....
		std::regex r("[0-9]{1,3}");
		std::smatch m;
		std::regex_search(illustrationCreditList[i].creditVector_[0], m, r);	
		nr=m.str();
		figNumberLabelTagBeginXML = illustrationCreditsClass::figNumberLabelTagBeginXML_;
		figNumberLabelTagBeginXML.replace(11, 1, nr);
		pathFilenameSourcesXML = illustrationCreditsClass::pathFilenameSourcesXML_;
		pathFilenameSourcesXML.replace(64, 32, illustrationCreditList[i].creditVector_[2]);
		
		//insert container into article
		toInsert=figNumberLabelTagBeginXML + 
			illustrationCreditList[i].creditVector_[0] + 			//=label
			illustrationCreditsClass::figNumberLabelTagEndXML_ +
			illustrationCreditsClass::figCaptionsTagBeginXML_ +
			illustrationCreditList[i].creditVector_[1] +			//=caption
			illustrationCreditsClass::figCaptionsTagEndXML_ +
			pathFilenameSourcesXML +								//=source path
			illustrationCreditsClass::figCreditTagBeginXML_ +			
			illustrationCreditList[i].creditVector_[3] + 			//=credits
			illustrationCreditsClass::figCreditTagEndXML_ + "\n";
					
			insertVector.push_back(toInsert);
			toInsert.clear();
		}
		
		insertVector.push_back("</sec>\n");	
	}
	
	else{
		//html needs two runs because of the different tags for credit label an figure number label
		insertVector.push_back("<p class=DAIabbildungsverz-h>Abbildungsnachweis</p>\n");	
		for(int i=1; i<illustrationCreditList.size(); i++){
		toInsert=illustrationCreditsClass::figNumberCreditLabelTagBegin_+
			illustrationCreditList[i].creditVector_[0] + 			//=label
			illustrationCreditsClass::figNumberCreditLabelTagEnd_+
			illustrationCreditsClass::figCreditTagBegin_+
			illustrationCreditList[i].creditVector_[3] + 			//=credits	
			illustrationCreditsClass::figCreditTagEnd_+ "\n";
			insertVector.push_back(toInsert);
			toInsert.clear();
		}
		
		for(int i=1; i<illustrationCreditList.size(); i++){
		toInsert=illustrationCreditsClass::figNumberLabelTagBegin_+
			illustrationCreditList[i].creditVector_[0] + 			//=label
			illustrationCreditsClass::figNumberLabelTagEnd_+
			illustrationCreditsClass::figNumberCaptionTagBegin_+
			illustrationCreditList[i].creditVector_[1] + 			//=credits	
			illustrationCreditsClass::figNumberCaptionTagEnd_+ "\n";
			insertVector2.push_back(toInsert);
			toInsert.clear();
		}
		
	}
	
	if(documentSections.lineNrFootnotesBeginIsSet==true){
	offset = documentSections.lineNrFootnotesBegin_;	
	}	
	if(documentSections.lineNrFootnotesBeginIsSet==false){
	offset = documentSections.lineNrBodyTextEnd_;	
	}
		
	articleFile.insert(articleFile.begin()+offset, insertVector.begin(), insertVector.end());
	
	if(htmlSelected==true){
		articleFile.insert(articleFile.begin()+offset, insertVector2.begin(), insertVector2.end());	
	}
	
}

void insert_metadataTemplates(vector<string> &articleFile, fileInformations &fileInfos) {
	
	articleFile.insert(articleFile.begin()+documentSections.lineNrBodyTextBegin_+1, fileInfos.metadataBegin.begin(), fileInfos.metadataBegin.end());
	
	if(documentSections.lineNrFootnotesBeginIsSet == true){
		articleFile.insert(articleFile.begin()+fileInfos.metadataBegin.size()+documentSections.lineNrFootnotesBegin_, fileInfos.metadataEnd.begin(), fileInfos.metadataEnd.end());	
	}
	else{
		articleFile.insert(articleFile.begin()+fileInfos.metadataBegin.size()+documentSections.lineNrBodyTextEnd_, fileInfos.metadataEnd.begin(), fileInfos.metadataEnd.end());	
	}
	

}

void insert_MSWordFootnoteTags(vector<string> &articleFile, vector<footNoteClass> &footnoteAddressContainer) {

	string footnoteBeginNew;
	string referenceFootnoteBeginNEW;
	string markerFootnoteSection;
	string referenceFootnoteEndNEW;
	string footnoteEndNew;
	
	if(htmlSelected==true){
		referenceFootnoteEndNEW = footNoteClass::referenceFootnoteEndNEW_;
		footnoteEndNew = footNoteClass::footnoteEndNEW_;
		markerFootnoteSection = footNoteClass::markerFootnoteSection_;
	}
	
	else{
		referenceFootnoteEndNEW = footNoteClass::referenceFootnoteEndNEWXML_;
		footnoteEndNew = footNoteClass::footnoteEndNEWXML_;
		markerFootnoteSection = footNoteClass::markerFootnoteSectionXML_;
	}
			
	
//First the footnote references___________________________________

	string footnoteNumberString;

	int numberOfDigits=0;

	int containerSize;
	int lineNumberText;
	int posBegin;
	int posEnd;
	int posBacklinkBegin;
	int lengthReplacingTag;

	containerSize=footnoteAddressContainer.size()-1;

	for(int i=containerSize; i>=1; --i) {
		footnoteNumberString.clear();


		if(htmlSelected==true){
			referenceFootnoteBeginNEW = footNoteClass::referenceFootnoteBeginNEW_;
			footnoteBeginNew = footNoteClass::footnoteBeginNEW_;	
			}
		else{
			referenceFootnoteBeginNEW = footNoteClass::referenceFootnoteBeginNEWXML_;
			footnoteBeginNew = footNoteClass::footnoteBeginNEWXML_;	
		}
		

		footnoteNumberString=std::to_string((i+0));

		if(i+1<=10) {
			numberOfDigits=1;
		}
		if(i+0>=10 && i+1<=100) {
			numberOfDigits=2;
		}
		if(i+0>=100 && i+1<=1000) {
			numberOfDigits=3;
		}

		//First replace end tag (+5, because <sup>)
		lineNumberText=footnoteAddressContainer[i].lineNumberReference_;

		posEnd=footnoteAddressContainer[i].adressFNReferenceEnd_+1;

		articleFile[lineNumberText].replace((posEnd+5+numberOfDigits), 10, referenceFootnoteEndNEW);

		//now replace begin tag...
        std::regex r1("#1#");
        referenceFootnoteBeginNEW = std::regex_replace(referenceFootnoteBeginNEW, r1, footnoteNumberString);
               

		posBegin=footnoteAddressContainer[i].adressFNReferenceBegin_;
		posEnd=footnoteAddressContainer[i].adressFNReferenceEnd_;

		if(htmlSelected==true){
			lengthReplacingTag=posEnd-posBegin+1;	
			}
		else{
			lengthReplacingTag=posEnd-posBegin+6; //+6 because the "<sup>" hast to be overwritten.	
		}

		articleFile[lineNumberText].replace(posBegin,lengthReplacingTag, referenceFootnoteBeginNEW);


//Now the footnotes___________________________________

		posBacklinkBegin=footnoteAddressContainer[i].adressFNBacklinkBegin_;

		lineNumberText=footnoteAddressContainer[i].lineNumberFootnote_;

		footnoteBeginNew = std::regex_replace(footnoteBeginNew, r1, footnoteNumberString);

		//34 and 78 are the lenghts of the old tags...
		// <li id="fn1" role="doc-endnote">
		// <a href="#fnref37" class="footnote-back" role="doc-backlink">â†©ï¸Ž</a></p></li>

		articleFile[lineNumberText+0].replace(posBacklinkBegin,78+numberOfDigits,footnoteEndNew);


		articleFile[lineNumberText].replace(0,(34+numberOfDigits),footnoteBeginNew);

		if(footnoteNumberString=="1") {
			
			articleFile[lineNumberText].insert(0, (markerFootnoteSection+"\n"));
		}
	}

}

string isolate_plainText(string articleFileLine) {

	unsigned int pos1=0;
	unsigned int pos2=0;

	for(size_t i=articleFileLine.size(); i>0; i--) {

		if(articleFileLine.at(i-1)=='>') {
			pos2=i;
		}

		if(articleFileLine.at(i-1)=='<') {
			pos1=i;
			articleFileLine.erase(pos1-1, (pos2-pos1+1));
		}
	}

	return articleFileLine;
}

int processParameter(vector<string> &parameterVector) {

	bool fileNameEntered=false;
	bool referenceListEntered=false;
	bool creditListEntered=false;
	bool metadataListEntered=false;
	bool unspecifiedValueListEntered=false;
	bool outputFormatSpecified=false;
	
	vector<string> enteredFunctions;
	vector<string> enteredValueLists;
	
		
	if(parameterVector.size()==0) {
		cout << "Parameters missing. " << endl;
		cout << "Please enter the name of the source file or enter \"--help\" when starting the application the next time" << endl;
		return 0;
	}


	//Extract output format and detect possible --help call
	vector<int> toDelete;
	for(int i=0; i<parameterVector.size(); i++){
		
		int pos;
		
		pos = parameterVector[i].find("--toXML");
		if(pos>=0){
			outputFormatSpecified=true;
			htmlSelected=false;
			toDelete.push_back(i);
		}
		
		pos = parameterVector[i].find("--toHTML");
		if(pos>=0){
			outputFormatSpecified=true;
			htmlSelected=true;
			toDelete.push_back(i);
		}
		
		pos = parameterVector[i].find("--help");
		if(pos>=0){
			show_help();
			return 0;
		}
	}
		
	//Delete output format from functions and value lists	
	for(int i : toDelete){
		parameterVector.erase(parameterVector.begin()+i);
	}
		
	//detect source format, entered function parameter and entered value lists		
	for(int i=0; i<parameterVector.size(); i++){
				
		int pos;
		
		pos = parameterVector[i].find("--");
		if(pos>=0){
		enteredFunctions.push_back(parameterVector[i]);	
		}
		
		pos = parameterVector[i].find(".csv");
		if(pos>=0){
		enteredValueLists.push_back(parameterVector[i]);	
		}
		
		pos = parameterVector[i].find(".html");
		if(pos>=0){
			fileNameEntered=true;
			fileInfos.sourceFormat_ = "html";
			fileInfos.fileNameSourceFile_=parameterVector[i];
		}
		
		pos = parameterVector[i].find(".xml");
		if(pos>=0){
			fileNameEntered=true;
			fileInfos.sourceFormat_ = "xml";
			fileInfos.fileNameSourceFile_=parameterVector[i];
		}
	}
	
	if(firstRun==true && fileNameEntered==false) {
			cout << "\n";
			cout << "NO SOURCE FILE WAS ENTERED" << endl;
			return 0;
		}

	//If no function parameters were entered, set default functions... 
	if(enteredFunctions.size()==0) {		//=default setting
		customBodyTagsSelected=true;
		figureReferenceTagsSelected=true;
		authorYearTagsSelected=true; 
		paragraphNumbersSelected=true;
		removeDispensableTagsSelected=false;
		insertCreditListSelected=true;
					
		cout << "No parameters entered, so the default functions will be set:\n- Set customized journal body tags\n- Set figure references tags\n- Set author year tags. CAUTION: AuthorYearList.csv required!\n- Set paragraph numbers\n- Insert tagged illustration credit section. CAUTION: IllustrationCreditList.csv requried!\n" << endl;
	}
	
	//... and set default value lists
	if(enteredValueLists.size()==0){ 			//= default setting
		
		fileInfos.fileNameAuthorYearList_ = "AuthorYearList.csv";
		referenceListEntered=true;
						
		fileInfos.fileNameCreditList_ = "IllustrationCreditList.csv";	
		creditListEntered=true;
				
		fileInfos.fileNameMetadataList_ = "MetadataValueList.csv";
		metadataListEntered=true;
	}

	//If funtion parameters were entered explicitly, set functions and get value lists if entered explicitly
	if(enteredFunctions.size()>0) {
		cout << "\n" << "You have chosen following functions: " << endl;

		for(int i=0; i<enteredFunctions.size(); i++) {
						
			if(enteredFunctions[i]=="--bodyTags") {
				customBodyTagsSelected=true;
				cout << "- Set customized journal body tags\n";
			}

			if(enteredFunctions[i]=="--figTags") {
				figureReferenceTagsSelected=true;
				cout << "- Set figure references tags\n";
			}

			if(enteredFunctions[i]=="--litTags") {
				authorYearTagsSelected=true; 
				cout << "- Set author year tags. CAUTION: AuthorYearList.csv required!\n";
			}

			
			if(enteredFunctions[i]=="--paragrNum") {
				paragraphNumbersSelected=true;
				cout << "- Set paragraph numbers\n";
			}

			if(enteredFunctions[i]=="--delTags") {
				removeDispensableTagsSelected=true;
				cout << "- Remove dispensable formattings/tags\n";
			}
			
			if(enteredFunctions[i]=="--illCred") {
				insertCreditListSelected=true;
				cout << "- Insert tagged illustration credit section. CAUTION: IllustrationCreditList.csv requried!\n";
			}
		}
	}
			
		//Output format info
		if(outputFormatSpecified==false){ //= default
		cout << "\nNo output format entered, so the default outout format will be: HTML"<< endl;
		}
		
		if(outputFormatSpecified==true){
			if(htmlSelected==false){
				cout << "\nYou have chosen the following output format: XML"<< endl;	
			}
			if(htmlSelected==true){
				cout << "\nYou have chosen the following output format: HTML"<< endl;	
			}
		}
		
		//Check if all necessary filenames of all necessary value lists are existing...
		for(int i=0; i<enteredValueLists.size(); i++){
		if(enteredValueLists[i]=="AuthorYearList.csv"){
			fileInfos.fileNameAuthorYearList_ = "AuthorYearList.csv";
			referenceListEntered=true;
		}
		if(enteredValueLists[i]=="IllustrationCreditList.csv"){
			fileInfos.fileNameCreditList_ = "IllustrationCreditList.csv";	
			creditListEntered=true;
		}
		if(enteredValueLists[i]=="MetadataValueList.csv"){
			fileInfos.fileNameMetadataList_ = "MetadataValueList.csv";
			metadataListEntered=true;
		}
		else{
			unspecifiedValueListEntered=true;
		}
	}
				
		//... and if missing: ask for filenames
		if(enteredValueLists.size()>0){
			if(authorYearTagsSelected==true && referenceListEntered==false){
				cout << "Author Year value list is missing." << endl; 
				cout << "Please enter now (CAUTION: .csv file with special separating character \"%\" only): ";
				getline(cin, fileInfos.fileNameAuthorYearList_);
				cout << "\nEntered filename: " << fileInfos.fileNameAuthorYearList_;
				cout << "\n" << endl;
			}
			
			if(insertCreditListSelected==true && creditListEntered==false){
				cout << "Illustrations Credit value list is missing." << endl;
				cout << "Please enter now: (CAUTION: .csv file with special separating character \"%\" only): ";
				getline(cin, fileInfos.fileNameCreditList_);
				cout << "\nEntered filename: " << fileInfos.fileNameCreditList_;
				cout << "\n" << endl;	
			}
		}		
	
	//MetadataValueList ist not optional.
	fileInfos.fileNameMetadataList_ = "MetadataValueList.csv";
	metadataListEntered=true;
 
 
	bool confirmed=false; 

	while(!confirmed) {
		cout << "\n Please check before running the application: " << endl;
		cout << "- Are you sure that you have converted the MS Word .docx-article-file with pandoc to .html?" << endl;
		cout << "- If chosen: Have you prepared the Author Year and Illustration Credit value lists as required (see --help)?" << endl;
		cout << "- Are all files saved in the same folder as the TagTool_WiZArD application?" << endl;
		cout << "- Please confirm (y/n): ";
		char input;
		cin >> input;
		cin.ignore(1,'\n');
		if(input=='y' || input=='Y') {
			confirmed=true;
		} else {
			return 0;
		}
	}
	return 1;
}

vector<string> remove_blankLines(vector<string> articleFile) {

	for(size_t i=articleFile.size()-1; i>0; i--) {

		if(articleFile[i].size()==1) {
			
			articleFile.erase(articleFile.begin()+i);
			
		}

	}

	return articleFile;
}

vector<string> remove_disp_formattings(vector<string> articleFile, vector<lineClass> &containerLines) {

	unsigned int posA=0;
	unsigned int posB=0;
	size_t vectorSize;
	
	string tagType;
	string newTagBegin, newTagEnd;

	
//Delete body formats

	for(size_t i=documentSections.lineNrHtmlHeadEnd_; i<articleFile.size(); i++) {
		vectorSize=containerLines.at(i).tagContainerLine_.size();

		if(vectorSize>0) {
			for(size_t y=vectorSize; y>0; y--) {
				tagType=containerLines.at(i).tagContainerLine_.at(y-1).typeOfTag_;
				posB=containerLines.at(i).tagContainerLine_.at(y-1).addressTagEnd_;
				posA=containerLines.at(i).tagContainerLine_.at(y-1).addressTagBegin_;
				if(tagType=="paragraphBegin") {
					newTagBegin="<p>";
					articleFile.at(i)=set_new_tags(articleFile.at(i), posA, posB, newTagBegin);
				}

				if(tagType=="italicWithFormatBegin") {
					newTagBegin="<i>";
					articleFile.at(i)=set_new_tags(articleFile.at(i), posA, posB, newTagBegin);
				}

				if(tagType=="spanBegin") {
					newTagBegin="<span>";
					articleFile.at(i)=set_new_tags(articleFile.at(i), posA, posB, newTagBegin);
				}
				if(tagType=="styleBegin") {
					newTagBegin="<style>";
					articleFile.at(i)=set_new_tags(articleFile.at(i), posA, posB, newTagBegin);
				}

				if(tagType=="languageBegin") {
					newTagBegin="<p>";
					articleFile.at(i)=set_new_tags(articleFile.at(i), posA, posB, newTagBegin);
				}

				if(tagType=="bodyBegin") {
					newTagBegin="<body>";
					articleFile.at(i)=set_new_tags(articleFile.at(i), posA, posB, newTagBegin);
				}

				if(tagType=="metaBegin") {
					newTagBegin="<meta>";
					articleFile.at(i)=set_new_tags(articleFile.at(i), posA, posB, newTagBegin);
				}

				if(tagType=="headBegin") {
					newTagBegin="<head>";
					articleFile.at(i)=set_new_tags(articleFile.at(i), posA, posB, newTagBegin);
				}



				if(tagType=="toDelete") {
					posB=containerLines.at(i).tagContainerLine_.at(y-1).addressTagEnd_;
					posA=containerLines.at(i).tagContainerLine_.at(y-1).addressTagBegin_;
					articleFile.at(i).erase(posA, (posB-posA+1));
				}
			}
		}
	}
	return articleFile;
}

void replace_HtmlHead(vector<string>& articleFile, string fileNameNewHead) {

	vector<string> newHead;
	vector<reducedValueClass> valueList;
	int insertPoint = documentSections.lineNrHtmlHeadEnd_;
	
	newHead=loadFileContent(fileNameNewHead);
		
	
	if(htmlSelected==false){  
		valueList=load_reduced_value_list(fileInfos.fileNameMetadataList_, valueList);
		search_replace(newHead, valueList);
	}

	articleFile.erase(articleFile.begin(), (articleFile.begin()+ insertPoint +1));
	articleFile.insert(articleFile.begin(), newHead.begin(), newHead.end());

}

string resolve_hyphens_in_figRef(string bracketContent){
	
	string numberPair;
	vector<string> numberPairs;
	vector<string> numberPairsResolved;
	string bracketContentNew;
	string numberSequence;
	 
	int pos1, pos2, pos3, nr1int, nr2int;
	int posHyphen;
	string nr1str, nr2str;
	
	//Detect non regular hyphens in bracket content and convert them
	std::regex r1{"[^.\\w^(^)\\s]{2,4}"};  
	std::stringstream result;
	
	result << std::regex_replace(bracketContent, r1, "-");
	
	bracketContentNew = result.str();
	
	
	//Extract number pairs...
	std::regex r2{"[0-9]{1,3}-[0-9]{1,3}"};
    std::smatch m2;
    std::sregex_iterator begin{ bracketContentNew.begin(), bracketContentNew.end(), r2};
    std::sregex_iterator end;
	
    for (std::sregex_iterator it = begin; it != end; ++it) {
        m2=*it;
        numberPair=m2.str();
        numberPairs.push_back(numberPair);
        numberPair.clear();
        } 
           
      
     //resolve number pairs...
	for(size_t z=0; z<numberPairs.size(); z++){
	
		pos1=numberPairs[z].find("-");
		
		if(pos1>0){
			posHyphen=pos1;
			}
		else{
    		cout << "------ Warning: Could not read figure reference correctely, please replace special characters: " << bracketContent << endl;
    		return bracketContent;
    		}
			
    	nr1str=numberPairs[z].substr(0, posHyphen);
		nr2str=numberPairs[z].substr(posHyphen+1, numberPairs[z].size()-1);
				
		nr1int=std::stoi(nr1str);
		nr2int=std::stoi(nr2str);
	
		for(size_t i=nr1int; i<=nr2int; i++){
						
			if(i==(nr2int)){
				numberSequence=numberSequence+std::to_string(i);
			}
			else{
				numberSequence=numberSequence+std::to_string(i)+". ";
			}
		}
		
		numberPairsResolved.push_back(numberSequence);
		numberSequence.clear();
		
		pos1=-1;
		pos2=-1;
	}

	
	for(size_t i=0; i<numberPairsResolved.size(); i++){
	
		pos3=bracketContentNew.find(numberPairs[i]);
	
		bracketContentNew.replace(pos3, (numberPairs[i].length()), numberPairsResolved[i]);
		
		}

	return bracketContentNew;	
}

void search_replace(vector<string> &articleFile, string termSearch, string termReplace) {

	int pos1;
	for(size_t i=0; i<articleFile.size(); i++) {
		if(pos1>=0) {
			std::regex r(termSearch);
			articleFile[i] = regex_replace(articleFile[i], r, termReplace);
		}
	}

}

void search_replace(vector<string> &articleFile, vector<reducedValueClass> valueList) {

	int pos1;
	for(size_t i=0; i<articleFile.size(); i++) {
		
		for(int y=0; y<valueList.size(); y++){
		pos1=articleFile[i].find(valueList[y].values_[0]);
			if(pos1>=0) {
				std::regex r(valueList[y].values_[0]);
				articleFile[i] = regex_replace(articleFile[i], r, valueList[y].values_[1]);
			}
		}
	}
	
}

void set_authorYearTags(vector<string> &articleFile, vector<authorYearListClass> &authorYearList, documentSectionsClass &documentSections) {

	int z;
	z=authorYearList.size();

	int pos1=0;
	int pos2=0;
	int pos10=0;
	int pos20=0;

	//As a reminder: citations_[0] = Author year abbreviation
	//citations_[1] = full citation
	//citations_[2] = Zenon-ID

	//Fist for author year abbreviations...
	for(size_t i=0; i<articleFile.size(); i++) { 

		for(size_t y=0; y<authorYearList.size(); y++) {

			pos1=articleFile[i].find(authorYearList[y].citations_[0]);

			if(pos1==3) {
				//Important: When preparing the MS Word file be sure that the position of the author year abbreviations
				//has to be directely at the beginning of the line without any blank spaces
				//(pos1==3 because of "<p>", otherwise the abbreviation will not be found.
				//This also prevents that abbreviations in full citactions will be altered.

				articleFile[i].erase(0,pos1);
				pos2=authorYearList[y].citations_[0].size();

				articleFile[i].insert(pos2, authorYearListClass::authorYearTagEnd);
				articleFile[i].insert(0, authorYearListClass::authorYearTagBegin);
			}

			if(pos1>3 && documentSections.lineNrFootnotesBegin_<i) {

				pos2=pos1+(authorYearList[y].citations_[0].size());

				articleFile[i].insert(pos2, authorYearListClass::authorYearInFNTagEnd);
				articleFile[i].insert(pos1, authorYearListClass::authorYearInFNTagBegin);
			}
		}

	}

	//Now for full citations...
	for(size_t i=0; i<articleFile.size(); i++) { 
		string fullCitationTagsMerged;

		for(size_t y=0; y<authorYearList.size(); y++) {

			pos10=articleFile[i].find(authorYearList[y].citations_[1]);

			if(pos10>=0) {

				pos20=pos10+authorYearList[y].citations_[1].size();

				if (authorYearList[y].citations_[2].length()>=5) { //In case of an added link...

					//Delete Carriage Return/Linefeed from Vector so that it will not be inserted...
					authorYearList[y].citations_[2].pop_back();

					//Merging of the link....
					fullCitationTagsMerged=authorYearListClass::fullCitationTag1LinkBegin_+authorYearList[y].citations_[2]+authorYearListClass::fullCitationTag2LinkBegin_;

					articleFile[i].insert(pos20, authorYearListClass::fullCitationTagLinkEnd_);
					articleFile[i].insert(pos10, fullCitationTagsMerged);
				}

				if (authorYearList[y].citations_[2].length()<=5) { //In case not link is added...
					articleFile[i].insert(pos20, authorYearListClass::fullCitationTagWithoutLinkEnd_);
					articleFile[i].insert(pos10, authorYearListClass::fullCitationTagWithoutLink_);
				}
			}
		}
	}

}

void set_authorYearTags_XML(vector<string> &articleFile, vector<authorYearListClass> &authorYearList, documentSectionsClass &documentSections) {

	int z;
	z=authorYearList.size();

	int pos1=0;
	int pos2=0;
	int pos10=0;
	int pos20=0;
	
	string authorYearTagBeginXML;
	string authorYearInFNTagBeginXML; 
	
	std::regex r1("##author_year###");

	//As a reminder: citations_[0] = Author year abbreviation
	//citations_[1] = full citation
	//citations_[2] = Zenon-ID

	//Fist for author year abbreviations...
	for(size_t i=0; i<articleFile.size(); i++) { 

		for(size_t y=0; y<authorYearList.size(); y++) {

			pos1=articleFile[i].find(authorYearList[y].citations_[0]);

			if(pos1==3) {
				//Important: When preparing the MS Word file be sure that the position of the author year abbreviations
				//has to be directely at the beginning of the line without any blank spaces
				//(pos1==3 because of "<p>", otherwise the abbreviation will not be found.
				//This also prevents that abbreviations in full citactions will be altered.

				articleFile[i].erase(0,pos1);
				pos2=authorYearList[y].citations_[0].size();

				articleFile[i].insert(pos2, authorYearListClass::authorYearTagEndXML_);
				
				//Insert author year in start tag
				authorYearTagBeginXML = authorYearListClass::authorYearTagBeginXML_;
						
				authorYearTagBeginXML = std::regex_replace(authorYearTagBeginXML, r1, authorYearList[y].citations_[0]);
								
				articleFile[i].insert(0, authorYearTagBeginXML);
												
				authorYearTagBeginXML = authorYearListClass::authorYearTagBeginXML_; //reset
			}

			if(pos1>3 && documentSections.lineNrFootnotesBegin_<i) {

				pos2=pos1+(authorYearList[y].citations_[0].size());

				articleFile[i].insert(pos2, authorYearListClass::authorYearInFNTagEndXML_);
				
				//Insert author year in start tag
				authorYearInFNTagBeginXML = authorYearListClass::authorYearInFNTagBeginXML_;
								
				authorYearInFNTagBeginXML = std::regex_replace(authorYearInFNTagBeginXML, r1, authorYearList[y].citations_[0]);
				
				articleFile[i].insert(pos1, authorYearInFNTagBeginXML);
			}
		}

	}
	//Now for full citations...
	for(size_t i=0; i<articleFile.size(); i++) { 
		string fullCitationTag1LinkBeginXML;
		fullCitationTag1LinkBeginXML = authorYearListClass::fullCitationTag1LinkBeginXML_;
		for(size_t y=0; y<authorYearList.size(); y++) {

			pos10=articleFile[i].find(authorYearList[y].citations_[1]);

			if(pos10>=0) {

				pos20=pos10+authorYearList[y].citations_[1].size();
				
				articleFile[i].insert(pos20, authorYearListClass::fullCitationTagLinkEndXML_);
				
				//Delete Carriage Return/Linefeed from Vector so that it will not be inserted...
				authorYearList[y].citations_[2].pop_back();
				
				fullCitationTag1LinkBeginXML = authorYearListClass::fullCitationTag1LinkBeginXML_;
				
				std::regex r2("##_BibID_##");
				
				fullCitationTag1LinkBeginXML = std::regex_replace(fullCitationTag1LinkBeginXML, r2, authorYearList[y].citations_[2]);
				
				articleFile[i].insert(pos10, fullCitationTag1LinkBeginXML);
				fullCitationTag1LinkBeginXML = authorYearListClass::fullCitationTag1LinkBeginXML_; //reset
				
			}
		}
	}

}

string set_custom_bodyTag(string articleFileLine, int posA, int posB, string newParagraphTag) {
	posB++;
	articleFileLine.insert(posB, newParagraphTag);
	articleFileLine.erase(posA, (posB-posA));
	return articleFileLine;
}

void set_custom_HeadlineTags(vector<string> &articleFile, vector<lineClass> &containerLines, documentSectionsClass &documentSections) {

	int numberOfLines=articleFile.size();
	
	int posA=0;
    int posB=0;
    int y=0;
    
    int nrHead1=0;
    int nrHead2=0;
    int nrHead3=0;
    int nrSubHeads=0;
    
    std::regex h1("#1#");
	std::regex h2("#2#");
	std::regex h3("#3#");
    
    string prefixH1="</sec>\n"; 
    //string prefixH2;
    //string prefixH3;
    
	string nrHead1Str;
	string nrHead2Str;
	string nrHead3Str;
    
    string toInsert;
    string head1TagBeginXML;
	string head2TagBeginXML;
	string head3TagBeginXML;
	
	//At first the beginning tags
    for(size_t i=0; i<numberOfLines-1; i++){

      	if(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head1Begin" ||
            containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head2Begin" ||
            containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head3Begin") {

            posA=containerLines.at(i).tagContainerLine_.at(0).addressTagBegin_;
            posB=containerLines.at(i).tagContainerLine_.at(0).addressTagEnd_;

				//Head 1...
				if(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head1Begin"){
           	 		y=0; //= first record in vector
           	 		
					//check for previous headlines to get the number of required section end tags
					if(nrHead1==0){
           	 			prefixH1 = "";
						}
					else{
						prefixH1 = "</sec>\n";
					}
					
					if(nrSubHeads>0){
						for(int v=0; v<(nrSubHeads); v++){
						prefixH1 = prefixH1 + "</sec>\n";
						}
					}
					
					nrHead1++; nrHead2=0; nrHead3=0;
           	 		
           	 		if(htmlSelected==true){
           	 			toInsert=newHeadlineTags[0];	
						}
						
					else{
												
						head1TagBeginXML=newHeadlineTagsXML[0];
						nrHead1Str = std::to_string(nrHead1);
						
						head1TagBeginXML=regex_replace(head1TagBeginXML, h1, nrHead1Str);
																	
						toInsert = prefixH1 + head1TagBeginXML;
						
						prefixH1="</sec>\n"; //reset
						nrSubHeads=0;
						}
           	 		}	
           	 	
				//Head 2...	
          		if(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head2Begin"){
            		y=1; // s. o.
            		/*
            		if(nrHead2==0){
           	 			prefixH2 = "";
						}
						else{
						prefixH2 = "</sec>\n";	
						} */
            		
               		nrHead2++; nrHead3=0; nrSubHeads++;
            		
            		if(htmlSelected==true){
           	 			toInsert=newHeadlineTags[1];	
						}
            		
            		else{
						head2TagBeginXML=newHeadlineTagsXML[1];
						nrHead2Str = std::to_string(nrHead2);
						
						head2TagBeginXML=regex_replace(head2TagBeginXML, h1, nrHead1Str);
						head2TagBeginXML=regex_replace(head2TagBeginXML, h2, nrHead2Str);
						
						
						//head2TagBeginXML.replace(15, 3, nrHead2Str);
						//head2TagBeginXML.replace(11, 3, nrHead1Str);
						
						toInsert = /*prefixH2 + */head2TagBeginXML;
						}
            		}
          		
				  
				//Head3...
				if(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head3Begin"){
           			y=2; // s. o.
           			/*
           			if(nrHead3==0){
          	 				prefixH3 = "";
						}
						else{
						prefixH3 = "</sec>\n";	
						}*/
           			
           			nrHead3++; nrSubHeads++;
           			
           			if(htmlSelected==true){
          	 			toInsert=newHeadlineTags[2];	
						}
           			
					else{
						head3TagBeginXML=newHeadlineTagsXML[2];
						nrHead3Str = std::to_string(nrHead3);
					
						head3TagBeginXML=regex_replace(head3TagBeginXML, h1, nrHead1Str);
						head3TagBeginXML=regex_replace(head3TagBeginXML, h2, nrHead2Str);
						head3TagBeginXML=regex_replace(head3TagBeginXML, h3, nrHead3Str);
					
						//head3TagBeginXML.replace(19, 3, nrHead3Str);
						//head3TagBeginXML.replace(15, 3, nrHead2Str);
						//head3TagBeginXML.replace(11, 3, nrHead1Str);
					
					toInsert = /*prefixH3 + */head3TagBeginXML;
					}
           		}
		
		   	posB++;
			articleFile[i].insert(posB, toInsert);
			articleFile[i].erase(posA, (posB-posA));
			
      		}
	}
	
	//Set closing section tags
	if(htmlSelected==false){
	
		int finalParagraph;
		
		for(int i=containerLines.size(); i>0; i--){
			if(containerLines[i].toBeNumbered==true){
			finalParagraph=i;
			break;	
			}
		}
		
		
		
		vector<string> closingTags;
		for(int i=0; i<=nrSubHeads; i++){
			closingTags.push_back("</sec>\n");
		}
			for(string v : closingTags){
				cout << v;
			}
		cout << "Inserted in " << finalParagraph << endl;
		articleFile.insert(articleFile.begin()+finalParagraph+1, closingTags.begin(), closingTags.end());	
		
	}

		
	//After alterating the file analyze again
    analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);


	//... now at the end of the headlines
    numberOfLines=articleFile.size();
    
	for(size_t i=0; i<numberOfLines-1; i++){
		
		size_t containerSize;
        containerSize=containerLines.at(i).tagContainerLine_.size();

        for(size_t z=0; z<containerSize; z++){
            if(containerLines.at(i).tagContainerLine_.at(z).typeOfTag_ == "head1End" ||
               containerLines.at(i).tagContainerLine_.at(z).typeOfTag_ == "head2End" ||
               containerLines.at(i).tagContainerLine_.at(z).typeOfTag_ == "head3End" ) {

                posA=containerLines.at(i).tagContainerLine_.at(z).addressTagBegin_;
                posB=(containerLines.at(i).tagContainerLine_.at(z).addressTagEnd_);
                y=3;

                posB++;
                
                if(htmlSelected==true){
                	toInsert = newHeadlineTags[y];
					}		
                
                else{
                	toInsert = newHeadlineTagsXML[y];	
				}
                
				articleFile[i].insert(posB, toInsert);
				articleFile[i].erase(posA, (posB-posA));
	           	}
        	}
        	}
	
}

void set_figureReferencesTags(vector<string> &articleFile) {

	vector<int> dotVector;
	vector<int> dashVector;
	
	vector<int> singleNumbers;
    vector<string> numberPairs;
    vector<string> separationCharacter;
	
	vector<figureReferencesClass> referenceContainer{};
	
	bool startBracketIsSet=false;

	int index1, index2, posA, posB, posDot, posDash, numberOfDigits;

	string toCheck1, toCheck2, toCheck3, bracketContent, language;

	
	//Extract bracket content

	for(size_t i=0; i<articleFile.size(); i++) {
		index1=0;
		index2=0;

				if(articleFile[i].size()>5) { //only 5 characters will be checked

			for(size_t y=0; y<articleFile[i].size(); y++) {

				for(int z1=index1; z1<=(index1+4); z1++) {
					toCheck1=toCheck1+articleFile[i][z1];
					if( z1==(index1+4) &&
					        (toCheck1=="(Fig." ||
					         toCheck1=="(fig." ||
					         toCheck1=="(Abb.")) {
						posA=z1-4;

						startBracketIsSet=true;
						numberOfDigits=toCheck1.size();
						
						if(toCheck1=="(Abb."){
							language = "ger";
							}
						else{
							language = "eng/fr";
							}

						toCheck1.clear();
					}
					if(z1==(index1+4)) {
						toCheck1.clear();
					}
				}

				for(int z2=index2; z2<=(index2+5); z2++) {
					toCheck2=toCheck2+articleFile[i][z2];
					if( z2==(index2+5) &&
					        (toCheck2=="(Figs." ||
					         toCheck2=="(figs.")) {
						posA=z2-5;

						startBracketIsSet=true;
						numberOfDigits=toCheck2.size();
						language = "eng/fr";
											
						toCheck2.clear();
					}
					if(z2==(index2+5)) {
						toCheck2.clear();
					}
				}

				toCheck3=articleFile[i][y];

				if(toCheck3=="." && startBracketIsSet==true) {
					posDot=y;
					dotVector.push_back(posDot);
				}

				if((toCheck3=="-") && startBracketIsSet==true) {
					posDash=y;
					dashVector.push_back(posDash);
				}

				if(toCheck3==")" && startBracketIsSet==true) {
					posB=y;
					startBracketIsSet=false;
					bracketContent=articleFile[i].substr(posA, ((posB-posA)+1));

		//resolve hyphens...		
		bracketContent = resolve_hyphens_in_figRef(bracketContent);

					referenceContainer.push_back( figureReferencesClass {bracketContent, posA, posB, dotVector, dashVector, numberOfDigits} );
					dotVector.clear();
					bracketContent.clear();
				}
				index1++;
				index2++;
			}
		}

	
	//Create new bracket content and insert concent in article file	
	
		string figReferenceTagBeginXML;
			
		string abbreviation;
		
		if(language=="ger"){
			abbreviation = "Abb. ";	
			}
		else{
			abbreviation = "Fig. ";	
			}
		
				
		for(int z=referenceContainer.size()-1; z>=0; z--){
		
			//1. extract fig.numbers..........
				
			std::regex r("[0-9]{1,3}");
			std::sregex_iterator begin{ referenceContainer[z].bracketContent_.cbegin(), referenceContainer[z].bracketContent_.cend(), r};
			std::sregex_iterator end;
			std::smatch match;
	
			vector<int> figureNumbers;
	
			string nr;
			int number;
	
			for (std::sregex_iterator i = begin; i != end; ++i) {
				match = *i;
				nr=match.str();
				number=stoi(nr);
				figureNumbers.push_back(number);
				}
	
			sort(figureNumbers.begin(), figureNumbers.end());
								
			//2. merge numbers  with tags...
			string bracketContentNew="(";
			
			//xml ....
			if(htmlSelected==false){
						
				for(int zz=0; zz<figureNumbers.size(); zz++){
					figReferenceTagBeginXML = figureReferencesClass::figReferenceTagBeginXML_;
					figReferenceTagBeginXML.replace(28,1,std::to_string(figureNumbers[zz]));
						if(zz==0){
						bracketContentNew = bracketContentNew + figReferenceTagBeginXML + abbreviation + std::to_string(figureNumbers[zz]) + figureReferencesClass::figReferenceTagEndXML_;
						}
						else{
						bracketContentNew = bracketContentNew + figReferenceTagBeginXML + ". " + std::to_string(figureNumbers[zz]) + figureReferencesClass::figReferenceTagEndXML_;	
						}
					}
			}
						
			//html ....
			if(htmlSelected==true){
							
				for(int zz=0; zz<figureNumbers.size(); zz++){
					if(zz==0){
						bracketContentNew = bracketContentNew + figureReferencesClass::figReferenceTagBegin_ + abbreviation + std::to_string(figureNumbers[zz]) + figureReferencesClass::figReferenceTagEnd_;
						}
						else{
						bracketContentNew = bracketContentNew + figureReferencesClass::figReferenceTagBegin_ + ". " + std::to_string(figureNumbers[zz]) + figureReferencesClass::figReferenceTagEnd_;	
						}
					}
			}
			
			bracketContentNew = bracketContentNew + ")";
		
			//Not insert in article file ...	
			articleFile[i].replace(referenceContainer[z].posBracketBegin_, (referenceContainer[z].posBracketEnd_-referenceContainer[z].posBracketBegin_+1), bracketContentNew);	
			
			referenceContainer.clear();
			
			}
	}
}

string set_new_tags(string articleFileLine, int posA, int posB, string newTagBegin, string newTagEnd) {
	int posTagEnd;
	posTagEnd=articleFileLine.find(newTagEnd);
	articleFileLine.insert(posTagEnd, "/p>");
	articleFileLine.erase((posTagEnd+3), (posTagEnd+8));

	posB++;
	articleFileLine.insert(posB, newTagBegin);
	articleFileLine.erase(posA, (posB-posA));
	return articleFileLine;

}

string set_new_tags(string articleFileLine, unsigned int posA,  unsigned int posB, string newTagBegin) {

	posB++;
	articleFileLine.insert(posB, newTagBegin);
	articleFileLine.erase(posA, (posB-posA));
	return articleFileLine;

}

string set_paragraphNumbers(string articleFileLine, int posA, int posB, string newParagraphTag, string paragraphNumberTagBegin, string paragraphNumberTagEnd, int * pointerParagrNumber) {

 	string newParagraphTagXML=newParagraphTagXML_;
 	string paragraphNumberTagEndXML=paragraphNumberTagEndXML_;

	int pos2=0;
	posB++;
	
	*pointerParagrNumber = *pointerParagrNumber + 1;
	string numberAsString = std::to_string(*pointerParagrNumber);
	
	//For html:
		if(htmlSelected==true){
		articleFileLine.insert(posB, newParagraphTag);
		pos2=(posB)+(newParagraphTag.size());
		}
		
	//For xml:
		if(htmlSelected==false){
		int posC;
		posC=newParagraphTagXML.find("#nr#");
		newParagraphTagXML.replace(posC, 4, numberAsString);
		articleFileLine.insert(posB, newParagraphTagXML);
		pos2=(posB)+(newParagraphTagXML.size());	
		}	
	
		
	//For html:
		if(htmlSelected==true){
		paragraphNumberTagEnd=paragraphNumberTagBegin+numberAsString+paragraphNumberTagEnd;	
		articleFileLine.insert(pos2, paragraphNumberTagEnd);
		}
	
	//For xml:
		if(htmlSelected==false){
		paragraphNumberTagEndXML=paragraphNumberTagBeginXML+numberAsString+paragraphNumberTagEndXML;	
		articleFileLine.insert(pos2, paragraphNumberTagEndXML);
		}
		
	articleFileLine.erase(posA, (posB-posA));

	return articleFileLine;
}

void show_help() {
	cout << "-------- Help: How to use TagTool_WiZArD application -------- \n" << endl;
	cout << "STEP 1:\nConvert the .docx article file by using pandoc into an .html-file (https://pandoc.org/)." << endl;
	cout << "(pandoc command: pandoc -s -o target.html source.docx)" << endl;
	cout << "(Tested with version pandoc 2.16.2)" << endl;
	cout << "Make sure that the .docx file was generated and prepared with MS Word. Otherwise the file will not be processed successfully." << endl;
	cout << "\nSTEP 2:\nCopy the list of references/bibliography and the illustration credits from the article and convert them into two separate .csv-files (see below)." << endl;
	cout <<	"This can be easily done by using LibeOffice Calc (\"Save as .csv\")" << endl;
	cout << "\nIMPORTANT: Be sure that 1.) the character encoding ist set to \"Unicode (UTF-8)\" and the separating character is set to \"%\"." << endl;
	cout << "(Examples: See AuthorYearList_EXAMPLE.csv and IllustrationCreditList_EXAMPLE.csv in the folder \"\")\n" << endl;	
	cout << "In case you name the files \"AuthorYearList.csv\" and \"IllustrationCreditList.csv\" you will not need to enter the filenames explicitly when starting the application." << endl;
	cout << "If you enter a different filename for .csv-file the application will ask you to specify the required files." << endl;
	cout << "\nSTEP 3:\nBe sure that the source article file and the required .csv-files are saved in the SAME FOLDER together with the tagtool.exe application and its  folder." << endl;
	cout << "\nSTEP 4:\nStart the application by typing \"tagtool.exe articlename.html\"" << endl;
	cout << "\nFunctions:" << endl;
	cout << "If no explicit parameters are entered in the command line, the following functions will be chosen automatically (*RECOMMENDED*): " << endl;
	cout << "- Set customized journal body tags\n- Set figure references tags\n- Set author year tags. *CAUTION*: AuthorYearList.csv *REQUIRED*\n- Set paragraph numbers\n- Insert tagged illustration credits section. *CAUTION*: IllustrationCreditList.csv *REQUIRED*\n- Output format will be HTML." << endl;
	cout << "- Note: If you want to choose the default functions in combination with an xml output enter \"tagtool.exe articlename.html --toXML\" when starting the application" << endl;
	cout << "- Note: The application will load by default the files named \"AuthorYearList.csv\" and \"IllustrationCreditList.csv\" to set author year and illustration credit tags, see above \n"<< endl;
	cout << "Alternatively you can combine the functions by entering following notations when starting the application in your command line (*NOT RECOMMENDED*)" << endl;
	
	show_options();
	
	cout << "\nStep 5:\nThe application writes an edited file with the ending \"_edited_1_.html\" respectively \"_edited_1_.xml\"" << endl;
	cout << "In case of html output fotmat it also writes a folder with data that are important for the correct conversion to MS Word (with the ending \"__ress\")" << endl;
	cout << "\nSTEP 6:\nOpen MS Word and open the edited .html-file. Save it immediately as a .docx-file" << endl;
	cout << "Important: For a correct represantation in MS Word the .html-file and the  folder need to be in the same folder" << endl;
}

void show_options(){
		
	cout << "--toXML = Set output format to xml" << endl;
	cout << "--toHTML = Set output format to html" << endl;
	cout << "--bodyTags = Set customized journal body tags" << endl;
	cout << "--figTags = Set figure references tags" << endl;
	cout << "--litTags = Set author year tags. *CAUTION*: AuthorYearList.csv *REQUIRED*" << endl;
	cout << "--paragrNum = Set paragraph numbers (recommended only if --bodyTags is chosen as well)" << endl;
	cout << "--illCred = Insert tagged illustration credits section. *CAUTION*: IllustrationCreditList.csv *REQUIRED*" << endl;
	cout << "--delTags = Remove dispensable formattings/tags (NOT RECOMMENDED)" << endl;
    
    cout << "(Enter blank spaces between parameters, e. g.:--bodyTags --delTags --figTags)" << endl;
    
}

void structure_xml_output_file(vector<string> &articleFile){
	
	vector<string> insertVector {"</body>\n", "<back>\n"};
		
	vector<reducedValueClass> xml2htmlList;
			
	xml2htmlList=load_reduced_value_list(fileInfos.fileNameHTML_XML_valueList_, xml2htmlList);
			
	search_replace(articleFile, xml2htmlList);
	
	if(documentSections.lineNrFootnotesBeginIsSet==false){
		articleFile.insert(articleFile.begin()+documentSections.lineNrBodyTextEnd_, insertVector.begin(), insertVector.end());	
	}
	if(documentSections.lineNrFootnotesBeginIsSet==false){
		articleFile.insert(articleFile.begin()+documentSections.lineNrFootnotesBegin_, insertVector.begin(), insertVector.end());	
	}	
		
	articleFile.push_back("</back>\n");		
	articleFile.push_back("</article>");
	replace_HtmlHead(articleFile, fileInfos.fileNameNewXMLHead_);
	
}

#endif // TTWFUNCTIONS_H