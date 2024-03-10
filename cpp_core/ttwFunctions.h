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
#include "ttwCustomFunctions.h"


using std::cout; using std::cin; using std::vector; using std::string; using std::ifstream; using std::ofstream;
using std::endl; using std::stringstream; using std::ostringstream;


void analyze_articleFile(vector<string> &articleFile, vector<tagClass> &containerTags, vector<lineClass> &containerLines, 
		struct documentSectionsClass &documentSections, vector<footNoteClass> &footnoteAdressContainer) {

	containerTags.clear();
	containerLines.clear();
	documentSections.clear();
	footnoteAdressContainer.clear();
	footnoteAdressContainer.push_back( footNoteClass {0, 0, 0, 0}); 	//Footnote 0 remains unused, so the index is corresponding with the article´s footnote numbers
	
	unsigned int footnoteNumber=1;

	bool footnoteBeginDetected=false;
	
	for(size_t i=0; i<articleFile.size(); i++) {

		string tagContent;
		string tagType;
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

					containerTags.push_back( tagClass {i, tagNr, pos1, pos2, tagContent, tagType});

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
		if(i>=documentSections.lineNrBodyBegin_ ) {
			get_lineNumbers_documentSections(containerLines[i].plainTextLine_, i, documentSections);
			}
		}
	
		
	//In case article contains no different sections
	if(documentSections.lineNrTextEndIsSet==false){
		documentSections.lineNrTextEnd_=documentSections.lineNrBodyEnd_;
	}
	
	//Check if paragraph is to be flagged as numerable
	detect_numberable_paragraphs(articleFile, containerLines, documentSections);
	
	//Detect lists...
	int lineBegin;
	int lineEnd;
	int serialNumber=1;
	string typeOfList;
		
	for(size_t i=0; i<articleFile.size(); i++) {
		if(i>=documentSections.lineNrBodyBegin_ && i<documentSections.lineNrTextEnd_){
								
			if(containerLines[i].tagContainerLine_[0].typeOfTag_=="listUnorderedBegin" 
				|| containerLines[i].tagContainerLine_[0].typeOfTag_=="listOrderedBegin"){
				lineBegin=i;
							
				if(containerLines[i].tagContainerLine_[0].typeOfTag_=="listUnorderedBegin"){
						typeOfList = "unordered";
					}
				if(containerLines[i].tagContainerLine_[0].typeOfTag_=="listOrderedBegin"){
						typeOfList = "ordered";
					}
			}
				
			if(containerLines[i].tagContainerLine_[0].typeOfTag_=="listUnorderedEnd" 
				|| containerLines[i].tagContainerLine_[0].typeOfTag_=="listOrderedEnd"
				|| containerLines[i].tagContainerLine_[0].typeOfTag_=="listEndXML"){
				lineEnd=i;
				documentSections.containerLists.push_back(listClass { serialNumber, lineBegin, lineEnd, typeOfList });
				serialNumber++;
				}
		}
	}
}

void check_footnotes_for_manual_paragraph_marks(vector<string> &articleFile, 
	struct documentSectionsClass &documentSections){
	
	int pos1;
	int pos2;
	int y;
	
	string mergedFootnote;
	string toInsert;
	vector<int> toDelete;
	
	for(int i=articleFile.size()-1; i>documentSections.lineNrFootnotesBegin_; i--) {
					
		pos1 = articleFile[i].find("<p>");
						
		if(pos1 == 0){
			search_replace(articleFile[i], "<p>", "");
			mergedFootnote = mergedFootnote.insert(0, articleFile[i]);
			
			//In case that there are more than one paragraph: 
			//Search until regular footnote is found and collect all parapgraphs...
			
			for(int y = i-1; y > documentSections.lineNrFootnotesBegin_; i--){
				pos2 = articleFile[y].find("<li id=\"fn");
				
				if(pos2 == 0){
					search_replace(articleFile[y], "</p>", " ");
					toInsert = articleFile[y];
					toInsert.pop_back();
					
					mergedFootnote = mergedFootnote.insert(0, toInsert);
					toDelete.insert(toDelete.begin(), y);
					break;
				}
				else{
					search_replace(articleFile[y], "<p>", "");
					search_replace(articleFile[y], "</p>", "");
					toInsert = articleFile[y];
					toInsert.pop_back();
					
					mergedFootnote = mergedFootnote.insert(0, toInsert);
					toDelete.insert(toDelete.begin(), y);
					y--;
				}
			}
		
		//Set marker in merged footnote:
		search_replace(mergedFootnote, "doc-endnote\"><p>", 
			"doc-endnote\"><p><span style=\"background-color:green;\">"
			"CAUTION: Tool has merged footnote lines with manual line break. "
			"Check the following: </span>");
	
		articleFile[i] = mergedFootnote;
			
		for (auto a : toDelete){
			
			articleFile.erase(articleFile.begin() + a );
		}
		mergedFootnote = "";
		toDelete.clear();	
		}
 	}
}
		
void console_print(string consoleMessage){
	
	if(silentModeSelected==false){
	cout << consoleMessage << endl;	
	}
		
}

