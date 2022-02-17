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

void detect_numberable_paragraphs(vector<string> articleFile);

void get_lineNumbers_documentSections(string, size_t, struct documentSectionsClass&);

void get_current_path(fileInformations &fileInfo);

vector<string> identifyParameters(string eingabe); 

void insert_image_credit_list(vector<string> &articleFile);

void insert_metadataTemplates(vector<string> &articleFile, fileInformations &fileInfo);

void insert_MSWordFootnoteTags(vector<string>&, vector<footNoteClass>&);

string isolate_plainText(string);

vector<reducedValueClass> load_reduced_value_list(string fileName, vector<reducedValueClass> valueList);

vector<authorYearListClass> load_value_list(string fileName, vector<authorYearListClass> authorYearList);

vector<illustrationCreditsClass> load_value_list(string fileName, vector<illustrationCreditsClass> illustrationCreditList);

vector<string> loadFileContent(string);

void load_resources();

int processParameter(vector<string> &); 

vector<string> remove_blankLines(vector<string>);

vector<string> remove_disp_formattings(vector<string>, vector<lineClass>&);

void replace_HtmlHead(vector<string> &, string);

string resolve_hyphens_in_figRef(string bracketContent);

void saveFile(vector<string>&, fileInformations&);

void search_replace(vector<string> &articleFile, string termSearch, string termReplace);

void search_replace(vector<string> &articleFile, vector<reducedValueClass> valueList);

void set_authorYearTags(vector<string>&, vector<authorYearListClass>&, struct documentSectionsClass&);

void set_authorYearTags_XML(vector<string>&, vector<authorYearListClass>&, struct documentSectionsClass&);

string set_custom_bodyTag(string, int, int, string);

void set_custom_HeadlineTags(vector<string>&, vector<lineClass>&, struct documentSectionsClass&);

void set_figureReferencesTags(vector<string>&);

string set_new_tags(string, unsigned int, unsigned int, string);

string set_paragraphNumbers(string, int, int, string, string, string, int* );

void show_help(); 

void show_options();

void structure_xml_output_file(vector<string> &articleFile);

void write_resources(fileInformations &fileInfo);

#endif // TTWDECLARATIONS_H