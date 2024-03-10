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

Following features are implemented for the use for the _Archäologischer Anzeiger_ and the special requirements:
- setting tags for author year references, headlines, figure references, illustration credits, article metadata and so on
- setting automatically paragraph numberings
- setting hyperlink references to external bibliographical resources by a value list (list of references/bibliography)

Beginning with version 1.1.0 the application allows to export also `.xml` files although it needs to be stressed that the `.xml` file is a non-valid intermediate product that will need manual finishing (see `ttw_help.html`).  

Although the use case and the implemented features for the semi-automatic formatting seems to be specific for the journal mentioned above, the application can be customized for other purposes since the design allows to alternate element tags or add functions for new kind of value lists and so on.

## Mode of operation

At first, the `.docx` file is converted by using pandoc into an `.html` file. The use of pandoc guarantees a uniform standardized and normalized `.html` structure. After finishing the modification with ttw the edited `.html` file can be opened in Microsoft Word and converted into a standard `.docx` for further processing purposes by the copy editor. For further details see `ttw_help.html`.

ttw consists of two components:

1.) The `Python` framework (`TagTool_WiZArd_Start`):
- it provides the GUI for all settings and the handling of the application
- it also runs several integrity checks on the files
(- step by step it will also take over the functions from the `c++` core)  

2.) The `c++` core (`tagtool_v2-0-0.exe`):
- it runs most of the main tasks
- using the `Python` framework it needs to be embedded into the framework´s main folder
- like in former releases it still can be run as a standalone application using a terminal.

## Prepare the main files

Note: ttw runs only on Windows. MacOS, Linux and so on are not supported yet.

1.) First, install pandoc on your machine:
ttw ist tested with pandoc version 2.16.2. Other versions may cause problems. 
You will find the release here: https://github.com/jgm/pandoc/releases (see "Assets": pandoc-2.16.2-windows-x86_64.msi).

2.) Create `TagTool_WiZArd_Start.exe` using this repo (`python_frame`): 
A simple way to create an `.exe` file from TagTool_WiZArd_Start.py and Settings.py:

pyinstaller -wF --icon="Logo.ico" TagTool_WiZArd_Start.py

(Note: Logo.ico and Logo.gif need to be placed in the same folder as TagTool_WiZArd_Start.py and Settings.py)

Result is `TagTool_WiZArd_Start.exe`.

3.) Create `tagtool_v2-0-0.exe` using this repo (`cpp_core`):
A simple way to create the `tagtool_v2-0-0.exe` file from the `c++` core is to use Embarcadero Dev-C++ 6.3.:
- Open the `.dev` file and add all `c++` files to your project (`main.cpp` and all header files (`.h`))
- If using Embarcadero Dev-C++ 6.3 add "`-std=c++17`" in Project Options -> Parameter s -> C++ compilers.
- Run "Rebuild all". 

Result is `tagtool_v2-0-0.exe`

## How to setup and run

**With the `Python` framework (recommended):**

1.) Create a folder (somewhere on your machine, no matter where) containing the following files:
- TagTool_WiZArd_Start.exe
- ttw_help.html
- Logo.ico
- Logo.gif
- tagtool_v2-0-0.exe (how to create the `.exe` file from the `c++` core see above)
- and the \resources folder (with all necessary files downloaded together with the ttw release)

If you create a shortcut on your desktop to start `TagTool_WiZArd_Start.exe` you don´t have to touch the ttw folder again.

2.) For the normal usage it is recommended to create a separate folder containing the article file that is to be processed (also somewhere on your machine, no matter where). 
Make sure that the 4 mandatory value lists are saved in this folder together with the article file:
- 01_MetadataValueList.csv
- 02_AuthorYearList.csv
- 03_ImageCreditList.csv
- 04_ToSearchAndReplaceList.csv

For preparing the `.csv` files and all other questions how to run the application see `ttw_help.html` resp. "About" -> "Help" in the menue bar after starting the application.

**Alternatively: Stand alone from console:**

After compiling the binary (tagtool_v2-0-0.exe, see above) open a terminal and run "tagtool_v2-0-0.exe" either with the parameter "--help" to get further informations or together with the name of the file you want to process.
Be sure not to omit the `.html`-ending of the file you want to process.
Be sure that all necessary files are saved in the **same folder** together with the `tagtool_v2-0-0.exe` file, i. e.
- 01_MetadataValueList.csv
- 02_AuthorYearList.csv
- 03_ImageCreditList.csv
- 04_ToSearchAndReplaceList.csv
- article.html
- tagtool_v2-0-0.exe
- \resources  

See "--help" to find all necessary informations to run the application in a standalone version.
For preparing the `.csv` files see `ttw_help.html`.

## New in v2.0.0

- Starting with v2.0.0 ttw comes with a GUI, based on `Python/tkinter`. Although the `c++` core can still be used as terminal standalone application (`tagtool_v2-0-0.exe`, see above), it is not recommended, because the `Python` framework does several integrity checks. 

Also new to previous versions: 
- The article file and value lists no longer need to be saved in the same folder with ttw, any directory can be chosen.
- After starting ttw, all parameters (functions, export format) can be set easily by using the GUI-widgets.
- ttw does the conversion from `pandoc` to `.html` automatically, so the source file will be a `.docx`.
- Result files will be saved in the same folder with the article source file. 
- ttw checks whether the mandatory `.csv` lists exist.
- ttw also runs some simple integrity checks on the `.csv` lists (see `ttw_help.html`)
- ttw checks the footnote section for manual paragraph breaks and merges seperated lines highlighting the changes.

## To be done

- Importing article metadata from json
- Exception handling
- Implementing the possibility to reload files that were converted with the application already
- Article languages settings
- Table conversion to `.xml`

## Technical remarks and requirements

- Windows only (Windows 10)
- pandoc version 2.16.2. Other versions may cause problems.

For the `Python` framework:
- Tested with `Python 3.12.0`

For the `c++` core: 
- TDM-GCC 9.2.0 32/64bit
- Tested with following IDE: Embarcadero Dev-C++ 6.3.

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
