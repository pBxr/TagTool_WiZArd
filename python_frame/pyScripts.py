import re

from bs4 import BeautifulSoup

def convert_footnotes_to_HTML(self):
    fileName = self.files.projectPath + self.settings.target
    fileName = fileName.replace(".html", "_edited_1_.xml")
    
    with open(fileName, 'r', encoding="utf8") as fp:

        soup = BeautifulSoup(fp, "html.parser")
        
        fnTags = soup.find_all('a', {'class': 'footnote-ref'})

        #First the footnote references
        index = 1
        for fnTag in fnTags:
            fnTag.sup.unwrap()

            fnTag.wrap(soup.new_tag('a', attrs={'style':f'mso-footnote-id:ftn{index}', 'ref' : f"#_ftn{index}", 'name' : f"_ftnref{index}", 'title' : ""}))
            fnTag.wrap(soup.new_tag('span', attrs={"class": "DAItext-fussnote"}))
            fnTag.wrap(soup.new_tag('span', style='font-family:"Noto Serif",serif;mso-bidi-font-family: "Times New Roman"'))
            fnTag.wrap(soup.new_tag('span', style='mso-special-character:footnote'))
            fnTag.wrap(soup.new_tag('span', attrs={"class": "DAItext-fussnote"}))
            fnTag.wrap(soup.new_tag('span', style='font-size:11.0pt;font-family:"Noto Serif",serif; mso-fareast-font-family:"Times New Roman";mso-bidi-font-family:"Times New Roman"; mso-ansi-language:DE;mso-fareast-language:EN-US; mso-bidi-language:AR-SA'))

            index +=1
          
        #Second the footnote section
        enTags = soup.find_all('li', {'role': 'doc-endnote'})
                    
        index = 1    
        for enTag in enTags:
            enTag.p.unwrap()

            enTag.a.decompose()
            enTag.wrap(soup.new_tag('div', attrs={'style' : 'mso-element:footnote', 'id': f"ftn{index}"}))
            enTag.wrap(soup.new_tag('p', attrs={'class': 'DAIfootnote'}))
            enTag.insert_before(soup.new_tag('span', attrs={'class': 'MsoFootnoteReference'}))

            index +=1
        
        divTags = soup.find_all('div', {'style' : 'mso-element:footnote'})

        index = 1
        for divTag in divTags:
            divTag.li.unwrap()

            fnNrs = divTag.find_all('span')
            for fnNr in fnNrs:
                fnNr.string = f"[{index}]"
            for fnNr in fnNrs:
                fnNr.wrap(soup.new_tag('a', attrs= {'style' : f"mso-footnote-id:ftn{index}", 'href' : f"#_ftnref{index}",
                                                    'name' : f"_ftn{index}", 'title' : ""}))
                fnNr.wrap(soup.new_tag('span' , attrs={'class' : 'MsoFootnoteReference'}))
                fnNr.wrap(soup.new_tag('span' , style='mso-special-character:footnote'))
            index +=1

        #Remove quotation marks in attributes
        articleFile = str(soup)

        #First the static tags
        toReplace = {
            "<span class=\"DAItext-fussnote\">" : "<span class=DAItext-fussnote>",
            "<p class=\"DAIfootnote\">" : "<p class=DAIfootnote>",
            "<span class=\"MsoFootnoteReference\">" : "<span class=MsoFootnoteReference>",
            "<span style=\"mso-special-character:footnote\">" : "<span style=\'mso-special-character:footnote\'>",
            "style=\"mso-element:footnote\"" : "style=\'mso-element:footnote\'"
                      }
        for x, y in toReplace.items():
            articleFile = articleFile.replace(x, y)

        #Now the dynamic tags
        results = re.findall("<div id=\"ftn\\d+\" style=\'mso-element:footnote\'>", articleFile)

        for res in results:
            new = res.replace("\"", "")
            articleFile = articleFile.replace(res, new)

        results = re.findall("style=\"mso-footnote-id:ftn\\d+\" title", articleFile)
        for res in results:
            new = res.replace("\"", "\'")
            articleFile = articleFile.replace(res, new)
            
        fp.close()

    with open(fileName, 'w', encoding="utf8") as fp:

        fp.write(articleFile)

        fp.close() 

def convert_tables_to_XML(self):
 
    fileName = self.files.projectPath + self.settings.target
    fileName = fileName.replace(".html", "_edited_1_.xml")
    
    with open(fileName, 'r', encoding="utf8") as fp:

        soup = BeautifulSoup(fp, features="xml")
        
        table_tags = soup.find_all('table')

        #--- XML-wraps
        for table_tag in table_tags:
            table_tag.wrap(soup.new_tag('table-wrap'))

        wrap_tags = soup.find_all('table-wrap')

        index = 1
        for wrap_tag in wrap_tags:
            wrap_tag['id'] = "tw-" + str(index)
            wrap_tag['position'] = 'anchor'
            index +=1

        #--- Convert table tags
        index = 1
        for table_tag in table_tags:
            #Set table attributes
            table_tag['rules'] = 'all'
            table_tag['width'] = '600'
            table_tag['id'] = "t-" + str(index)

            results = table_tag.find_all('th')
            for res in results:
                res.name = 'td'

            #remove tags
            table_tag.tbody.unwrap()
            table_tag.thead.unwrap()

            #remove tags and content
            table_tag.colgroup.decompose()

            #remove attributes
            results = table_tag.find_all('tr', {'class':True})
            for res in results:
                del res['class']

            index += 1
        
        fp.close()
   
    with open(fileName, 'w', encoding="utf8") as fp:

        fp.write(str(soup))

        fp.close()       
