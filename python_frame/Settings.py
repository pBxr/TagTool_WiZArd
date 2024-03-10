import os, sys

class ttwSettings:
    def __init__(self):
        
        self.reset_settings()
       
    def check_ready_to_run(self):
        if self.selectedFileIsReady == True\
           and self.mandatoryFilesAreComplete == True\
           and self.functionsAreReady == True:
                self.readyToRun = True
        else:
            self.readyToRun = False
        
    def reset_settings(self):
        self.cwd = os.getcwd()
        
        self.target = ""
        
        #Possible export formats
        self.ttwExportFormat = {
                1 : {'Arg' : '--toHTML',
                    'Description' : 'to HTML'},
                2 : {'Arg' : '--toXML',
                    'Description' : 'to XML'}
                }

        #Possible functions
        self.ttwFunctions = {
                1 : {'Arg' : '--DAIStyle',
                    'Description' : 'Custom function: Apply custom DAI citation style features (not yet fully implemented)'},
                2 : {'Arg' : '--bodyTags',
                    'Description' : 'Set customized journal body tags'},
                3 : {'Arg' : '--figTags',
                    'Description' : 'Set figure references tags'},
                4 : {'Arg' : '--litTags',
                    'Description' : 'Set author year tags. *CAUTION*: 02_AuthorYearList.csv *REQUIRED*'},
                5 : {'Arg' : '--paragrNum',
                    'Description' : 'Set paragraph numbers (recommended only if --bodyTags is chosen as well)'},                              
                6 : {'Arg' : '--illCred',
                    'Description' : 'Insert tagged illustration credits section. *CAUTION*: 03_IllustrationCreditList.csv *REQUIRED*'},
                7 : {'Arg' : '--addSR',
                        'Description' : 'Additional search and replace. *CAUTION*: 04_ToSearchAndReplaceList.csv *REQUIRED*'}
                }

        self.ttwFlagsSet = {1 : {'Arg' : '--silent', 'Description' : 'Silent Mode'}}

        #Set default functions
        self.ttwFunctionsSet = {}
        i = 0
        for x, y in self.ttwFunctions.items():
                self.ttwFunctionsSet[i] = {'Arg' : y['Arg'], 'Description' : 'DEFAULT: ' + y['Description']}
                i += 1

        #Set default export format
        self.ttwExportFormatSet = {}
        
        self.set_export_format('--toHTML', 'DEFAULT: ' + 'to HTML')

        #Status
        self.xmlExportIsSet = False
        self.selectedFileIsReady = False
        self.mandatoryFilesAreComplete = False
        self.functionsAreReady = True

        self.pandocSuccessful = False
        self.ttwSuccessful = False
        self.ressourcesSuccessful = False
        self.resultSuccessful = False

        self.readyToRun = False

        self.expectedResultHtml = ""

        #Status messages    
        self.PandocConversionText = "Pandoc conversion"
        self.ArticleConversionText = "TagTool conversion"
        self.RessourcesWrittenText = "Ressources written"
        self.resultText = "Final success: No result yet"

        self.PandocConversionBG = "#e1e1d0"
        self.ArticleConversionBG = "#e1e1d0"
        self.RessourcesWrittenBG = "#e1e1d0"
        self.tboxResultBG = "#e1e1d0"

        #Colors
        self.okGreen = "#ccffcc"
        self.colorNeutral = "#e1e1d0"
        
    def set_export_format(self, arg, desc):
            self.ttwExportFormatSet = {1 : {'Arg' : arg, 'Description' : desc}}
            for x, y, in self.ttwExportFormatSet.items():
                if y['Description'] == "to XML":
                    self.xmlExportIsSet = True
                else:
                    self.xmlExportIsSet = False
                
    def set_functions(self, args):
            i = 0
            self.ttwFunctionsSet = {}
            for arg in args:
                for x, y in self.ttwFunctions.items():
                    if arg == y['Arg']:
                        self.ttwFunctionsSet[i] = {'Arg' : arg, 'Description' : y['Description']}
                        i += 1

