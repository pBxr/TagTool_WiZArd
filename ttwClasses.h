#pragma once
#ifndef TTWCLASSES_H
#define TTWCLASSES_H

#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <regex>
#include <filesystem>



using std::cout; using std::cin; using std::vector; using std::string; using std::ifstream; using std::ofstream;
using std::endl; using std::stringstream;

vector<string> loadFileContent(string);

class fileInformations {
public:
    int lapCounter_;
    string sourceFormat_;
    string fileNameSourceFile_;
    string fileNameArticleFile_;
    string fileNameAuthorYearList_;
    string fileNameCreditList_;
    string fileNameMetadataList_;
    string newFileNameFor_;
    string workingPath_;
    
	string folderWriting_;

	string toReplaceIn_;
	string toReplaceInHtmlHead_;

    string fileNameTemplMetadBegin_;
    string fileNameTemplMetadEnd_;
    
    string fileNameNewHtmlHead_;
    string fileNameNewXMLHead_;
	string fileNameColorschememapping_;
    string fileNameFilelist_;
    string fileNameHeader_;
    string fileNameItem0001_;
    
    string fileNameHTML_XML_valueList_; 
    
    vector<string> metadataBegin;
	vector<string> metadataEnd;
    
    vector<string> colorschememapping;
    vector<string> filelist;
    vector<string> header;
    vector<string> item0001;
    
    fileInformations(){
    lapCounter_=0;
    fileNameArticleFile_ = "";
    toReplaceIn_ = "###Name_of_the_edited_file###";
    toReplaceInHtmlHead_ = "###Name__folder###";
    }
    
	void set_lapCounter(){
      lapCounter_++;
    }
    
};


class tagClass {
    public:
    size_t lineNumber_;
    unsigned int serialNumber_;
    unsigned int addressTagBegin_;
    unsigned int addressTagEnd_;
    string typeOfTag_;
    string tagContent_;

    tagClass(size_t nrLine, unsigned int nrOrder, unsigned int a, unsigned int e, string content, string type) {
    lineNumber_ = nrLine;
    serialNumber_= nrOrder;
    addressTagBegin_ = a;
    addressTagEnd_ = e;
    tagContent_= content;
    typeOfTag_= type;
    }

    tagClass(size_t nrLine, unsigned int nrOrder, unsigned int a, unsigned int e, string content ) {
    lineNumber_ = nrLine;
    serialNumber_= nrOrder;
    addressTagBegin_ = a;
    addressTagEnd_ = e;
    tagContent_= content;
    }
    tagClass(){
        lineNumber_ ={};
        serialNumber_= {};
        addressTagBegin_ = {};
        addressTagEnd_ = {};
        tagContent_= {};
        typeOfTag_= {};
    }

};

class lineClass {
    public:
    size_t lineNumber_;
    vector<tagClass> tagContainerLine_;
    string plainTextLine_;
    string lineCategory_;

    lineClass(size_t nr, vector<tagClass> tagContainerLine, string lineContent, string lineCategory){
    lineNumber_=nr;
    tagContainerLine_ = tagContainerLine;
    plainTextLine_= lineContent;
    lineCategory_= lineCategory;

    }

    lineClass() {
        lineNumber_={};
        tagContainerLine_={};
        plainTextLine_={};
        lineCategory_={};
    }

};

struct documentSectionsClass {

    int lineNrHtmlHeadEnd_=0;

    int lineNrAbstractBegin_=0;
    bool lineNrAbstractBeginIsSet_ =false;

    int lineNrKeywordsBegin_=0;
    bool lineNrKeywordsBeginIsSet = false;

    int lineNrAbbreviationsBegin_=0;
    bool lineNrAbbreviationsBeginIsSet =false;

    int lineNrAddressBegin_=0;
    bool lineNrAddressIsSet =false;

    int lineNrSourcesIllustrationsBegin_=0;
    bool lineNrSourcesIllustrationsIsSet = false;

    int lineNrBodyTextBegin_=0;
    bool lineNrBodyTextIsSet = false;

    int lineNrBodyTextEnd_=0;
    bool lineNrBodyTextEndIsSet = false;

    int lineNrTextEnd_;
    bool lineNrTextEndIsSet= false;

