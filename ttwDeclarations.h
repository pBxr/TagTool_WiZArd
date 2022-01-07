#pragma once
#ifndef TTWDECLARATIONS_H
#define TTWDECLATATIONS_H

#include "ttwClasses.h"

using std::cout; using std::cin; using std::vector; using std::string; using std::ifstream; using std::ofstream;
using std::endl; using std::stringstream;


void analyze_articleFile(vector<string>&, vector<tagClass>&, vector<lineClass>&, struct documentSectionsClass&, vector<footNoteClass>&);

string classify_tag(string, size_t, struct documentSectionsClass&);

void create_target_file_and_folder_names(fileInformations&);

void detect_footnotes(string, size_t, unsigned int, unsigned int, vector<footNoteClass>&);

void detect_footnoteBacklinks(string, unsigned int, unsigned int, vector<footNoteClass>&);

string detect_numberable_paragraphs(vector<tagClass>, string, struct documentSectionsClass&, int);

void get_lineNumbers_documentSections(string, size_t, struct documentSectionsClass&);

string get_current_path(fileInformations &fileInfo);

vector<string> identifyParameters(string eingabe); 

void insert_metadataTemplates(vector<string> &articleFile, fileInformations &fileInfo);

void insert_MSWordFootnoteTags(vector<string>&, vector<footNoteClass>&);

string isolate_plainText(string);

vector<authorYearListClass> load_authorYearList(string dateiName, vector<authorYearListClass> authorYearList);

vector<string> loadFileContent(string);

void load_ressources();

int processParameter(vector<string> &); 

vector<string> remove_blankLines(vector<string>, vector<tagClass>&, vector<lineClass>&);

vector<string> remove_disp_formattings(vector<string>, vector<lineClass>&);

void replace_HtmlHead(vector<string> &, string);

void saveFile(vector<string>&, fileInformations&);

void search_replace(vector<string> &articleFile, string termSearch, string termReplace);

void search_replace(string &textZeile, string termSearch, string termReplace);

void set_authorYearTags(vector<string>&, vector<authorYearListClass>&, struct documentSectionsClass&);

string set_custom_bodyTag(string, int, int, string);

string set_custom_HeadlineTags(string, int, int, vector<string>&, int);

void set_figureReferencesTags(string, string, string);

string set_new_tags(string, unsigned int, unsigned int, string);

string set_paragraphNumbers(string, int, int, string, string, string, int* );

void show_help(); 

void show_options();

void write_ressources(fileInformations &fileInfo);


#endif // TTWDECLARATIONS_H