class files:
    def __init__(self, settings):

        self.settings = settings

        self.projectPath = ""

        self.fileName = ""

        #Mandatory Files (values represent the number of mandatory cells for checking process)
        self.mandatoryFiles = {
            "01_MetadataValueList.csv" : 1,\
            "02_AuthorYearList.csv" : 2, \
            "03_IllustrationCreditList.csv" : 3,
            "04_ToSearchAndReplaceList.csv" : 1
            }

    def check_mandatory_files(self):
        
        detectedFiles = os.listdir(self.projectPath)
        
        self.files1 = set(list(self.mandatoryFiles.keys())) - set(detectedFiles)
        checkedFiles = []
        missingFiles = []
        
        for file in self.files1:
            missingFiles.append(file)
            checkedFiles.append("[MISSING] -> " + file)

        if len(self.files1)>0:
            self.settings.mandatoryFilesAreComplete = False
            
        else:
            self.settings.mandatoryFilesAreComplete = True
            
         
        self.files2 = set(list(self.mandatoryFiles.keys())) - set(missingFiles)

        for file in self.files2:
            checkedFiles.append("[OK] -> " + file)
        
        checkedFiles.sort()

        if self.settings.mandatoryFilesAreComplete == True:
            checkedFiles = self.check_value_lists()
        
        return checkedFiles

    def check_result(self):
        detectedFiles = os.listdir(self.projectPath)
        mandatoryResults = []
        self.settings.expectedResultHtml = self.settings.target.replace(".html", "_edited_1_.html")
        
        toCheckPandoc = self.settings.target

        if self.settings.xmlExportIsSet == True:
            toCheckArticle = self.settings.target.replace(".html", "_edited_1_.xml")   
        else:
            toCheckArticle = self.settings.expectedResultHtml
        toCheckRessources = toCheckArticle.replace(".html", "_ress")

        #Generate etries...
        if self.settings.xmlExportIsSet == True:
            self.settings.RessourcesWrittenText = ""
        else:
            self.settings.RessourcesWrittenText = "Ressources written"

        #...check and set colors...
        if toCheckPandoc in detectedFiles:
            self.settings.pandocSuccessful = True
            self.settings.PandocConversionBG = self.settings.okGreen
            
        else:
            self.settings.pandocSuccessful = False
            self.settings.PandocConversionBG = "red"

        if toCheckArticle in detectedFiles:
            self.settings.ttwSuccessful = True
            self.settings.ArticleConversionBG = self.settings.okGreen
        else:
            self.settings.ttwSuccessful = False
            self.settings.ArticleConversionBG = "red"

        if self.settings.xmlExportIsSet == False:
            if toCheckRessources in detectedFiles:
                self.settings.ressourcesSuccessful = True
                self.settings.RessourcesWrittenBG = self.settings.okGreen
            else:
                self.settings.ressourcesSuccessful = False
                self.settings.RessourcesWrittenBG = "red"
        
        #Final result
        if  self.settings.pandocSuccessful == True\
            and self.settings.ttwSuccessful == True\
            and (self.settings.xmlExportIsSet == True
            or self.settings.ressourcesSuccessful == True):
            self.settings.resultSuccessful = True
            self.settings.tboxResultBG = self.settings.okGreen
            self.settings.resultText = "Conversion successful. All necessary files found."
        else:
            self.settings.resultSuccessful = False
            self.settings.tboxResultBG = "red"
            self.settings.resultText = "Necessary file(s) missing\n\nCheck source files and value lists and restart."


    def check_value_lists(self):
        resErrors = []
        resWarnings = []
        errorDetected = False
        warningDetected = False

        for x, numberMandatoryCells in self.mandatoryFiles.items():
            with open(self.projectPath + x, 'r', encoding="utf8") as file:
                i=0
                irregularCells = []
                emptyCells = []
                emptyLines = []
                warningSet = False
                lastLineEmpty = False

                for line in file:
                    if not line.strip():
                        emptyLines.append(i+1) #+1 so the result corresponds to the usual line numbers 
                                    
                    res = line.count("|")
                    if res != numberMandatoryCells:
                        irregularCells.append(i+1)
                  
                    cells = line.split("|")
                    for cell in cells:
                        if cell == "" and warningSet == False:
                            emptyCells.append(i+1)
                          
                    i += 1
                    
                for line in file:
                    pass
                lastLine = line
                if "\n" in lastLine:
                    lastLineEmpty = True
                    
                #----- Prepare results
                #- First: Errors:
                if len(irregularCells) > 0:
                    if len(irregularCells) > 1:
                        resAsStr = str(irregularCells)
                    else:
                        resAsStr = str(irregularCells) 
                        resErrors.append(x + ": " + "Check number of cells/pipe symbols (\"|\") in line " + resAsStr)

                if len(emptyLines)>0:
                    resAsStr = str(emptyLines)
                    resErrors.append(x + ": " + "Empty lines: " + resAsStr)

                #- Second: Warnings:    
                if lastLineEmpty == True:
                        resWarnings.append(x + ": " + "Empty last line")

                if len(emptyCells) > 0:
                    resWarnings.append(x + ": " + "Empty cell(s) detected")
            file.close()

        #----- Print results
        resultMessage = []

        if len(resErrors)>0:
            resultMessage.append("ERRORS:")
            for resError in resErrors:
                resultMessage.append(resError)
            errorDetected = True
        else:
            resultMessage.append("NO ERRORS DETECTED\n")

        if len(resWarnings)>0:
            resultMessage.append("\nIf process fails check the following (non critical warning):")
            for resWarning in resWarnings:
                resultMessage.append(resWarning)
                warningDetected = True
        else:
            resultMessage.append("No warnings detected")

        if errorDetected == True: #Only errors lead to red flag for ready to run
            self.settings.mandatoryFilesAreComplete = False

        return resultMessage