    size_t lineNrFootnotesBegin_=0;
    bool lineNrFootnotesBeginIsSet = false;

    static vector<string> checkStringAbstract;
    static vector<string> checkStringAbbreviations;
    static vector<string> checkStringAddress;
    static vector<string> checkStringKeywords;
    static vector<string> checkStringSourcesIllustrations;

	void log(){
	cout << "lineNrAbstractBegin_: " << lineNrAbstractBegin_ << endl;
	cout << "lineNrKeywordsBegin_: " << lineNrKeywordsBegin_ << endl;
	cout << "lineNrAbbreviationsBegin_: " << lineNrAbbreviationsBegin_ << endl;
	cout << "lineNrSourcesIllustrationsBegin_: " << lineNrSourcesIllustrationsBegin_ << endl;
	cout << "lineNrBodyTextBegin_: " << lineNrBodyTextBegin_ << endl;
	cout << "lineNrBodyTextEnd_: " << lineNrBodyTextEnd_ << endl;
	cout << "lineNrFootnotesBegin_: " << lineNrFootnotesBegin_ << endl;
			
	}

    void clear(){
    lineNrAbstractBegin_=0;
    lineNrAbstractBeginIsSet_ =false;

    lineNrKeywordsBegin_=0;
    lineNrKeywordsBeginIsSet = false;

    lineNrAbbreviationsBegin_=0;
    lineNrAbbreviationsBeginIsSet=false;

    lineNrAddressBegin_=0;
    lineNrAddressIsSet=false;

    lineNrSourcesIllustrationsBegin_=0;
    lineNrSourcesIllustrationsIsSet =false;

    lineNrBodyTextBegin_=0;
    lineNrBodyTextIsSet  = false;

    lineNrBodyTextEnd_=0;
    lineNrBodyTextEndIsSet=false;

    lineNrTextEnd_=0;
    lineNrTextEndIsSet= false;

    lineNrFootnotesBegin_=0;
    lineNrFootnotesBeginIsSet=false;
    }

};

vector<string> documentSectionsClass::checkStringAbstract = {"Abstract", "Zusammenfassung"};
vector<string> documentSectionsClass::checkStringAbbreviations = {"rzungen", "rzungsverzeichnis", "Literaturverzeichnis", "Bibliographie", "Bibliography", "Abbreviations", "References", "Abbreviazioni"};
vector<string> documentSectionsClass::checkStringAddress = {"Adresse", "Adressen", "Anschrift",  "Addres"};
vector<string> documentSectionsClass::checkStringKeywords = {"Schlagworte", "Schlagw", "Keywords"};
vector<string> documentSectionsClass::checkStringSourcesIllustrations = {"Abbildungsnachweis", "Sources of illustrations", "Sources of Illustrations", "Illustration Credits", "Illustration credits"};

class figureReferencesClass{
    public:
        int posBracketBegin_;
        int posBracketEnd_;
        vector<int> posDots_;
        vector<int> posDashes_;
        int numberOfDigits_;
        string bracketContent_;
        
		string language_;
        vector<int> singleNumbers_;
        vector<string> numberPairs_;
        vector<string> separationCharacter_;

        static string figReferenceTagBegin_;
        static string figReferenceTagEnd_;
        
        static string figReferenceTagBeginXML_;
        static string figReferenceTagEndXML_;

        figureReferencesClass(string bracketContent, int bracketBegin, int bracketEnd, vector<int> dots, vector<int> dashes, int digits) : bracketContent_ (bracketContent), posBracketBegin_(bracketBegin), posBracketEnd_ (bracketEnd), posDots_ (dots), posDashes_ (dashes), numberOfDigits_ (digits) {}

    };

string figureReferencesClass::figReferenceTagBegin_="<span class=DAItext-abbildung>";
string figureReferencesClass::figReferenceTagEnd_="</span>";

string figureReferencesClass::figReferenceTagBeginXML_="<xref ref-type=\"fig\" rid=\"f-#\">";
string figureReferencesClass::figReferenceTagEndXML_="</xref>";


class authorYearListClass{
public:
    size_t serialNumber_;
    vector<string> citations_;

    static string authorYearTagBegin;
    static string authorYearTagEnd;
    
