import time
import subprocess
import os

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

        self.NERresultPath = self.files.projectPath+"NER_results/"

        if not os.path.exists(self.NERresultPath):
            os.makedirs(self.NERresultPath)
        
        self.logCollector = []
        
        self.logCollector.append(f"TagToolWiZArd NER plugin Log file: {self.year:4d}-{self.month:02d}-{self.day:02d}, {self.hour:02d}:{self.minute:02d}:{self.second:02d}\n")

        self.logCollector.append("\nChosen NER parameters:\n")
        self.logCollector.append("Model: " + settings.NER_ModelIsSet + "\n")
        self.logCollector.append("Task: " + settings.NER_TaskIsSet + "\n")
        self.logCollector.append("Threshold: " + str(settings.NER_ThresholdIsSet) + "\n")
        self.logCollector.append("Source: " + settings.NER_SourceIsSet + "\n")

        
    def add_to_log(self, logInput):
        
        self.logCollector.append(logInput)
        
    def save_log(self):

        with open(self.NERresultPath + "01_log.txt", 'w', encoding="utf8") as fp:
        
            for logEntry in self.logCollector:
                fp.write(logEntry)
            fp.close()
        

    def save_results(self, resultJSONList, resultForCSVList):

        self.save_log()
                
        #Now the .csv list
        intro=("Place name|Suggested ID\nNote|\"(*NOT LIKELY*)\" means, "
               "that the entity is not of type \"archaeological-site\", "
               "\"archaeological-area\" or \"populated-place\".\n")
    
        resultForCSVList_sorted = sorted(resultForCSVList)

        with open(self.NERresultPath + "02_Gazetteer_IDs_DRAFT.csv", 'w', encoding="utf8") as fp:
            fp.write(intro + "\n")
            #print(intro)
            for item in resultForCSVList_sorted:    
                fp.write(item + "\n")
                #print(item)
            fp.close()
        
        #Now the complete .json file
        with open(self.NERresultPath + "03_Gazetteer_result_detailed.json", 'w', encoding="utf8") as fp:
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
    filteredLocationNames = []
    
    for element in locationNames:

        filteredElement = element.replace(" ", "")
        
        if len(filteredElement) > 2:
            filteredLocationNames.append(element)
         
    return filteredLocationNames    
   
def prepare_folder_and_input_text(files, settings):    

    #Prepare folder
    pathNERresults = files.projectPath + "NER_results"

    if not os.path.exists(pathNERresults):
        os.makedirs(pathNERresults)

    #Convert text to the selected input format
    if settings.NER_SourceIsSet == 'Convert .docx to .txt and get text':
        pandocParameter = "00_Plain_article_text.txt"
    else:
        pandocParameter = "00_Plain_article_text.html"

    #Put together the pandoc call to convert the .docx file into the selected format and save it
    pandocCall = "pandoc -o " + "\"" + pathNERresults + "\\" + pandocParameter + "\"" + " " + "\"" + files.projectPath + files.fileName + "\""

    FNULL = open(os.devnull, 'w') #For subprocess
    subprocess.run(pandocCall, stdout=FNULL, stderr=FNULL, shell=False)

    #Return the plain text for the pipeline. If a structured format like .html is selected, text gets extracted with bs4.
    plainTextPath = pathNERresults + "\\" + pandocParameter

    if settings.NER_SourceIsSet == 'Convert .docx to .txt and get text':
        with open(plainTextPath, 'r', encoding="utf8") as fp:
            inputText = fp.read()
            fp.close()
        return inputText
    else:
        with open(plainTextPath, 'r', encoding="utf8") as fp:
            soup = BeautifulSoup(fp, "html.parser")
            text = soup.get_text()
            #Remove blank lines
            inputText = str(text).replace('\n\n','')
            return inputText


def return_location_names(nerResults, settings, logGenerator):
    
    logGenerator.add_to_log("\n1. NER result:\n")
    for result in nerResults:
        logGenerator.add_to_log(str(result)+"\n")
    
    #Extract names using B/I span
    listNames = []
    toInsert = ""

    for nerResult in reversed(nerResults):
        #Threshold only for the beginning of a location name)
        if (nerResult['entity'] == "B-LOC") and (float(nerResult['score']) > float(settings.NER_ThresholdIsSet)):
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
       
    logGenerator.add_to_log("\n2. Filtered entities\n")
    for entry in filteredLocationNames:
        logGenerator.add_to_log(entry +", ")
    logGenerator.add_to_log("\n")
    
    return filteredLocationNames


def run_NER_process(files, settings):

    logGenerator = log_NER_Class(files, settings)
    
    inputText = prepare_folder_and_input_text(files, settings)
    
    try:
        #Now run NER
        tokenizer = AutoTokenizer.from_pretrained(settings.NER_ModelIsSet)
        model = AutoModelForTokenClassification.from_pretrained(settings.NER_ModelIsSet)
        nlp = pipeline("ner", model=model, tokenizer=tokenizer)
            
        nerResults = nlp(inputText)
        
        #Now extract names, get iDAI.gazetteer entries and save log and results
        extractedLocationNames = return_location_names(nerResults, settings, logGenerator)
        resultJSONList, resultForCSVList = call_gazetteer(extractedLocationNames, logGenerator)
        logGenerator.save_log()
        logGenerator.save_results(resultJSONList, resultForCSVList)
        
    except:
        textInfo = ("Some unexpected problem occured while starting the NER pipeline.\n\n"
                    "Check your environment.\n\n"
                    "See \"About\" -> \"Help\" for instructions.\n\n\n")
        logGenerator.add_to_log(textInfo)
        logGenerator.save_log()
        return False, textInfo

    else:
        textInfo = "Process finished. Check result"
        return True, textInfo
        
            
            
    
