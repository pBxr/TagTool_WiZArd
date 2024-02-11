# TagTool_WiZArD application (ttw)

## Introductory remarks

Dealing with scientific articles in the humanities most of the authors and copy editors are still used to work with `.docx` documents to write and edit text documents.
Solutions to prepare `.docx` articles semi-automatically for complex and enhanced output formats require specific knowledge that many institutions or involved actors do not have.
The use case for this test application is a scientific archaeological journal (_Archäologischer Anzeiger_, see: https://publications.dainst.org/journals/aa).
The articles are usually edited at first in `.docx` before they are typeset in `.indd/.idml` (to create a `.pdf` in a complex layout for the print version) and finally converted from `.indd/.idml` to `jats xml` to be displayed in an instance of the eLife Lens 2.0.0.
Due to this complex process it is reasonable to add preferably all annotations and main formatting tags ideally already at the beginning of the process in the `.docx` version to keep the conversion chain managable.

## Approach

The approach was to find a way for semi-automatic alterations of text documents using a format which is less complicated and - even more important - to which many actors are familiar with: `.html`.
The `.html` format as intermediate product ensures that involved actors with even non specific knowledge are able to retrace and to understand the mechanism and the results of the alterations. Errors can be detected and mistakes corrected easily as well.

## Features

The use of the application for the _Archäologischer Anzeiger_ and the special requirements are the reason for the features used in this test version like:

- setting automatically paragraph numberings
- setting hyperlink references to external bibliographical resources by a value list (list of references/bibliography)
- setting tags for figure references
- and so on
Beginning with version 1.1.0 the application allows to export not only `.html` but also `.xml` files although it needs to be stressed that the `.xml` file is a non-valid intermediate product that will need manual finishing (see `--help` function).  

Although the use case and the implemented features for the semi-automatic formatting seems to be specific for the journal mentioned above, the application can be customized for other purposes since the design allows to alternate element tags or add functions for new kind of value lists and so on.

## Mode of operation

Before starting the application the `.docx` file has to be converted by using pandoc into an `.html` file (see `--help` function). The use of pandoc guarantees a uniform standardized and normalized `.html` structure. After finishing the modification by the application the `.html` file can be opened in Microsoft Word and converted into a standard `.docx` for further processing purposes by the copy editor.

## To be done

- Integrating the pandoc conversion
- Importing article metadata from json
- Exception handling
- Implementing the possibility to reload files that were converted with the application already
- Article languages settings
- Reducing the `.html` tags that Microsoft Word needs to open the converted file correctly to a minimum

## Technical remarks and requirements

- Windows only (Windows 10)
- TDM-GCC 9.2.0 32/64bit
- Tested with following IDE: Embarcadero Dev-C++ 6.3. If using Embarcadero Dev-C++ 6.3 add "`-std=c++17`" in Project Options -> Parameter s -> C++ compilers.
- Tested with pandoc version 2.16.2. Other versions may cause problems.

## How to run

After compiling the binary (tagtool_v1-3-1.exe) open a terminal and run "tagtool_v1-3-1.exe" either with the parameter "--help" to get further informations or together with the name of the file you want to process.
Be sure not to omit the `.html`-ending of the file you want to process.
Be sure that all necessary files are saved in the *same folder* together with the `.exe` file, i. e.
- 01_MetadataValueList.csv
- 02_AuthorYearList.csv
- 03_ImageCreditList.csv
- 04_ToSearchAndReplaceList.csv
- article.html
- tagtool_v1-3-1.exe
- \resources  

For preparing the `.csv` files see "--help" function.

## New in v1.3.1

- Added a silent mode that can be called with the argument `--silent`. In this case the applications offers no dialogues or any log information, so it can cooperate better with additional features (e. g. the features and `batch` file added by fabfab1)
- For reasons of clearity a new header file `ttwCustomFunctions` collects functions that don´t belong to the core resp. are needed for special purposes, e. g. in context of a single project or for special feautues (e. g. helping to prepare a special citation style) 
- Deleted unused feature "Remove dispensable formattings/tags"
- Deleted confirmation dialogue "Please check before running the application: ..."
- Suppressed function and dialogue for a next run (until ttw is prepared better for re-editing the same content multiple times, see above).

## New in v1.3.0: Prepared for a Web Extension (ttw_webx) to integrate ttw into small closed networks

A separate web extension (ttw_webx) can be used optionally to integrate TagTool_WiZArD application starting with v1.3.0 into a web-compatible framework for small closed networks. 

Therefore new in v1.3.0: Additional mode implemented when ttw is called from web extension, that means:
- Receive temp ID from ttw_webx and enable interaction with temp folders created by ttw_webx (load source files and save ttw results there)
- Suppress console messages when called web from extension
- Return help statment for this mode to be received and displayed by ttw_webx

## See also

- For ttw_webx see https://github.com/pBxr/ttw_WebExtension
- ID_Extractor (ID_Ex) for extracting IDs and references from `.jats` article files, especially for the above mentioned journals, see   https://github.com/pBxr/ID_Extractor