    static string fullCitationTag1LinkBegin_;
    static string fullCitationTag2LinkBegin_;
    static string fullCitationTagWithoutLink_;

    static string fullCitationTagLinkEnd_;
    static string fullCitationTagWithoutLinkEnd_;

    static string authorYearInFNTagBegin;
    static string authorYearInFNTagEnd;
    
    //xml
    
    static string authorYearTagBeginXML_;
    static string authorYearTagEndXML_;
    
	static string fullCitationTag1LinkBeginXML_;
	static string fullCitationTagLinkEndXML_;
	
	static string authorYearInFNTagBeginXML_;
    static string authorYearInFNTagEndXML_;

    authorYearListClass(size_t number, string authorYear, string fullCitation, string zenonID){
    serialNumber_=number;
    citations_.push_back(authorYear);
    citations_.push_back(fullCitation);
    citations_.push_back(zenonID);
    }

};

//html
string authorYearListClass::authorYearTagBegin="<p class=DAIreferences><span class=DAIreference-lable><span lang=EN-GB style='mso-ansi-language:EN-GB'>";
string authorYearListClass::authorYearTagEnd="<span style='mso-tab-count:1\'>     </span></span></span>";

string authorYearListClass::authorYearInFNTagBegin="<span class=DAInotes-reference-link>";
string authorYearListClass::authorYearInFNTagEnd="</span>";

string authorYearListClass::fullCitationTag1LinkBegin_ = "<a href=\u0022";
string authorYearListClass::fullCitationTag2LinkBegin_ = "\u0022><span class=DAIreference-hyperlink><span lang=EN-GB style=\'mso-ansi-language:EN-GB\'>";

string authorYearListClass::fullCitationTagWithoutLink_ = "<span class=DAIreference-hyperlink><span lang=EN-GB style=\'mso-ansi-language:EN-GB\'>";
string authorYearListClass::fullCitationTagWithoutLinkEnd_ = "</span></span><span lang=EN-GB style=\'mso-ansi-language:EN-GB\'><o:p></o:p></span></p>";

string authorYearListClass::fullCitationTagLinkEnd_ = "</span></span></a><span lang=EN-GB style=\'mso-ansi-language:EN-GB\'><o:p></o:p></span></p>";


//xml
string authorYearListClass::authorYearTagBeginXML_="<ref id=\"ref-##author_year###\"><label>";
string authorYearListClass::authorYearTagEndXML_="</label>";

string authorYearListClass::fullCitationTag1LinkBeginXML_= "<mixed-citation><ext-link xmlns:xlink=\"http://www.w3.org/1999/xlink\" ext-link-type=\"uri\" specific-use=\"zenon\" xlink:href=\"##_BibID_##\">";

string authorYearListClass::fullCitationTagLinkEndXML_ = "</ext-link></mixed-citation></ref>";

string authorYearListClass::authorYearInFNTagBeginXML_="<xref ref-type=\"bibr\" rid=\"ref-##author_year###\">";
string authorYearListClass::authorYearInFNTagEndXML_="</xref>";


class reducedValueClass{
public:
	vector<string> values_;
	
	reducedValueClass(string value1, string value2){
	values_.push_back(value1);
	values_.push_back(value2);
		
	}
	 
	
	
};

class footNoteClass {
public:
    unsigned int footnoteNumber_;
    size_t lineNumberReference_;
    unsigned int lineNumberFootnote_;
    unsigned int adressFNReferenceBegin_;
    unsigned int adressFNReferenceEnd_;
    unsigned int adressFNBegin_;
    unsigned int adressFNEnd_;
    unsigned int adressFNBacklinkBegin_;
    unsigned int adressFNBacklinkEnd_;

    static string referenceFootnoteBeginNEW_;
    static string referenceFootnoteEndNEW_;
    static string footnoteBeginNEW_;
    static string footnoteEndNEW_;
    static string markerFootnoteSection_;
    
    static string referenceFootnoteBeginNEWXML_;
    static string referenceFootnoteEndNEWXML_;
    static string footnoteBeginNEWXML_;
    static string footnoteEndNEWXML_;
    static string markerFootnoteSectionXML_;

    //When detecting of a footnote reference: Create footnote number and save the adress of the reference

