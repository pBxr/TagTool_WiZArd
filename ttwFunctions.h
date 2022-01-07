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

	for(size_t i=0; i<articleFile.size(); i++) {

		string tagType;
		string tagContent;
		string lineContent;
		string lineType;

		//Isolate plain text
		lineContent=isolate_plainText(articleFile.at(i));

		//Classify type of tags
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
						if(footnoteBeginDetected==false) {
							documentSections.lineNrFootnotesBegin_=i;
							footnoteBeginDetected=true;
						}

					}

					if(tagType=="footnotePandocBacklink") {
						detect_footnoteBacklinks(tagContent, pos1, pos2, footnoteAdressContainer);

					}

					if(tagType=="textBodyPandocEnd" && documentSections.lineNrBodyTextEndIsSet==false) {
						documentSections.lineNrBodyTextEnd_=i;
						documentSections.lineNrBodyTextEndIsSet=true;
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
			break;
		}
	}


	//Check if paragraph is to be flagged as numerable
	for(size_t i=0; i<articleFile.size(); i++) {
		string lineType;
		lineType=detect_numberable_paragraphs(containerLines[i].tagContainerLine_, containerLines[i].plainTextLine_, documentSections, i);
		containerLines[i].lineCategory_=lineType;
	}
}

string classify_tag(string tagContent, size_t i, documentSectionsClass &documentSections) {

	int checkPos1=-1;
	int checkPos2=-1;


	//footnotes pandoc ......
	if((checkPos1=tagContent.find("<a href=\"#fn")==0)&&(checkPos2=tagContent.find("<a href=\"#fnref")==-1)) {
		return "footnoteReferencePandoc";
	}

	if((checkPos1=tagContent.find("<li id=\"fn")==0)&&(checkPos2=tagContent.find("doc-endnote"))>0) {
		return "footnotePandoc";
	}

	if((checkPos1=tagContent.find("<a href=\"#fnref")==0)&&(checkPos2=tagContent.find("doc-backlink"))>0) {
		return "footnotePandocBacklink";
	}
	if((checkPos1=tagContent.find("<section class=\"footnotes\"")==0)) {
		return "textBodyPandocEnd";
	}


	if((checkPos1=tagContent.find("<html"))==0) {
		return "htmlBegin";
	}
	if((checkPos1=tagContent.find("/html>"))>0) {
		return "htmlEnd";
	}

	if((checkPos1=tagContent.find("<head"))==0) {
		return "headBegin";
	}
	if((checkPos1=tagContent.find("/head>"))>0) {
		documentSections.lineNrHtmlHeadEnd_=i;

		return "headEnd";
	}

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


	if((checkPos1=tagContent.find("<body"))==0) {
		documentSections.lineNrBodyTextBegin_=i;
		documentSections.lineNrBodyTextIsSet=true;
		return "bodyBegin";

	}
	if((checkPos1=tagContent.find("/body>"))>0) {
		return "bodyEnd";
	}

	if((checkPos1=tagContent.find("<div"))==0) {
		if((checkPos2=tagContent.find("ootnote"))>0) {
			return "divFootnoteBegin";
		} else {
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
		} else {
			return "paragraphBegin";
		}
	}

	if((checkPos1=tagContent.find("<a class"))==0) {
		if((checkPos2=tagContent.find("ootnote"))>0) {
			return "footnoteAnchorBegin";
		} else {
			return "anchorBegin";
		}
	}

	if((checkPos1=tagContent.find("/a>"))>0) {
		return "anchorEnd";
	}

	if((checkPos1=tagContent.find("<a style"))==0) {
		if((checkPos2=tagContent.find("ootnote"))>0) {
			return "footnoteStyleBegin";
		} else {
			return "styleBegin";
		}
	}


	if((checkPos1=tagContent.find("<h1"))==0) {
		return "head1Begin";
	}
	if((checkPos1=tagContent.find("/h1>"))>0) {
		return "head1End";
	}

	if((checkPos1=tagContent.find("<h2"))==0) {
		return "head2Begin";
	}
	if((checkPos1=tagContent.find("/h2>"))>0) {
		return "head2End";
	}

	if((checkPos1=tagContent.find("<h3"))==0) {
		return "head3Begin";
	}
	if((checkPos1=tagContent.find("/h3>"))>0) {
		return "head3End";
	}


	if((checkPos1=tagContent.find("<h4"))==0) {
		return "head4Begin";
	}
	if((checkPos1=tagContent.find("/h4>"))>0) {
		return "head4End";
	}

	if((checkPos1=tagContent.find("<h5"))==0) {
		return "head5Begin";
	}
	if((checkPos1=tagContent.find("/h5>"))>0) {
		return "head5End";
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

string detect_numberable_paragraphs(vector<tagClass> containerTags, string plainTextLine, struct documentSectionsClass &documentSections, int i) {

	int pruefZahl=plainTextLine.find("&nbsp;");

	if(!containerTags.size()) {
		return "noTextParagraph";
	}

	if(pruefZahl==0) {
		return "blankLine";
	}

	if(i>documentSections.lineNrTextEnd_) {
		return "noTextParagraph";
	}

	if(containerTags.at(0).typeOfTag_=="paragraphBegin" && plainTextLine.size()<125) {
		return "noTextParagraph";
	}

	return "other";
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

void insert_metadataTemplates(vector<string> &articleFile, fileInformations &fileInfos) {
	
	articleFile.insert(articleFile.begin()+documentSections.lineNrBodyTextBegin_+1, fileInfos.metadataBegin.begin(), fileInfos.metadataBegin.end());
	articleFile.insert(articleFile.begin()+fileInfos.metadataBegin.size()+documentSections.lineNrBodyTextEnd_, fileInfos.metadataEnd.begin(), fileInfos.metadataEnd.end());

}

void insert_MSWordFootnoteTags(vector<string> &articleFile, vector<footNoteClass> &footnoteAddressContainer) {

	string referenceFootnoteBeginNEW;
	string referenceFootnoteEndNEW = footNoteClass::referenceFootnoteEndNEW_;
	string footnoteBeginNew;
	string footnoteEndNew = footNoteClass::footnoteEndNEW_;
	string markerFootnoteSection = footNoteClass::markerFootnoteSection_;


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

		referenceFootnoteBeginNEW = footNoteClass::referenceFootnoteBeginNEW_;
		footnoteBeginNew = footNoteClass::footnoteBeginNEW_;

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

		//now replace begin tag; positions are counted by hand...

		referenceFootnoteBeginNEW.replace(59,1,footnoteNumberString);
		referenceFootnoteBeginNEW.replace(43,1,footnoteNumberString);
		referenceFootnoteBeginNEW.replace(29,1,footnoteNumberString);

		posBegin=footnoteAddressContainer[i].adressFNReferenceBegin_;
		posEnd=footnoteAddressContainer[i].adressFNReferenceEnd_;

		lengthReplacingTag=posEnd-posBegin+1;

		articleFile[lineNumberText].replace(posBegin,lengthReplacingTag, referenceFootnoteBeginNEW);


//Now the footnotes___________________________________

		posBacklinkBegin=footnoteAddressContainer[i].adressFNBacklinkBegin_;

		lineNumberText=footnoteAddressContainer[i].lineNumberFootnote_;


		//Positions are counted by hand ("\" are not counted)

		footnoteBeginNew.replace(271,1,footnoteNumberString);
		footnoteBeginNew.replace(122,1,footnoteNumberString);
		footnoteBeginNew.replace(109,1,footnoteNumberString);
		footnoteBeginNew.replace(92,1,footnoteNumberString);
		footnoteBeginNew.replace(40,1,footnoteNumberString);


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

	int size;
	bool referenceListEntered=false;

	size=parameterVector.size();

	if(size==0) {
		cout << "Parameters missing. " << endl;
		cout << "Please enter the name of the .html-source file or enter \"--help\" when starting the application the next time" << endl;
		return 0;
	}

	for(int i=0; i<size; i++) {
		string test(parameterVector[i]);

		if(test=="--help") {
			show_help();
			return 0;
		}
	}

	if(firstRun==true) {

		string test(parameterVector[0]);
		int pos;
		pos = test.find(".html");
		if(pos>0) {
			fileNameEntered=true;
			fileInfos.fileNameSourceFile_=test;
			parameterVector.erase(parameterVector.begin());
			
		} else {
			cout << "\n";
			cout << "NO .HTML-SOURCE FILE WAS ENTERED" << endl;
			return 0;
		}
		
		size=parameterVector.size();
		for(int i=0; i<size; i++) {
		string test2(parameterVector[i]);
		int pos2;
		pos2 = test2.find(".txt");
			if(pos2>0){
				fileInfos.fileNameAuthorYearList_ = test2;
				cout << "You have entered following filename for the list of references/bibliography: " << test2 << endl;
				referenceListEntered=true;
				parameterVector.erase(parameterVector.begin());
			}
		}
		if(referenceListEntered==false){
		fileInfos.fileNameAuthorYearList_ = "RefList.txt";
		
		}
	}
			
	
	size=parameterVector.size();

	if(size==0) {
		customBodyTagsSelected=true;
		figureReferenceTagsSelected=true;
		authorYearTagsSelected=true; 
		removeBlankLinesSelected=true;
		paragraphNumbersSelected=true;
		removeDispensableTagsSelected=false; 
		cout << "No parameters entered, so the default functions will be set:\n- Set customized journal body tags\n- Set figure references tags\n- Set author year tags\n- Remove dispensable blank lines\n- Set paragraph numbers\n" << endl;
	}

	if(size>0) {
		cout << "\n" << "You have chosen following functions: " << endl;

		for(int i=0; i<size; i++) {

			string toCheck(parameterVector[i]);

			if(toCheck=="--bodyTags") {
				customBodyTagsSelected=true;
				cout << "- Set customized journal body tags\n";
			}

			if(toCheck=="---figTags") {
				figureReferenceTagsSelected=true;
				cout << "- Set figure references tags\n";
			}

			if(toCheck=="--litTags") {
				authorYearTagsSelected=true; 
				cout << "- Set author year tags\n";
			}

			if(toCheck=="--delLines") {
				removeBlankLinesSelected=true;
				cout << "- Remove dispensable blank lines\n";
			}

			if(toCheck=="-paragrNum") {
				paragraphNumbersSelected=true;
				cout << "- Set paragraph numbers\n";
			}

			if(toCheck=="--delTags") {
				removeDispensableTagsSelected=true;
				cout << "- Remove dispensable formattings/tags\n";
			}
		}
	}

	bool confirmed=false; 

	while(!confirmed) {
		cout << "\n Please check before running the application: " << endl;
		cout << "- Are you sure that you have converted the MS Word .docx-article-file with pandoc to .html?" << endl;
		cout << "- If chosen: Have you prepared the list of references/bibliography as required (see --help)?" << endl;
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

vector<string> remove_blankLines(vector<string> articleFile, vector<tagClass> &containerTags, vector<lineClass> &containerLines) {

	for(size_t i=articleFile.size(); i>0; i--) {

		if(containerLines.at(i-1).lineCategory_=="blankLine") {
			articleFile.erase(articleFile.begin()+(i-1));
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

void replace_HtmlHead(vector<string>& articleFile, string fileNameHtmlHead) {

	vector<string> newHtmlHead;

	newHtmlHead=loadFileContent(fileNameHtmlHead);

	articleFile.erase(articleFile.begin(), (articleFile.begin()+documentSections.lineNrHtmlHeadEnd_+1));
	articleFile.insert(articleFile.begin(), newHtmlHead.begin(), newHtmlHead.end());


}

void search_replace(vector<string> &articleFile, string termSearch, string termReplace) {

	int pos1;

	for(size_t z=0; z<4; z++) { // 4x, weil Begriff mehrfach vorkommen kann

		for(size_t i=0; i<articleFile.size(); i++) {
			pos1=articleFile[i].find(termSearch);
			if(pos1>=0) {
				articleFile[i].replace(pos1, (termSearch.size()), termReplace);
			}
		}
	}
}

void search_replace(string &textZeile, string termSearch, string termReplace) {

	int pos1;

	for(size_t i=0; i<4; i++) { // 4x, weil Begriff mehrfach vorkommen kann
		pos1=textZeile.find(termSearch);
		if(pos1>=0) {
			textZeile.replace(pos1, (termSearch.size()), termReplace);
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

string set_custom_bodyTag(string articleFileLine, int posA, int posB, string newParagraphTag) {
	posB++;
	articleFileLine.insert(posB, newParagraphTag);
	articleFileLine.erase(posA, (posB-posA));
	return articleFileLine;
}

string set_custom_HeadlineTags(string articleFileLine, int posA, int posB, vector<string> &newHeadlineTags, int y) {

	posB++;
	articleFileLine.insert(posB, newHeadlineTags[y]);
	articleFileLine.erase(posA, (posB-posA));

	return articleFileLine;
}

void set_figureReferencesTags(vector<string> &articleFile, string figReferenceTagBegin, string figReferenceTagEnd) {

	vector<int> dotVector;
	vector<int> dashVector;
	vector<figureReferencesClass> referenceContainer{};

	bool startBracketIsSet=false;

	int index1, index2, posA, posB, posDot, posDash, numberOfDigits;

	string toCheck1, toCheck2, toCheck3, contentBracket;

	size_t lentghOfLine;

	for(size_t i=0; i<articleFile.size(); i++) {
		index1=0;
		index2=0;

		lentghOfLine=articleFile[i].size();

		if(lentghOfLine>5) { //only 5 characters will be checked

			for(size_t y=0; y<lentghOfLine; y++) {

				for(int z1=index1; z1<=(index1+4); z1++) {
					toCheck1=toCheck1+articleFile[i][z1];
					if( z1==(index1+4) &&
					        (toCheck1=="(Fig." ||
					         toCheck1=="(fig." ||
					         toCheck1=="(Abb.")) {
						posA=z1-4;

						startBracketIsSet=true;
						numberOfDigits=toCheck1.size();

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

				if((toCheck3=="-" || toCheck3=="â€“") && startBracketIsSet==true) {
					posDash=y;
					dashVector.push_back(posDash);
				}

				if(toCheck3==")" && startBracketIsSet==true) {
					posB=y;
					startBracketIsSet=false;
					contentBracket=articleFile[i].substr(posA, ((posB-posA)+1));

					referenceContainer.push_back( figureReferencesClass {contentBracket, posA, posB, dotVector, dashVector, numberOfDigits} );
					dotVector.clear();
					contentBracket.clear();
				}
				index1++;
				index2++;
			}
		}

//Insert

		if(referenceContainer.size()>0) {

			int amoutOfDots;

			for(int z=referenceContainer.size()-1; z>=0; z--) {
				articleFile[i].insert(referenceContainer[z].posBracketEnd_, figReferenceTagEnd);
				amoutOfDots=referenceContainer[z].posDots_.size()-1;

				for(int y=amoutOfDots; y>=0; y--) {
					if((referenceContainer[z].posBracketBegin_+(referenceContainer[z].numberOfDigits_-1))
					        == referenceContainer[z].posDots_[y] ) {
						articleFile[i].insert((referenceContainer[z].posDots_[y]-referenceContainer[z].numberOfDigits_+2), figReferenceTagBegin);
					} else {
						articleFile[i].insert(referenceContainer[z].posDots_[y]+1, figReferenceTagBegin);
					}
					if(y>0) {
						articleFile[i].insert(referenceContainer[z].posDots_[y], figReferenceTagEnd);
					}
				}
			}
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

	int pos2=0;
	posB++;

	articleFileLine.insert(posB, newParagraphTag);
	pos2=(posB)+(newParagraphTag.size());
	*pointerParagrNumber = *pointerParagrNumber + 1;
	string numberAsString = std::to_string(*pointerParagrNumber);
	paragraphNumberTagEnd=paragraphNumberTagBegin+numberAsString+paragraphNumberTagEnd;
	articleFileLine.insert(pos2, paragraphNumberTagEnd);
	articleFileLine.erase(posA, (posB-posA));

	return articleFileLine;
}

void show_help() {
	cout << "-------- Help: How to use TagTool_WiZArD application -------- \n" << endl;
	cout << "Step 1: Convert the .docx-article file by using pandoc into an .html-file (https://pandoc.org/)" << endl;
	cout << "(pandoc command: pandoc -s -o target.html source.docx) \n " << endl;
	cout << "Step 2: Copy the list of references/bibliography from the article and save it into a .txt-file." << endl;
	cout << "Be sure that the txt-editor is set to utf8." << endl;	
	cout << "In case you name the list of references \"RefList.txt\" you do not need to enter the filename explicitly when starting the application." << endl;
	cout << "Otherwise when starting the application the second parameter after the filename of the .html-article has to be the filename of the .txt-file of the list of references." << endl;
	cout << "\nImportant 1: The author year abbreviation, the full citation and the identifyer must be separated by the separating charachter \"%\"" << endl;
	cout << "\nExample:" << endl;
	cout << "Filonik 2013%J. Filonik, Athenian impiety trials: a reappraisal, Dike 16, 2013, 11-96%https://zenon.dainst.org/Record/001407098" << endl;
	cout << "Packer 1978%J. Packer, Inns at Pompeii: a short survey, CronPom 4, 1978, 5-53%https://zenon.dainst.org/Record/000434505" << endl;
	cout << "an so on" << endl;
	cout << "\nImportant:" << endl;  
	cout << "- Avoid any headline or any blank lines at the beginning and/or at the end of the list" << endl;
	cout << "- Avoid blank spaces at the beginning and/or at the end of the line or befor or after the separating character \"%\" \n" << endl;
	cout << "Step 3: Be sure that the .html-article-file and the .txt-list of references-file are saved in the same folder together with the TagTool_WiZArD application \n " << endl;
	cout << "Step 4: Start the application by typing \"tagtool articlename.html\" " << endl;
	cout << "If no explicit parameters are entered in the command line, the following functions will be chosen automatically (recommended): " << endl;
	cout << "- Set customized journal body tags\n- Set figure references tags\n- Set author year tags\n- Set paragraph numbers\n- Remove dispensable blank lines" << endl;
	cout << "- Note: The application will load by default the file named \"RefList.txt\" to set author year tags, see above \n"<< endl;
	cout << "Alternatively you can combine the functions by entering following notations when starting the application in your command line (not recommended)" << endl;
	cout << "--bodyTags (= Set customized journal body tags)" << endl;
	cout << "--figTags (= Set figure references tags)" << endl;
	cout << "--litTags (= Set author year tags (separate prepared list of references/bibliography required))" << endl;
	cout << "--paragrNum (= Set paragraph numbers (recommended only if --bodyTags is chosen as well))" << endl;
	cout << "--delLines (= Remove dispensable blank lines)" << endl;
	cout << "--delTags (= Remove dispensable formattings/tags (not recommended))" << endl;
	cout << "Example \"tagtool articlename.html --bodyTags --paragrNum --delLines\" \n" << endl;
	cout << "Step 5: The application writes an edited file with the ending \"_edited_1_.html\"" << endl;
	cout << "It also writes a folder with data that are important for the correct conversion to MS Word (with the ending \"__ress\")" << endl;
	cout << "Step 6: Open MS Word and open the edited .html-file. Save it immediately as a .docx-file" << endl;
	cout << "Important: For a correct represantation in MS Word the .html-file and the ressources folder need to be in the same folder" << endl;
}

void show_options(){
	
	cout << "Please enter one or more of the following parameters:" << endl;
    cout << "--bodyTags = Set customized journal body tags " << endl;
    cout << "--figTags = Set figure references tags" << endl;
    cout << "--litTags = Set author year tags (separate prepared list of references/bibliography required)" << endl;
    cout << "--paragrNum = Set paragraph numbers (recommended only if --bodyTags is chosen as well)" << endl;
    cout << "--delLines = Remove dispensable blank lines" << endl;
    cout << "--delTags = Remove dispensable formattings/tags (not recommended)" << endl;
    cout << "(Enter blank spaces between parameters, e. g.:--bodyTags --delLines --figTags)" << endl;
    
}

#endif // TTWFUNCTIONS_H