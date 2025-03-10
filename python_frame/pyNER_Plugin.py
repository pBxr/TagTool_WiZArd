import time
import subprocess
import os
import re

from bs4 import BeautifulSoup

#Make sure that the basic functions are not beeing affected by this plugin in case of missing libraries.
#A quick check of the environment before running is done by the basic_NER_lib_check() function in the main file.
try:
    import json
    import requests
    from transformers import AutoTokenizer, AutoModelForTokenClassification
    from transformers import pipeline    
except:
    pass


class log_NER_Class:

    def __init__(self, files, settings):

        self.files = files

        self.actualTime = time.localtime()
        self.year, self.month, self.day = self.actualTime[0:3]
        self.hour, self.minute, self.second = self.actualTime[3:6]

        #Create session folder for log and NER results
        timeStamp = (f"{self.year:4d}{self.month:02d}{self.day:02d}_{self.hour:02d}{self.minute:02d}{self.second:02d}_")
        
        files.NERresultPath = self.files.projectPath + timeStamp + "NER_results/"

        if not os.path.exists(files.NERresultPath):
            os.makedirs(files.NERresultPath)
        
        self.logCollector = []
        
        self.logCollector.append(f"TagToolWiZArd NER plugin Log file: {self.year:4d}-{self.month:02d}-{self.day:02d}, {self.hour:02d}:{self.minute:02d}:{self.second:02d}\n\n")
        self.logCollector.append(f"File: {files.fileName}\n\n")
        self.logCollector.append("Selected parameters:\n")
        for x, y in settings.NER_Parameters.items():
            self.logCollector.append(x + ": " + str(y) + "\n")
        
        
    def add_to_log(self, logInput):
        
        self.logCollector.append(logInput)
        
    def save_log(self, files):

        with open(files.NERresultPath + "01_log.txt", 'w', encoding="utf8") as fp:
        
            for logEntry in self.logCollector:
                fp.write(logEntry)
            fp.close()
        

    def save_results(self, files, resultJSONList, resultForCSVList):

        self.save_log(files)
                
        #Now the .csv list
        intro=("Place name|Suggested ID\nNote|\"(*NOT LIKELY*)\" means, "
               "that the entity is not of type \"archaeological-site\", "
               "\"archaeological-area\" or \"populated-place\".\n")
    
        resultForCSVList_sorted = sorted(resultForCSVList)

        with open(files.NERresultPath + "02_Gazetteer_IDs_DRAFT.csv", 'w', encoding="utf8") as fp:
            fp.write(intro + "\n")
            for item in resultForCSVList_sorted:    
                fp.write(item + "\n")
            fp.close()
        
        #Now the complete .json file
        with open(files.NERresultPath + "03_Gazetteer_result_detailed.json", 'w', encoding="utf8") as fp:
            resultJSON = json.dumps(resultJSONList,
                          indent=4, sort_keys=False,
                          separators=(',', ': '), ensure_ascii=False)
            fp.write(resultJSON)
            fp.close()

            
def call_gazetteer(results, logGenerator):

    listGazetteer = []
    listForCSV = []
    csvRow = ""
    logGenerator.add_to_log("\n3. iDAI.gazetteer query result")
    
    for result in results:
        
        #Most simple way to call gazetteer, only for testing purposes, more elaborated filters following. 
        #See also the README.md file here https://github.com/pBxr/NER_Plugin_for_ttw on this point.
        toSearch = "https://gazetteer.dainst.org/search.json?q=" + result
        response = requests.get(toSearch)
        resultListComplete = response.json()
                     
        logGenerator.add_to_log(f"\n--------------------------------------------------------------\nSearching in iDAI.gazetteer for \"{result}\"\n")
        
        logGenerator.add_to_log(f"Number of results: {resultListComplete['total']}\n")
        
        resultList = resultListComplete['result']
        i=1

        for item in resultList:
            if item['prefName']['title']:
                logGenerator.add_to_log(f"Nr. {i}: Preferred Name: {item['prefName']['title']}\n")
            
            if "types" in item:
                logGenerator.add_to_log("Type: ")
                
                for entry in item['types']:
                    logGenerator.add_to_log(entry +", ")
                logGenerator.add_to_log("\n")
            
            if "@id" in item:
                logGenerator.add_to_log(item['@id'])
            
            if item['prefName']['title'] and "@id" in item:
                
                if ("types" in item) and ('archaeological-area' in item['types']
                    or 'populated-place' in item['types']
                    or 'archaeological-site' in item['types']):
                    
                    csvRow = result + "|" + item['@id']
                    logGenerator.add_to_log("\n")
                    listForCSV.append(csvRow)
                else:
                    result2 = result + "(*NOT LIKELY*)"
                    csvRow = result2 + "|" + item['@id']
                    logGenerator.add_to_log("\n")
                    
                    listForCSV.append(csvRow) #To save only the needed entries
            i+=1
        
        logGenerator.add_to_log("\n--------------------------------------------------------------\n")        
        toSearch=""
        
        listGazetteer.append(resultListComplete) #To save the complete result

    return listGazetteer, listForCSV


