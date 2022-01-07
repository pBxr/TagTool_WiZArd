# TagTool_WiZArD application

## Preliminary remark
Dealing with scientific articles in the humanities most of the actors (authors, copy editors) are still used to work with .docx format to write, process and edit scientific articles. This leads to the problem, that in the humanities the most parts of the editing process are still very often carried out in .docx and mostly by hand, because a .docx environment requires special skills to process semi-automatic alterations of the files.
Reliable solutions for semi-automatic processing .docx files require specific knowledge that many of the institutions or actors do not have.
The use case for this test application was a scientific archaeological journal (Archäologischer Anzeiger, see: https://publications.dainst.org/journals/aa). 
The articles are usually edited at first in .docx, then processed in .indd/.idml (for the print version) and finally in jats xml to be displayed in an instance of the eLife Lens 2.0.0.
Due to this complex process chain it is very important to add annotations and main formatting tags ideally at the beginning of the process in the .docx version to keep it managable and reasonable. 
## Approach
The approach was to find a way for semi-automatic manipulation of textfiles using a format which is less complicated and to which many actors are familiar with: html.
The .html format as intermediate product ensures that even actors with non specific knowledge are able to retrace and understand the results of the alterations. Errors can be detected and corrected easily as well.
## Features
The use for the aricles of the Archäologischer Anzeiger is the reason for some special implemented features in this test version like:
* setting automatical paragraph numberings
* setting hyperlink references to external bibliographical ressources by a value list (list of references/bibliography)
* setting tags for figure references
* and so on
Although the use case and the implemented features for annotations and semi-automatic formatting seems to be very specific for one journal, the application can be customized for other purposes since the design allows to alterate element tags or add functions for new kind of value lists and so on.   
## Mode of operation
Before starting the application the .docx file has to be converted by using pandoc into an .html file (see show_help() function). The use of pandoc guarantees a uniform standardized and normalized .html structure. After finishing the modification by the application the .html file can be opened in Microsoft Word and converted into a standard .docx for further processing purposes by the copy editor and so on. 
## To be done
* Integrating the pandoc conversion
* An elaborate error/exception handling is not implemented yet
* The feature to import the article metadata from json is still in progress
* Explicitly no efforts were made yet to clear up the extensive html head that MS Word needs to re-read the .html file correctly. With the provided head the file is readable although minor errors may occur.
## Technical remarks
* Windows only (Windows 10)
* TDM-GCC 9.2.0 32/64bit
* Tested with following IDE: Embarcadero Dev-C++ 6.3. If using Embarcadero Dev-C++ 6.3 add "-std=c++17" in Project Options -> Parameter s -> C++ compilers.