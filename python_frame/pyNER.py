import subprocess
import os


def run_NER_process(files, settings):
    """
    This is only the first step to get a plugin integrated into ttw.
    The complete NER functions will be inserted during the next commits.
    """

    inputText = prepare_folder_and_input_Text(files, settings)

    
def prepare_folder_and_input_Text(files, settings):    

    #Prepare folder
    pathNERresults = files.projectPath + "NER_results"

    if not os.path.exists(pathNERresults):
        os.makedirs(pathNERresults)

    #Convert text to the chosen input format for NER pipeline
    if settings.NER_SettingsSet['Source'] == 'Convert .docx to .txt and get text':
        pandocParameter = "00_Plain_article_text.txt"
    else:
        pandocParameter = "00_Plain_article_text.html"

    #Put together the command to call pandoc to convert the .docx file into the chosen format
    pandocCall = "pandoc -o " + "\"" + pathNERresults + "\\" + pandocParameter + "\"" + " " + "\"" + files.projectPath + files.fileName + "\""

    FNULL = open(os.devnull, 'w') #For subprocess
    subprocess.run(pandocCall, stdout=FNULL, stderr=FNULL, shell=False)

    #Return the plain text to NER.
    #If a structured file is chosen, text gets extracted with bs4
    plainTextPath = pathNERresults + "\\" + pandocParameter

    if settings.NER_SettingsSet['Source'] == 'Convert .docx to .txt and get text':
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
 