def filter_NER_results(locationNames):
    """
    This is only a simple placeholder for a more elaborated function.
    """

    locationNamesFiltered = []

    #Check length after deleting all non-alphanumeric characters, words < 3 characters are unlikely
    for name in locationNames:
        nameStripped = re.sub("[\\W\\d\\s]", "", name) 
        if len(nameStripped) > 2:
            locationNamesFiltered.append(name)
    
    return locationNamesFiltered    
   
def prepare_folder_and_input_text(files, settings):    

    #Prepare folder
    pathNERresults = files.projectPath + "NER_results"

    if not os.path.exists(pathNERresults):
        os.makedirs(pathNERresults)

    #Put together the pandoc call to convert the .docx file into the selected format and save it
    pandocParameter = "00_Plain_article_text." + settings.NER_Parameters['Source']

    pandocCall = "pandoc -o " + "\"" + files.NERresultPath + "\\" + pandocParameter + "\"" + " " + "\"" + files.projectPath + files.fileName + "\""

    FNULL = open(os.devnull, 'w') #For subprocess
    subprocess.run(pandocCall, stdout=FNULL, stderr=FNULL, shell=False)

    #Return the plain text for the pipeline.
    #If a structured format like .html is selected, text gets extracted with bs4 and saved as well.
    sourceFilePath = files.NERresultPath + "\\" + pandocParameter

    if settings.NER_Parameters['Source'] != 'html':
        with open(sourceFilePath, 'r', encoding="utf8") as fp:
            inputText = fp.read()
            fp.close()
        return inputText
    else:
        with open(sourceFilePath, 'r', encoding="utf8") as fp:
            soup = BeautifulSoup(fp, "html.parser")
            text = soup.get_text()
            #Remove blank lines
            inputText = str(text).replace('\n\n','')
            fp.close()

            targetFilePath = files.NERresultPath + "\\" + "00_Plain_article_from_html.txt"          
            with open(targetFilePath, 'w', encoding="utf8") as fp:
                for line in inputText:
                    fp.write(line)
                fp.close()
            os.remove(sourceFilePath)
            return inputText


def return_location_names(nerResults, settings, logGenerator):
    
    #Extract names using B/I span
    listNames = []
    toInsert = ""

    for nerResult in reversed(nerResults):
        #Threshold only for the beginning of a location name)
        if (nerResult['entity'] == "B-LOC") and (float(nerResult['score']) > float(settings.NER_Parameters['Threshold'])):
            toInsert = nerResult['word'] + toInsert
            listNames.append(toInsert)
            toInsert = ""

        if nerResult['entity'] == "I-LOC":
            if "##" in nerResult['word']:
                cleaned = nerResult['word'].replace("##", "")
                toInsert = cleaned + toInsert
            else:
                toInsert =  " " + nerResult['word']  + toInsert

    #Repair wrong breaks
    toInsert = ""
    substring = ""

    listNamesFixed = []
                
    for nerResult in listNames:

        if "##" in nerResult:
            substring = nerResult.replace("##", "")
        else:
            toInsert = nerResult + substring
            listNamesFixed.append(toInsert)
            toInsert = ""
            substring = ""

    listNamesFixed.sort()
   
    #Delete duplicates
    locationNames = list(set(listNamesFixed))
    locationNames.sort()

    #Remove unlikely elemtents
    filteredLocationNames = filter_NER_results(locationNames)

    #Add to log   
    logGenerator.add_to_log("\n1. Filtered entities\n")
    for entry in filteredLocationNames:
        logGenerator.add_to_log(entry +", ")
    logGenerator.add_to_log("\n")

    logGenerator.add_to_log("\n2. Verbose NER result:\n")
    for result in nerResults:
        logGenerator.add_to_log(str(result)+"\n")
    return filteredLocationNames


def run_NER_process(files, settings):

    logGenerator = log_NER_Class(files, settings)
    
    inputText = prepare_folder_and_input_text(files, settings)

    try:
        #Now run NER
        tokenizer = AutoTokenizer.from_pretrained(settings.NER_Parameters['Model'])
        model = AutoModelForTokenClassification.from_pretrained(settings.NER_Parameters['Model'])
        nlp = pipeline("ner", model=model, tokenizer=tokenizer)
            
        nerResults = nlp(inputText)
        
        #Now extract names, get iDAI.gazetteer entries and save log and results
        extractedLocationNames = return_location_names(nerResults, settings, logGenerator)
        resultJSONList, resultForCSVList = call_gazetteer(extractedLocationNames, logGenerator)
        logGenerator.save_log(files)
        logGenerator.save_results(files, resultJSONList, resultForCSVList)
        
    except:
        textInfo = ("Some unexpected problem occured while starting the NER pipeline.\n\n"
                    "Check your environment.\n\n"
                    "See \"About\" -> \"Help\" for instructions.\n\n\n")
        logGenerator.add_to_log(textInfo)
        logGenerator.save_log(files)
        return False, textInfo

    else:
        textInfo = "Process finished."
        return True, textInfo
        
            
            
    
