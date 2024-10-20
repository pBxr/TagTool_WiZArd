import os, sys
import platform
import tkinter
import subprocess
import webbrowser

from tkinter import ttk, Text
from tkinter import messagebox
from tkinter import filedialog
from tkinter.font import Font
from tkinter.messagebox import showinfo
from tkinter import Menu

from PIL import ImageTk, Image

from Settings import ttwSettings
from Settings import files

import pyScripts as pyScr


class MainWindow(tkinter.Frame):
        
        def __init__(self, root):
                super().__init__(root)
                               
                self.settings = ttwSettings()
                self.files = files(self.settings)

                self.textFont = Font(family="Calibri", size=11)
                self.grid(padx = (10, 10), pady = (10, 10))

                self.reset_app() #Default settings
                            
                self.menu = tkinter.Menu(root)
                root.config(menu=self.menu)
                self.create_MenuBar()
                  
                self.actualize_widgets()

                #Check os
                operatingSystem = platform.system()
                if operatingSystem != "Windows":
                    tkinter.messagebox.showwarning(title="ERROR", \
                                     message="ATTENTION: Tool only for WINDOWS")
                    root.destroy()

        def actualize_widgets(self):
                self.settings.check_ready_to_run()
                
                heightRow1 = 15
                
                #Article file select box ------First Section-----------------
                self.groupArticleFileBox = tkinter.LabelFrame(self)
                self.groupArticleFileBox["text"] = "Files"
                self.groupArticleFileBox.grid(column = 1, row = 1, rowspan = heightRow1, sticky="nw")
                self.tboxArticle = Text(self.groupArticleFileBox, height=4, width=80,
                                        background=self.bgColorTboxArticle)
                self.tboxArticle.configure(font=self.textFont)
                self.tboxArticle.grid()
                self.tboxArticle.insert('end', self.articleFileBoxMessage)
                self.tboxArticle.config(state='disabled')

                #Mandatory file box
                self.groupMandatoryFilesBox = tkinter.LabelFrame(self)
                self.groupMandatoryFilesBox["text"] = "Check mandatory files in selected project folder..."
                self.groupMandatoryFilesBox.grid(column = 2, row = 1, rowspan = heightRow1, padx = 15, sticky="w")
                self.tboxFiles = Text(self.groupMandatoryFilesBox, height = heightRow1, width= 80,
                                      background=self.bgColorTboxFiles)
                self.tboxFiles.configure(font=self.textFont)
                self.tboxFiles.grid()
                self.tboxFiles.insert('end', self.mandatoryFilesBoxMessage)
                self.tboxFiles.config(state='disabled')

                #Export format group box
                self.groupExportFormat = tkinter.LabelFrame(self)
                self.groupExportFormat["text"] = "Chosen Export Format"
                self.groupExportFormat.grid(column = 3, row = 1, sticky="nw")
                self.labelFormat = tkinter.Label(self.groupExportFormat)
                self.labelFormat["text"] = self.settings.ttwExportFormatSet[1]['Description']
                if self.settings.xmlExportIsSet:
                        self.labelFormat["fg"] = "red"
                else:
                        self.labelFormat["fg"] = "black"
                self.labelFormat.configure(font=self.textFont)
                self.labelFormat.grid()
                                
                #Load file button ------Margin buttons
                self.buttonLoadFile = ttk.Button(self, text = "Select Article File", style = "TButton",
                    command=lambda: self.load_files())
                self.buttonLoadFile["width"] = 20
                self.buttonLoadFile.grid(column = 3, row = 2, sticky="w")

                #Reset_app button
                self.buttonResetApp = ttk.Button(self, text = "Reset TagTool", style = "TButton",
                    command=lambda: self.reset_app())
                self.buttonResetApp["width"] = 20
                self.buttonResetApp.grid(column = 3, row = 3, sticky="w")

                #Logo
                canvas = tkinter.Canvas(self, width=80, height=80)
                canvas.grid(row=4,column=3, sticky="n")
                self.image = tkinter.PhotoImage(file='Logo.gif')
                canvas.create_image((2, 2), image=self.image, anchor="nw")

                #Read to run box
                self.groupReadyToRun = tkinter.LabelFrame(self)
                self.groupReadyToRun["text"] = "Ready to run?"
                self.groupReadyToRun.grid(column = 3, row = 5, sticky="nw")
                self.signalBox = tkinter.Label(self.groupReadyToRun)
                self.signalBox["width"] = 17
                if self.settings.readyToRun == True:
                    self.signalBox["bg"] = self.settings.okGreen
                else:
                    self.signalBox["bg"] = "red"
                self.signalBox.grid()

                #Run button
                self.buttonRunApp = ttk.Button(self, text = "Run TagTool", style = "TButton",
                        command=lambda: self.start_process())
                self.buttonRunApp["width"] = 20
                self.buttonRunApp.grid(column = 3, row = 6, sticky="w")

                #Function group box ---------Second section-----------------
                self.groupFunctions = tkinter.LabelFrame(self)
                self.groupFunctions["text"] = "Chosen Functions"
                self.groupFunctions.grid(column = 1, row = heightRow1 + 1, columnspan = 2, sticky="w")
                self.tboxFunctions = Text(self.groupFunctions, height=10, width=163,
                                          background=self.bgColorTboxFunctions)
                self.tboxFunctions.configure(font=self.textFont)
                self.tboxFunctions.grid()

                for x, y in self.settings.ttwFunctionsSet.items():
                        self.tboxFunctions.insert("end", y['Description'] + "\n")
                       
                self.tboxFunctions.config(state='disabled')
                
                #Set functions button ------Margin buttons
                self.buttonSetFunctions = ttk.Button(self, text = "Change Functions", style = "TButton",
                    command=lambda: self.set_functions())
                self.buttonSetFunctions["width"] = 20
                self.buttonSetFunctions.grid(column = 3, row = heightRow1 + 1, sticky="nw")

                #NER plugin button
                self.buttonStartNER = ttk.Button(self, text = "Open NER plugin", style = "TButton",
                    command=lambda: self.basic_NER_lib_check())
                self.buttonStartNER["width"] = 20
                self.buttonStartNER.grid(column = 3, row = heightRow1 + 2, sticky="nw")
                
                #Result box ---------Third section-----------------
                self.groupResult = tkinter.LabelFrame(self)
                self.groupResult["text"] = "Results"
                self.result = Text(self.groupResult, height=10, width=160,
                                          background=self.bgColorTboxFunctions)
                
                self.result.configure(font=self.textFont)
                
                self.groupResult.grid(column = 1, row = heightRow1 + 2, columnspan = 2, sticky="w")

                self.PandocConversion = tkinter.Label(self.groupResult)
                self.PandocConversion["text"] = self.settings.PandocConversionText
                self.PandocConversion["bg"] = self.settings.PandocConversionBG
                self.PandocConversion.configure(font=self.textFont)
                self.PandocConversion.grid()

                self.ArticleConversion = tkinter.Label(self.groupResult)
                self.ArticleConversion["text"] = self.settings.ArticleConversionText
                self.ArticleConversion["bg"] = self.settings.ArticleConversionBG
                self.ArticleConversion.configure(font=self.textFont)
                self.ArticleConversion.grid()

                self.RessourcesWritten = tkinter.Label(self.groupResult)
                self.RessourcesWritten["text"] = self.settings.RessourcesWrittenText
                self.RessourcesWritten["bg"] = self.settings.RessourcesWrittenBG
                self.RessourcesWritten.configure(font=self.textFont)
                self.RessourcesWritten.grid()

                self.tboxResult = Text(self, height=3, width=80,
                                          background=self.settings.tboxResultBG)
                self.tboxResult.configure(font=self.textFont)
                self.tboxResult.grid(column = 1, row = heightRow1 + 3, columnspan = 2, sticky="w")
                self.tboxResult.insert("end", self.settings.resultText)
                self.tboxResult.config(state='disabled')

                #Open browser button
                self.buttonOpenBrowser = ttk.Button(self, text = "Open Result in Browser", style = "TButton",
                        command=lambda: self.open_browser())
                self.buttonOpenBrowser["width"] = 25
                self.buttonOpenBrowser.grid(column = 2, row = heightRow1 + 3, sticky="nw")
           
        def basic_NER_lib_check(self):

                if NER_Plugin_Switch == False:
                    textInfo = ("NER Plugin must be activated first.\n\n"
                                "See \"About\" -> \"Help\" for instructions.\n\n\n")
                    tkinter.messagebox.showwarning(title="ERROR", \
                                         message=textInfo)
                    return
                
                if self.files.fileName == "":
                    tkinter.messagebox.showwarning(title="ERROR", \
                                         message="No file selected!")
                    self.settings.selectedFileIsReady = False
                    self.bgColorTboxArticle = "red"
                    self.actualize_widgets()
                    return

                #Extensive tests ommitted, but at least a quick check,
                #whether it can be assumed that the necessary environment exists.
                try:
                    from transformers import pipeline
                except ModuleNotFoundError as err:
                    textInfo = ("The required NER libraries do not seem to be installed.\n\n"
                                "Check your environment.\n\n"
                                "See \"About\" -> \"Help\" for instructions.\n\n\n")
                    tkinter.messagebox.showwarning(title="ERROR", \
                                         message=textInfo)
                    return
                else:
                    yesnoResult = tkinter.messagebox.askyesno(title="Important Information", \
                     message=(      "The now opening NER Plugin must to be run before "
                                    "preparing the file \"04_ToSearchAndReplaceList.csv\".\n\n"
                                    "So:\n"
                                    "1. Run the NER Pluging first. Its results will be saved "
                                    "in the file \"NER_results\\02_Gazetteer_IDs_DRAFT.csv\"\n\n"
                                    "2. Copy the entries you have approved and selected into "
                                    "\"04_ToSearchAndReplaceList.csv\"\n\n"
                                    "3. After having prepared the other mandatory .csv files run TagTool.\n\n\n"
                                    "Do you wish to continue?"
                                    ))
                    if yesnoResult == False:
                        return
                    else:
                        self.set_NER_settings()

        def create_MenuBar(self):
            self.menueBarFile = tkinter.Menu(self.menu, tearoff=False)

            self.menueBarFile.add_command(label="Select Article File", command=self.load_files)
            self.menueBarFile.add_command(label="Exit", command=root.destroy)
            self.menu.add_cascade(label="File", menu=self.menueBarFile)
            
            self.menueBarMode = tkinter.Menu(self.menu, tearoff=False)
            self.menueBarMode.add_command(label="To HTML",
                                          command=lambda: self.set_export('--toHTML', 'to HTML'))
            self.menueBarMode.add_command(label="To XML",
                                          command=lambda: self.set_export('--toXML', 'to XML'))
            self.menu.add_cascade(label="Change Export Format", menu=self.menueBarMode)

            self.menueBarAbout = tkinter.Menu(self.menu, tearoff=False)
            self.menueBarAbout.add_command(label="Help", command=lambda: self.show_help())
            self.menueBarAbout.add_command(label="Info", command=lambda: self.show_info())
            self.menu.add_cascade(label="About", menu=self.menueBarAbout)
            
        def load_files(self):
                filetypes = [('Word files', '*.docx')]
                
                fullPath = filedialog.askopenfilename(title='Open a file',
                                              initialdir=os.getcwd(), filetypes=filetypes)
                self.files.projectPath = os.path.dirname(fullPath) + "/"

                self.files.fileName = fullPath.replace(self.files.projectPath, "")
                
                if self.files.fileName == "":
                        tkinter.messagebox.showwarning(title="ERROR", \
                                     message="No file selected!")
                        self.settings.selectedFileIsReady = False
                        self.bgColorTboxArticle = "red"
                        self.actualize_widgets()
                        
                else:
                        self.settings.selectedFileIsReady = True
                        self.bgColorTboxArticle = self.settings.okGreen
                        showinfo(title='Selected File', message = self.files.fileName)
                        self.articleFileBoxMessage = "File selected:\n\n" + self.files.fileName
                        self.mandatoryFilesBoxMessage = ""
                        self.mandatoryFilesChecked = self.files.check_mandatory_files()
                        
                        if self.settings.mandatoryFilesAreComplete == False:
                            self.bgColorTboxFiles = "red"
                        else:
                            self.bgColorTboxFiles = self.settings.okGreen
                            
                        for file in self.mandatoryFilesChecked:
                            
                            self.mandatoryFilesBoxMessage = self.mandatoryFilesBoxMessage + file + "\n"
                                                
                        self.actualize_widgets()

        def open_browser(self):
                if self.settings.xmlExportIsSet == True:
                        tkinter.messagebox.showwarning(title="ERROR", \
                                message="You have chosen .xml as output.\n\nNo browser version available.")
                        return
                elif self.settings.xmlExportIsSet == False and self.settings.resultSuccessful == False:
                        tkinter.messagebox.showwarning(title="ERROR", \
                                message="No approved result available.\n\nTry again after running.")
                        return
                elif self.settings.xmlExportIsSet == False and self.settings.resultSuccessful == True:
                        webbrowser.open_new(self.files.projectPath+self.settings.expectedResultHtml)       
                
        def reset_app(self):
                self.bgColorTboxArticle = self.settings.colorNeutral
                self.bgColorTboxFiles = self.settings.colorNeutral
                self.bgColorTboxFunctions = self.settings.okGreen

                self.articleFileBoxMessage = "No files selected\n\n(Menue \"Files\" -> \"Select Article File\" or press button on the right)"
                self.mandatoryFilesBoxMessage = "No files found\n\n(will be shown automatically when choosing Article File)"

                file = ""
                
                self.settings.reset_settings()
                self.actualize_widgets()

        def run_NER_Plugin(self, window):

            #In this version the default settings cannot be changed so they are hard coded
            success, textInfo = pyNER.run_NER_process(self.files, self.settings)
    
            tkinter.messagebox.showinfo(title="Info", \
                                     message=textInfo)
         
            window.destroy()

        def save_functions(self, vals, window):
    
                self.tboxFunctions.delete(1.0, "end")
                 
                args = []
                empty = True
                for val in vals:
                        arg = str(val.get())
                        args.append(arg)
                        if arg !="":
                            empty = False
                       
                self.settings.set_functions(args)
                self.labels = []
                self.groupFunctions.grid_forget()
                
                if empty == True:
                    self.settings.functionsAreReady = False
                    self.bgColorTboxFunctions = "red"
                else:
                    self.settings.functionsAreReady = True
                    self.bgColorTboxFunctions = self.settings.okGreen
                self.actualize_widgets()
                
                window.destroy()
                

        def set_export(self, arg, desc):
                self.settings.set_export_format(arg, desc)
                self.actualize_widgets()

       
        def set_functions(self):
            setFunctionsWindow = tkinter.Toplevel()
            setFunctionsWindow.geometry('600x300')
            setFunctionsWindow.title('TagTool_WiZArd functions')
            setFunctionsWindow.iconbitmap(self.settings.cwd+"\\Logo.ico")
                        
            self.group = tkinter.LabelFrame(setFunctionsWindow)
            self.group["text"] = "Choose TagTool_WiZArd functions"
            self.group.grid(sticky="w", pady = 10, padx = 10)

            vals = []
            self.functions = []
            checkBoxes = []
            i=0
            for x, y in self.settings.ttwFunctions.items():
                val = tkinter.StringVar()
                vals.append(val)
                
                checkBox = ttk.Checkbutton(self.group, text = y['Description'], variable = vals[i],
                                           onvalue = y['Arg'], offvalue ="")
                checkBox.grid(sticky="w")
                checkBoxes.append(vals[i])
                i += 1

            self.buttonSaveFunctions = ttk.Button(setFunctionsWindow, text = "Save Functions and Exit", style = "TButton",
                    command=lambda: self.save_functions(vals, setFunctionsWindow))
            self.buttonSaveFunctions.grid(sticky="e")

            self.tboxInfo = Text(setFunctionsWindow, height=1, width=70, background="#ffff66")
            self.tboxInfo.configure(font=self.textFont)
            infoText = "Caution: It is not recommended to change the default settings."
            self.tboxInfo.insert("end", infoText)
            self.tboxInfo.grid(sticky = "w", pady = 10, padx = 10)
            self.tboxInfo.config(state='disabled')
        

        def set_NER_settings(self):
            
            setNER_PluginWindow = tkinter.Toplevel()
            setNER_PluginWindow.geometry('600x500')
            setNER_PluginWindow.title('Named Entity Recognition Plugin')
            setNER_PluginWindow.iconbitmap(self.settings.cwd+"\\Logo.ico")

            #Models
            self.groupModels = tkinter.LabelFrame(setNER_PluginWindow)
            self.groupModels["text"] = "Models"
            self.groupModels.grid(sticky="w", pady = 10, padx = 10)
            self.tboxModels = Text(self.groupModels, height=len(self.settings.NER_Settings['Model']), width=70,
                                      background=self.settings.colorNeutral)
            self.tboxModels.configure(font=self.textFont)
            self.tboxModels.grid()
            for item in self.settings.NER_Settings['Model']:
                self.tboxModels.insert("end", item + "\n")
            self.tboxModels.config(state='disabled')

            #Entities
            self.groupEntities = tkinter.LabelFrame(setNER_PluginWindow)
            self.groupEntities["text"] = "Entity Types"
            self.groupEntities.grid(sticky="w", pady = 10, padx = 10)
            self.tboxEntities = Text(self.groupEntities, height=len(self.settings.NER_Settings['Entity Type']), width=70,
                                      background=self.settings.colorNeutral)
            self.tboxEntities.configure(font=self.textFont)
            self.tboxEntities.grid()
            for item in self.settings.NER_Settings['Entity Type']:
                self.tboxEntities.insert("end", item + "\n")
            self.tboxEntities.config(state='disabled')

            #Sources
            self.groupSources = tkinter.LabelFrame(setNER_PluginWindow)
            self.groupSources["text"] = "Ways of Source Text Extraction"
            self.groupSources.grid(sticky="w", pady = 10, padx = 10)
            self.tboxSources = Text(self.groupSources, height=len(self.settings.NER_Settings['Source']), width=70,
                                      background=self.settings.colorNeutral)
            self.tboxSources.configure(font=self.textFont)
            self.tboxSources.grid()
            for item in self.settings.NER_Settings['Source']:
                self.tboxSources.insert("end", item + "\n")
            self.tboxSources.config(state='disabled')

            #In this version the default settings cannot be changed so they are hard coded
            #Selected Settings
            self.groupSettings = tkinter.LabelFrame(setNER_PluginWindow)
            self.groupSettings["text"] = "Selected Settings"
            self.groupSettings.grid(sticky="w", pady = 10, padx = 10)
            self.tboxSettings = Text(self.groupSettings, height=3, width=70,
                                      background=self.settings.okGreen)
            self.tboxSettings.configure(font=self.textFont)
            self.tboxSettings.grid()
            for x, y in self.settings.NER_SettingsSet.items():
                self.tboxSettings.insert("end", x + ": " + y + "\n")
            self.tboxSettings.config(state='disabled')
            
            self.buttonRunNER = ttk.Button(setNER_PluginWindow, text = "Run NER Plugin", style = "TButton",
                    command=lambda: self.run_NER_Plugin(setNER_PluginWindow))
            self.buttonRunNER.grid(sticky="e")

            self.tboxInfo = Text(setNER_PluginWindow, height=1, width=70, background="#ffff66")
            self.tboxInfo.configure(font=self.textFont)
            infoText = "NOTE: In this test versions this selected settings are predefined."
            self.tboxInfo.insert("end", infoText)
            self.tboxInfo.grid(sticky = "w", pady = 10, padx = 10)
            self.tboxInfo.config(state='disabled')


        def show_help(self):
                webbrowser.open_new(self.settings.cwd+"\\ttw_help.html")
                

        def show_info(self):
            textInfo = "title: \"TagTool_WiZArd\"\n"\
                    "version: "+versionNumber+"\n"\
                    "for complete documentation see\nhttps://github.com/pBxr/TagTool_WiZArd"
            tkinter.messagebox.showinfo(title="Info", \
                                    message=textInfo)                                
        
        def start_process(self):
            if self.settings.readyToRun == False:
                tkinter.messagebox.showwarning(title="ERROR", \
                                     message="Tool is not ready to run.\n\nAll boxes must be green.\n\nCheck settings.")
            else:
                FNULL = open(os.devnull, 'w') #For subprocess

                #Prepare Args
                argsTagTool = []
                for x, y in self.settings.ttwFunctionsSet.items():
                    argsTagTool.append(y['Arg'])

                for x, y in self.settings.ttwFlagsSet.items():
                    argsTagTool.append(y['Arg'])

                for x, y, in self.settings.ttwExportFormatSet.items():
                    argsTagTool.append(y['Arg'])
                    if y['Description'] == "to XML":
                        self.settings.xmlExportIsSet = True
                
                #Step 1: Convert .docx -> .html with pandoc
                pandocCall = "pandoc -s -o "

                self.settings.target = self.files.fileName.replace(".docx", ".html")
                pandocCall = pandocCall + "\"" + self.files.projectPath + self.settings.target\
                        + "\"" + " " + "\"" +  self.files.projectPath + self.files.fileName + "\""
                #print("pandocCall:", pandocCall)
                subprocess.run(pandocCall, stdout=FNULL, stderr=FNULL, shell=False)

                #Step 2: Run ttw
                versionNumberCall = versionNumber.replace(".","-")
                ttwCall = "\"" + self.settings.cwd + "\\tagtool_v"+versionNumberCall+".exe\"" + " \""\
                          + self.files.projectPath + self.settings.target + "\""

                #In case of whitespaces
                testWhitespace = " "
                if testWhitespace in self.files.projectPath:
                        pathForTtw = self.files.projectPath.replace(" ", "*%20*")
                else:
                        pathForTtw = self.files.projectPath
                  
                ttwCall = ttwCall + " " + " ".join(argsTagTool) + " " + "--fromPy" + pathForTtw
                #print(ttwCall)
                
                subprocess.run(ttwCall, stdout=True, stderr=FNULL, shell=True)

                #Step 3: Additional tasks (prepared in Python, not C++ in order to replace the cpp core functions successively)
                if self.settings.xmlExportIsSet == True:
                        pyScr.convert_tables_to_XML(self)
                                
                self.files.check_result()
                self.actualize_widgets()
                                               
if __name__=='__main__':

    root = tkinter.Tk()
    global versionNumber
    versionNumber = "2.1.0"

    #Here is the switch if you want to test the NER Plugin
    global NER_Plugin_Switch
    NER_Plugin_Switch = False
    if NER_Plugin_Switch == True:
        import pyNER 
    
    currentDirectory = os.getcwd()
    titleText = "Welcome to TagToolWiZArd application " + "(v"+versionNumber+")"
    root.title(titleText)
    root.state('zoomed')

    logo = currentDirectory + "\\Logo.ico"
     
    root.geometry("1300x700")
    root.iconbitmap(logo)
    app = MainWindow(root)
    app.mainloop()

              
    
