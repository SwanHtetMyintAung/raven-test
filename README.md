### This is a simple template for raven

resources folder is to add metadata for the executable, like icon
remember to replace the data with yours, app.ico file with your own

assets folder is meant for xml files. you should open that folder inside raven designer.

emb_xml.py file is a python script that turns xml file into a C string so that
when you publish your app, u don't need to add xml file explicitly, that python script
is optional. the script reads xml file inside assets folder and turn it into C file to 
src folder.