    footNoteClass(unsigned int fussNotenNummer, size_t numberOfLines, unsigned int posBegin, unsigned int posEnd) {
        footnoteNumber_=fussNotenNummer;
        lineNumberReference_=numberOfLines;
        adressFNReferenceBegin_=posBegin;
        adressFNReferenceEnd_=posEnd;

    }

    void clear() {
        footnoteNumber_ =0;
        lineNumberReference_ =0;;
        lineNumberFootnote_ =0;
        adressFNReferenceBegin_ =0;
        adressFNReferenceEnd_ =0;
        adressFNBegin_ =0;
        adressFNEnd_ =0;
        adressFNBacklinkBegin_ =0;
        adressFNBacklinkEnd_ =0;
    }


    footNoteClass() {
     footnoteNumber_ ={};
     lineNumberReference_ ={};
     lineNumberFootnote_ = {};
     adressFNReferenceBegin_ ={};
     adressFNReferenceEnd_ ={};
     adressFNBegin_ ={};
     adressFNEnd_ ={};
     adressFNBacklinkBegin_ ={};
     adressFNBacklinkEnd_ ={};

}

};

string footNoteClass::referenceFootnoteBeginNEW_ = "<a style=\'mso-footnote-id:ftn#1#\' href=\"#_ftn#1#\" name=\"_ftnref#1#\" title=\"\"><span class=DAItext-fussnote><span style='font-family:\"Noto Serif\",serif;mso-bidi-font-family: \"Times New Roman\"\'><span style=\'mso-special-character:footnote\'><![if !supportFootnotes]><span class=DAItext-fussnote><span style=\'font-size:11.0pt;font-family:\"Noto Serif\",serif; mso-fareast-font-family:\"Times New Roman\";mso-bidi-font-family:\"Times New Roman\"; mso-ansi-language:DE;mso-fareast-language:EN-US; mso-bidi-language:AR-SA\'>";
string footNoteClass::referenceFootnoteEndNEW_ = "</span></span><![endif]></span></span></span></a>";
string footNoteClass::footnoteBeginNEW_ = "<div style=\'mso-element:footnote\' id=ftn#1#><p class=DAIfootnote><a style=\'mso-footnote-id:ftn#1#\' href=\"#_ftnref#1#\" name=\"_ftn#1#\" title=\"\"><span class=MsoFootnoteReference><span style=\'mso-special-character:footnote\'><![if !supportFootnotes]><span class=MsoFootnoteReference>[#1#]</span></span><![endif]></span></span></span></a> ";
string footNoteClass::footnoteEndNEW_ = "</p></div>";
string footNoteClass::markerFootnoteSection_ = "<div style=\'mso-element:footnote-list\'><![if !supportFootnotes]><br clear=all><hr align=left size=1 width=\"33%\"><![endif]>";

string footNoteClass::referenceFootnoteBeginNEWXML_= "<xref ref-type=\"fn\" rid=\"fn-#1#\">["; 
string footNoteClass::referenceFootnoteEndNEWXML_ = "]</xref>";
string footNoteClass::footnoteBeginNEWXML_ = "<fn id=\"fn-#1#\"><label>#1#</label><p>";
string footNoteClass::footnoteEndNEWXML_ = "</p></fn>";
string footNoteClass::markerFootnoteSectionXML_ = "<fn-group content-type=\"footnotes\"><title>Fussnoten</title>";


class illustrationCreditsClass{
public:	
	size_t serialNumber_;
	vector<string> creditVector_;
	
	//html ...
	static string figNumberLabelTagBegin_;
	static string figNumberLabelTagEnd_;
	
	static string figCreditTagBegin_;
	static string figCreditTagEnd_;


	//xml ....
	static string posterImageXML_;
	static string figNumberLabelTagBeginXML_;
	static string figNumberLabelTagEndXML_;
	
	static string figCaptionsTagBeginXML_;
	static string figCaptionsTagEndXML_;
	
	static string pathFilenameSourcesXML_;
	
	static string figCreditTagBeginXML_;
	static string figCreditTagEndXML_;
	
	
	illustrationCreditsClass(size_t number, string figLabel, string captions, string pathSourceFile, string credits){
    serialNumber_=number;
    creditVector_.push_back(figLabel);
    creditVector_.push_back(captions);
    creditVector_.push_back(pathSourceFile);
    creditVector_.push_back(credits);
	}
	
