import re

from bs4 import BeautifulSoup


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