string create_replacement_string(string toReplace, string replaceValue){
	
	int checkPos1;
	int checkPos2;
	
	string url;
		
	//Check where the function is called from:
	if(callFromAddSearchReplace==true){
	
		//Check if replaceValue is a link... 					
		checkPos1=replaceValue.find("http");
		   
		if(checkPos1>=0 && searchAndReplaceDone==false){
		url=replaceValue;
		
			//If a @-marker for a single search and replace is set, erase the marker before replacing
			checkPos2=toReplace.find("@");
		
			if(checkPos2==0 && toReplace.size()>1){
		  		toReplace=toReplace.substr(1, toReplace.size());
			}
		
				
			//...and if so, build a full tagged link
			
			if(htmlSelected==true){
				replaceValue="<a href=\"" + url + "\"><span class=DAIbody-hyperlink-extrafeatures>" + toReplace + "</span></a>";
			}
			
			if(htmlSelected==false){
				replaceValue="<ext-link xmlns:xlink=\"http://www.w3.org/1999/xlink\" ext-link-type=\"uri\" specific-use=\"extrafeatures\" xlink:href=\"" + url + "\">" + toReplace + "</ext-link>";
			}
		}
	}
	
	return replaceValue;	
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
		documentSections.lineNrBodyBegin_=i;
		documentSections.lineNrBodyIsSet=true;
		return "bodyBegin";
	}
	
	if((checkPos1=tagContent.find("/body>"))>0) {
		if(documentSections.lineNrBodyEndIsSet==false){
			documentSections.lineNrBodyEnd_=i;
			documentSections.lineNrBodyEndIsSet=true;
			}
		return "bodyEnd";
	}
	
	if((checkPos1=tagContent.find("<!-- CHECK POSITION OF CLOSING TAG-->"))==0) { // For xml: This marker will be set by set_custom_HeadlineTags
		documentSections.lineNrTextEnd_=i;
		documentSections.markerEndOfTextIsSet=true;
		return "markerEndOfText";
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
	
	if((checkPos1=tagContent.find("<section class=\"footnotes footnotes-end-of-document\" role=\"doc-endnotes\">"))==0){
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
		
	//numbered text paragraphs
	if((checkPos1=tagContent.find("<p id="))==0) {
		return "countedTextParagraphXML";
	}
	
	if((checkPos1=tagContent.find("<p class=DAIbody-text>"))==0) {
		return "countedTextParagraphHTML";
	}
	
	
	//lists....

	if((checkPos1=tagContent.find("<ul"))==0) {
		return "listUnorderedBegin";
	}
	
	if((checkPos1=tagContent.find("<list list-type=\"bullet\">"))==0) {
		return "listUnorderedBegin";
	}

	if((checkPos1=tagContent.find("/ul>"))>0) {
		return "listUnorderedEnd";
	}

	if((checkPos1=tagContent.find("<ol"))==0) {
		return "listOrderedBegin";
	}
	
	if((checkPos1=tagContent.find("<list list-type=\"ordered\">"))==0) {
		return "listOrderedBegin";
	}

	if((checkPos1=tagContent.find("/ol>"))>0) {
		return "listOrderedEnd";
	}
		
	if((checkPos1=tagContent.find("</list>"))==0) {
		return "listEndXML";
	}

	if((checkPos1=tagContent.find("<li>"))==0) {
		return "listElementBegin";
	}
	
	if((checkPos1=tagContent.find("<p class=DAIbody-text-list-bulleted"))==0) {
		return "listElementBeginConverted";
	}
	
	if((checkPos1=tagContent.find("/li>"))>0) {
		return "listElementEnd";
	}
	
	if((checkPos1=tagContent.find("</list-item>"))>0) {
		return "listElementEnd";
	}

	if((checkPos1=tagContent.find("<p class=DAIbody-text-list-numbered"))==0) {
		return "listElementBeginOrderedListConverted";
	}
	
	//xml images container
	if((checkPos1=tagContent.find("<sec id=\"images-container\">"))==0) {
		documentSections.lineNrImageContainerXML_=i;
			if(documentSections.lineNrBodyEndIsSet==false){
				documentSections.lineNrBodyEnd_=i;
				documentSections.lineNrBodyEndIsSet=true;
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

	if((checkPos1=tagContent.find("<span class=DAIbody-medium>"))==0) {
		return "strongBeginConverted";
	}

	if((checkPos1=tagContent.find("<em>"))==0) {
		return "emphasizedBegin";
	}
	if((checkPos1=tagContent.find("/em>"))>0) {
		return "emphasizedEnd";
	}

	if((checkPos1=tagContent.find("<span class=DAIitalic>"))==0) {
		return "emphasizedBeginConverted";
	}

	if((checkPos1=tagContent.find("<sup>"))==0) {
		return "superscriptBegin";
	}
	if((checkPos1=tagContent.find("/sup>"))>0) {
		return "superscriptEnd";
	}
	
	if((checkPos1=tagContent.find("<span class=DAIbody-subscript>"))==0) {
		return "superscriptBeginConverted";
	}
		
	if((checkPos1=tagContent.find("<sub"))==0) {
		return "subscriptBegin";
	}
	if((checkPos1=tagContent.find("/sub>"))>0) {
		return "subscriptEnd";
	}
	
	if((checkPos1=tagContent.find("<span class=DAIbody-superscript>"))==0) {
		return "subscriptBegin";
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


void convert_selected_tags(string typeOfTagBegin, vector<string> &articleFile, vector<lineClass> &containerLines, struct documentSectionsClass &documentSections) {
	
	string typeOfTagEnd;
	string newTagBegin;
	string newTagEnd;
	
	if(typeOfTagBegin=="emphasizedBegin"){
		typeOfTagEnd = "emphasizedEnd";
		
		if(htmlSelected==true){
		newTagBegin = italicTagBegin_;
		newTagEnd = variousSpanTagEnd_;
		}
		if(htmlSelected==false){
		newTagBegin = italicBeginXML_;
		newTagEnd = italicEndXML_;	
		}
	}
	
	if(typeOfTagBegin=="strongBegin"){
		typeOfTagEnd="strongEnd";
		
		if(htmlSelected==true){
		newTagBegin = strongBegin_;
		newTagEnd = variousSpanTagEnd_;	
		}
		
		if(htmlSelected==false){
		newTagBegin = strongBeginXML_;
		newTagEnd = strongEndXML_;	
		}
	}
	
	if(typeOfTagBegin=="superscriptBegin"){
		typeOfTagEnd="superscriptEnd";
		
		if(htmlSelected==true){
		newTagBegin = superscriptBegin_;
		newTagEnd = variousSpanTagEnd_;
		}
	}
	
	if(typeOfTagBegin=="subscriptBegin"){
		typeOfTagEnd="subscriptEnd";
		
		if(htmlSelected==true){
		newTagBegin = subscriptBegin_;
		newTagEnd = variousSpanTagEnd_;
		}
	}
	
	for(int i=documentSections.lineNrTextEnd_; i>=documentSections.lineNrBodyBegin_; i--){
		
		for(int y=containerLines[i].tagContainerLine_.size()-1; y>=0; y--){
	
			if(containerLines[i].tagContainerLine_[y].typeOfTag_== typeOfTagEnd){
			
				articleFile[i].replace(containerLines[i].tagContainerLine_[y].addressTagBegin_, containerLines[i].tagContainerLine_[y].tagContent_.size(), newTagEnd);
			
			}	
			
			if(containerLines[i].tagContainerLine_[y].typeOfTag_==typeOfTagBegin){
			
				articleFile[i].replace(containerLines[i].tagContainerLine_[y].addressTagBegin_, containerLines[i].tagContainerLine_[y].tagContent_.size(), newTagBegin);
			
			}
		}
	}
}


void convert_lists(vector<string> &articleFile, vector<lineClass> &containerLines, documentSectionsClass &documentSections){
	
	if(documentSections.containerLists.size()<=0){
		return;
	}
	
	string toInsertUnorderedBegin;
	string toInsertOrderedBegin;
	string toInsertOrderedEnd;
	string toInsertStartTagListXML;
	
	if(htmlSelected==true){
		toInsertUnorderedBegin = listClass::unorderedListTagItemBegin_;
		toInsertOrderedBegin = listClass::orderedListTagItemBegin_;
	}
	
	if(htmlSelected==false){
		toInsertUnorderedBegin = listClass::unorderedListTagItemBeginXML_;
		toInsertOrderedBegin = listClass::unorderedListTagItemBeginXML_;
		toInsertOrderedEnd = listClass::unorderedListTagItemEndXML_;
	}
		
	for(int i=documentSections.containerLists.size()-1; i>=0; i--){
	
		if(documentSections.containerLists[i].listType_=="unordered"){
			toInsertStartTagListXML=listClass::unorderedListTagBeginXML_;
		}
		
		if(documentSections.containerLists[i].listType_=="ordered"){
			toInsertStartTagListXML=listClass::orderedListTagBeginXML_;
		}
		
		for(int y=documentSections.containerLists[i].lineEnd_-1; y>documentSections.containerLists[i].lineBegin_; y--){	
			int paragrBegin;
			int listBegin;
			int listEnd;
						
			listBegin=articleFile[y].find("<li><p>");
		
			if(listBegin==0){
				if(htmlSelected==false){
					articleFile[y].erase(listBegin, 4);	
				}
				if(htmlSelected==true){
					articleFile[y].erase(listBegin, 7);	
				}
			}
		
			listEnd=articleFile[y].find("</li>");	
					
			if(listEnd>=3){
					
				if(htmlSelected==true){
					articleFile[y].erase(listEnd, 5);
				}
			
				if(htmlSelected==false){
					articleFile[y].replace(listEnd, 5, toInsertOrderedEnd);
				}
			}
					
			if(documentSections.containerLists[i].listType_=="unordered"){
				articleFile[y].insert(0, toInsertUnorderedBegin);	
				}
		
			if(documentSections.containerLists[i].listType_=="ordered"){
				articleFile[y].insert(0, toInsertOrderedBegin);
				}
		}
	
		if(htmlSelected==false){
			int a;
			int b;
			a=articleFile[documentSections.containerLists[i].lineBegin_].find("<ul>");
			b=articleFile[documentSections.containerLists[i].lineEnd_].find("</ul>");
			
			if(a>=0){
				articleFile[documentSections.containerLists[i].lineBegin_].erase(a, 4);
				articleFile[documentSections.containerLists[i].lineBegin_].insert(a, toInsertStartTagListXML);	
			}
		
			if(b>=0){
				articleFile[documentSections.containerLists[i].lineEnd_].erase(b, 5);
				articleFile[documentSections.containerLists[i].lineEnd_].insert(b, listClass::listTagEndXML_);	
			}
		}
	}	
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

void detect_numberable_paragraphs(vector<string> articleFile, vector<lineClass> &containerLines, documentSectionsClass &documentSections){
	
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
	//The footnote section is flagged during the classification of the tags

	bool abstract=false;
	bool abbreviations=false;
	bool address=false;
	bool keywords=false;
	bool illusCredits=false;
	int y;

	for(string toCheck : documentSections.checkStringAbstract) {
		y=articleFileLine.find(toCheck);
		if((i>documentSections.lineNrBodyBegin_+10) && y>=0 && y<5) {
			abstract=true;
			documentSections.lineNrAbstractBeginIsSet_=true;
			documentSections.lineNrAbstractBegin_=i;
		}
	}


	for(string toCheck : documentSections.checkStringAbbreviations) {
		y=articleFileLine.find(toCheck);
		if((i>documentSections.lineNrBodyBegin_+10) && y>=0 && y<10) {
			abbreviations=true;
			documentSections.lineNrAbbreviationsBeginIsSet=true;
			documentSections.lineNrAbbreviationsBegin_ =i;
		}
	}

	for(string toCheck : documentSections.checkStringAddress) {
		y=articleFileLine.find(toCheck);
		if((i>documentSections.lineNrBodyBegin_+10) && y>=0 && y<5) {
			address=true;
			documentSections.lineNrAddressIsSet=true;
			documentSections.lineNrAddressBegin_ =i;
		}
	}

	for(string toCheck : documentSections.checkStringKeywords) {
		y=articleFileLine.find(toCheck);
		if((i>documentSections.lineNrBodyBegin_+10) && y>=0 && y<5) {
			keywords=true;
			documentSections.lineNrKeywordsBegin_ =i;
			documentSections.lineNrKeywordsBeginIsSet =true;
		}
	}

	for(string toCheck : documentSections.checkStringSourcesIllustrations) {
		y=articleFileLine.find(toCheck);
		if((i>documentSections.lineNrBodyBegin_+10) && y>=0 && y<5) {
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

void insert_image_credit_list(vector<string> &articleFile, fileInformations &fileInfos, struct documentSectionsClass &documentSections){
	
	vector<illustrationCreditsClass> illustrationCreditList;
	
	illustrationCreditList=load_value_list(fileInfos.fileNameCreditList_, fileInfos.ttwRootPath_, illustrationCreditList);
	
	string nr;

	int number;
		
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
		
		//prepare insert container into article
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
		//html-Version needs two runs resp. two lists because of the different tags 
		//for the credit label and the figure number label
				
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
			illustrationCreditList[i].creditVector_[1] + 			//=caption	
			illustrationCreditsClass::figNumberCaptionTagEnd_+ "\n";
			insertVector2.push_back(toInsert);
			toInsert.clear();
		}
		
	}
	
	//offset defines the end of the article where the lists are to be inserted
	//i. e. before the footnotes or at the end of the body text
	
	int offset; 
	
	if(documentSections.lineNrFootnotesBeginIsSet==true){
		offset = documentSections.lineNrFootnotesBegin_;	
	}	
	
	if(documentSections.lineNrFootnotesBeginIsSet==false){
		
		if(documentSections.markerEndOfTextIsSet==true){
			offset = documentSections.lineNrTextEnd_+1;
			}
		else{
			offset = documentSections.lineNrBodyEnd_;		
		}	
	
	}
		
	articleFile.insert(articleFile.begin()+offset, insertVector.begin(), insertVector.end());
	
	if(htmlSelected==true){
		articleFile.insert(articleFile.begin()+offset, insertVector2.begin(), insertVector2.end());	
	}
	
}

void insert_metadataTemplates(vector<string> &articleFile, fileInformations &fileInfos, struct documentSectionsClass &documentSections) {
	
	vector<reducedValueClass> values;
		
	articleFile.insert(articleFile.begin()+documentSections.lineNrBodyBegin_+1, fileInfos.metadataBegin.begin(), fileInfos.metadataBegin.end());
	
	if(documentSections.lineNrFootnotesBeginIsSet == true){
		articleFile.insert(articleFile.begin()+fileInfos.metadataBegin.size()+documentSections.lineNrFootnotesBegin_, fileInfos.metadataEnd.begin(), fileInfos.metadataEnd.end());	
	}
	
	else{
		articleFile.insert(articleFile.begin()+fileInfos.metadataBegin.size()+documentSections.lineNrBodyEnd_, fileInfos.metadataEnd.begin(), fileInfos.metadataEnd.end());	
	}

  
  	values=load_reduced_value_list(fileInfos.fileNameMetadataList_, fileInfos.ttwRootPath_, values);
   
   	search_replace(articleFile, values);
     
  
}

void insert_FootnoteTags(vector<string> &articleFile, vector<footNoteClass> &footnoteAddressContainer, struct documentSectionsClass &documentSections) {
	
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
			lengthReplacingTag=posEnd-posBegin+6; //+6 because the "<sup>" hast to be overwritten.	
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

	if (htmlSelected==false){
		articleFile.push_back("</fn-group>\n");
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

int processParameters(vector<string> &parameterVector, fileInformations &fileInfos) {

	bool fileNameEntered=false;
	bool referenceListEntered=false;
	bool creditListEntered=false;
	bool metadataListEntered=false;
	bool toSearchAndReplaceListEntered=false;
	bool unspecifiedValueListEntered=false;
	bool outputFormatSpecified=false;
	
	vector<string> enteredFunctions;
	vector<string> enteredValueLists;

	//Note: In the web extension (ttw_webx) all dialogues, parameter settings, errors etc. 
	//are handled in the browser, for the console version it is implemented below.
				
	if(parameterVector.size()==0) {
		cout << "Parameters missing. " << endl;
		cout << "Please enter the name of the source file or enter \"--help\" when starting the application the next time" << endl;
		return 0;
	}


	//Isolate process specifications from functions 		
	
	vector<int> toDelete;
	bool showHelp=false;
	
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
		
		pos = parameterVector[i].find("--tempID");
		if(pos>=0){
			//extract temp folder name
			fileInfos.nameTempDirectory_ = parameterVector[i].substr(8);
			callFromWebSelected=true;
			silentModeSelected=true;
			toDelete.push_back(i);
		}
		
		pos = parameterVector[i].find("--fromPy");
		if(pos>=0){
			//extract temp folder name
			string rawPath;
			rawPath = parameterVector[i].substr(8);
			search_replace(rawPath, "*%20*", " "); //"*%20*" was inserted by Python frame to replace whitespaces
			fileInfos.nameProjectDirectory_ = rawPath;
			cout << "fileInfos.nameProjectDirectory_: " << fileInfos.nameProjectDirectory_ << endl;
			
			callFromPythonSelected=true;
			toDelete.push_back(i);
		}
		
		pos = parameterVector[i].find("--silent");
		if(pos>=0){
			silentModeSelected=true;
			toDelete.push_back(i);
		}
		
		pos = parameterVector[i].find("--help");
		if(pos>=0){
			showHelp=true;
		}
	}
	
	if(showHelp){
			show_help();
			return 0;
		}
		
	if(silentModeSelected==false){
		cout << "\n\n********************************************************************************************************\n" << endl;
   		cout << "   Welcome to TagTool_WiZArD application (" << versionTag << ")"<< endl;
   		cout << "\n   (tagtool_" << versionNumber << ".exe)" << endl;
    	cout << "\n\n********************************************************************************************************\n" << endl;
	}		

	
	//... delete process specifications from functions and value lists	
	for(int i : toDelete){
		parameterVector.erase(parameterVector.begin()+i);
	}
		
	//Extract source format, entered function parameter, entered value lists and outputformat		
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
			cout << "NO SOURCE FILE WAS ENTERED. Check also if required filename extension was entered correctly (\".html\" or \".xml\")." << endl;
			return 0;
	}

	
	if(firstRun==true){
	//Get article and path...
        get_current_path(fileInfos);
    }
	
	//Detect and process function parameters_______________________________________
	
	//... if no function parameters were entered, set default functions... 
	if(enteredFunctions.size()==0) {		//=default setting
		applyCitationStyleSelected=true;
		customBodyTagsSelected=true;
		figureReferenceTagsSelected=true;
		authorYearTagsSelected=true; 
		paragraphNumbersSelected=true;
		insertCreditListSelected=true;
		toSearchAndReplaceSelected=true;
					
		console_print("No parameters entered, so the default functions will be set:\n- Apply custom DAI citation style features (not yet fully implemented)\n- Set customized journal body tags\n- Set figure references tags\n- Set author year tags\n- Set paragraph numbers\n- Insert tagged illustration credits section\n- Search and replace according to the separate value list.\n");
	
	}
			
	//... if funtion parameters were entered explicitly, set functions and get value lists if entered explicitly
		
	int numberParameters=0;
		
	if(enteredFunctions.size()>0) {
		
		console_print("\nYou have chosen following functions: ");	
					
		for(int i=0; i<enteredFunctions.size(); i++) {
			
			if(enteredFunctions[i]=="--DAIStyle") {
				applyCitationStyleSelected=true;
				numberParameters++;
				console_print("- Apply DAI citation Style\n");
			}
						
			if(enteredFunctions[i]=="--bodyTags") {
				customBodyTagsSelected=true;
				numberParameters++;
				console_print("- Set customized journal body tags\n");
			}

			if(enteredFunctions[i]=="--figTags") {
				figureReferenceTagsSelected=true;
				numberParameters++;
				console_print("- Set figure references tags\n");
			}

			if(enteredFunctions[i]=="--litTags") {
				authorYearTagsSelected=true;
				numberParameters++; 
				console_print("- Set author year tags. CAUTION: 02_AuthorYearList.csv required!\n");
			}

			if(enteredFunctions[i]=="--paragrNum") {
				paragraphNumbersSelected=true;
				numberParameters++;
				console_print("- Set paragraph numbers\n");
			}
			
			if(enteredFunctions[i]=="--illCred") {
				insertCreditListSelected=true;
				numberParameters++;
				console_print("- Insert tagged illustration credit section. CAUTION: 03_IllustrationCreditList.csv requried!\n");
			}
			
			if(enteredFunctions[i]=="--addSR") {
				toSearchAndReplaceSelected=true;
				numberParameters++;
				console_print("- Additional search and replace selected. CAUTION: 04_ToSearchAndReplaceList.csv requried!\n");
			}
		}
	}	
		
		//... check, if all parameters have been entered corretly
		if(numberParameters!=enteredFunctions.size()){
			cout << "- UNKNOWN PARAMETER DETECTED. Please check the parameters you have entered and restart the application";
			return 0;
		}
					
		//Output format info
		if(outputFormatSpecified==false){ //= default
		console_print("\nNo output format entered, so the default outout format will be: HTML");
		}
		
		if(outputFormatSpecified==true){
			if(htmlSelected==false){
				console_print("\nYou have chosen the following output format: XML");	
			}
			if(htmlSelected==true){
				console_print("\nYou have chosen the following output format: HTML");	
			}
		}
	
	//Detect and process value lists_______________________________________________
		//... if no value lists were entered set default file names
		
		if(enteredValueLists.size()==0){ 			//= default setting
			
			fileInfos.fileNameMetadataList_ = "01_MetadataValueList.csv";
			metadataListEntered=true;
			
			fileInfos.fileNameAuthorYearList_ = "02_AuthorYearList.csv";
			referenceListEntered=true;
						
			fileInfos.fileNameCreditList_ = "03_IllustrationCreditList.csv";	
			creditListEntered=true;
				
			fileInfos.fileNameToSearchAndReplaceList_ = "04_ToSearchAndReplaceList.csv";
			toSearchAndReplaceListEntered=true;
		
		}
		
			
		//... or check if all filenames of all necessary value lists are existing...
		for(int i=0; i<enteredValueLists.size(); i++){
		if(enteredValueLists[i]=="02_AuthorYearList.csv"){
			fileInfos.fileNameAuthorYearList_ = "02_AuthorYearList.csv";
			referenceListEntered=true;
		}
		if(enteredValueLists[i]=="03_IllustrationCreditList.csv"){
			fileInfos.fileNameCreditList_ = "03_IllustrationCreditList.csv";	
			creditListEntered=true;
		}
		if(enteredValueLists[i]=="01_MetadataValueList.csv"){
			fileInfos.fileNameMetadataList_ = "01_MetadataValueList.csv";
			metadataListEntered=true;
		}
		if(enteredValueLists[i]=="04_ToSearchAndReplaceList.csv"){
			fileInfos.fileNameToSearchAndReplaceList_ = "04_ToSearchAndReplaceList.csv";
			toSearchAndReplaceListEntered=true;
		}
		
		else{
			unspecifiedValueListEntered=true;
		}
	}
				
		//... and if missing: ask for filenames
		if(enteredValueLists.size()>0){
			if(authorYearTagsSelected==true && referenceListEntered==false){
				cout << "Author Year value list is missing." << endl; 
				cout << "Please enter now (CAUTION: .csv file with special separating character \"|\" only): ";
				getline(cin, fileInfos.fileNameAuthorYearList_);
				cout << "\nEntered filename: " << fileInfos.fileNameAuthorYearList_;
				cout << "\n" << endl;
			}
			
			if(insertCreditListSelected==true && creditListEntered==false){
				cout << "Illustrations Credit value list is missing." << endl;
				cout << "Please enter now: (CAUTION: .csv file with special separating character \"|\" only): ";
				getline(cin, fileInfos.fileNameCreditList_);
				cout << "\nEntered filename: " << fileInfos.fileNameCreditList_;
				cout << "\n" << endl;	
			}
			
			if(metadataListEntered==false){
				cout << "Metadata value list is missing." << endl;
				cout << "Please enter now: (CAUTION: .csv file with special separating character \"|\" only): ";
				getline(cin, fileInfos.fileNameMetadataList_);
				cout << "\nEntered filename: " << fileInfos.fileNameMetadataList_;
				cout << "\n" << endl;	
			}
			if(toSearchAndReplaceSelected==true && toSearchAndReplaceListEntered==false){
				cout << "Value list for additional search and replace is missing." << endl;
				cout << "Please enter now: (CAUTION: .csv file with special separating character \"|\" only): ";
				getline(cin, fileInfos.fileNameToSearchAndReplaceList_);
				cout << "\nEntered filename: " << fileInfos.fileNameToSearchAndReplaceList_;
				cout << "\n" << endl;	
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

void replace_HtmlHead(vector<string>& articleFile, fileInformations &fileInfos, struct documentSectionsClass &documentSections) {

	vector<string> newHead;
	vector<reducedValueClass> valueList;
	int insertPoint = documentSections.lineNrHtmlHeadEnd_;
	
		if(htmlSelected==true){
		newHead=loadFileContent(fileInfos.fileNameNewHtmlHead_);	
		}
	
		if(htmlSelected==false){
		newHead=loadFileContent(fileInfos.fileNameNewXMLHead_);	
		}	
	
	if(htmlSelected==false){  
		valueList=load_reduced_value_list(fileInfos.fileNameMetadataList_, fileInfos.ttwRootPath_, valueList);
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


void search_replace(string &textLine, string termSearch, string termReplace) {

	int pos1;
	int z;
	vector<int> positions;
	
	string testTerm;
				
	
			
	for(int y=0; y+termSearch.size()<=(textLine.size()); y++){
	
		z=0;
		
		//Add characters for a comparison with termSearch
		for(z=y; z<termSearch.size()+y; z++){
			testTerm=testTerm+textLine[z];	
		}	
		
		pos1=testTerm.find(termSearch);
		
		if(!pos1){
		positions.push_back(z-termSearch.size());	
		}
		
		testTerm.clear();
	}
	
	//In case of match(es) replace all matches...
	if(positions.size()>0){
	
		for(int p=positions.size()-1; p>=0; p--){
			textLine.replace(positions[p], termSearch.size(), termReplace);	
		}
	}
	
}




void search_replace(vector<string> &articleFile, string termSearch, string termReplace) {

	int pos1;
	int z;
	vector<int> positions;
	
	string testTerm;
				
	for(int i=0; i<articleFile.size(); i++){
		
		positions.clear();
			
		for(int y=0; y+termSearch.size()<=(articleFile[i].size()); y++){
		
			z=0;
			
			//Add characters for a comparison with termSearch
			for(z=y; z<termSearch.size()+y; z++){
				testTerm=testTerm+articleFile[i][z];	
			}	
			
			pos1=testTerm.find(termSearch);
			
			if(!pos1){
			positions.push_back(z-termSearch.size());	
			}
			
			testTerm.clear();
		}
		
		//In case of match(es) replace all matches...
		if(positions.size()>0){
		
			for(int p=positions.size()-1; p>=0; p--){
				articleFile[i].replace(positions[p], termSearch.size(), termReplace);	
			}
		}
	}
}

void search_replace(vector<string> &textVector, vector<reducedValueClass> valueList) {

	int pos1;
	int pos2;
	int z;
	
	vector<int> positions;
	string termSearch;
	string termReplace;
	string testTerm;
		
	for(size_t i=0; i<textVector.size(); i++) {
		
		for(int y=0; y<valueList.size(); y++){
		
		pos1=textVector[i].find(valueList[y].values_[0]);
		
			if(pos1>=0) {		
				termSearch = valueList[y].values_[0];
				
				//Check if replacement value contains a link:
				termReplace=create_replacement_string(valueList[y].values_[0], valueList[y].values_[1]);
												
				positions.clear();
			
				for(int y2=0; y2+termSearch.size()<=(textVector[i].size()); y2++){
					z=0;
					
					//Add characters for a comparison with termSearch
					for(z=y2; z<termSearch.size()+y2; z++){
						testTerm=testTerm+textVector[i][z];	
					}	
										
					pos2=testTerm.find(termSearch);
					
					if(!pos2){
					positions.push_back(z-termSearch.size());	
					}
					
					testTerm.clear();
				}
				
				//In case of match(es) replace all matches...
				if(positions.size()>0){
				
					for(int p=positions.size()-1; p>=0; p--){
									
					textVector[i].replace(positions[p], termSearch.size(), termReplace);	
					
					}
				}	
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

void set_authorYearTags_XML(vector<string> &articleFile, 
	vector<authorYearListClass> &authorYearList, documentSectionsClass &documentSections, 
	vector<footNoteClass>& footnoteAdressContainer, vector<tagClass>& containerTags, vector<lineClass>& containerLines) {

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
				
				int pos30;
				pos30=articleFile[i].find("</p>");
				articleFile[i].replace(pos30, 5, "\n");
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
				
				int pos30;
				pos30=articleFile[i].find("</p>");
				articleFile[i].replace(pos30, 5, "\n");
				int pos40;
				pos40=articleFile[i].find("<p>");
				articleFile[i].erase(pos40, pos40+3);
								
			}
		}
	}

	//Setting the start and the end tag of the list
	analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);
	
	if (documentSections.lineNrReferencesBeginIsSet==true){
		
		int lineNr;
		string toInsert;
		
		lineNr = documentSections.lineNrReferencesBegin_ + (authorYearList.size()*2);	
		toInsert = authorYearListClass::referencesListTagEndXML_ + "\n";
		articleFile[lineNr].insert(0, toInsert);
		
		toInsert = authorYearListClass::referencesListTagBeginXML_ + "\n";
		articleFile[documentSections.lineNrReferencesBegin_].insert(0, toInsert);
		
	}
	 

}

string set_custom_bodyTag(string articleFileLine, int posA, int posB) {
	string toInsert;
	
	if(htmlSelected==true){
		toInsert=newParagraphTag;
		}
	
	if(htmlSelected==false){
		toInsert=newParagraphTagSimpleXML_;
		}
	
	posB++;
	articleFileLine.insert(posB, toInsert);
	articleFileLine.erase(posA, (posB-posA));
	return articleFileLine;
}


void set_custom_HeadlineTags(vector<string> &articleFile, vector<lineClass> &containerLines, 
	documentSectionsClass &documentSections, vector<tagClass> &containerTags, vector<footNoteClass> &footnoteAdressContainer) {

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
    string prefixH2="</sec>\n"; 
    string prefixH3="</sec>\n"; 
        
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
           	 		
					//check if section end tag for the previsous headline is needed
					if(nrHead1==0){
           	 			prefixH1 = "";
						}
					if(nrHead1>0){
						prefixH1 = "</sec>\n";
					}
					if(nrHead2>0){
						prefixH1 = prefixH1 + "</sec>\n";
					}
					
					if(nrHead3>0){
						prefixH1 = prefixH1 + "</sec>\n";
					}
					
					nrHead1++; nrHead2=0; nrHead3=0;
           	 		
           	 		if(htmlSelected==true){
           	 			toInsert=newHeadlineTags[y];	
						}
						
					else{
												
						head1TagBeginXML=newHeadlineTagsXML[y];
						nrHead1Str = std::to_string(nrHead1);
						
						head1TagBeginXML=regex_replace(head1TagBeginXML, h1, nrHead1Str);
																	
						toInsert = prefixH1 + head1TagBeginXML;
						
						prefixH1="</sec>\n"; //reset
						nrSubHeads=0;
						}
           	 		}	
           	 	
				//Head 2...	
          		if(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head2Begin"){
            		y=1; // see above
            		
            		if(nrHead2==0){
           	 			prefixH2 = "";
						}
					if(nrHead2>0){
						prefixH2 = "</sec>\n";	
						}
					if(nrHead3>0){
						prefixH2 = prefixH2 + "</sec>\n";
					}	 
            		
               		nrHead2++; nrHead3=0; nrSubHeads++;
            		
            		if(htmlSelected==true){
           	 			toInsert=newHeadlineTags[y];	
						}
            		
            		else{
						head2TagBeginXML=newHeadlineTagsXML[y];
						nrHead2Str = std::to_string(nrHead2);
						
						head2TagBeginXML=regex_replace(head2TagBeginXML, h1, nrHead1Str);
						head2TagBeginXML=regex_replace(head2TagBeginXML, h2, nrHead2Str);
						
						toInsert = prefixH2 + head2TagBeginXML;
						
						prefixH2="</sec>\n"; //reset
						}
            		}
          		
				  
				//Head3...
				if(containerLines.at(i).tagContainerLine_.at(0).typeOfTag_ == "head3Begin"){
           			y=2; // see above
           			
           			if(nrHead3==0){
          	 			prefixH3 = "";
						}
						else{
						prefixH3 = "</sec>\n";	
						}
           			
           			nrHead3++; nrSubHeads++;
           			
           			if(htmlSelected==true){
          	 			toInsert=newHeadlineTags[y];	
						}
           			
					else{
						head3TagBeginXML=newHeadlineTagsXML[y];
						nrHead3Str = std::to_string(nrHead3);
					
						head3TagBeginXML=regex_replace(head3TagBeginXML, h1, nrHead1Str);
						head3TagBeginXML=regex_replace(head3TagBeginXML, h2, nrHead2Str);
						head3TagBeginXML=regex_replace(head3TagBeginXML, h3, nrHead3Str);
					
					toInsert = prefixH3 + head3TagBeginXML;
					prefixH3="</sec>\n"; //reset
					}
           		}
		
		   	posB++;
			articleFile[i].insert(posB, toInsert);
			articleFile[i].erase(posA, (posB-posA));
			
      		}
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
                y=3; //= last record in vector

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

	//After alterating the file analyze again
    analyze_articleFile(articleFile, containerTags, containerLines, documentSections, footnoteAdressContainer);


	//Now check if opening section tags are not closed properly at the end of the article
		
	if(htmlSelected==false){
		if(nrHead1>0 || nrHead2 > 0 || nrHead3>0){
			articleFile[documentSections.lineNrTextEnd_].insert(0, "</sec>\n<!-- CHECK POSITION OF CLOSING TAG-->\n");
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

 	string newParagraphTagXML=newParagraphTagNumberXML_;
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
	
	//In case of a help call from ttw_webx the tool returns 
	//a console output that read by .php and displayed in the browser window directly 
	
	if(callFromWebSelected==true){
		//For web extension...
		cout << "-------- Help: How to use TagTool_WiZArD application - Extension for small closed networks -------- \n" << endl;
		cout << "Notice: The tool is tested with .docx documents generated and prepared with MS Word. In case the .docx file was created by LibreOffice or other applications the results may differ; check carefully." << endl;
		cout << "\nSTEP 1:\nThe tool itself (tagtool_" << versionNumber << ".exe) requires 1 article file (.docx) and 4 additional .csv value lists:\n- A Metadata Value List \n- A Author Year List\n- The Illustration Credit List\n- A additional Search And Replace List." << endl;
		cout << "\n*CAUTION*: All 4 .csv value lists are *REQUIRED*. In case you do not intend special alterations (references, illustrations, search and replace) you can use the .csv value lists with no entries." << endl;
		cout << "\nThe .csv value lists can easily be prepared by using LibeOffice Calc, see under \"Examples for .csv lists\" in the web version." << endl;
		cout << "\nIMPORTANT: Be sure that \n- the character encoding (\"character set\"/\"Zeichensatz\") is set to \"Unicode (UTF-8)\",\n- the separating character (\"field delimiter\"/\"Feldtrenner\") is set to \"|\",\n- that no \"string delimiter\"/\"Zeichenketten-Trenner\" is entered (to avoid possible conflicts with similar characters)." << endl;
		cout << "\nFunctions:" << endl;
		cout << "If no explicit parameters are chosen, the following functions will be chosen automatically (*RECOMMENDED*): " << endl;
		cout << "- Set customized journal body tags\n- Set figure references tags\n- Set author year tags\n- Set paragraph numbers\n- Insert tagged illustration credits section\n- Search and replace by using the separate value list (see remarks below)\n- Output format will be HTML." << endl;
		cout << "Note: If you chose .xml as output format the tool will create in most cases only a non-valid xml version. Manual completion is necessary, especially concerning the section endings. Check especially the marker \"<!-- CHECK POSITION OF CLOSING TAG-->\" that is set in the xml file automatically." << endl;
		
		cout << "\nAlternatively you can combine the functions mentioned above (*NOT RECOMMENDED*)." << endl;
		
		cout << "\nStep 2:\nThe application writes an edited file with the ending \"_edited_1_.html\" respectively \"_edited_1_.xml\"" << endl;
		cout << "In case of html output format it also writes a folder with data that are important for the correct conversion to MS Word (with the ending \"__ress\")." << endl;
		cout << "In case of xml output the tool generates only a raw and non-valid xml version (see above). " << endl;
		cout << "\nSTEP 3:\nDownload the files. In case of a chosen .html output: Open MS Word and open the edited .html-file. Save it immediately as a .docx-file" << endl;
		cout << "Important: For a correct represantation in MS Word the .html-file, the \\resources folder and the folder \"\\__ress\" need to be in the same folder." << endl;
			
	}else{
		//For console version...
		cout << "-------- Help: How to use TagTool_WiZArD application -------- \n" << endl;
		cout << "STEP 1:\nConvert the .docx article file by using pandoc into an .html-file (https://pandoc.org/)." << endl;
		cout << "(pandoc command: pandoc -s -o target.html source.docx)" << endl;
		cout << "IMPORTANT:\n- Tested with version pandoc 2.16.2, other versions may cause problems." << endl;
		cout << "- Tested with .docx documents generated and prepared with MS Word. In case the .docx file was created by LibreOffice or other applications the results may differ; check carefully." << endl;
		cout << "\nSTEP 2:\nThe tool itself (tagtool_" << versionNumber << ".exe) requires 4 additional .csv value lists as well as the mandatory resources folder, saved IN THE SAME DIRECTORY together with the article file:\n- 01_MetadataValueList.csv\n- 02_AuthorYearList.csv\n- 03_IllustrationCreditList.csv\n- 04_ToSearchAndReplaceList.csv\n- articlefile.html\n- tagtool_" << versionNumber << ".exe\n- \\resources" << endl;
		cout << "\n*CAUTION*: All .csv value lists are *REQUIRED*. In case you do not intend special alterations (references, illustrations, search and replace) you can use the value lists 02, 03 and 04 with no entries." << endl;
		cout << "\nDo not change any files in the resources folder." << endl;
		cout << "\nThe .csv value lists can easily be prepared by using LibeOffice Calc:" << endl;
		cout << "- Prepare each file concerning the categories and so on like in the example files in the folder \\resources: 01_MetadataValueList_TEMPLATE.csv, 02_AuthorYearList_EXAMPLE.csv, 03_IllustrationCreditList_EXAMPLE.csv and 04_ToSearchAndReplaceList_EXAMPLE.csv." << endl;	
		cout <<	"- Finally use the option \"Save as .csv\" for the conversion." << endl;
		cout << "\nIMPORTANT: Be sure that \n- the character encoding (\"character set\"/\"Zeichensatz\") is set to \"Unicode (UTF-8)\",\n- the separating character (\"field delimiter\"/\"Feldtrenner\") is set to \"|\",\n- that no \"string delimiter\"/\"Zeichenketten-Trenner\" is entered (to avoid possible conflicts with similar characters)." << endl;
		cout << "\nIn case you name the files in the above mentioned way you will not need to enter the filenames explicitly when starting the application.\nIf you enter different filenames for .csv-files the application will ask you to specify the required files (*NOT RECOMMENDED*)." << endl;
		cout << "\nSTEP 3:\nStart the application by typing \"tagtool_" << versionNumber << ".exe articlename.html\"" << endl;
		cout << "\nFunctions:" << endl;
		cout << "If no explicit parameters are entered in the command line, the following functions will be chosen automatically (*RECOMMENDED*): " << endl;
		cout << "- Apply custom DAI citation style features (not yet fully implemented)\n- Set customized journal body tags\n- Set figure references tags\n- Set author year tags\n- Set paragraph numbers\n- Insert tagged illustration credits section\n- Search and replace by using the separate value list (see remarks below)\n- Output format will be HTML." << endl;
		cout << "Note: The application will load by default the files named \"01_MetadataValueList.csv\", \"02_AuthorYearList.csv\",\"03_IllustrationCreditList.csv\" and \"04_ToSearchAndReplaceList.csv\", see above."<< endl;
		cout << "Note: The references will be tagged directly in the article file while in opposite the metadata and the image credits will be inserted into the article file additionally. Therefore the bibliography/references must remain in the article." << endl;
		cout << "Note: If you want to choose the default functions in combination with an (raw and non-valid) xml output enter \"tagtool_" << versionNumber << ".exe articlename.html --toXML\" when starting the application." << endl;
		cout << "\nAlternatively you can combine the functions by entering following notations when starting the application in your command line (*NOT RECOMMENDED*)." << endl;
		
		show_options();
		
		cout << "\nStep 4:\nThe application writes an edited file with the ending \"_edited_1_.html\" respectively \"_edited_1_.xml\"" << endl;
		cout << "In case of html output format it also writes a folder with data that are important for the correct conversion to MS Word (with the ending \"__ress\")." << endl;
		cout << "In case of xml output the tool generates only a raw and non-valid xml version (see above). " << endl;
		cout << "\nSTEP 5:\nOpen MS Word and open the edited .html-file. Save it immediately as a .docx-file" << endl;
		cout << "Important: For a correct represantation in MS Word the .html-file, the \\resources folder and the folder \"\\__ress\" need to be in the same folder." << endl;
	}
}

void show_options(){
		
	cout << "(Enter blank spaces between parameters, e. g.:--bodyTags --figTags --paragrNum)" << endl;
	cout << "--toHTML = Set output format to html" << endl;
	cout << "--toXML = Set output format to xml. Improtant: This function will create in most cases only a non-valid xml version. Manual completion is necessary, especially concerning the section endings. Check especially the marker \"<!-- CHECK POSITION OF CLOSING TAG-->\" that is set in the xml file automatically." << endl;
	cout << "--DAIStyle = Custom function: Apply custom DAI citation style features (not yet fully implemented)" << endl;
	cout << "--bodyTags = Set customized journal body tags" << endl;
	cout << "--figTags = Set figure references tags" << endl;
	cout << "--litTags = Set author year tags. *CAUTION*: 02_AuthorYearList.csv *REQUIRED*" << endl;
	cout << "--paragrNum = Set paragraph numbers (recommended only if --bodyTags is chosen as well)" << endl;
	cout << "--illCred = Insert tagged illustration credits section. *CAUTION*: 03_IllustrationCreditList.csv *REQUIRED*" << endl;
	cout << "--addSR = Additional search and replace based on a value list: The purpose of this funcion is mainly to set tagged hyperlinks.\nNote: Other and especially more complex find-replace operations should be done in other applications for better reliability.\nIf a plain url is entered as replacement string, the tool creates the whole tagged link automatically prepared for the chosen output format (.html or .xml). If you want to avoid to add links to all occurrences of the search expression the \"@\" character can be used as a prefix (both in the search expression in the .csv file and in the article text). The tool will remove this prefix when creating the tagged link. See \"04_ToSearchAndReplaceList_EXAMPLE.csv\" in the folder \\resources. *CAUTION*: 04_ToSearchAndReplaceList.csv *REQUIRED*." << endl;
	 
    
}

void structure_xml_output_file(vector<string> &articleFile, fileInformations& fileInfos, 
	struct documentSectionsClass &documentSections){
	
	vector<string> insertVector {"</body>\n", "<back>\n"};
		
	vector<reducedValueClass> xml2htmlList;
			
	xml2htmlList=load_reduced_value_list(fileInfos.fileNameHTML_XML_valueList_, fileInfos.ttwRootPath_, xml2htmlList);
			
	search_replace(articleFile, xml2htmlList);
	
	
	if(documentSections.lineNrFootnotesBeginIsSet==false){
					
		if(documentSections.markerEndOfTextIsSet==true){
			articleFile.insert(articleFile.begin()+documentSections.lineNrTextEnd_, insertVector.begin(), insertVector.end());	
		}
		else{
			articleFile.insert(articleFile.begin()+documentSections.lineNrBodyEnd_, insertVector.begin(), insertVector.end());
		}
	}
	if(documentSections.lineNrFootnotesBeginIsSet==true){
		articleFile.insert(articleFile.begin()+documentSections.lineNrFootnotesBegin_, insertVector.begin(), insertVector.end());	
	}	
			
	articleFile.push_back("</back>\n");		
	articleFile.push_back("</article>");
	
	replace_HtmlHead(articleFile, fileInfos, documentSections);
	
}




#endif // TTWFUNCTIONS_H