	illustrationCreditsClass(size_t number, string figLabel, string credits){
    serialNumber_=number;
    creditVector_.push_back(figLabel);
    creditVector_.push_back(credits);
	}
	
		
};

//html...
string illustrationCreditsClass::figNumberLabelTagBegin_ = "<p class=DAIabbildungsverz><span class=DAIabbildungsverz-nummer>";
string illustrationCreditsClass::figNumberLabelTagEnd_ = "</span>";
	
string illustrationCreditsClass::figCreditTagBegin_ = "<span class=DAIabbildungsverz-text><span style='font-family:\"Noto Sans\",sans-serif; mso-bidi-font-family:\"Times New Roman\"'>";
string illustrationCreditsClass::figCreditTagEnd_ = "</span></span></p>";

//xml...
string illustrationCreditsClass::posterImageXML_ = "<fig id=\"poster-image\" fig-type=\"poster-image\"><graphic xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"###_Insert path and file name_###\"/></fig>";
string illustrationCreditsClass::figNumberLabelTagBeginXML_ = "<fig id=\"f-#\" fig-type=\"content-image\"><label>";
string illustrationCreditsClass::figNumberLabelTagEndXML_ = "</label>";

string illustrationCreditsClass::figCaptionsTagBeginXML_ = "<caption><p>";
string illustrationCreditsClass::figCaptionsTagEndXML_ = "</p></caption>";

string illustrationCreditsClass::pathFilenameSourcesXML_ = "<graphic xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"###_Insert path and filename_###\"/>";

string illustrationCreditsClass::figCreditTagBeginXML_ = "<attrib>Source: ";
string illustrationCreditsClass::figCreditTagEndXML_ = "</attrib></fig>";

//Headline tags...
string head1TagBegin="<p class=DAIbody-h1>";
string head2TagBegin="<p class=DAIbody-h2>";
string head3TagBegin="<p class=DAIbody-h3>";
string head1_3TagEnd="</p>";

string head1TagBeginXML_="<sec id=\"s-#1#\"><title>";
string head2TagBeginXML_="</sec>\n<sec id=\"s-#1#.#2#\"><title>";
string head3TagBeginXML_="</sec>\n<sec id=\"s-#1#.#2#.#3#\"><title>";
string head1_3TagEndXML="</title>";

vector<string> newHeadlineTags = {head1TagBegin, head2TagBegin, head3TagBegin, head1_3TagEnd};

vector<string> newHeadlineTagsXML = {head1TagBeginXML_, head2TagBeginXML_, head3TagBeginXML_, head1_3TagEndXML};

//Other tags...
string newParagraphTag="<p class=DAIbody-text>";
string paragraphNumberTagBegin="<span class=DAItext-absatzzahlen>";
string paragraphNumberTagEnd="<span style='mso-tab-count:1'>        </span></span>";

string newParagraphTagXML_="<p id=\"p-#nr#\">";
string paragraphNumberTagBeginXML="<named-content content-type=\"paragraph-counter\">";
string paragraphNumberTagEndXML_="</named-content>";


//Global settings...

bool firstRun=true;
bool nextRunIsSet=true;
bool htmlSelected=true; //=default setting;


bool authorYearTagsSet=false;
bool footnoteTagsSet=false;
bool htmlHeadReplaced=false;
bool dispensableTagsRemoved=false;
bool paragraphNumbersSet=false;
bool imageContainerInserted=false;
bool customBodyTagsSet=false;
bool figureReferenceTagsSet=false;
bool reducedCreditListSelected=false; // Only Fig. number and credit line
bool customBodyTagsSelected;
bool figureReferenceTagsSelected;
bool insertCreditListSelected;

bool authorYearTagsSelected; 
bool removeDispensableTagsSelected; 
bool paragraphNumbersSelected; 
bool fileNameEntered=false; 

fileInformations fileInfos;
vector<tagClass> containerTags{};
vector<lineClass> containerLines{};
vector<footNoteClass> footnoteAdressContainer{};
documentSectionsClass documentSections;
vector<string> articleFile{};
vector<string> litListeVector;

#endif // TTWCLASSES_H
